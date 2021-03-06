/*-
 * Copyright (c) 2014-2015 MongoDB, Inc.
 * Copyright (c) 2008-2014 WiredTiger, Inc.
 *	All rights reserved.
 *
 * See the file LICENSE for redistribution information.
 */

struct __wt_config {
	WT_SESSION_IMPL *session;
	const char *orig;
	const char *end;
	const char *cur;

	int depth, top;
	const int8_t *go;
};

struct __wt_config_check {
	const char *name;
	const char *type;
	int (*checkf)(WT_SESSION_IMPL *, WT_CONFIG_ITEM *);
	const char *checks;
	const WT_CONFIG_CHECK *subconfigs;
};

#define	WT_CONFIG_REF(session, n)					\
	(S2C(session)->config_entries[WT_CONFIG_ENTRY_##n])
struct __wt_config_entry {
	const char *method;			/* method name */

#define	WT_CONFIG_BASE(session, n)	(WT_CONFIG_REF(session, n)->base)
	const char *base;			/* configuration base */

	const WT_CONFIG_CHECK *checks;		/* check array */
};

struct __wt_config_parser_impl {
	WT_CONFIG_PARSER iface;

	WT_SESSION_IMPL *session;
	WT_CONFIG config;
	WT_CONFIG_ITEM config_item;
};

/*
 * DO NOT EDIT: automatically built by dist/api_config.py.
 * configuration section: BEGIN
 */
#define	WT_CONFIG_ENTRY_colgroup_meta			 0
#define	WT_CONFIG_ENTRY_connection_add_collator		 1
#define	WT_CONFIG_ENTRY_connection_add_compressor	 2
#define	WT_CONFIG_ENTRY_connection_add_data_source	 3
#define	WT_CONFIG_ENTRY_connection_add_extractor	 4
#define	WT_CONFIG_ENTRY_connection_async_new_op		 5
#define	WT_CONFIG_ENTRY_connection_close		 6
#define	WT_CONFIG_ENTRY_connection_load_extension	 7
#define	WT_CONFIG_ENTRY_connection_open_session		 8
#define	WT_CONFIG_ENTRY_connection_reconfigure		 9
#define	WT_CONFIG_ENTRY_cursor_close			10
#define	WT_CONFIG_ENTRY_cursor_reconfigure		11
#define	WT_CONFIG_ENTRY_file_meta			12
#define	WT_CONFIG_ENTRY_index_meta			13
#define	WT_CONFIG_ENTRY_session_begin_transaction	14
#define	WT_CONFIG_ENTRY_session_checkpoint		15
#define	WT_CONFIG_ENTRY_session_close			16
#define	WT_CONFIG_ENTRY_session_commit_transaction	17
#define	WT_CONFIG_ENTRY_session_compact			18
#define	WT_CONFIG_ENTRY_session_create			19
#define	WT_CONFIG_ENTRY_session_drop			20
#define	WT_CONFIG_ENTRY_session_log_flush		21
#define	WT_CONFIG_ENTRY_session_log_printf		22
#define	WT_CONFIG_ENTRY_session_open_cursor		23
#define	WT_CONFIG_ENTRY_session_reconfigure		24
#define	WT_CONFIG_ENTRY_session_rename			25
#define	WT_CONFIG_ENTRY_session_rollback_transaction	26
#define	WT_CONFIG_ENTRY_session_salvage			27
#define	WT_CONFIG_ENTRY_session_strerror		28
#define	WT_CONFIG_ENTRY_session_truncate		29
#define	WT_CONFIG_ENTRY_session_upgrade			30
#define	WT_CONFIG_ENTRY_session_verify			31
#define	WT_CONFIG_ENTRY_table_meta			32
#define	WT_CONFIG_ENTRY_wiredtiger_open			33
#define	WT_CONFIG_ENTRY_wiredtiger_open_all		34
#define	WT_CONFIG_ENTRY_wiredtiger_open_basecfg		35
#define	WT_CONFIG_ENTRY_wiredtiger_open_usercfg		36
/*
 * configuration section: END
 * DO NOT EDIT: automatically built by dist/flags.py.
 */
