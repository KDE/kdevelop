/* Copyright (C) 2002 
   Mickael Marchand <marchand@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
   */

#ifndef __KDEVPART_SVN_H__
#define __KDEVPART_SVN_H__

#include <kdevplugin.h>
#include <qpopupmenu.h>
#include <kdialogbase.h>
#include <subversion-1/svn_client.h>
#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "commitdlg.h"

class SvnPart : public KDevPlugin
{
	Q_OBJECT

	public:
		SvnPart(QObject *parent, const char *name, const QStringList &);
		~SvnPart();

		bool recursive;
		bool force;
		bool verbose;

		void readConf();
		
	protected:
		svn_client_auth_baton_t* createAuthBaton();
		//ask for login/pass if needed
		static svn_error_t *promptUser(char **result, const char *prompt, svn_boolean_t hide, 
				void *baton, apr_pool_t *pool);
		//debug and info messages
		void svnDebug(const char *dbg);
		void svnMsg(const char *msg);
		void svnLog(const char *msg);
		//to receive logs from the lib
		static svn_error_t *log_msg_receiver(void *baton, apr_hash_t *changed_paths, svn_revnum_t rev, 
				const char *author, const char *date, const char *msg, apr_pool_t *pool);
		//to create logs for the lib
		static svn_error_t *get_log_message (const char **log_msg, const char **tmp_file,apr_array_header_t *commit_items, void *baton, apr_pool_t *pool);

		void *make_log_msg_baton (const char *base_dir, apr_pool_t *pool);
		// to receive notifications from the svn lib
		static void notify (void *baton, const char *path, svn_wc_notify_action_t action,
				svn_node_kind_t kind, const char *mime_type, svn_wc_notify_state_t content_state,
				svn_wc_notify_state_t prop_state, svn_revnum_t revision);
		void get_notifier(svn_wc_notify_func_t *notify_func_p, void **notify_baton_p,
				svn_boolean_t is_checkout, svn_boolean_t suppress_final_line, 
				apr_pool_t *pool);
		//display SVN errors in kdevelop (wrapper function for handleSvnError)
		void Error(svn_error_t *err);
		//get infos about a svn error and display it
		void handleSvnError(svn_error_t *, int, apr_status_t);
		void generate_status_codes (char *str_status, enum svn_wc_status_kind text_status,
				enum svn_wc_status_kind prop_status, svn_boolean_t locked, svn_boolean_t copied);
		void print_short_format (const char *path, svn_wc_status_t *status);
		void print_long_format (const char *path, svn_boolean_t show_last_committed, 
				svn_wc_status_t *status);
		int num_lines(const char *msg);

	private slots:
		void contextMenu(QPopupMenu *popup, const Context *context);
		//actions
		void slotCommit();
		void slotRevert();
		void slotUpdate();
		void slotAdd();
		void slotRemove();
		void slotLog();
		void slotDiff();
		void slotCleanup();
		void projectConfigWidget(KDialogBase *dlg);
		void slotStatus(bool remote);
		void slotStatusRemote();
		void slotStatusLocal();

	private:
		QString popupfile;  
		QString auth_username;
		QString auth_password;

		apr_pool_t *pool;
		CommitDialog *winlog;

		struct log_msg_baton
		{
			const char *message;
			const char *message_encoding; /* the locale/encoding of the message. */
			const char *base_dir; /* UTF-8! */
		};

		struct notify_baton
		{
			svn_boolean_t received_some_change;
			svn_boolean_t is_checkout;
			svn_boolean_t suppress_final_line;
			svn_boolean_t sent_first_txdelta;
			apr_pool_t *pool;
		};

		struct log_message_receiver_baton
		{
			svn_boolean_t first_call;
		};
};

#endif

/* vim: set ai ts=8 sw=8 : */
