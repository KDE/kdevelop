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
#include "kdevappfrontend.h"
#include "commitdlg.h"
#include "svn_part.h"
#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_config.h>
#include <subversion-1/svn_path.h>
#include <subversion-1/svn_utf.h>
#include "svnoptionswidget.h"
#include "domutil.h"

typedef KGenericFactory<SvnPart> svnFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevsvn, svnFactory( "kdevsvn" ) );

	SvnPart::SvnPart(QObject *parent, const char *name, const QStringList& )
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

SvnPart::~SvnPart()
{
	svn_pool_destroy (pool);
	apr_terminate();
}

#define EDITOR_PREFIX_TXT "SVN:"
//stolen in svn cmndline code and modified
svn_error_t *SvnPart::get_log_message (const char **log_msg, apr_array_header_t *commit_items,
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
	struct log_msg_baton *lmb = (log_msg_baton*)baton;
	
	if (! (commit_items || commit_items->nelts))
	{
		*log_msg = "";
		return SVN_NO_ERROR;
	}

	if (!lmb) kdDebug() << "NULL baton" << endl;

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
			svn_client_commit_item_t *item = ((svn_client_commit_item_t **) commit_items->elts)[i];
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
		d.resize(400,400);
		d.setLog(tmp_message->data);
		if (d.exec() == QDialog::Rejected) {
			*log_msg=NULL; //abort
			return SVN_NO_ERROR;
		}

		QString msg = d.logMessage();
//		msg2 = (const char*)msg;
		svn_error_t *err = svn_utf_cstring_to_utf8(&msg2, (const char*)msg,NULL,pool);
		if (err) {
			*log_msg=NULL;
			svn_handle_error(err,stderr,FALSE);
			return err;
		}

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

void *SvnPart::make_log_msg_baton (const char *base_dir, apr_pool_t *pool)
{
	struct log_msg_baton *baton = (log_msg_baton*)apr_palloc (pool, sizeof (*baton));
	baton->base_dir = base_dir ? base_dir : ".";
	return baton;
}

void SvnPart::readConf() {
	QDomDocument &dom = *this->projectDom();
	recursive = DomUtil::readBoolEntry(dom,"/kdevsvn/recurse",false);
	force = DomUtil::readBoolEntry(dom,"/kdevsvn/force",false);
	verbose = DomUtil::readBoolEntry(dom,"/kdevsvn/verbose",false);
}

void SvnPart::projectConfigWidget(KDialogBase *dlg) {
	QVBox *vbox = dlg->addVBoxPage(i18n("Subversion"));
	svnOptionsWidget *w = new svnOptionsWidget(this, (QWidget *)vbox, "Subversion config widget");
	connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void SvnPart::contextMenu(QPopupMenu *popup, const Context *context) {
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

svn_client_auth_baton_t *SvnPart::createAuthBaton() {
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

void SvnPart::slotCommit() {
	svn_client_commit_info_t *commit_info=NULL;
	svn_wc_notify_func_t notify_func=NULL;
	void *notify_baton=NULL;
	const char *base_dir;
	svn_client_auth_baton_t *auth_baton=createAuthBaton();
	apr_array_header_t *targets = apr_array_make (pool, 5, sizeof (const char *));
	apr_array_header_t *condensed_targets;
	svn_client_get_commit_log_t log_msg_func=SvnPart::get_log_message;
	const char *xml_dst=NULL;
	svn_revnum_t revision=SVN_INVALID_REVNUM;
	svn_boolean_t nonrecursive=!recursive;

	kdDebug() << "SVN commit " << popupfile.utf8() << endl;
	(*((const char **) apr_array_push ((apr_array_header_t*)targets))) = apr_pstrdup (pool, popupfile.utf8());
	//prepare targets
	svn_error_t *error = svn_path_condense_targets (&base_dir, &condensed_targets, targets, pool);
	if (error) {
		svn_handle_error(error,stderr,false);
		return;
	}

	if ((! condensed_targets) || (! condensed_targets->nelts))
	{
		const char *parent_dir, *base_name;
		error = svn_wc_get_actual_target (base_dir, &parent_dir, &base_name, pool);
		if (error) {
			svn_handle_error(error,stderr,false);
			return;
		}
		
		if (base_name)
			base_dir = apr_pstrdup (pool, parent_dir);
		kdDebug() << "parent_dir :" << parent_dir << endl;
		kdDebug() << "base_dir :" << base_dir << endl;
		kdDebug() << "base_name :" << base_name << endl;
	}
	
	get_notifier (&notify_func, &notify_baton, FALSE, FALSE, pool);
	//commit
	error = svn_client_commit (&commit_info, notify_func, notify_baton,
			auth_baton, targets,
			log_msg_func, make_log_msg_baton(base_dir,pool),
			xml_dst, revision, nonrecursive, pool);
	if (error) {
		svn_handle_error(error,stderr,false);
	} else {
		if ((commit_info) && (SVN_IS_VALID_REVNUM (commit_info->revision))) {
			QString ci;
			ci.sprintf ("Committed revision %" SVN_REVNUM_T_FMT ".", commit_info->revision);	
			kdDebug() << ci << endl;
			kdDebug() << "Done" << endl;
		}
	}
}

void SvnPart::slotUpdate() {
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

void SvnPart::slotAdd() {
	svn_boolean_t recurse=recursive;
	svn_wc_notify_func_t notify_func=NULL;
	void *notify_baton=NULL;

	kdDebug() << "SVN add " << popupfile.utf8() << endl;
	get_notifier (&notify_func, &notify_baton, FALSE, FALSE, pool);
	svn_error_t *err= svn_client_add (popupfile.utf8(), recurse,notify_func,notify_baton,pool);
	if (err)
		kdDebug() << err->message << endl;
	else
		kdDebug() << "Done" << endl;
}

void SvnPart::slotCleanup() {
	kdDebug() << "SVN cleanup " << popupfile << endl;
	svn_error_t *err= svn_client_cleanup(popupfile.utf8(),pool);
	if (err)
		kdDebug() << err->message << endl;
	else
		kdDebug() << "Done" << endl;
}

void SvnPart::slotRemove() {
	svn_client_commit_info_t *commit_info=NULL;
	//	svn_wc_adm_access_t *optional_adm_access; // not used in cmdline
	svn_boolean_t force_rem=force;
	svn_client_auth_baton_t *auth_baton=createAuthBaton();
	svn_client_get_commit_log_t log_msg_func=NULL;//FIXME
	void *log_msg_baton=NULL;//FIXME
	svn_wc_notify_func_t notify_func=NULL;
	void *notify_baton=NULL;
	//apr_pool_t *subpool = svn_pool_create (pool);

	get_notifier (&notify_func, &notify_baton, FALSE, FALSE, pool);
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

void SvnPart::slotLog() {
}

void SvnPart::slotDiff() {
}

void SvnPart::notify (void *baton, const char *path, svn_wc_notify_action_t action,
	svn_node_kind_t kind, const char *mime_type, svn_wc_notify_state_t content_state,
	svn_wc_notify_state_t prop_state, svn_revnum_t revision) 
{
	struct notify_baton *nb = (struct notify_baton*)baton;
	char statchar_buf[3] = "_ ";

	/* the pool (BATON) is typically the global pool; don't keep filling it */
	apr_pool_t *subpool = svn_pool_create (nb->pool);

	const char *path_native;
	svn_error_t *err;

	/* Always print some path */
	if (path[0] == '\0')
		path = ".";

	err = svn_utf_cstring_from_utf8 (&path_native, path, subpool);
	if (err)
	{
		svnDebug ("WARNING: error decoding UTF-8 for ?");
		svn_pool_destroy (subpool);
		return;
	}
	switch (action)
	{
		case svn_wc_notify_update_delete:
			nb->received_some_change = TRUE;
			svnMsg (QString("D  ")+QString(path_native));
			break;

		case svn_wc_notify_update_add:
			nb->received_some_change = TRUE;
			svnMsg (QString("A  ")+QString(path_native));
			break;

		case svn_wc_notify_restore:
			svnMsg (QString("Restored  ")+QString(path_native));
			break;

		case svn_wc_notify_revert:
			svnMsg (QString("Reverted  ")+QString(path_native));
			break;

		case svn_wc_notify_resolve:
			svnMsg (QString("Resolved conflicted state of ")+QString(path_native));
			break;

		case svn_wc_notify_add:
			/* We *should* only get the MIME_TYPE if PATH is a file.  If we
			   do get it, and the mime-type is not in the "text/" grouping,
			   note that this is a binary addition.  */
			if (mime_type
					&& ((strlen (mime_type)) > 5)
					&& ((strncmp (mime_type, "text/", 5)) != 0))
				svnMsg (QString("A (bin)  ")+QString(path_native));
			else
				printf ("A         %s\n", path_native);
				svnMsg (QString("A         ")+QString(path_native));
			break;

		case svn_wc_notify_delete:
			nb->received_some_change = TRUE;
			svnMsg (QString("D         ")+QString(path_native));
			break;

		case svn_wc_notify_update_update:
			{
				/* If this is an inoperative dir change, do no notification.
				   An inoperative dir change is when a directory gets closed
				   without any props having been changed. */
				if (! ((kind == svn_node_dir)
							&& ((prop_state == svn_wc_notify_state_inapplicable)
								|| (prop_state == svn_wc_notify_state_unknown)
								|| (prop_state == svn_wc_notify_state_unchanged))))
				{
					nb->received_some_change = TRUE;

					if (kind == svn_node_file)
					{
						if (content_state == svn_wc_notify_state_conflicted)
							statchar_buf[0] = 'C';
						else if (content_state == svn_wc_notify_state_merged)
							statchar_buf[0] = 'G';
						else if (content_state == svn_wc_notify_state_modified)
							statchar_buf[0] = 'U';
					}

					if (prop_state == svn_wc_notify_state_conflicted)
						statchar_buf[1] = 'C';
					else if (prop_state == svn_wc_notify_state_merged)
						statchar_buf[1] = 'G';
					else if (prop_state == svn_wc_notify_state_modified)
						statchar_buf[1] = 'U';

					svnMsg (QString(statchar_buf)+QString(" ")+ QString(path_native));
				}
			}
			break;

		case svn_wc_notify_update_external:
			/* Currently this is used for checkouts and switches too.  If we
			   want different output, we'll have to add new actions. */
			svnMsg (QString("Fecthing external item into ")+ QString(path_native));
			break;

		case svn_wc_notify_update_completed:
			{
				if (! nb->suppress_final_line)
				{
					if (SVN_IS_VALID_REVNUM (revision))
					{
						if (nb->is_checkout) 
						{
							QString msg;
							msg.sprintf ("Checked out revision %" SVN_REVNUM_T_FMT ".\n", revision);
							svnMsg(msg);
						}
						else
						{
							if (nb->received_some_change) 
							{
								QString msg;
								msg.sprintf ("Updated to revision %" SVN_REVNUM_T_FMT ".\n", revision);
								svnMsg(msg);
							}
							else
							{
								QString msg;
								msg.sprintf ("At revision %" SVN_REVNUM_T_FMT ".\n", revision);
								svnMsg(msg);
							}
						}
					}
					else  /* no revision */
					{
						if (nb->is_checkout)
							svnMsg("Checkout complete.");
						else
							svnMsg("Update complete");
					}
				}
			}

			break;

		case svn_wc_notify_commit_modified:
			svnMsg (QString("Sending        ")+ QString(path_native));
			break;

		case svn_wc_notify_commit_added:
			if (mime_type
					&& ((strlen (mime_type)) > 5)
					&& ((strncmp (mime_type, "text/", 5)) != 0))
				svnMsg (QString("Adding  (bin)  ") + QString(path_native));
			else
				svnMsg(QString("Adding         ") + QString(path_native));
			break;

		case svn_wc_notify_commit_deleted:
			svnMsg (QString("Deleting       ") + QString(path_native));
			break;

		case svn_wc_notify_commit_replaced:
			svnMsg(QString("Replacing      ")+QString(path_native));
			break;

		case svn_wc_notify_commit_postfix_txdelta:
			if (! nb->sent_first_txdelta)
			{
				svnMsg("Transmitting file data");
				nb->sent_first_txdelta = TRUE;
			}

			svnMsg(".");
			break;

		default:
			break;
	}

	svn_pool_destroy (subpool);
}

void SvnPart::get_notifier(svn_wc_notify_func_t *notify_func_p, void **notify_baton_p,
	svn_boolean_t is_checkout, svn_boolean_t suppress_final_line, 
	apr_pool_t *pool) 
{
	struct notify_baton *nb = (struct notify_baton*)apr_palloc (pool, sizeof (*nb));

	nb->received_some_change = FALSE;
	nb->sent_first_txdelta = FALSE;
	nb->is_checkout = is_checkout;
	nb->suppress_final_line = suppress_final_line;
	nb->pool = pool;

	*notify_func_p = SvnPart::notify;
	*notify_baton_p = nb;
}

void SvnPart::svnDebug(const char *dbg) {
	kdDebug() << dbg << endl;
//	SvnPart::me->appFrontend()->insertStderrLine(dbg);
}

void SvnPart::svnMsg(const char *msg) {
	kdDebug() << msg << endl;
//	SvnPart::me->appFrontend()->insertStdoutLine(msg);
}

#include "svn_part.moc"
/* vim: set ai ts=8 sw=8 : */
