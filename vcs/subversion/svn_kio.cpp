/* This file is part of the KDE project
   Copyright (C) 2003 Mickael Marchand <marchand@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qcstring.h>
#include <qsocket.h>
#include <qdatetime.h>
#include <qbitarray.h>

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>
#include <ksock.h>
#include <dcopclient.h>
#include <qcstring.h>

#include <subversion-1/svn_sorts.h>
#include <subversion-1/svn_path.h>
#include <subversion-1/svn_utf.h>
#include <subversion-1/svn_ra.h>
#include <subversion-1/svn_time.h>
#include <subversion-1/svn_cmdline.h>

#include <kmimetype.h>
#include <qfile.h>

#include "svn_kio.h"
#include <apr_portable.h>
// #include "commitdlg.h"
#include <ktextedit.h>

using namespace KIO;
using namespace SvnGlobal;

typedef struct
{
	/* Holds the directory that corresponds to the REPOS_URL at RA->open()
	 *      time. When callbacks specify a relative path, they are joined with
	 *           this base directory. */
	const char *base_dir;
	svn_wc_adm_access_t *base_access;

	/* An array of svn_client_commit_item_t * structures, present only
	 *      during working copy commits. */
	apr_array_header_t *commit_items;

	/* A hash of svn_config_t's, keyed off file name (i.e. the contents of
	 *      ~/.subversion/config end up keyed off of 'config'). */
	apr_hash_t *config;

	/* The pool to use for session-related items. */
	apr_pool_t *pool;

} svn_client__callback_baton_t;

static svn_error_t *
open_tmp_file (apr_file_t **fp,
               void *callback_baton,
               apr_pool_t *pool)
{
  svn_client__callback_baton_t *cb = (svn_client__callback_baton_t *) callback_baton;
  const char *truepath;
  const char *ignored_filename;

  if (cb->base_dir)
    truepath = apr_pstrdup (pool, cb->base_dir);
  else
    truepath = "";

  /* Tack on a made-up filename. */
  truepath = svn_path_join (truepath, "tempfile", pool);

  /* Open a unique file;  use APR_DELONCLOSE. */
  SVN_ERR (svn_io_open_unique_file (fp, &ignored_filename,
                                    truepath, ".tmp", TRUE, pool));

  return SVN_NO_ERROR;
}

static svn_error_t *write_to_string(void *baton, const char *data, apr_size_t *len) {
	kbaton *tb = ( kbaton* )baton;
	svn_stringbuf_appendbytes(tb->target_string, data, *len);
	return SVN_NO_ERROR;
}

static int
compare_items_as_paths (const svn_sort__item_t*a, const svn_sort__item_t*b) {
  return svn_path_compare_paths ((const char *)a->key, (const char *)b->key);
}

kio_svnProtocol::kio_svnProtocol(const QCString &pool_socket, const QCString &app_socket)
	: SlaveBase("kio_svn", pool_socket, app_socket) {
		kdDebug(9036) << "kio_svnProtocol::kio_svnProtocol()" << endl;

		m_counter = 0;

		apr_initialize();
                // Make sure to properly initialize svn client, besides other things, this sets up
                // NLS support for environments that don't use UTF-8
                svn_cmdline_init("kdevsvnd",NULL);
		// CleanUP ctx preventing crash in svn_client_update and other
		memset(&ctx, 0, sizeof(ctx));
		pool = svn_pool_create (NULL);

		svn_error_t *err = svn_client_create_context(&ctx, pool);
                if ( err ) {
                        kdDebug(9036) << "kio_svnProtocol::kio_svnProtocol() create_context ERROR" << endl;
                        error( KIO::ERR_SLAVE_DEFINED, err->message );
                        return;
                }

		err = svn_config_ensure (NULL,pool);
		if ( err ) {
			kdDebug(9036) << "kio_svnProtocol::kio_svnProtocol() configensure ERROR" << endl;
			error( KIO::ERR_SLAVE_DEFINED, err->message );
			return;
		}
		svn_config_get_config (&ctx->config, NULL, pool);

		ctx->log_msg_func = kio_svnProtocol::commitLogPrompt;
		ctx->log_msg_baton = this; //pass this so that we can get a dcopClient from it
		//TODO
		ctx->cancel_func = NULL;
		// progress notifications
		ctx->progress_func = kio_svnProtocol::progressCallback;
		ctx->progress_baton = this;

		apr_array_header_t *providers = apr_array_make(pool, 15, sizeof(svn_auth_provider_object_t *));

		svn_auth_provider_object_t *provider;

		//disk cache
		svn_client_get_simple_provider(&provider,pool);
		APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;
		svn_client_get_username_provider(&provider,pool);
		APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;

		//interactive prompt
		svn_client_get_simple_prompt_provider (&provider,kio_svnProtocol::checkAuth,this,2,pool);
		APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;
		//we always ask user+pass, no need for a user only question
/*		svn_client_get_username_prompt_provider
 *		(&provider,kio_svnProtocol::checkAuth,this,2,pool);
		APR_ARRAY_PUSH(providers, svn_auth_provider_object_t*) = provider;*/

		//SSL disk cache, keep that one, because it does nothing bad :)
		svn_client_get_ssl_server_trust_file_provider (&provider, pool);
		APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
		svn_client_get_ssl_client_cert_file_provider (&provider, pool);
		APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
		svn_client_get_ssl_client_cert_pw_file_provider (&provider, pool);
		APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

		//SSL interactive prompt, where things get hard
		svn_client_get_ssl_server_trust_prompt_provider (&provider, kio_svnProtocol::trustSSLPrompt, (void*)this, pool);
		APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
		svn_client_get_ssl_client_cert_prompt_provider (&provider, kio_svnProtocol::clientCertSSLPrompt, (void*)this, 2, pool);
		APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
		svn_client_get_ssl_client_cert_pw_prompt_provider (&provider, kio_svnProtocol::clientCertPasswdPrompt, (void*)this, 2, pool);
		APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

		svn_auth_open(&ctx->auth_baton, providers, pool);
}

kio_svnProtocol::~kio_svnProtocol(){
	kdDebug(9036) << "kio_svnProtocol::~kio_svnProtocol()" << endl;
	svn_pool_destroy(pool);
	apr_terminate();
}

void kio_svnProtocol::initNotifier(bool is_checkout, bool is_export, bool suppress_final_line, apr_pool_t *spool) {
	m_counter=0;//reset counter
	ctx->notify_func = kio_svnProtocol::notify;
	struct notify_baton *nb = ( struct notify_baton* )apr_palloc(spool, sizeof( struct notify_baton ) );
	nb->master = this;
	nb->received_some_change = FALSE;
	nb->sent_first_txdelta = FALSE;
	nb->is_checkout = is_checkout;
	nb->is_export = is_export;
	nb->suppress_final_line = suppress_final_line;
	nb->in_external = FALSE;
	nb->had_print_error = FALSE;
	nb->pool = svn_pool_create (spool);

	ctx->notify_baton = nb;
}

svn_error_t* kio_svnProtocol::checkAuth(svn_auth_cred_simple_t **cred, void *baton, const char *realm, const char *username, svn_boolean_t may_save, apr_pool_t *pool) {
	kdDebug(9036) << "kio_svnProtocol::checkAuth() " << endl;
	kio_svnProtocol *p = ( kio_svnProtocol* )baton;
	svn_auth_cred_simple_t *ret = (svn_auth_cred_simple_t*)apr_pcalloc (pool, sizeof (*ret));

	p->info.keepPassword = true;
	p->info.verifyPath=true;
	kdDebug(9036 ) << "auth current URL : " << p->myURL.url() << endl;
	p->info.url = p->myURL;
	p->info.username = username; //( const char* )svn_auth_get_parameter( p->ctx->auth_baton, SVN_AUTH_PARAM_DEFAULT_USERNAME );
	if (realm) {
		p->info.prompt = i18n("Username and Password for %1.").arg(realm);
	}

//	if ( !p->checkCachedAuthentication( p->info ) ){
		p->openPassDlg( p->info );
//	}
	ret->username = apr_pstrdup(pool, p->info.username.utf8());
	ret->password = apr_pstrdup(pool, p->info.password.utf8());
	if (may_save) ret->may_save = p->info.keepPassword;
	*cred = ret;
	return SVN_NO_ERROR;
}

void kio_svnProtocol::recordCurrentURL(const KURL& url) {
	myURL = url;
}

//don't implement mimeType() until we don't need to download the whole file

void kio_svnProtocol::get(const KURL& url ){
	kdDebug(9036) << "kio_svn::get(const KURL& url)" << endl ;

	QString remoteServer = url.host();
	infoMessage(i18n("Looking for %1...").arg( remoteServer ) );

	apr_pool_t *subpool = svn_pool_create (pool);
	kbaton *bt = (kbaton*)apr_pcalloc(subpool, sizeof(*bt));
	bt->target_string = svn_stringbuf_create("", subpool);
	bt->string_stream = svn_stream_create(bt,subpool);
	svn_stream_set_write(bt->string_stream,write_to_string);

	QString target = makeSvnURL( url );
	kdDebug(9036) << "SvnURL: " << target << endl;
	recordCurrentURL( KURL( target ) );

	//find the requested revision
	svn_opt_revision_t rev;
	svn_opt_revision_t endrev;
	int idx = target.findRev( "?rev=" );
	if ( idx != -1 ) {
		QString revstr = target.mid( idx+5 );
#if 0
		kdDebug(9036) << "revision string found " << revstr  << endl;
		if ( revstr == "HEAD" ) {
			rev.kind = svn_opt_revision_head;
			kdDebug(9036) << "revision searched : HEAD" << endl;
		} else {
			rev.kind = svn_opt_revision_number;
			rev.value.number = revstr.toLong();
			kdDebug(9036) << "revision searched : " << rev.value.number << endl;
		}
#endif
		svn_opt_parse_revision( &rev, &endrev, revstr.utf8(), subpool );
		target = target.left( idx );
		kdDebug(9036) << "new target : " << target << endl;
	} else {
		kdDebug(9036) << "no revision given. searching HEAD " << endl;
		rev.kind = svn_opt_revision_head;
	}
	initNotifier(false, false, false, subpool);

	svn_error_t *err = svn_client_cat (bt->string_stream, svn_path_canonicalize( target.utf8(),subpool ),&rev,ctx, subpool);
	if ( err ) {
		error( KIO::ERR_SLAVE_DEFINED, err->message );
		svn_pool_destroy( subpool );
		return;
	}

	// Send the mimeType as soon as it is known
	QByteArray *cp = new QByteArray();
	cp->setRawData( bt->target_string->data, bt->target_string->len );
	KMimeType::Ptr mt = KMimeType::findByContent(*cp);
	kdDebug(9036) << "KMimeType returned : " << mt->name() << endl;
	mimeType( mt->name() );

	totalSize(bt->target_string->len);

	//send data
	data(*cp);

	data(QByteArray()); // empty array means we're done sending the data
	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::stat(const KURL & url){
	kdDebug(9036) << "kio_svn::stat(const KURL& url) : " << url.url() << endl ;

	void *ra_baton, *session;
	svn_ra_plugin_t *ra_lib;
	svn_node_kind_t kind;
	apr_pool_t *subpool = svn_pool_create (pool);

	QString target = makeSvnURL( url);
	kdDebug(9036) << "SvnURL: " << target << endl;
	recordCurrentURL( KURL( target ) );

	//find the requested revision
	svn_opt_revision_t rev;
	svn_opt_revision_t endrev;
	int idx = target.findRev( "?rev=" );
	if ( idx != -1 ) {
		QString revstr = target.mid( idx+5 );
#if 0
		kdDebug(9036) << "revision string found " << revstr  << endl;
		if ( revstr == "HEAD" ) {
			rev.kind = svn_opt_revision_head;
			kdDebug(9036) << "revision searched : HEAD" << endl;
		} else {
			rev.kind = svn_opt_revision_number;
			rev.value.number = revstr.toLong();
			kdDebug(9036) << "revision searched : " << rev.value.number << endl;
		}
#endif
		svn_opt_parse_revision( &rev, &endrev, revstr.utf8( ), subpool );
		target = target.left( idx );
		kdDebug(9036) << "new target : " << target << endl;
	} else {
		kdDebug(9036) << "no revision given. searching HEAD " << endl;
		rev.kind = svn_opt_revision_head;
	}

	//init
	svn_error_t *err = svn_ra_init_ra_libs(&ra_baton,subpool);
	if ( err ) {
		kdDebug(9036) << "init RA libs failed : " << err->message << endl;
		return;
	}
	//find RA libs
	err = svn_ra_get_ra_library(&ra_lib,ra_baton,svn_path_canonicalize( target.utf8(), subpool ),subpool);
	if ( err ) {
		kdDebug(9036) << "RA get libs failed : " << err->message << endl;
		return;
	}
	kdDebug(9036) << "RA init completed" << endl;

	//start session
	svn_ra_callbacks_t *cbtable = (svn_ra_callbacks_t*)apr_pcalloc(subpool, sizeof(*cbtable));
	kio_svn_callback_baton_t *callbackbt = (kio_svn_callback_baton_t*)apr_pcalloc(subpool, sizeof( *callbackbt ));

	cbtable->open_tmp_file = open_tmp_file;
	cbtable->get_wc_prop = NULL;
	cbtable->set_wc_prop = NULL;
	cbtable->push_wc_prop = NULL;
	cbtable->auth_baton = ctx->auth_baton;

	callbackbt->base_dir = target.utf8();
	callbackbt->pool = subpool;
	callbackbt->config = ctx->config;

	err = ra_lib->open(&session,svn_path_canonicalize( target.utf8(), subpool ),cbtable,callbackbt,ctx->config,subpool);
	if ( err ) {
		kdDebug(9036)<< "Open session " << err->message << endl;
		return;
	}
	kdDebug(9036) << "Session opened to " << target << endl;
	//find number for HEAD
	if (rev.kind == svn_opt_revision_head) {
		err = ra_lib->get_latest_revnum(session,&rev.value.number,subpool);
		if ( err ) {
			kdDebug(9036)<< "Latest RevNum " << err->message << endl;
			return;
		}
		kdDebug(9036) << "Got rev " << rev.value.number << endl;
	}

	//get it
	ra_lib->check_path(session,"",rev.value.number,&kind,subpool);
	kdDebug(9036) << "Checked Path" << endl;
	UDSEntry entry;
	switch ( kind ) {
		case svn_node_file:
			kdDebug(9036) << "::stat result : file" << endl;
			createUDSEntry(url.filename(),"",0,false,0,entry);
			statEntry( entry );
			break;
		case svn_node_dir:
			kdDebug(9036) << "::stat result : directory" << endl;
			createUDSEntry(url.filename(),"",0,true,0,entry);
			statEntry( entry );
			break;
		case svn_node_unknown:
		case svn_node_none:
			//error XXX
		default:
			kdDebug(9036) << "::stat result : UNKNOWN ==> WOW :)" << endl;
			;
	}
	finished();
	svn_pool_destroy( subpool );
}

void kio_svnProtocol::listDir(const KURL& url){
	kdDebug(9036) << "kio_svn::listDir(const KURL& url) : " << url.url() << endl ;

	apr_pool_t *subpool = svn_pool_create (pool);
	apr_hash_t *dirents;

	QString target = makeSvnURL( url);
	kdDebug(9036) << "SvnURL: " << target << endl;
	recordCurrentURL( KURL( target ) );

	//find the requested revision
	svn_opt_revision_t rev;
	svn_opt_revision_t endrev;
	int idx = target.findRev( "?rev=" );
	if ( idx != -1 ) {
		QString revstr = target.mid( idx+5 );
		svn_opt_parse_revision( &rev, &endrev, revstr.utf8(), subpool );
#if 0
		kdDebug(9036) << "revision string found " << revstr  << endl;
		if ( revstr == "HEAD" ) {
			rev.kind = svn_opt_revision_head;
			kdDebug(9036) << "revision searched : HEAD" << endl;
		} else {
			rev.kind = svn_opt_revision_number;
			rev.value.number = revstr.toLong();
			kdDebug(9036) << "revision searched : " << rev.value.number << endl;
		}
#endif
		target = target.left( idx );
		kdDebug(9036) << "new target : " << target << endl;
	} else {
		kdDebug(9036) << "no revision given. searching HEAD " << endl;
		rev.kind = svn_opt_revision_head;
	}

	initNotifier(false, false, false, subpool);
	svn_error_t *err = svn_client_ls (&dirents, svn_path_canonicalize( target.utf8(), subpool ), &rev, false, ctx, subpool);
	if ( err ) {
		error( KIO::ERR_SLAVE_DEFINED, err->message );
		svn_pool_destroy( subpool );
		return;
	}

  apr_array_header_t *array;
  int i;

  array = svn_sort__hash (dirents, compare_items_as_paths, subpool);

  UDSEntry entry;
  for (i = 0; i < array->nelts; ++i) {
	  entry.clear();
      const char *utf8_entryname, *native_entryname;
      svn_dirent_t *dirent;
      svn_sort__item_t *item;

      item = &APR_ARRAY_IDX (array, i, svn_sort__item_t);

      utf8_entryname = (const char*)item->key;

      dirent = (svn_dirent_t*)apr_hash_get (dirents, utf8_entryname, item->klen);

      svn_utf_cstring_from_utf8 (&native_entryname, utf8_entryname, subpool);
			const char *native_author = NULL;

			//XXX BUGGY
/*			apr_time_exp_t timexp;
			apr_time_exp_lt(&timexp, dirent->time);
			apr_os_exp_time_t *ostime;
			apr_os_exp_time_get( &ostime, &timexp);

			time_t mtime = mktime( ostime );*/

			if (dirent->last_author)
				svn_utf_cstring_from_utf8 (&native_author, dirent->last_author, subpool);

			if ( createUDSEntry(QString( native_entryname ), QString( native_author ), dirent->size,
						dirent->kind==svn_node_dir ? true : false, 0, entry) )
				listEntry( entry, false );
	}
	listEntry( entry, true );

	finished();
	svn_pool_destroy (subpool);
}

bool kio_svnProtocol::createUDSEntry( const QString& filename, const QString& user, long long int size, bool isdir, time_t mtime, UDSEntry& entry) {
	kdDebug(9036) << "MTime : " << ( long )mtime << endl;
	kdDebug(9036) << "UDS filename : " << filename << endl;
	UDSAtom atom;
	atom.m_uds = KIO::UDS_NAME;
	atom.m_str = filename;
	entry.append( atom );

	atom.m_uds = KIO::UDS_FILE_TYPE;
	atom.m_long = isdir ? S_IFDIR : S_IFREG;
	entry.append( atom );

	atom.m_uds = KIO::UDS_SIZE;
	atom.m_long = size;
	entry.append( atom );

	atom.m_uds = KIO::UDS_MODIFICATION_TIME;
	atom.m_long = mtime;
	entry.append( atom );

	atom.m_uds = KIO::UDS_USER;
	atom.m_str = user;
	entry.append( atom );

	return true;
}

// not used, at least for KDevelop
// void kio_svnProtocol::copy(const KURL & src, const KURL& dest, int /*permissions*/, bool /*overwrite*/) {
// 	kdDebug(9036) << "kio_svnProtocol::copy() Source : " << src.url() << " Dest : " << dest.url() << endl;
// 
// 	apr_pool_t *subpool = svn_pool_create (pool);
// 	svn_client_commit_info_t *commit_info = NULL;
// 
// 	KURL nsrc = src;
// 	KURL ndest = dest;
// 	nsrc.setProtocol( chooseProtocol( src.protocol() ) );
// 	ndest.setProtocol( chooseProtocol( dest.protocol() ) );
// 	QString srcsvn = nsrc.url();
// 	QString destsvn = ndest.url();
// 
// 	recordCurrentURL( nsrc );
// 
// 	//find the requested revision
// 	svn_opt_revision_t rev;
// 	int idx = srcsvn.findRev( "?rev=" );
// 	if ( idx != -1 ) {
// 		QString revstr = srcsvn.mid( idx+5 );
// 		kdDebug(9036) << "revision string found " << revstr  << endl;
// 		if ( revstr == "HEAD" ) {
// 			rev.kind = svn_opt_revision_head;
// 			kdDebug(9036) << "revision searched : HEAD" << endl;
// 		} else {
// 			rev.kind = svn_opt_revision_number;
// 			rev.value.number = revstr.toLong();
// 			kdDebug(9036) << "revision searched : " << rev.value.number << endl;
// 		}
// 		srcsvn = srcsvn.left( idx );
// 		kdDebug(9036) << "new src : " << srcsvn << endl;
// 	} else {
// 		kdDebug(9036) << "no revision given. searching HEAD " << endl;
// 		rev.kind = svn_opt_revision_head;
// 	}
// 
// 	initNotifier(false, false, false, subpool);
// 	svn_error_t *err = svn_client_copy(&commit_info, srcsvn.utf8(), &rev, destsvn.utf8(), ctx, subpool);
// 	if ( err ) {
// 		error( KIO::ERR_SLAVE_DEFINED, err->message );
// 		svn_pool_destroy (subpool);
// 	}
// 
// 	finished();
// 	svn_pool_destroy (subpool);
// }

void kio_svnProtocol::mkdir( const KURL::List& list, int /*permissions*/ ) {
	kdDebug(9036) << "kio_svnProtocol::mkdir(LIST) : " << list << endl;

	apr_pool_t *subpool = svn_pool_create (pool);
	svn_client_commit_info_t *commit_info = NULL;

	recordCurrentURL( list[ 0 ] );

	apr_array_header_t *targets = apr_array_make(subpool, list.count()+1, sizeof(const char *));

	KURL::List::const_iterator it = list.begin(), end = list.end();
	for ( ; it != end; ++it ) {
		QString cur = makeSvnURL( *it );
		kdDebug( 9036 ) << "kio_svnProtocol::mkdir raw url for subversion : " << cur << endl;
		const char *_target = apr_pstrdup( subpool, svn_path_canonicalize( apr_pstrdup( subpool, cur.utf8() ), subpool ) );
		(*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) = _target;
	}

	initNotifier(false, false, false, subpool);
	svn_error_t *err = svn_client_mkdir(&commit_info,targets,ctx,subpool);
	if ( err ) {
		error( KIO::ERR_COULD_NOT_MKDIR, QString::fromLocal8Bit(err->message) );
		svn_pool_destroy (subpool);
        return;
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::mkdir( const KURL& url, int /*permissions*/ ) {
	kdDebug(9036) << "kio_svnProtocol::mkdir() : " << url.url() << endl;

	apr_pool_t *subpool = svn_pool_create (pool);
	svn_client_commit_info_t *commit_info = NULL;

	QString target = makeSvnURL( url);
	kdDebug(9036) << "SvnURL: " << target << endl;
	recordCurrentURL( KURL( target ) );

	apr_array_header_t *targets = apr_array_make(subpool, 2, sizeof(const char *));
	(*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) = apr_pstrdup( subpool, target.utf8() );

	initNotifier(false, false, false, subpool);
	svn_error_t *err = svn_client_mkdir(&commit_info,targets,ctx,subpool);
	if ( err ) {
		error( KIO::ERR_COULD_NOT_MKDIR, err->message );
		svn_pool_destroy (subpool);
        return;
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::del( const KURL& url, bool /*isfile*/ ) {
	kdDebug(9036) << "kio_svnProtocol::del() : " << url.url() << endl;

	apr_pool_t *subpool = svn_pool_create (pool);
	svn_client_commit_info_t *commit_info = NULL;

	QString target = makeSvnURL(url);
	kdDebug(9036) << "SvnURL: " << target << endl;
	recordCurrentURL( KURL( target ) );

	apr_array_header_t *targets = apr_array_make(subpool, 2, sizeof(const char *));
	(*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) = apr_pstrdup( subpool, target.utf8() );

	initNotifier(false, false, false, subpool);
	svn_error_t *err = svn_client_delete(&commit_info,targets,false/*force remove locally modified files in wc*/,ctx,subpool);
	if ( err ) {
		error( KIO::ERR_CANNOT_DELETE, err->message );
		svn_pool_destroy (subpool);
        return;
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::rename(const KURL& src, const KURL& dest, bool /*overwrite*/) {
	kdDebug(9036) << "kio_svnProtocol::rename() Source : " << src.url() << " Dest : " << dest.url() << endl;

	apr_pool_t *subpool = svn_pool_create (pool);
	svn_client_commit_info_t *commit_info = NULL;

	KURL nsrc = src;
	KURL ndest = dest;
	nsrc.setProtocol( chooseProtocol( src.protocol() ) );
	ndest.setProtocol( chooseProtocol( dest.protocol() ) );
	QString srcsvn = nsrc.url();
	QString destsvn = ndest.url();

	recordCurrentURL( nsrc );

	//find the requested revision
	svn_opt_revision_t rev;
	int idx = srcsvn.findRev( "?rev=" );
	if ( idx != -1 ) {
		QString revstr = srcsvn.mid( idx+5 );
		kdDebug(9036) << "revision string found " << revstr  << endl;
		if ( revstr == "HEAD" ) {
			rev.kind = svn_opt_revision_head;
			kdDebug(9036) << "revision searched : HEAD" << endl;
		} else {
			rev.kind = svn_opt_revision_number;
			rev.value.number = revstr.toLong();
			kdDebug(9036) << "revision searched : " << rev.value.number << endl;
		}
		srcsvn = srcsvn.left( idx );
		kdDebug(9036) << "new src : " << srcsvn << endl;
	} else {
		kdDebug(9036) << "no revision given. searching HEAD " << endl;
		rev.kind = svn_opt_revision_head;
	}

	initNotifier(false, false, false, subpool);
	svn_error_t *err = svn_client_move(&commit_info, srcsvn.utf8(), &rev, destsvn.utf8(), false/*force remove locally modified files in wc*/, ctx, subpool);
	if ( err ) {
		error( KIO::ERR_CANNOT_RENAME, err->message );
		svn_pool_destroy (subpool);
		return;
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::special( const QByteArray& data ) {
// 	kdDebug(9036) << "kio_svnProtocol::special" << endl;

	QDataStream stream(data, IO_ReadOnly);
	int tmp;

	stream >> tmp;
	kdDebug(9036) << "kio_svnProtocol::special " << tmp << endl;

	switch ( tmp ) {
		case SVN_CHECKOUT:
			{
				KURL repository, wc;
				int revnumber;
				QString revkind;
				stream >> repository;
				stream >> wc;
				stream >> revnumber;
				stream >> revkind;
				kdDebug(9036) << "kio_svnProtocol CHECKOUT from " << repository.url() << " to " << wc.url() << " at " << revnumber << " or " << revkind << endl;
				checkout( repository, wc, revnumber, revkind );
				break;
			}
		case SVN_UPDATE:
			{
				KURL::List list;
				int revnumber;
				QString revkind;
				stream >> list;
				stream >> revnumber;
				stream >> revkind;
				kdDebug(9036) << "kio_svnProtocol UPDATE " << endl;
				update( list, revnumber, revkind );
				break;
			}
		case SVN_COMMIT:
			{
				KURL::List wclist;
				while ( !stream.atEnd() ) {
					KURL tmp;
					stream >> tmp;
					wclist << tmp;
				}
				kdDebug(9036) << "kio_svnProtocol COMMIT" << endl;
				commit( wclist );
				break;
			}
		case SVN_COMMIT_2:
			{
				bool recurse, keeplocks;
				KURL::List wclist;
				stream >> recurse;
				stream >> keeplocks;
				while ( !stream.atEnd() ) {
					KURL tmp;
					stream >> tmp;
					wclist << tmp;
				}
				kdDebug(9036) << "kio_svnProtocol COMMIT2" << endl;
				commit2( recurse, keeplocks, wclist );
				break;
			}
		case SVN_LOG:
			{
				kdDebug(9036) << "kio_svnProtocol LOG" << endl;
				int revstart, revend;
				QString revkindstart, revkindend;
                bool discorverChangedPath, strictNodeHistory;
				KURL::List targets;

				stream >> revstart;
				stream >> revkindstart;
				stream >> revend;
				stream >> revkindend;
                stream >> discorverChangedPath;
                stream >> strictNodeHistory;
				while ( !stream.atEnd() ) {
					KURL tmp;
					stream >> tmp;
					targets << tmp;
				}
				svn_log( revstart, revkindstart, revend, revkindend,
                         discorverChangedPath, strictNodeHistory, targets );
				break;
			}
		case SVN_IMPORT:
			{
				KURL wc,repos;
				stream >> repos;
				stream >> wc;
				kdDebug(9036) << "kio_svnProtocol IMPORT" << endl;
				import(repos,wc);
				break;
			}
		case SVN_ADD:
			{
				KURL::List wcList;
				stream >> wcList;
				kdDebug(9036) << "kio_svnProtocol ADD" << endl;
				add(wcList);
				break;
			}
		case SVN_DEL:
			{
				KURL::List wclist;
				stream >> wclist;
				kdDebug(9036) << "kio_svnProtocol DEL" << endl;
				wc_delete(wclist);
				break;
			}
		case SVN_REVERT:
			{
				KURL::List wclist;
				stream >> wclist;
				kdDebug(9036) << "kio_svnProtocol REVERT" << endl;
				wc_revert(wclist);
				break;
			}
		case SVN_STATUS:
			{
				KURL wc;
				bool checkRepos=false;
				bool fullRecurse=false;
				stream >> wc;
				stream >> checkRepos;
				stream >> fullRecurse;
				wc_status(wc,checkRepos,fullRecurse);
				break;
			}
		case SVN_STATUS_2:
			{
				KURL wc;
				QString revkind;
				int revnumber;
				bool checkRepos, fullRecurse, getAll, noIgnore;
				stream >> checkRepos;
				stream >> fullRecurse;
				stream >> getAll;
				stream >> noIgnore;
				stream >> revnumber;
				stream >> revkind;
				stream >> wc;
				wc_status2(wc,checkRepos,fullRecurse, getAll, noIgnore, revnumber, revkind);
				break;
			}
		case SVN_MKDIR:
			{
				KURL::List list;
				stream >> list;
				kdDebug(9036) << "kio_svnProtocol MKDIR" << endl;
				mkdir(list,0);
				break;
			}
		case SVN_RESOLVE:
			{
				KURL url;
				bool recurse;
				stream >> url;
				stream >> recurse;
				kdDebug(9036) << "kio_svnProtocol RESOLVE" << endl;
				wc_resolve(url,recurse);
				break;
			}
		case SVN_SWITCH:
			{
				KURL wc,url;
				bool recurse;
				int revnumber;
				QString revkind;
				stream >> wc;
				stream >> url;
				stream >> recurse;
				stream >> revnumber;
				stream >> revkind;
				kdDebug(9036) << "kio_svnProtocol SWITCH" << endl;
				svn_switch(wc,url,revnumber,revkind,recurse);
				break;
			}
		case SVN_SWITCH_RELOCATE:
			{
				KURL wc, origUrl, newUrl;
				bool recurse;
				stream >> wc;
				stream >> origUrl;
				stream >> newUrl;
				stream >> recurse;
				svn_switch_relocate( wc, origUrl, newUrl, recurse );
				break;
			}
		case SVN_DIFF:
			{
				KURL url1,url2;
				int rev1, rev2;
				bool recurse, pegdiff;
				QString revkind1, revkind2;
				stream >> url1;
				stream >> url2;
				stream >> rev1;
				stream >> revkind1;
				stream >> rev2;
				stream >> revkind2;
				stream >> recurse >> pegdiff;
				kdDebug(9036) << "kio_svnProtocol DIFF" << endl;
				svn_diff(url1,url2,rev1,rev2,revkind1,revkind2,recurse,pegdiff);
				break;
			}
		case SVN_BLAME:
			{
				KURL url;
				int urlMode;
				int pegRev, startRev, endRev;
				QString pegRevKind, startRevKind, endRevKind;
				stream >> url;
				stream >> urlMode;
// 				stream >> pegRev;
// 				stream >> pegRevKind;
				stream >> startRev;
				stream >> startRevKind;
				stream >> endRev;
				stream >> endRevKind;

				blame(url, (UrlMode)urlMode, startRev, startRevKind, endRev, endRevKind);
				break;
			}
        case SVN_INFO:
            {
                KURL pathOrUrl;
                int pegRev, rev;
                QString pegRevKind, revKind;
                bool recurse = false;
                stream >> pathOrUrl;
                stream >> pegRev;
                stream >> pegRevKind;
                stream >> rev;
                stream >> revKind;
                stream >> recurse;
                svn_info( pathOrUrl, pegRev, pegRevKind, rev, revKind, recurse );
                break;
            }
		case SVN_COPY:
			{
				KURL src, dest;
				int srcRev;
				QString srcRevKind;
				stream >> src;
				stream >> srcRev;
				stream >> srcRevKind;
				stream >> dest;
				svn_copy( src, srcRev, srcRevKind, dest );
                break;
			}
		case SVN_MERGE:
			{
				KURL src1, src2, wc_target;
				int rev1, rev2;
				QString revKind1, revKind2;
				bool recurse, ignore_ancestry, force, dry_run;
				stream >> src1 >> rev1 >> revKind1;
				stream >> src2 >> rev2 >> revKind2;
				stream >> wc_target;
				stream >> recurse >> ignore_ancestry >> force >> dry_run;
				svn_merge( src1, rev1, revKind1, src2, rev2, revKind2, wc_target,
						   recurse, ignore_ancestry, force, dry_run );
				break;
			}
		default:
			{
				kdDebug(9036) << "kio_svnProtocol DEFAULT" << endl;
				break;
			}
	}
}
/**
 * not used anywhere, anymore
*/
void kio_svnProtocol::popupMessage( const QString& message ) {
// 	QByteArray params;
// 	QDataStream stream(params, IO_WriteOnly);
// 	stream << message;
//
// 	if ( !dcopClient()->send( "kded","ksvnd","popupMessage(QString)", params ) )
// 		kdWarning() << "Communication with KDED:KSvnd failed" << endl;
}

void kio_svnProtocol::blame( KURL url, UrlMode /*mode*/,/* int pegRev, QString pegRevKind,*/ int startRev, QString startRevKind, int endRev, QString endRevKind )
{
	kdDebug(9036) << " __TIME__ " << __TIME__ << endl;
// 	kdDebug(9036) << " PegRev " << pegRev << pegRevKind << endl;
	kdDebug(9036) << " StartRev" << startRev << startRevKind << endl;
	kdDebug(9036) << " EndRev" << endRev << endRevKind << endl;

	apr_pool_t *subpool = svn_pool_create (pool);
	const char* path_or_url = apr_pstrdup( subpool, url.pathOrURL().utf8() );;

	svn_opt_revision_t rev1 = createRevision( startRev, startRevKind, subpool );
	svn_opt_revision_t rev2 = createRevision( endRev, endRevKind, subpool );
// 	svn_opt_revision_t revPeg = createRevision( pegRev, pegRevKind, subpool );

    //initNotifier(false, false, false, subpool);
	svn_client_blame_receiver_t receiver = kio_svnProtocol::blameReceiver;
	svn_error_t *err = svn_client_blame( path_or_url, &rev1, &rev2, receiver, (void*)this, ctx, subpool );
	if ( err )
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit(err->message) );
	finished();
	svn_pool_destroy (subpool);

}

svn_error_t* kio_svnProtocol::blameReceiver( void *baton, apr_int64_t line_no, svn_revnum_t rev, const char *author, const char *date, const char *line, apr_pool_t *pool )
{
	kio_svnProtocol *p = (kio_svnProtocol*)baton;
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "LINE", QString::number(line_no) );
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "REV", QString::number(rev) );
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "AUTHOR", author );
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "DATE", date );
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "CONTENT", QString::fromLocal8Bit(line) );

	p->incCounter();
	return SVN_NO_ERROR;
}

/**
	KDevelop has no way to retrieve URL of working copy.
	Thus retreiving URL from WC should be done here, using subversion native API.
	Thus, svn_log should get another flag (bool repositHistory )specifying between file:/// or URL
*/
void kio_svnProtocol::svn_log( int revstart, const QString& revkindstart, int revend, const QString& revkindend,
							bool discorverChangedPaths, bool strictNodeHistory,
							const KURL::List& urls )
{
// 	kdDebug(9036) << " from revision " << revstart << " or " << revkindstart << " to " << " revision " << revend << " or " << revkindend << endl;
	kdDebug(9036) << " __TIME__ " << __TIME__ << endl;

	apr_pool_t *subpool = svn_pool_create (pool);

	// TODO HEAD:1 was removed from SVN API 1.2, instead callers should specify HEAD:0
	svn_opt_revision_t rev1 = createRevision( revstart, revkindstart, subpool );
	svn_opt_revision_t rev2 = createRevision( revend, revkindend, subpool );

	m_counter = 0;
	apr_array_header_t *targets = apr_array_make(subpool, 1+urls.count(), sizeof(const char *));

	for ( QValueListConstIterator<KURL> it = urls.begin(); it != urls.end() ; ++it ) {
		KURL nurl = *it;
		const char *path =
				apr_pstrdup( subpool, svn_path_canonicalize( nurl.pathOrURL().utf8(), subpool ) );
		kdDebug(9036) << path << endl;
		*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) = path;

		setMetaData(QString::number( counter() ).rightJustify( 10,'0' )+ "requrl", nurl.pathOrURL() );
		incCounter();
	}

	svn_log_message_receiver_t receiver = kio_svnProtocol::receiveLogMessage;
	svn_error_t *err = svn_client_log2(targets, &rev1, &rev2, 0, discorverChangedPaths, strictNodeHistory, receiver, this, ctx, subpool);
	if ( err ){
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit(err->message) );
		svn_pool_destroy (subpool);
        return;
	}

	finished();
	svn_pool_destroy (subpool);
}

// save for one revision
svn_error_t* kio_svnProtocol::receiveLogMessage(void *baton, apr_hash_t *changed_paths, svn_revnum_t revision,
        const char *author, const char *date, const char *message, apr_pool_t *pool )
{
	kio_svnProtocol *p = (kio_svnProtocol*)baton;
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "rev", QString::number(revision) );
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "author", author );
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "date", date );
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "logmsg", QString::fromLocal8Bit(message) );
	if( changed_paths != NULL ){
		QString pathlist;
		void *onePath;
		const char *pathkey;
		apr_hash_index_t *hi;
		for (hi = apr_hash_first(pool, changed_paths); hi; hi = apr_hash_next(hi)) {
			apr_hash_this(hi, (const void**) &pathkey, NULL, &onePath);
			svn_log_changed_path_t *cp = (svn_log_changed_path_t*)onePath;
			kdDebug(9036) << "OnePath: " << cp->copyfrom_path << " and key: " << pathkey << endl;
			pathlist += cp->action;
			pathlist += " ";
// 			pathlist += cp->copyfrom_path;
			pathlist += pathkey;
			pathlist += "\n";
		}
		kdDebug(9036) << "pathlist: " << pathlist <<endl;
		p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "pathlist", pathlist );
	}
	p->incCounter();
	return SVN_NO_ERROR;
}

svn_opt_revision_t kio_svnProtocol::createRevision( int revision, const QString& revkind, apr_pool_t *pool ) {
	svn_opt_revision_t result;//,endrev;
	// TODO support svn_opt_revision_date
	if ( revision != -1 ) {
		result.value.number = revision;
		result.kind = svn_opt_revision_number;
	} else if ( revkind == "WORKING" ) {
		result.kind = svn_opt_revision_working;
	} else if ( revkind == "BASE" ) {
		result.kind = svn_opt_revision_base;
	} else if ( revkind == "HEAD" ) {
		result.kind = svn_opt_revision_head;
	} else if ( revkind == "COMMITTED" ) {
		result.kind = svn_opt_revision_committed;
	} else if ( revkind == "PREV" ) {
		result.kind = svn_opt_revision_previous;
	}
// 	} else if ( !revkind.isNull() ) {
// 		svn_opt_parse_revision(&result,&endrev,revkind.utf8(),pool);
    else if ( revkind == "UNSPECIFIED" ){
        result.kind = svn_opt_revision_unspecified;
    }
	else {
		result.kind = svn_opt_revision_unspecified;
	}
	return result;
}

void kio_svnProtocol::svn_diff(const KURL & url1, const KURL& url2,int rev1, int rev2,const QString& revkind1,const QString& revkind2,bool recurse, bool pegdiff )
{
	kdDebug(9036) << "kio_svn::diff : " << url1.path() << " at revision " << rev1 << " or " << revkind1 << " with "
		<< url2.path() << " at revision " << rev2 << " or " << revkind2
		<< endl ;

	apr_pool_t *subpool = svn_pool_create (pool);
	apr_array_header_t *options = svn_cstring_split( "", "\t\r\n", TRUE, subpool );

// 	KURL nurl1 = url1;
// 	KURL nurl2 = url2;
// 	nurl1.setProtocol( chooseProtocol( url1.protocol() ) ); //svn+https -> https for eg
// 	nurl2.setProtocol( chooseProtocol( url2.protocol() ) );
// 	recordCurrentURL( nurl1 );
// 	QString source = makeSvnURL( nurl1 );
// 	QString target = makeSvnURL( nurl2 );

// 	const char *path1 = svn_path_canonicalize( apr_pstrdup( subpool, source.utf8() ), subpool );
// 	const char *path2 = svn_path_canonicalize( apr_pstrdup( subpool, target.utf8() ), subpool );

	//remove file:/// so we can diff for working copies, needs a better check (so we support URL for file:/// _repositories_ )
// 	if ( nurl1.protocol() == "file" ) {
// 		path1 = svn_path_canonicalize( apr_pstrdup( subpool, nurl1.path().utf8() ), subpool );
// 	}
// 	if ( nurl2.protocol() == "file" ) {
// 		path2 = svn_path_canonicalize( apr_pstrdup( subpool, nurl2.path().utf8() ), subpool );
// 	}
	
	// all the commentted codes above are redundancy. url1/url2 is only file:// , svn:// or https://
	// svn+https etc. are not handed out here.
	const char *path1 = apr_pstrdup( subpool, url1.pathOrURL().utf8() );
	const char *path2 = apr_pstrdup( subpool, url2.pathOrURL().utf8() );;
	
	kdDebug( 9036 ) << "1 : " << path1 << " 2: " << path2 << endl;

	svn_opt_revision_t revision1,revision2;
	revision1 = createRevision(rev1, revkind1, subpool);
	revision2 = createRevision(rev2, revkind2, subpool);

	char *templ;
    templ = apr_pstrdup ( subpool, "/tmp/tmpfile_XXXXXX" );
	apr_file_t *outfile = NULL;
	apr_file_mktemp( &outfile, templ , APR_READ|APR_WRITE|APR_CREATE|APR_TRUNCATE, subpool );

	initNotifier(false, false, false, subpool);
	svn_error_t *err = 0;
	if( pegdiff ){
		svn_opt_revision_t peg_rev = createRevision(-1, "BASE", subpool );
		err = svn_client_diff_peg( options, path1, &peg_rev, &revision1, &revision2,
								recurse, false, false, outfile, NULL, ctx, subpool );
	} else{
		err = svn_client_diff( options, path1, &revision1, path2, &revision2, recurse,
							false, false, outfile, NULL, ctx, subpool );
	}

	if ( err ){
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit(err->message) );
		svn_pool_destroy (subpool);
		return;
	}
	//read the content of the outfile now
	QStringList tmp;
	apr_file_close(outfile);
	QFile file(templ);
	if ( file.open(  IO_ReadOnly ) ) {
		QTextStream stream(  &file );
		QString line;
		while ( !stream.atEnd() ) {
			line = stream.readLine();
			tmp << line;
		}
		file.close();
	}
	for ( QStringList::Iterator itt = tmp.begin(); itt != tmp.end(); itt++ ) {
		setMetaData(QString::number( m_counter ).rightJustify( 10,'0' )+ "diffresult", ( *itt ) );
		m_counter++;
	}
	//delete temp file
	file.remove();

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::svn_switch( const KURL& wc, const KURL& repos, int revnumber, const QString& revkind, bool recurse) {
	kdDebug(9036) << "kio_svn::switch : " << wc.path() << " at revision " << revnumber << " or " << revkind << endl ;

	apr_pool_t *subpool = svn_pool_create (pool);

	KURL nurl = repos;
	KURL dest = wc;
	nurl.setProtocol( chooseProtocol( repos.protocol() ) );
	dest.setProtocol( "file" );
// 	recordCurrentURL( nurl );
// 	QString source = dest.path();
// 	QString target = makeSvnURL( repos );

	const char *path = svn_path_canonicalize( apr_pstrdup( subpool, dest.path().utf8() ), subpool );
	const char *url = svn_path_canonicalize( apr_pstrdup( subpool, nurl.url().utf8() ), subpool );
	kdDebug(9036) << " WC path: " << path << " Repository URL: " << url << endl;

	svn_opt_revision_t rev = createRevision( revnumber, revkind, subpool );

	initNotifier(false, false, false, subpool);
	svn_error_t *err = svn_client_switch (NULL/*result revision*/, path, url, &rev, recurse, ctx, subpool);
	if ( err ){
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit( err->message ) );
		svn_pool_destroy (subpool);
        return;
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::svn_switch_relocate( const KURL &wc, const KURL &origUrl, const KURL &newUrl,
										   bool recurse )
{
	apr_pool_t *subpool = svn_pool_create( pool );

	const char *wcPath = svn_path_canonicalize( apr_pstrdup( subpool, wc.path().utf8() ), subpool );
	const char *fromUrl = apr_pstrdup( subpool, origUrl.url().utf8() );
	const char *toUrl = apr_pstrdup( subpool, newUrl.url().utf8() );
	kdDebug(9036) << " WC path: " << wcPath << " from: " << fromUrl << " to: " << toUrl << endl;

	svn_error_t *err = svn_client_relocate( wcPath, fromUrl, toUrl, recurse, ctx, pool );
	
	if ( err ){
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit( err->message ) );
		svn_pool_destroy (subpool);
        return;
	}
	m_counter = 0L;
	setMetaData(QString::number( counter() ).rightJustify( 10,'0' )+ "string",
				QString("switched to %1").arg( toUrl ) );
	finished();
	svn_pool_destroy( subpool );
}

void kio_svnProtocol::update( const KURL::List &list, int revnumber, const QString& revkind ) {
	kdDebug(9036) << "kio_svn::update : __TIME__" << __TIME__ << endl;

	apr_pool_t *subpool = svn_pool_create (pool);

	apr_array_header_t *targets = apr_array_make(subpool, 1+list.count(), sizeof(const char *));
	svn_opt_revision_t rev = createRevision( revnumber, revkind, subpool );

	for( QValueList<KURL>::ConstIterator it = list.begin(); it != list.end(); ++it ){
		KURL nurl = *it;
		*( const char ** )apr_array_push(targets) = svn_path_canonicalize( nurl.path().utf8(), subpool );
	}

	initNotifier(false, false, false, subpool);
	svn_error_t *err = svn_client_update2( NULL, targets, &rev,
										   true/*recurse*/, false/*ignore_external*/,
										   ctx, subpool);
	if ( err ){
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit(err->message) );
		svn_pool_destroy (subpool);
		return;
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::import( const KURL& repos, const KURL& wc ) {
	kdDebug(9036) << "kio_svnProtocol::import() : " << wc.url() << " into " << repos.url() << endl;

	apr_pool_t *subpool = svn_pool_create (pool);
// 	svn_client_commit_info_t *commit_info =
// 			(svn_client_commit_info_t*) apr_palloc( subpool, sizeof(svn_client_commit_info_t) );
	svn_commit_info_t *commit_info = svn_create_commit_info( subpool );
	bool nonrecursive = false;

	const char *path = apr_pstrdup( subpool, svn_path_canonicalize( wc.path().utf8(), subpool ) );
	const char *url = apr_pstrdup( subpool, svn_path_canonicalize( repos.url().utf8(), subpool ) );

    initNotifier(false, false, false, subpool);
	kdDebug(9036) << " Executing import: " << path << " to " << url << endl;
	
	svn_error_t *err = svn_client_import2(&commit_info, path, url, nonrecursive, false, ctx, subpool);
	if ( err ){
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit(err->message) );
		svn_pool_destroy (subpool);
		return;
	}

    svn_pool_destroy (subpool);
	finished();
}

void kio_svnProtocol::checkout( const KURL& repos, const KURL& wc, int revnumber, const QString& revkind ) {
	kdDebug(9036) << "kio_svn::checkout : " << repos.url() << " into " << wc.path() << " at revision " << revnumber << " or " << revkind << endl ;

	apr_pool_t *subpool = svn_pool_create (pool);
	KURL nurl = repos;
	KURL dest = wc;
	nurl.setProtocol( chooseProtocol( repos.protocol() ) );
	dest.setProtocol( "file" );
	QString target = makeSvnURL( repos );
	recordCurrentURL( nurl );
	QString dpath = dest.path();

	//find the requested revision
	svn_opt_revision_t rev = createRevision( revnumber, revkind, subpool );

	initNotifier(true, false, false, subpool);
	svn_error_t *err = svn_client_checkout (NULL/* rev actually checkedout */, svn_path_canonicalize( target.utf8(), subpool ), svn_path_canonicalize ( dpath.utf8(), subpool ), &rev, true, ctx, subpool);
	if ( err ){
		error( KIO::ERR_SLAVE_DEFINED, err->message );
		svn_pool_destroy (subpool);
        return;
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::commit(const KURL::List& wc)
{
	commit2(true, true, wc);
}

void kio_svnProtocol::commit2(bool recurse, bool keeplocks, const KURL::List& wc) {
	kdDebug(9036) << "kio_svnProtocol::commit2() : " << wc << endl;
	apr_pool_t *subpool = svn_pool_create (pool);
	svn_client_commit_info_t *commit_info = NULL;

	apr_array_header_t *targets = apr_array_make(subpool, 1+wc.count(), sizeof(const char *));

	for ( QValueListConstIterator<KURL> it = wc.begin(); it != wc.end() ; ++it ) {
		KURL nurl = *it;
		nurl.setProtocol( "file" );
		recordCurrentURL( nurl );
		(*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) = svn_path_canonicalize( nurl.path().utf8(), subpool );
	}

	initNotifier(false, false, false, subpool);
	kdDebug(9036) << "recurse: " << recurse << " keeplocks: " << keeplocks <<endl;
	svn_error_t *err = svn_client_commit2(&commit_info,targets,recurse,keeplocks,ctx,subpool);

	if ( err ){
		char errbuf[512];
		svn_strerror(err->apr_err, errbuf, 512);
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit(err->message) + "\n: " +  QString::fromLocal8Bit(errbuf) );
		svn_pool_destroy (subpool);
		return;
	}

	if ( commit_info ) {
		for ( QValueListConstIterator<KURL> it = wc.begin(); it != wc.end() ; ++it ) {
			KURL nurl = *it;
			nurl.setProtocol( "file" );

			QString userstring = i18n ( "Nothing to commit." );
			if ( SVN_IS_VALID_REVNUM( commit_info->revision ) )
				userstring = i18n( "Committed revision %1." ).arg(commit_info->revision);
			setMetaData(QString::number( m_counter ).rightJustify( 10,'0' )+ "path", nurl.path() );
			setMetaData(QString::number( m_counter ).rightJustify( 10,'0' )+ "action", "0" );
			setMetaData(QString::number( m_counter ).rightJustify( 10,'0' )+ "kind", "0" );
			setMetaData(QString::number( m_counter ).rightJustify( 10,'0' )+ "mime_t", "" );
			setMetaData(QString::number( m_counter ).rightJustify( 10,'0' )+ "content", "0" );
			setMetaData(QString::number( m_counter ).rightJustify( 10,'0' )+ "prop", "0" );
			setMetaData(QString::number( m_counter ).rightJustify( 10,'0' )+ "rev" , QString::number( commit_info->revision ) );
			setMetaData(QString::number( m_counter ).rightJustify( 10,'0' )+ "string", userstring );
			m_counter++;
		}
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::add(const KURL::List& list) {
	kdDebug(9036) << "kio_svnProtocol::add() __TIME__" << __TIME__ << endl;

	apr_pool_t *subpool = svn_pool_create (pool);
	bool nonrecursive = false;
	initNotifier(false, false, false, subpool);

	svn_error_t *err = NULL;
	for( QValueList<KURL>::ConstIterator it = list.begin(); it != list.end(); ++it ){
		
		KURL nurl = (*it);
		nurl.setProtocol( "file" );
		recordCurrentURL( nurl );
		kdDebug(9036) << " Schedule to Add: " << nurl.path().utf8() << endl;
		err = svn_client_add( svn_path_canonicalize( nurl.path().utf8(), subpool ),
							  nonrecursive, ctx, subpool);
		if( err ) break;
	}
	if ( err ){
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit(err->message) );
		svn_pool_destroy (subpool);
		return;
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::wc_delete(const KURL::List& wc) {
	kdDebug(9036) << "kio_svnProtocol::wc_delete() : " << wc << endl;

	apr_pool_t *subpool = svn_pool_create (pool);
	svn_client_commit_info_t *commit_info = NULL;
	bool force = false;
	
	apr_array_header_t *targets = apr_array_make(subpool, 1+wc.count(), sizeof(const char *));

	for ( QValueListConstIterator<KURL> it = wc.begin(); it != wc.end() ; ++it ) {
		KURL nurl = *it;
		nurl.setProtocol( "file" );
		recordCurrentURL( nurl );
		(*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) = svn_path_canonicalize( nurl.path().utf8(), subpool );
	}

	initNotifier(false, false, false, subpool);
	svn_error_t *err = svn_client_delete(&commit_info,targets,force,ctx,subpool);
	
	if ( err )
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit(err->message) );

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::wc_revert(const KURL::List& wc) {
	kdDebug(9036) << "kio_svnProtocol::revert() : " << wc << endl;

	apr_pool_t *subpool = svn_pool_create (pool);
	bool nonrecursive = false;

	apr_array_header_t *targets = apr_array_make(subpool, 1 + wc.count(), sizeof(const char *));

	for ( QValueListConstIterator<KURL> it = wc.begin(); it != wc.end() ; ++it ) {
		KURL nurl = *it;
		nurl.setProtocol( "file" );
		recordCurrentURL( nurl );
		(*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) = svn_path_canonicalize( nurl.path().utf8(), subpool );
	}

	initNotifier(false, false, false, subpool);
	svn_error_t *err = svn_client_revert(targets,nonrecursive,ctx,subpool);
	if ( err ){
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit( err->message ) );
		svn_pool_destroy (subpool);
		return;
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::wc_status(const KURL& wc, bool checkRepos, bool fullRecurse, bool getAll, int revnumber, const QString& revkind) {
	kdDebug(9036) << "kio_svnProtocol::wc_status() : " << wc.url() << " checkRepos " << checkRepos << " fullRecurse " << fullRecurse << " getAll " << getAll << endl;

	wc_status2( wc, checkRepos, fullRecurse, getAll, false, revnumber, revkind );
}

void kio_svnProtocol::wc_status2(const KURL& wc, bool checkRepos, bool fullRecurse, bool getAll, bool noIgnore, int revnumber, const QString& revkind) {
	kdDebug(9036) << "kio_svnProtocol::wc_status2() : " << wc.url() << " checkRepos " << checkRepos << " fullRecurse " << fullRecurse << " getAll " << getAll << " noIgnore " << noIgnore << " revnumber " << revnumber << " revkind " << revkind << endl;
	kdDebug(9036) << " __TIME__ " << __TIME__ << endl;

	apr_pool_t *subpool = svn_pool_create (pool);
	svn_revnum_t result_rev;

	KURL nurl = wc;
	nurl.setProtocol( "file" );
	recordCurrentURL( nurl );

	svn_opt_revision_t rev = createRevision( revnumber, revkind, subpool );

	initNotifier(false, false, false, subpool);

	svn_error_t *err = svn_client_status(&result_rev, svn_path_canonicalize( nurl.path().utf8(), subpool ), &rev, kio_svnProtocol::status, this, fullRecurse, getAll, checkRepos, noIgnore, ctx, subpool);

	if ( err ){
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit(err->message) );
		svn_pool_destroy (subpool);
        return;
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::svn_info( KURL pathOrUrl, int pegRev, QString pegRevKind, int rev, QString revKind, bool recurse )
{
    kdDebug(9036) << " kio_svnProtocol::svn_info(): pegRev " << pegRev << " pegKind " << pegRevKind << " rev " << rev << " revKind " << revKind << " recurse " << recurse << endl;

    apr_pool_t *subpool = svn_pool_create (pool);
    svn_opt_revision_t peg_rev = createRevision( pegRev, pegRevKind, subpool );
    svn_opt_revision_t revision = createRevision( rev, revKind, subpool );

    svn_error_t *err = svn_client_info( pathOrUrl.pathOrURL().utf8(),
                                        &peg_rev, &revision,
                                        kio_svnProtocol::infoReceiver,
                                        this,
                                        recurse,
                                        ctx, pool );

    if ( err ){
        error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit(err->message) );
		svn_pool_destroy (subpool);
        return;
    }
    svn_pool_destroy( subpool );
    finished();
}

svn_error_t* kio_svnProtocol::infoReceiver( void *baton, const char *path,
                                       const svn_info_t *info, apr_pool_t *pool)
{
    kio_svnProtocol *p= (kio_svnProtocol*)baton ;
    if( !p )
        return SVN_NO_ERROR;
    
    p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "PATH", QString::fromUtf8( path ));
    p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "URL", QString( info->URL ) );
    p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "REV", QString::number( info->rev ));
    p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "KIND", QString::number( info->kind ));
    p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "REPOS_ROOT_URL", QString( info->repos_root_URL ) );
    p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "REPOS_UUID", QString(info->repos_UUID) );
    p->incCounter();

    return SVN_NO_ERROR;
}

void kio_svnProtocol::svn_copy( const KURL &srcUrl, int srcRev, const QString &srcRevKind,
								const KURL &destUrl )
{
	kdDebug(9036) << " kio: svn_copy src: " << srcUrl << " Dest Url: " << destUrl << " revnum: " << srcRev << " revKind: " << srcRevKind << endl;
	apr_pool_t *subpool = svn_pool_create (pool);
	svn_commit_info_t *commit_info = svn_create_commit_info( subpool );
	
	svn_opt_revision_t rev = createRevision( srcRev, srcRevKind, subpool );

	// TODO more elegant notification mechanism
	initNotifier(false, false, false, subpool);
	svn_error_t *err = svn_client_copy2( &commit_info,
										 srcUrl.pathOrURL().utf8(), &rev,
										 destUrl.pathOrURL().utf8(),
										 ctx, subpool);
	
	if ( err ) {
		apr_status_t errcode = err->apr_err;
		char buf[512];
		svn_strerror(errcode, buf, 512);
		error( KIO::ERR_SLAVE_DEFINED, QString::fromLocal8Bit(buf) );
		svn_pool_destroy (subpool);
		return;
	}
	
	if( commit_info ){
		setMetaData(QString::number( counter() ).rightJustify( 10,'0' )+ "string",
					i18n("Copied Revision %1").arg( commit_info->revision) );
	} else {
		setMetaData(QString::number( counter() ).rightJustify( 10,'0' )+ "string",
					i18n("Copied") );
	}
	
	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::svn_merge(const KURL &src1, int revNum1, QString revKind1,
								const KURL &src2, int revNum2, QString revKind2,
								const KURL &target_wc,
								bool recurse, bool ignore_ancestry, bool force, bool dry_run )
{
	kdDebug(9036) << " KIO::svn_merge src1 " << src1.pathOrURL().utf8() << " src2 " << src2.pathOrURL().utf8() << " target " << target_wc.pathOrURL().utf8() << endl;
	apr_pool_t *subpool = svn_pool_create( pool );

	svn_opt_revision_t rev1 = createRevision( revNum1, revKind1, subpool );
	svn_opt_revision_t rev2 = createRevision( revNum2, revKind2, subpool );

	initNotifier( false, false, false, subpool );
	svn_error_t *err = svn_client_merge( src1.pathOrURL().utf8(), &rev1,
										 src2.pathOrURL().utf8(), &rev2,
										 target_wc.pathOrURL().utf8(),
										 recurse, ignore_ancestry, force, dry_run,
									     ctx, pool );
	if ( err ) {
		apr_status_t errcode = err->apr_err;
		char buf[512];
		svn_strerror(errcode, buf, 512);
		error( KIO::ERR_SLAVE_DEFINED,
			   QString::fromLocal8Bit(err->message) + "\n "+ QString::fromLocal8Bit(buf) );
		svn_pool_destroy (subpool);
		return;
	}
	
	finished();
	svn_pool_destroy( subpool );
}

//change the proto and remove trailing /
//remove double / also
QString kio_svnProtocol::makeSvnURL ( const KURL& url ) const {
	QString kproto = url.protocol();
	KURL tpURL = url;
	tpURL.cleanPath( true );
	QString svnUrl;
	if ( kproto == "kdevsvn+http" ) {
		kdDebug(9036) << "http:/ " << url.url() << endl;
		tpURL.setProtocol("http");
		svnUrl = tpURL.url(-1);
		return svnUrl;
	}
	else if ( kproto == "kdevsvn+https" ) {
		kdDebug(9036) << "https:/ " << url.url() << endl;
		tpURL.setProtocol("https");
		svnUrl = tpURL.url(-1);
		return svnUrl;
	}
	else if ( kproto == "kdevsvn+ssh" ) {
		kdDebug(9036) << "svn+ssh:/ " << url.url() << endl;
		tpURL.setProtocol("svn+ssh");
		svnUrl = tpURL.url(-1);
		return svnUrl;
	}
	else if ( kproto == "kdevsvn+svn" ) {
		kdDebug(9036) << "svn:/ " << url.url() << endl;
		tpURL.setProtocol("svn");
		svnUrl = tpURL.url(-1);
		return svnUrl;
	}
	else if ( kproto == "kdevsvn+file" ) {
		kdDebug(9036) << "file:/ " << url.url() << endl;
		tpURL.setProtocol("file");
		svnUrl = tpURL.url(-1);
		//hack : add one more / after file:/
		int idx = svnUrl.find("/");
		svnUrl.insert( idx, "//" );
		return svnUrl;
	}
	return tpURL.url(-1);
}

QString kio_svnProtocol::chooseProtocol ( const QString& kproto ) const {
	if ( kproto == "svn+http" ) return QString( "http" );
	else if ( kproto == "svn+https" ) return QString( "https" );
	else if ( kproto == "svn+ssh" ) return QString( "svn+ssh" );
	else if ( kproto == "svn" ) return QString( "svn" );
	else if ( kproto == "svn+file" ) return QString( "file" );
	return kproto;
}
/** Certificate is not yet valid. */
#define SVN_AUTH_SSL_NOTYETVALID 0x00000001
/** Certificate has expired. */
#define SVN_AUTH_SSL_EXPIRED     0x00000002
/** Certificate's CN (hostname) does not match the remote hostname. */
#define SVN_AUTH_SSL_CNMISMATCH  0x00000004
/** @brief Certificate authority is unknown (i.e. not trusted) */
#define SVN_AUTH_SSL_UNKNOWNCA   0x00000008
/** @brief Other failure. This can happen if neon has introduced a new
 * failure bit that we do not handle yet. */
#define SVN_AUTH_SSL_OTHER       0x40000000
svn_error_t *kio_svnProtocol::trustSSLPrompt(svn_auth_cred_ssl_server_trust_t **cred_p, void *baton, const char *realm, apr_uint32_t failures, const svn_auth_ssl_server_cert_info_t *ci, svn_boolean_t may_save, apr_pool_t *pool)
{
	kio_svnProtocol *p = (kio_svnProtocol*)baton;
	// prepare params.
	QByteArray params, replyData;
	QCString replyType;
	QDataStream arg(params, IO_WriteOnly);

	arg << i18n( "This certificate from server could not be trusted automatically. Will you trust this certificate? " );
	arg << QString::fromLocal8Bit(ci->hostname);
	arg << QString::fromLocal8Bit(ci->fingerprint);
	arg << QString::fromLocal8Bit(ci->valid_from) << QString::fromLocal8Bit(ci->valid_until);
	arg << QString::fromLocal8Bit(ci->issuer_dname) << QString::fromLocal8Bit(ci->ascii_cert) ;
	// call dcop
	int ret = p->dcopClient()->call( "kded", "kdevsvnd",
			"sslServerTrustPrompt(QString, QString, QString, QString, QString, QString, QString)",
			params, replyType, replyData );
	if (!ret){
		kdWarning() << " failed to prompt SSL_Server_Trust_Prompt " << endl;
 		return SVN_NO_ERROR;
	}
	if (replyType != "int"){
		kdWarning() << " abnormal reply type " << endl;
		return SVN_NO_ERROR;
	}
	int resultCode;
	QDataStream replyStream( replyData, IO_ReadOnly );
	replyStream >> resultCode;

	if( resultCode == -1 ){
		kdWarning() << " SSL server trust rejected " << endl;
		*cred_p = 0L; //FIXME when rejected, maybe more elegant methods..
	} else if( resultCode == 0 ){ //accept once
		*cred_p = (svn_auth_cred_ssl_server_trust_t*)apr_pcalloc (pool, sizeof (svn_auth_cred_ssl_server_trust_t));
		kdDebug(9036) << " accept once " << endl;
		(*cred_p)->may_save = false;
		(*cred_p)->accepted_failures = 0;
	} else if( resultCode == 1 ){ //accept permanently
		*cred_p = (svn_auth_cred_ssl_server_trust_t*)apr_pcalloc (pool, sizeof (svn_auth_cred_ssl_server_trust_t));
		kdDebug(9036) << " accept permanently " << endl;
		(*cred_p)->may_save = true;
		(*cred_p)->accepted_failures = failures;
	} else{
		kdWarning() << " SSL server trust failed for some reason" << endl;
		*cred_p = 0L;
	}

	return SVN_NO_ERROR;
}
/** TODO fully implemented, but there is no way to test this yet.*/
svn_error_t *kio_svnProtocol::clientCertSSLPrompt(
		svn_auth_cred_ssl_client_cert_t **cred_p, void *baton, const char *realm, svn_boolean_t may_save, apr_pool_t *pool)
{
	kdDebug(9036) << " clientCertSSLPrompt " << endl;
// 	kio_svnProtocol *p = (kio_svnProtocol*)baton;
// 	QByteArray reply;
// 	QByteArray params;
// 	QCString replyType;
// 	call dcop
// 	if (!p->dcopClient()->call("kded","kdevsvnd", "sslCertFile()",params,replyType,reply)) {
// 		kdWarning()<<" Communication with dcop failed - fail to get certfile "<<endl;
// 		return SVN_NO_ERROR;
// 	}
// 	if (replyType != "QString") {
// 		kdWarning()<<" unexpected reply type "<<endl;
// 		return SVN_NO_ERROR;
// 	}
// 	save reply data
// 	QString fileName;
// 	QDataStream replyStream( reply, IO_ReadOnly );
// 	replyStream >> fileName;
// 	allocate memory
// 	*cred_p = (svn_auth_cred_ssl_client_cert_t*) apr_palloc (pool, sizeof(svn_auth_cred_ssl_client_cert_t));
// 	(*cred_p)->cert_file = apr_pstrdup( pool, fileName.utf8() );
// 	(*cred_p)->may_save = may_save;
	return SVN_NO_ERROR;
}

/** TODO fully implemented, but there is no way to test this yet.*/
svn_error_t *kio_svnProtocol::clientCertPasswdPrompt(
		svn_auth_cred_ssl_client_cert_pw_t **cred_p, void *baton, const char *realm, svn_boolean_t may_save, apr_pool_t *pool)
{
	kdDebug(9036) << " Password Prompt Callback " << endl;
	kdDebug(9036) << " realm " << realm << " <--realm " << endl;
// 	kio_svnProtocol *p = ( kio_svnProtocol* )baton;
// 	// prepare dcop
// 	QByteArray reply;
// 	QByteArray params;
// 	QCString replyType;
// 	QDataStream arg( params, IO_WriteOnly );
// 	arg << i18n( "Enter password for subversion repository access" ) + "\n" + QString(realm);
// 	// call dcop
// 	if (!p->dcopClient()->call("kded","kdevsvnd", "sslPasswdDlg(QString)",params,replyType,reply)) {
// 		kdWarning()<<" Communication with dcop failed - fail to show passwd dlg"<<endl;
// 		return SVN_NO_ERROR;
// 	}
// 	if (replyType != "QCString") {
// 		kdWarning()<<" unexpected reply type "<<endl;
// 		return SVN_NO_ERROR;
// 	}
// 	// save reply data
// 	QCString retstr, passwd;
// 	QDataStream replyStream( reply, IO_ReadOnly );
// 	replyStream >> retstr;
//
// 	if( retstr.left(1) == "-1" ){
// 		kdDebug(9036) << " Null string received for passwd " << endl;
// 	} else{
// 		passwd = retstr.right( retstr.length() - 1 );
// 		kdDebug(9036) << " PassWD : " << passwd << endl;
// 	}
//
// 	svn_auth_cred_ssl_client_cert_pw_t *newcred = (svn_auth_cred_ssl_client_cert_pw_t*) apr_palloc (pool, sizeof (svn_auth_cred_ssl_client_cert_pw_t ) );
//
// 	newcred->password = apr_pstrdup(pool, (const char*) passwd );
// 	newcred->may_save = false;
// 	*cred_p = newcred;

	return SVN_NO_ERROR;
}


svn_error_t *kio_svnProtocol::commitLogPrompt( const char **log_msg, const char **file,
                    apr_array_header_t *commit_items, void *baton, apr_pool_t *pool )
{
	*file = NULL; // if omitting this, it will segfault at import operation.
	QCString replyType;
	QByteArray params;
	QByteArray reply;
	QString result;// slist;
	QStringList slist;
	kio_svnProtocol *p = ( kio_svnProtocol* )baton;
	svn_stringbuf_t *message = NULL;

	for (int i = 0; i < commit_items->nelts; i++) {
		QString list;
		svn_client_commit_item_t *item = ((svn_client_commit_item_t **) commit_items->elts)[i];
		const char *path = item->path;
		char text_mod = '_', prop_mod = ' ';

		if (! path)
			path = item->url;
		else if (! *path)
			path = ".";

		if (! path)
			path = ".";

		if ((item->state_flags & SVN_CLIENT_COMMIT_ITEM_DELETE) && (item->state_flags & SVN_CLIENT_COMMIT_ITEM_ADD))
			text_mod = 'R';
		else if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_ADD)
			text_mod = 'A';
		else if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_DELETE)
			text_mod = 'D';
		else if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_TEXT_MODS)
			text_mod = 'M';
		if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_PROP_MODS)
			prop_mod = 'M';

		list += text_mod;
		list += " ";
		list += prop_mod;
		list += "  ";
		list += path;
		kdDebug(9036) << " Commiting items : " << list << endl;
		slist << list;
// 		slist += list;
	}

	QDataStream stream(params, IO_WriteOnly);
	stream << slist.join("\n");

	kdDebug(9036) << " __TIME__ " << __TIME__ << endl;
	if ( !p->dcopClient()->call( "kded","kdevsvnd","commitDialog(QString)", params, replyType, reply ) ) {
		kdWarning() << "Communication with KDED:KDevSvnd failed" << endl;
		svn_error_t *err = svn_error_create( SVN_ERR_EXTERNAL_PROGRAM, NULL,
											 apr_pstrdup( pool, "Fail to call kded_kdevsvnd via DCOP. If this is your first problem, try to restart KDE" ) );
		return err;
	}

	if ( replyType != "QString" ) {
		kdWarning() << "Unexpected reply type" << endl;
		svn_error_t *err = svn_error_create( SVN_ERR_EXTERNAL_PROGRAM, NULL,
											 apr_pstrdup( pool, "Fail to call kded_kdevsvnd via DCOP." ) );
		return err;
	}

	QDataStream stream2 ( reply, IO_ReadOnly );
	stream2 >> result;

	if ( result.isNull() ) { //cancelled
		*log_msg = NULL;
		svn_error_t *err = svn_error_create( SVN_ERR_CANCELLED, NULL,
											 apr_pstrdup( pool, "Commit interruppted" ) );
		return err;
	}

	message = svn_stringbuf_create( result.utf8(), pool );
	*log_msg = message->data;

	return SVN_NO_ERROR;
}

void kio_svnProtocol::notify(void *baton, const char *path, svn_wc_notify_action_t action, svn_node_kind_t kind, const char *mime_type, svn_wc_notify_state_t content_state, svn_wc_notify_state_t prop_state, svn_revnum_t revision) {
	kdDebug(9036) << "NOTIFY : " << path << " updated at revision " << revision << " action : " << action << ", kind : " << kind << " , content_state : " << content_state << ", prop_state : " << prop_state << endl;

	QString userstring;
	struct notify_baton *nb = ( struct notify_baton* ) baton;

	//// Convert notification to a user readable string
	switch ( action ) {
		case svn_wc_notify_add : //add
			if (mime_type && (svn_mime_type_is_binary (mime_type)))
				userstring = i18n( "A (bin) %1" ).arg( path );
			else
				userstring = i18n( "A %1" ).arg( path );
			break;
		case svn_wc_notify_copy: //copy
			userstring = i18n( "Copied %1 " ).arg( path );
			break;
		case svn_wc_notify_delete: //delete
			nb->received_some_change = TRUE;
			userstring = i18n( "D %1" ).arg( path );
			break;
		case svn_wc_notify_restore : //restore
			userstring=i18n( "Restored %1." ).arg( path );
			break;
		case svn_wc_notify_revert : //revert
			userstring=i18n( "Reverted %1." ).arg( path );
			break;
		case svn_wc_notify_failed_revert: //failed revert
			userstring=i18n( "Failed to revert %1.\nTry updating instead." ).arg( path );
			break;
		case svn_wc_notify_resolved: //resolved
			userstring=i18n( "Resolved conflicted state of %1." ).arg( path );
			break;
		case svn_wc_notify_skip: //skip
			if ( content_state == svn_wc_notify_state_missing )
				userstring=i18n("Skipped missing target %1.").arg( path );
			else
				userstring=i18n("Skipped  %1.").arg( path );
			break;
		case svn_wc_notify_update_delete: //update_delete
			nb->received_some_change = TRUE;
			userstring=i18n( "D %1" ).arg( path );
			break;
		case svn_wc_notify_update_add: //update_add
			nb->received_some_change = TRUE;
			userstring=i18n( "A %1" ).arg( path );
			break;
		case svn_wc_notify_update_update: //update_update
			{
				/* If this is an inoperative dir change, do no notification.
				   An inoperative dir change is when a directory gets closed
				   without any props having been changed. */
				if (! ((kind == svn_node_dir)
							&& ((prop_state == svn_wc_notify_state_inapplicable)
								|| (prop_state == svn_wc_notify_state_unknown)
								|| (prop_state == svn_wc_notify_state_unchanged)))) {
					nb->received_some_change = TRUE;

					if (kind == svn_node_file) {
						if (content_state == svn_wc_notify_state_conflicted)
							userstring = "C";
						else if (content_state == svn_wc_notify_state_merged)
							userstring = "G";
						else if (content_state == svn_wc_notify_state_changed)
							userstring = "U";
					}

					if (prop_state == svn_wc_notify_state_conflicted)
						userstring += "C";
					else if (prop_state == svn_wc_notify_state_merged)
						userstring += "G";
					else if (prop_state == svn_wc_notify_state_changed)
						userstring += "U";
					else
						userstring += " ";

					if (! ((content_state == svn_wc_notify_state_unchanged
									|| content_state == svn_wc_notify_state_unknown)
								&& (prop_state == svn_wc_notify_state_unchanged
									|| prop_state == svn_wc_notify_state_unknown)))
						userstring += QString( " " ) + path;
				}
				break;
			}
		case svn_wc_notify_update_completed: //update_completed
			{
				if (! nb->suppress_final_line) {
					if (SVN_IS_VALID_REVNUM (revision)) {
						if (nb->is_export) {
							if ( nb->in_external )
								userstring = i18n("Exported external at revision %1.").arg( revision );
							else
								userstring = i18n("Exported revision %1.").arg( revision );
						} else if (nb->is_checkout) {
							if ( nb->in_external )
								userstring = i18n("Checked out external at revision %1.").arg( revision );
							else
								userstring = i18n("Checked out revision %1.").arg( revision);
						} else {
							if (nb->received_some_change) {
								if ( nb->in_external )
									userstring=i18n("Updated external to revision %1.").arg( revision );
								else
									userstring = i18n("Updated to revision %1.").arg( revision);
							} else {
								if ( nb->in_external )
									userstring = i18n("External at revision %1.").arg( revision );
								else
									userstring = i18n("At revision %1.").arg( revision);
							}
						}
					} else  /* no revision */ {
						if (nb->is_export) {
							if ( nb->in_external )
								userstring = i18n("External export complete.");
							else
								userstring = i18n("Export complete.");
						} else if (nb->is_checkout) {
							if ( nb->in_external )
								userstring = i18n("External checkout complete.");
							else
								userstring = i18n("Checkout complete.");
						} else {
							if ( nb->in_external )
								userstring = i18n("External update complete.");
							else
								userstring = i18n("Update complete.");
						}
					}
				}
			}
			if (nb->in_external)
				nb->in_external = FALSE;
			break;
		case svn_wc_notify_update_external: //update_external
			nb->in_external = TRUE;
			userstring = i18n("Fetching external item into %1." ).arg( path );
			break;
		case svn_wc_notify_status_completed: //status_completed
			if (SVN_IS_VALID_REVNUM (revision))
				userstring = i18n( "Status against revision: %1.").arg( revision );
			break;
		case svn_wc_notify_status_external: //status_external
             userstring = i18n("Performing status on external item at %1.").arg( path );
			break;
		case svn_wc_notify_commit_modified: //commit_modified
			userstring = i18n( "Sending %1").arg( path );
			break;
		case svn_wc_notify_commit_added: //commit_added
			if (mime_type && svn_mime_type_is_binary (mime_type)) {
				userstring = i18n( "Adding (bin) %1.").arg( path );
			} else {
				userstring = i18n( "Adding %1.").arg( path );
			}
			break;
		case svn_wc_notify_commit_deleted: //commit_deleted
			userstring = i18n( "Deleting %1.").arg( path );
			break;
		case svn_wc_notify_commit_replaced: //commit_replaced
			userstring = i18n( "Replacing %1.").arg( path );
			break;
		case svn_wc_notify_commit_postfix_txdelta: //commit_postfix_txdelta
			if (! nb->sent_first_txdelta) {
				nb->sent_first_txdelta = TRUE;
				userstring=i18n("Transmitting file data ");
			} else {
				userstring=".";
			}
			break;

			break;
		case svn_wc_notify_blame_revision: //blame_revision
			userstring = i18n("Blame %1.").arg(path);
			break;
		default:
			break;
	}
	//// End convert
	kio_svnProtocol *p = ( kio_svnProtocol* )nb->master;
	if (!p) kdDebug(9036) << " Null Pointer at Line " << __LINE__ << endl;

	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "path" , QString::fromUtf8( path )); 
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "action", QString::number( action ));
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "kind", QString::number( kind ));
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "mime_t", QString::fromUtf8( mime_type ));
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "content", QString::number( content_state ));
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "prop", QString::number( prop_state ));
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "rev", QString::number( revision ));
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "string", userstring );
	kdDebug(9036) << " kio_svnProtocol::notify() userstring " << userstring << endl;
	p->incCounter();
}

void kio_svnProtocol::status(void *baton, const char *path, svn_wc_status_t *status) {
	kdDebug(9036) << "STATUS : " << path << ", wc text status : " << status->text_status
									 << ", wc prop status : " << status->prop_status
									 << ", repos text status : " << status->repos_text_status
									 << ", repos prop status : " << status->repos_prop_status
									 << endl;

	QByteArray params;
	kio_svnProtocol *p = ( kio_svnProtocol* )baton;

	QDataStream stream(params, IO_WriteOnly);
	long int rev = status->entry ? status->entry->revision : 0;
	stream << QString::fromUtf8( path ) << status->text_status << status->prop_status << status->repos_text_status << status->repos_prop_status << rev;

	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "path", QString::fromUtf8( path ));
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "text", QString::number( status->text_status ));
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "prop", QString::number( status->prop_status ));
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "reptxt", QString::number( status->repos_text_status ));
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "repprop", QString::number( status->repos_prop_status ));
	p->setMetaData(QString::number( p->counter() ).rightJustify( 10,'0' )+ "rev", QString::number( rev ));
	p->incCounter();
}

void kio_svnProtocol::progressCallback( apr_off_t processed, apr_off_t total, void *baton, apr_pool_t *pool)
{
	kio_svnProtocol *p = (kio_svnProtocol*)baton;
	if( total > -1 )
		p->totalSize( total );
	if( processed > -1 )
		p->processedSize( processed );
}

void kio_svnProtocol::wc_resolve( const KURL& wc, bool recurse ) {
	kdDebug(9036) << "kio_svnProtocol::wc_resolve() : " << wc.url() << endl;

	apr_pool_t *subpool = svn_pool_create (pool);

	KURL nurl = wc;
	nurl.setProtocol( "file" );
	recordCurrentURL( nurl );

	initNotifier(false, false, false, subpool);
	svn_error_t *err = svn_client_resolved(svn_path_canonicalize( nurl.path().utf8(), subpool ), recurse,ctx,subpool);
	if ( err )
		error( KIO::ERR_SLAVE_DEFINED, err->message );

	finished();
	svn_pool_destroy (subpool);
}

extern "C"
{
	KDE_EXPORT int kdemain(int argc, char **argv)    {
		KInstance instance( "kio_kdevsvn" );

		kdDebug(9036) << "*** Starting kio_kdevsvn " << endl;

		if (argc != 4) {
			kdDebug(9036) << "Usage: kio_kdevsvn  protocol domain-socket1 domain-socket2" << endl;
			exit(-1);
		}

		kio_svnProtocol slave(argv[2], argv[3]);
		slave.dispatchLoop();

		kdDebug(9036) << "*** kio_kdevsvn Done" << endl;
		return 0;
	}
}

