/**
 *    Copyright (C) 2013 10gen Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects
 *    for all of the code used other than as permitted herein. If you modify
 *    file(s) with this exception, you may extend this exception to your
 *    version of the file(s), but you are not obligated to do so. If you do not
 *    wish to do so, delete this exception statement from your version. If you
 *    delete this exception statement from all source files in the program,
 *    then also delete it in the license file.
 */
#pragma once

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

#include "mongo/base/disallow_copying.h"
#include "mongo/platform/cstdint.h"
#include "mongo/platform/unordered_map.h"
#include "mongo/util/concurrency/mutex.h"
#include "mongo/util/time_support.h"

namespace mongo {

/**
 * DeadlineMonitor
 *
 * Monitors tasks which are required to complete before a deadline.  When
 * a deadline is started on a _Task*, either the deadline must be stopped,
 * or _Task::kill() will be called when the deadline arrives.
 *
 * Each instance of a DeadlineMonitor spawns a thread which waits for one of the
 * following conditions:
 *    - a task is added to the monitor
 *    - a task is removed from the monitor
 *    - the nearest deadline has arrived
 *
 * Ownership:
 * The _Task* must not be freed until the deadline has elapsed or stopDeadline()
 * has been called.
 *
 * NOTE: Each instance of this class spawns a new thread.  It is intended to be a stop-gap
 *       solution for simple deadline monitoring until a more robust solution can be
 *       implemented.
 *
 * NOTE: timing is based on wallclock time, which may not be precise.
 */
template <typename _Task>
class DeadlineMonitor {
    MONGO_DISALLOW_COPYING(DeadlineMonitor);

public:
    DeadlineMonitor()
        : _tasks(),
          _deadlineMutex("DeadlineMonitor"),
          _newDeadlineAvailable(),
          _nearestDeadlineWallclock(kMaxDeadline),
          _monitorThread(&mongo::DeadlineMonitor<_Task>::deadlineMonitorThread, this) {}

    ~DeadlineMonitor() {
        // ensure the monitor thread has been stopped before destruction
        _monitorThread.interrupt();
        _monitorThread.join();
    }

    /**
     * Start monitoring a task for deadline lapse.  User must call stopDeadline() before
     * deleting the task.  Note that stopDeadline() cannot be called from within the
     * kill() method.
     * @param   task        the task to kill()
     * @param   timeoutMs   number of milliseconds before the deadline expires
     */
    void startDeadline(_Task* const task, uint64_t timeoutMs) {
        uint64_t now = curTimeMillis64();
        scoped_lock lk(_deadlineMutex);

        // insert or update the deadline
        std::pair<typename TaskDeadlineMap::iterator, bool> inserted =
            _tasks.insert(std::make_pair(task, now + timeoutMs));

        if (!inserted.second)
            inserted.first->second = now + timeoutMs;

        if (inserted.first->second < _nearestDeadlineWallclock) {
            // notify the monitor thread of the new, closer deadline
            _nearestDeadlineWallclock = inserted.first->second;
            _newDeadlineAvailable.notify_one();
        }
    }

    /**
     * Stop monitoring a task.  Can be called multiple times, before or after a
     * deadline has expired (as long as the task remains allocated).
     * @return true  if the task was found and erased
     */
    bool stopDeadline(_Task* const task) {
        scoped_lock lk(_deadlineMutex);
        return _tasks.erase(task);
    }

private:
    /**
     * Main deadline monitor loop.  Waits on a condition variable until a task
     * is started, stopped, or the nearest deadline arrives.  If a deadline arrives,
     * _Task::kill() is invoked.
     */
    void deadlineMonitorThread() {
        scoped_lock lk(_deadlineMutex);
        while (true) {
            // get the next interval to wait
            uint64_t now = curTimeMillis64();

            // wait for a task to be added or a deadline to expire
            while (_nearestDeadlineWallclock > now) {
                uint64_t nearestDeadlineMs;
                if (_nearestDeadlineWallclock == kMaxDeadline) {
                    _newDeadlineAvailable.wait(lk.boost());
                } else {
                    nearestDeadlineMs = _nearestDeadlineWallclock - now;
                    _newDeadlineAvailable.timed_wait(
                        lk.boost(), boost::posix_time::milliseconds(nearestDeadlineMs));
                }
                now = curTimeMillis64();
            }

            if (_tasks.empty()) {
                // all deadline timers have been stopped
                _nearestDeadlineWallclock = kMaxDeadline;
                continue;
            }

            // set the next interval to wait for deadline completion
            _nearestDeadlineWallclock = kMaxDeadline;
            typename TaskDeadlineMap::iterator i = _tasks.begin();
            while (i != _tasks.end()) {
                if (i->second < now) {
                    // deadline expired
                    i->first->kill();
                    _tasks.erase(i++);
                } else {
                    if (i->second < _nearestDeadlineWallclock)
                        // nearest deadline seen so far
                        _nearestDeadlineWallclock = i->second;
                    ++i;
                }
            }
        }
    }

    static const uint64_t kMaxDeadline;
    typedef unordered_map<_Task*, uint64_t> TaskDeadlineMap;
    TaskDeadlineMap _tasks;       // map of running tasks with deadlines
    mongo::mutex _deadlineMutex;  // protects all non-const members, except _monitorThread
    boost::condition _newDeadlineAvailable;  // condition variable for timeout, start and stop
    uint64_t _nearestDeadlineWallclock;      // absolute time of the nearest deadline
    boost::thread _monitorThread;            // the deadline monitor thread
};

template <typename _Task>
const uint64_t DeadlineMonitor<_Task>::kMaxDeadline = std::numeric_limits<uint64_t>::max();

}  // namespace mongo
