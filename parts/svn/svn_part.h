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

class svnPart : public KDevPlugin
{
	Q_OBJECT

	public:
		svnPart(QObject *parent, const char *name, const QStringList &);
		~svnPart();

		bool recursive;
		bool force;
		bool verbose;

		void readConf();
		
	protected:
		svn_client_auth_baton_t* createAuthBaton();

	private slots:
		void contextMenu(QPopupMenu *popup, const Context *context);
		void slotCommit();
		void slotUpdate();
		void slotAdd();
		void slotRemove();
		void slotLog();
		void slotDiff();
		void slotCleanup();
		void projectConfigWidget(KDialogBase *dlg);
		static svn_error_t *get_log_message (const char **log_msg, apr_array_header_t *commit_items,
				void *baton, apr_pool_t *pool);
		void *make_log_msg_baton (apr_pool_t *pool);

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

};


#endif

/* vim: set ai ts=8 sw=8 : */
