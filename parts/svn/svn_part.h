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
		//debug and info messages
		static void svnDebug(const char *dbg);
		static void svnMsg(const char *msg);
		//to create logs for the lib
		static svn_error_t *get_log_message (const char **log_msg, apr_array_header_t *commit_items,
				void *baton, apr_pool_t *pool);
		void *make_log_msg_baton (const char *base_dir, apr_pool_t *pool);
		// to receive notifications from the svn lib
		static void notify (void *baton, const char *path, svn_wc_notify_action_t action,
				svn_node_kind_t kind, const char *mime_type, svn_wc_notify_state_t content_state,
				svn_wc_notify_state_t prop_state, svn_revnum_t revision);
		void get_notifier(svn_wc_notify_func_t *notify_func_p, void **notify_baton_p,
				svn_boolean_t is_checkout, svn_boolean_t suppress_final_line, 
				apr_pool_t *pool);

	private slots:
		void contextMenu(QPopupMenu *popup, const Context *context);
		//actions
		void slotCommit();
		void slotUpdate();
		void slotAdd();
		void slotRemove();
		void slotLog();
		void slotDiff();
		void slotCleanup();
		void projectConfigWidget(KDialogBase *dlg);

	private:
		QString popupfile;  
		QString auth_username;
		QString auth_password;

		apr_pool_t *pool;
		apr_status_t apr_err;

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

};

#endif

/* vim: set ai ts=8 sw=8 : */
