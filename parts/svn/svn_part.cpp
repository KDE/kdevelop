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

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <qpopupmenu.h>
#include <kpopupmenu.h>
#include <qfileinfo.h>
#include <kdebug.h>
#include <unistd.h>
#include <qvbox.h>
#include <qregexp.h>

#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "commitdlg.h"
#include "svn_part.h"
#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_config.h>
#include "svnoptionswidget.h"
#include "domutil.h"

typedef KGenericFactory<svnPart> svnFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevsvn, svnFactory( "kdevsvn" ) );

	svnPart::svnPart(QObject *parent, const char *name, const QStringList& )
: KDevPlugin(parent, name)
{
	setInstance(svnFactory::instance());
	connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
			this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
	connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
			this, SLOT(projectConfigWidget(KDialogBase*)) );
	apr_err = apr_initialize ();
	if (apr_err)
	{//error , popup something ...
	}
	pool = svn_pool_create (NULL);
	svn_error_t *err = svn_config_ensure (pool);
	if (err)
	{//error , popup something ...
		svn_pool_destroy (pool);
	}
	readConf();
}

svnPart::~svnPart()
{
	svn_pool_destroy (pool);
	apr_terminate();
}

#define EDITOR_PREFIX_TXT "SVN:"
//stolen in svn cmndline code and modified
svn_error_t *svnPart::get_log_message (const char **log_msg, apr_array_header_t *commit_items,
		void *baton, apr_pool_t *pool)
{
	const char *default_msg = "\n"
		EDITOR_PREFIX_TXT 
		" ---------------------------------------------------------------------\n" 
		EDITOR_PREFIX_TXT " Enter Log.  Lines beginning with '" 
		EDITOR_PREFIX_TXT "' are removed automatically\n"
		EDITOR_PREFIX_TXT "\n"
		EDITOR_PREFIX_TXT " Current status of the target files and directories:\n"
		EDITOR_PREFIX_TXT "\n";
	svn_stringbuf_t *message = NULL;

	if (! (commit_items || commit_items->nelts))
	{
		*log_msg = "";
		return SVN_NO_ERROR;
	}

	while (! message)
	{
		/* We still don't have a valid commit message.  Use $EDITOR to
			 get one.  Note that svn_cl__edit_externally will still return
			 a UTF-8'ized log message. */
		int i;
		svn_stringbuf_t *tmp_message = svn_stringbuf_create (default_msg, pool);
		const char *msg2;  /* ### shim for svn_cl__edit_externally */

		for (i = 0; i < commit_items->nelts; i++)
		{
			svn_client_commit_item_t *item
				= ((svn_client_commit_item_t **) commit_items->elts)[i];
			const char *path = item->path;
			char text_mod = '_', prop_mod = ' ';

			if (! path)
				path = item->url;

			if ((item->state_flags & SVN_CLIENT_COMMIT_ITEM_DELETE)
					&& (item->state_flags & SVN_CLIENT_COMMIT_ITEM_ADD))
				text_mod = 'R';
			else if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_ADD)
				text_mod = 'A';
			else if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_DELETE)
				text_mod = 'D';
			else if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_TEXT_MODS)
				text_mod = 'M';

			if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_PROP_MODS)
				prop_mod = 'M';

			svn_stringbuf_appendcstr (tmp_message, EDITOR_PREFIX_TXT);
			svn_stringbuf_appendcstr (tmp_message, "   ");
			svn_stringbuf_appendbytes (tmp_message, &text_mod, 1); 
			svn_stringbuf_appendbytes (tmp_message, &prop_mod, 1); 
			svn_stringbuf_appendcstr (tmp_message, "   ");
			svn_stringbuf_appendcstr (tmp_message, path);
			svn_stringbuf_appendcstr (tmp_message, "\n");
		}

		CommitDialog d;
		d.resize(150,100);
		d.setLog(tmp_message->data);
		if (d.exec() == QDialog::Rejected)
			return SVN_NO_ERROR;

		QString msg = d.logMessage();
		msg2 = msg.utf8();

		if (msg2)
			message = svn_stringbuf_create (msg2, pool);

		/* Strip the prefix from the buffer. */
		//		if (message);
		//TODO
		//message = strip_prefix_from_buffer (message, EDITOR_PREFIX_TXT, pool);
	}

	*log_msg = message ? message->data : NULL;
	return SVN_NO_ERROR;
}

void *svnPart::make_log_msg_baton (apr_pool_t *pool)
{
	struct log_msg_baton *baton = (log_msg_baton*)apr_palloc (pool, sizeof (*baton));
	return baton;
}

void svnPart::readConf() {
	QDomDocument &dom = *this->projectDom();
	recursive = DomUtil::readBoolEntry(dom,"/kdevsvn/recurse",false);
	force = DomUtil::readBoolEntry(dom,"/kdevsvn/force",false);
	verbose = DomUtil::readBoolEntry(dom,"/kdevsvn/verbose",false);
}

void svnPart::projectConfigWidget(KDialogBase *dlg) {
	QVBox *vbox = dlg->addVBoxPage(i18n("Subversion"));
	svnOptionsWidget *w = new svnOptionsWidget(this, (QWidget *)vbox, "Subversion config widget");
	connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void svnPart::contextMenu(QPopupMenu *popup, const Context *context) {
	if (context->hasType("file")) {
		const FileContext *fcontext = static_cast<const FileContext*>(context);
		popupfile = fcontext->fileName();
		QFileInfo fi(popupfile);
		popup->insertSeparator();

		KPopupMenu *sub = new KPopupMenu(popup);
		QString name = fi.fileName();
		sub->insertTitle( i18n("Actions for %1").arg(name) );
		sub->insertItem( i18n("Commit"),
				this, SLOT(slotCommit()) );
		sub->insertItem( i18n("Update"),
				this, SLOT(slotUpdate()) );
		sub->insertItem( i18n("Add to Repository"),
				this, SLOT(slotAdd()) );
		sub->insertItem( i18n("Remove From Repository"),
				this, SLOT(slotRemove()) );
		sub->insertSeparator();
		sub->insertItem( i18n("Diff to Repository"),
				this, SLOT(slotDiff()) );
		sub->insertItem( i18n("Log"),
				this, SLOT(slotLog()) );
		if (fi.isDir())
			sub->insertItem( i18n("Cleanup"),
					this, SLOT(slotCleanup()) );
		popup->insertItem(i18n("Subversion"), sub);
	}
}

svn_client_auth_baton_t *svnPart::createAuthBaton() {
	svn_client_auth_baton_t *auth_baton;

	auth_baton = (svn_client_auth_baton_t*)apr_pcalloc (pool, sizeof(*auth_baton));

	auth_baton->prompt_callback = NULL;//FIXME svn_cl__prompt_user;
	auth_baton->prompt_baton = NULL;

	//i am not sure this will ever be used, since projects should be imported manually for now
	//so login/pass should be in .svn/ dirs
	//if it's necessary (if someone ask for it), i'll add a prompt dialog for login/pass
	if (!auth_username.isEmpty()) {
		auth_baton->username = auth_username;
		auth_baton->overwrite = TRUE;
	}
	if (!auth_password.isEmpty()) {
		auth_baton->password = auth_password;
		auth_baton->overwrite = TRUE;
	}
	return auth_baton;
}

void svnPart::slotCommit() {
	svn_client_commit_info_t *commit_info=NULL;
	svn_wc_notify_func_t notify_func=NULL;
	void *notify_baton=NULL;
	svn_client_auth_baton_t *auth_baton=createAuthBaton();
	apr_array_header_t *targets = apr_array_make (pool, 2, sizeof (const char *));
	//disabled for now, it's broken
	svn_client_get_commit_log_t log_msg_func=NULL;//svnPart::get_log_message;
	void *log_msg_baton=NULL;//make_log_msg_baton(pool);
	const char *xml_dst=NULL;
	svn_revnum_t revision=SVN_INVALID_REVNUM;
	svn_boolean_t nonrecursive=!recursive;

	kdDebug() << "SVN commit " << popupfile.utf8() << endl;
	//from cmdlne/util.c
	(*((const char **) apr_array_push ((apr_array_header_t*)targets))) = apr_pstrdup (pool, popupfile.utf8());

	svn_error_t *err = svn_client_commit (&commit_info, notify_func, notify_baton,
			auth_baton, targets,
			log_msg_func, log_msg_baton,
			xml_dst, revision,
			nonrecursive, pool);
	if (err) {
		svn_handle_error(err,stderr,false);
	} else {
		if ((commit_info) && (SVN_IS_VALID_REVNUM (commit_info->revision))) {
			QString ci;
			ci.sprintf ("Committed revision %" SVN_REVNUM_T_FMT ".", commit_info->revision);	
			kdDebug() << ci << endl;
			kdDebug() << "Done" << endl;
		}
	}
}

void svnPart::slotUpdate() {
	svn_client_auth_baton_t *auth_baton = createAuthBaton();
	const char *xml_src=NULL; //not used here
	svn_client_revision_t revision;
	svn_boolean_t recurse=recursive;

	svn_wc_notify_func_t notify_func=NULL;
	void *notify_baton=NULL;

	revision.kind = svn_client_revision_head;//FIXME, not sure about that one ;)

	//we could use the condensed targets array, see the cmdline 'svn' code for that
	//though our path is already quite clean
	kdDebug() << "SVN update " << popupfile.utf8() << endl;
	svn_error_t *err = svn_client_update (auth_baton, popupfile.utf8(), xml_src, &revision, 
			recurse, notify_func, notify_baton, pool);
	if (err)
		kdDebug() << err->message << endl;
	else
		kdDebug() << "Done" << endl;
}

void svnPart::slotAdd() {
	svn_boolean_t recurse=recursive;
	svn_wc_notify_func_t notify_func=NULL;
	void *notify_baton=NULL;

	kdDebug() << "SVN add " << popupfile.utf8() << endl;
	svn_error_t *err= svn_client_add (popupfile.utf8(), recurse,notify_func,notify_baton,pool);
	if (err)
		kdDebug() << err->message << endl;
	else
		kdDebug() << "Done" << endl;
}

void svnPart::slotCleanup() {
	kdDebug() << "SVN cleanup " << popupfile << endl;
	svn_error_t *err= svn_client_cleanup(popupfile.utf8(),pool);
	if (err)
		kdDebug() << err->message << endl;
	else
		kdDebug() << "Done" << endl;
}

void svnPart::slotRemove() {
	svn_client_commit_info_t *commit_info=NULL;
	//	svn_wc_adm_access_t *optional_adm_access; // not used in cmdline
	svn_boolean_t force_rem=force;
	svn_client_auth_baton_t *auth_baton=createAuthBaton();
	svn_client_get_commit_log_t log_msg_func=NULL;//FIXME
	void *log_msg_baton=NULL;//FIXME
	svn_wc_notify_func_t notify_func=NULL;
	void *notify_baton=NULL;
	//apr_pool_t *subpool = svn_pool_create (pool);

	svn_error_t *err = svn_client_delete (&commit_info, popupfile.utf8(),NULL, force_rem,
			auth_baton, log_msg_func,log_msg_baton, notify_func,notify_baton, pool);
	if (err)
		kdDebug() << err->message << endl;
	else {
		if ((commit_info) && (SVN_IS_VALID_REVNUM (commit_info->revision))) {
			QString ci;
			ci.sprintf ("Committed revision %" SVN_REVNUM_T_FMT ".", commit_info->revision);	
			kdDebug() << ci << endl;
		}
		kdDebug() << "Done" << endl;
	}
}

void svnPart::slotLog() {
}

void svnPart::slotDiff() {
}

#include "svn_part.moc"
/* vim: set ai ts=8 sw=8 : */
