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

#include <kmimetype.h>

#include "svn.h"
#include <apr_portable.h>

using namespace KIO;

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
		kdDebug() << "kio_svnProtocol::kio_svnProtocol()" << endl;

		kdDebug() << "Loading KDED module" << endl;
		QCString module = "ksvnd";
		QCString replyType;
		QByteArray replyData;
		QByteArray params;
		QDataStream stream(params, IO_WriteOnly);
		stream << module;
		dcopClient()->call( "kded", "kded", "loadModule(QCString)", params, replyType, replyData);
		//check answer ?

		apr_initialize();
		pool = svn_pool_create (NULL);
		svn_error_t *err = svn_config_ensure (NULL,pool);
		if ( err ) {
			kdDebug() << "kio_svnProtocol::kio_svnProtocol() configensure ERROR" << endl;
			error( KIO::ERR_SLAVE_DEFINED, err->message );
			return;
		}
		svn_config_get_config (&ctx.config,NULL,pool);

		//for now but TODO
		ctx.notify_func = kio_svnProtocol::notify;
		ctx.notify_baton = this; //pass this so that we can get a dcopClient from it
		ctx.log_msg_func = kio_svnProtocol::commitLogPrompt;
		ctx.log_msg_baton = this; //pass this so that we can get a dcopClient from it
		//TODO
		ctx.cancel_func = NULL;

		apr_array_header_t *providers = apr_array_make(pool, 9, sizeof(svn_auth_provider_object_t *));

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
		svn_client_get_ssl_server_trust_prompt_provider (&provider, kio_svnProtocol::trustSSLPrompt, NULL, pool);
		APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
		svn_client_get_ssl_client_cert_prompt_provider (&provider, kio_svnProtocol::clientCertSSLPrompt, NULL, 2, pool);
		APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
		svn_client_get_ssl_client_cert_pw_prompt_provider (&provider, kio_svnProtocol::clientCertPasswdPrompt, NULL, 2, pool);
		APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

		svn_auth_open(&ctx.auth_baton, providers, pool);
}

kio_svnProtocol::~kio_svnProtocol(){
	kdDebug() << "kio_svnProtocol::~kio_svnProtocol()" << endl;
	kdDebug() << "Unloading KDED module" << endl;
	QCString module = "ksvnd";
	QCString replyType;
	QByteArray replyData;
	QByteArray params;
	QDataStream stream(params, IO_WriteOnly);
	stream << module;
	dcopClient()->call( "kded", "kded", "unloadModule(QCString)", params, replyType, replyData);

	svn_pool_destroy(pool);
	apr_terminate();
}

svn_error_t* kio_svnProtocol::checkAuth(svn_auth_cred_simple_t **/*cred*/, void *baton, const char *realm, const char *username, svn_boolean_t /*may_save*/, apr_pool_t *pool) {
	kdDebug() << "kio_svnProtocol::checkAuth() for " << realm << endl;
	kio_svnProtocol *p = ( kio_svnProtocol* )baton;
	svn_auth_cred_simple_t *ret = (svn_auth_cred_simple_t*)apr_pcalloc (pool, sizeof (*ret));
	
//XXX readd me when debug is complete		p->info.keepPassword = true;
	p->info.verifyPath=true;
	kdDebug( ) << "auth current URL : " << p->myURL.url() << endl;
	p->info.url = p->myURL;
	p->info.username = username; //( const char* )svn_auth_get_parameter( p->ctx->auth_baton, SVN_AUTH_PARAM_DEFAULT_USERNAME );
	if ( !p->checkCachedAuthentication( p->info ) ){
		p->openPassDlg( p->info );
	}
	ret->username = apr_pstrdup(pool, p->info.username.utf8());
	ret->password = apr_pstrdup(pool, p->info.password.utf8());
	return SVN_NO_ERROR;
}

void kio_svnProtocol::recordCurrentURL(const KURL& url) {
	myURL = url;
}

//don't implement mimeType() until we don't need to download the whole file

void kio_svnProtocol::get(const KURL& url ){
	kdDebug() << "kio_svn::get(const KURL& url)" << endl ;

	QString remoteServer = url.host();
	infoMessage(i18n("Looking for %1...").arg( remoteServer ) );

	apr_pool_t *subpool = svn_pool_create (pool);
	kbaton *bt = (kbaton*)apr_pcalloc(subpool, sizeof(*bt));
	bt->target_string = svn_stringbuf_create("", subpool);
	bt->string_stream = svn_stream_create(bt,subpool);
	svn_stream_set_write(bt->string_stream,write_to_string);

	QString target = makeSvnURL( url );
	kdDebug() << "SvnURL: " << target << endl;
	recordCurrentURL( KURL( target ) );
	
	//find the requested revision
	svn_opt_revision_t rev;
	svn_opt_revision_t endrev;
	int idx = target.findRev( "?rev=" );
	if ( idx != -1 ) {
		QString revstr = target.mid( idx+5 );
#if 0
		kdDebug() << "revision string found " << revstr  << endl;
		if ( revstr == "HEAD" ) {
			rev.kind = svn_opt_revision_head;
			kdDebug() << "revision searched : HEAD" << endl;
		} else {
			rev.kind = svn_opt_revision_number;
			rev.value.number = revstr.toLong();
			kdDebug() << "revision searched : " << rev.value.number << endl;
		}
#endif
		svn_opt_parse_revision( &rev, &endrev, revstr.utf8(), subpool );
		target = target.left( idx );
		kdDebug() << "new target : " << target << endl;
	} else {
		kdDebug() << "no revision given. searching HEAD " << endl;
		rev.kind = svn_opt_revision_head;
	}

	svn_error_t *err = svn_client_cat (bt->string_stream, svn_path_canonicalize( target.utf8(),subpool ),&rev,&ctx, subpool);
	if ( err ) {
		error( KIO::ERR_SLAVE_DEFINED, err->message );
		svn_pool_destroy( subpool );
		return;
	}

	// Send the mimeType as soon as it is known
	QByteArray *cp = new QByteArray();
	cp->setRawData( bt->target_string->data, bt->target_string->len );
	KMimeType::Ptr mt = KMimeType::findByContent(*cp);
	kdDebug() << "KMimeType returned : " << mt->name() << endl;
	mimeType( mt->name() );

	totalSize(bt->target_string->len);

	//send data
	data(*cp);

	data(QByteArray()); // empty array means we're done sending the data
	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::stat(const KURL & url){
	kdDebug() << "kio_svn::stat(const KURL& url) : " << url.url() << endl ;

	void *ra_baton, *session;
	svn_ra_plugin_t *ra_lib;
	svn_node_kind_t kind;
	apr_pool_t *subpool = svn_pool_create (pool);

	QString target = makeSvnURL( url);
	kdDebug() << "SvnURL: " << target << endl;
	recordCurrentURL( KURL( target ) );
	
	//find the requested revision
	svn_opt_revision_t rev;
	svn_opt_revision_t endrev;
	int idx = target.findRev( "?rev=" );
	if ( idx != -1 ) {
		QString revstr = target.mid( idx+5 );
#if 0
		kdDebug() << "revision string found " << revstr  << endl;
		if ( revstr == "HEAD" ) {
			rev.kind = svn_opt_revision_head;
			kdDebug() << "revision searched : HEAD" << endl;
		} else {
			rev.kind = svn_opt_revision_number;
			rev.value.number = revstr.toLong();
			kdDebug() << "revision searched : " << rev.value.number << endl;
		}
#endif
		svn_opt_parse_revision( &rev, &endrev, revstr.utf8( ), subpool );
		target = target.left( idx );
		kdDebug() << "new target : " << target << endl;
	} else {
		kdDebug() << "no revision given. searching HEAD " << endl;
		rev.kind = svn_opt_revision_head;
	}

	//init
	svn_error_t *err = svn_ra_init_ra_libs(&ra_baton,subpool);
	if ( err ) {
		kdDebug() << "init RA libs failed : " << err->message << endl;
		return;
	}
	//find RA libs
	err = svn_ra_get_ra_library(&ra_lib,ra_baton,svn_path_canonicalize( target.utf8(), subpool ),subpool);
	if ( err ) {
		kdDebug() << "RA get libs failed : " << err->message << endl;
		return;
	}
	kdDebug() << "RA init completed" << endl;
	
	//start session
	svn_ra_callbacks_t *cbtable = (svn_ra_callbacks_t*)apr_pcalloc(subpool, sizeof(*cbtable));	
	kio_svn_callback_baton_t *callbackbt = (kio_svn_callback_baton_t*)apr_pcalloc(subpool, sizeof( *callbackbt ));

	cbtable->open_tmp_file = open_tmp_file;
	cbtable->get_wc_prop = NULL;
	cbtable->set_wc_prop = NULL;
	cbtable->push_wc_prop = NULL;
	cbtable->auth_baton = ctx.auth_baton;

	callbackbt->base_dir = target.utf8();
	callbackbt->pool = subpool;
	callbackbt->config = ctx.config;
	
	err = ra_lib->open(&session,svn_path_canonicalize( target.utf8(), subpool ),cbtable,callbackbt,ctx.config,subpool);
	if ( err ) {
		kdDebug()<< "Open session " << err->message << endl;
		return;
	}
	kdDebug() << "Session opened to " << target << endl;
	//find number for HEAD
	if (rev.kind == svn_opt_revision_head) {
		err = ra_lib->get_latest_revnum(session,&rev.value.number,subpool);
		if ( err ) {
			kdDebug()<< "Latest RevNum " << err->message << endl;
			return;
		}
		kdDebug() << "Got rev " << rev.value.number << endl;
	}
	
	//get it
	ra_lib->check_path(session,"",rev.value.number,&kind,subpool);
	kdDebug() << "Checked Path" << endl;
	UDSEntry entry;
	switch ( kind ) {
		case svn_node_file:
			kdDebug() << "::stat result : file" << endl;
			createUDSEntry(url.filename(),"",0,false,0,entry);
			statEntry( entry );
			break;
		case svn_node_dir:
			kdDebug() << "::stat result : directory" << endl;
			createUDSEntry(url.filename(),"",0,true,0,entry);
			statEntry( entry );
			break;
		case svn_node_unknown:
		case svn_node_none:
			//error XXX
		default:
			kdDebug() << "::stat result : UNKNOWN ==> WOW :)" << endl;
			;
	}
	finished();
	svn_pool_destroy( subpool );
}

void kio_svnProtocol::listDir(const KURL& url){
	kdDebug() << "kio_svn::listDir(const KURL& url) : " << url.url() << endl ;

	apr_pool_t *subpool = svn_pool_create (pool);
	apr_hash_t *dirents;

	QString target = makeSvnURL( url);
	kdDebug() << "SvnURL: " << target << endl;
	recordCurrentURL( KURL( target ) );
	
	//find the requested revision
	svn_opt_revision_t rev;
	svn_opt_revision_t endrev;
	int idx = target.findRev( "?rev=" );
	if ( idx != -1 ) {
		QString revstr = target.mid( idx+5 );
		svn_opt_parse_revision( &rev, &endrev, revstr.utf8(), subpool );
#if 0
		kdDebug() << "revision string found " << revstr  << endl;
		if ( revstr == "HEAD" ) {
			rev.kind = svn_opt_revision_head;
			kdDebug() << "revision searched : HEAD" << endl;
		} else {
			rev.kind = svn_opt_revision_number;
			rev.value.number = revstr.toLong();
			kdDebug() << "revision searched : " << rev.value.number << endl;
		}
#endif
		target = target.left( idx );
		kdDebug() << "new target : " << target << endl;
	} else {
		kdDebug() << "no revision given. searching HEAD " << endl;
		rev.kind = svn_opt_revision_head;
	}

	svn_error_t *err = svn_client_ls (&dirents, svn_path_canonicalize( target.utf8(), subpool ), &rev, false, &ctx, subpool);
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

bool kio_svnProtocol::createUDSEntry( const QString& filename, const QString& user, long int size, bool isdir, time_t mtime, UDSEntry& entry) {
	kdDebug() << "MTime : " << ( long )mtime << endl;
	kdDebug() << "UDS filename : " << filename << endl;
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

void kio_svnProtocol::copy(const KURL & src, const KURL& dest, int /*permissions*/, bool /*overwrite*/) {
	kdDebug() << "kio_svnProtocol::copy() Source : " << src.url() << " Dest : " << dest.url() << endl;
	
	apr_pool_t *subpool = svn_pool_create (pool);
	svn_client_commit_info_t *commit_info;

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
		kdDebug() << "revision string found " << revstr  << endl;
		if ( revstr == "HEAD" ) {
			rev.kind = svn_opt_revision_head;
			kdDebug() << "revision searched : HEAD" << endl;
		} else {
			rev.kind = svn_opt_revision_number;
			rev.value.number = revstr.toLong();
			kdDebug() << "revision searched : " << rev.value.number << endl;
		}
		srcsvn = srcsvn.left( idx );
		kdDebug() << "new src : " << srcsvn << endl;
	} else {
		kdDebug() << "no revision given. searching HEAD " << endl;
		rev.kind = svn_opt_revision_head;
	}

	svn_error_t *err = svn_client_copy(&commit_info, srcsvn.utf8(), &rev, destsvn.utf8(), &ctx, subpool);
	if ( err ) {
		error( KIO::ERR_SLAVE_DEFINED, err->message );
		svn_pool_destroy( subpool );
		return;
	}
	
	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::mkdir( const KURL& url, int /*permissions*/ ) {
	kdDebug() << "kio_svnProtocol::mkdir() : " << url.url() << endl;
	
	apr_pool_t *subpool = svn_pool_create (pool);
	svn_client_commit_info_t *commit_info = NULL;

	QString target = makeSvnURL( url);
	kdDebug() << "SvnURL: " << target << endl;
	recordCurrentURL( KURL( target ) );
	
	apr_array_header_t *targets = apr_array_make(subpool, 2, sizeof(const char *));
	(*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) = apr_pstrdup( subpool, target.utf8() );

	svn_error_t *err = svn_client_mkdir(&commit_info,targets,&ctx,subpool);
	if ( err ) {
		error( KIO::ERR_COULD_NOT_MKDIR, err->message );
		svn_pool_destroy( subpool );
		return;
	}
	
	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::del( const KURL& url, bool /*isfile*/ ) {
	kdDebug() << "kio_svnProtocol::del() : " << url.url() << endl;
	
	apr_pool_t *subpool = svn_pool_create (pool);
	svn_client_commit_info_t *commit_info = NULL;

	QString target = makeSvnURL(url);
	kdDebug() << "SvnURL: " << target << endl;
	recordCurrentURL( KURL( target ) );
	
	apr_array_header_t *targets = apr_array_make(subpool, 2, sizeof(const char *));
	(*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) = apr_pstrdup( subpool, target.utf8() );

	svn_error_t *err = svn_client_delete(&commit_info,targets,false/*force remove locally modified files in wc*/,&ctx,subpool);
	if ( err ) {
		error( KIO::ERR_CANNOT_DELETE, err->message );
		svn_pool_destroy( subpool );
		return;
	}
	
	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::rename(const KURL& src, const KURL& dest, bool /*overwrite*/) {
	kdDebug() << "kio_svnProtocol::rename() Source : " << src.url() << " Dest : " << dest.url() << endl;
	
	apr_pool_t *subpool = svn_pool_create (pool);
	svn_client_commit_info_t *commit_info;

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
		kdDebug() << "revision string found " << revstr  << endl;
		if ( revstr == "HEAD" ) {
			rev.kind = svn_opt_revision_head;
			kdDebug() << "revision searched : HEAD" << endl;
		} else {
			rev.kind = svn_opt_revision_number;
			rev.value.number = revstr.toLong();
			kdDebug() << "revision searched : " << rev.value.number << endl;
		}
		srcsvn = srcsvn.left( idx );
		kdDebug() << "new src : " << srcsvn << endl;
	} else {
		kdDebug() << "no revision given. searching HEAD " << endl;
		rev.kind = svn_opt_revision_head;
	}

	svn_error_t *err = svn_client_move(&commit_info, srcsvn.utf8(), &rev, destsvn.utf8(), false/*force remove locally modified files in wc*/, &ctx, subpool);
	if ( err ) {
		error( KIO::ERR_CANNOT_RENAME, err->message );
		svn_pool_destroy( subpool );
		return;
	}
	
	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::special( const QByteArray& data ) {
	kdDebug() << "kio_svnProtocol::special" << endl;

	QDataStream stream(data, IO_ReadOnly);
	int tmp;

	stream >> tmp;

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
				kdDebug() << "kio_svnProtocol CHECKOUT from " << repository.url() << " to " << wc.url() << " at " << revnumber << " or " << revkind << endl;
				checkout( repository, wc, revnumber, revkind );
				break;
			}
		case SVN_UPDATE: 
			{
				KURL wc;
				int revnumber;
				QString revkind;
				stream >> wc;
				stream >> revnumber;
				stream >> revkind;
				kdDebug() << "kio_svnProtocol UPDATE " << wc.url() << " at " << revnumber << " or " << revkind << endl;
				update(wc, revnumber, revkind );
				break;
			}
		case SVN_COMMIT: 
			{
				KURL wc;
				stream >> wc;
				kdDebug() << "kio_svnProtocol COMMIT" << endl;
				commit( wc );
				break;
			}
		case SVN_LOG: 
			{
				kdDebug() << "kio_svnProtocol LOG" << endl;
				break;
			}
		case SVN_IMPORT: 
			{
				kdDebug() << "kio_svnProtocol IMPORT" << endl;
				break;
			}
		case SVN_ADD: 
			{
				KURL wc;
				stream >> wc;
				kdDebug() << "kio_svnProtocol ADD" << endl;
				add(wc);
				break;
			}
		case SVN_DEL: 
			{
				KURL wc;
				stream >> wc;
				kdDebug() << "kio_svnProtocol DEL" << endl;
				wc_delete(wc);
				break;
			}
		case SVN_REVERT: 
			{
				KURL wc;
				stream >> wc;
				kdDebug() << "kio_svnProtocol REVERT" << endl;
				wc_revert(wc);
				break;
			}
		case SVN_STATUS: 
			{
				KURL wc;
				stream >> wc;
				kdDebug() << "kio_svnProtocol STATUS" << endl;
				wc_status(wc);
				break;
			}
		default:
			{
				kdDebug() << "kio_svnProtocol DEFAULT" << endl;
				break;
			}
	}
}

void kio_svnProtocol::update( const KURL& wc, int revnumber, const QString& revkind ) {
	kdDebug() << "kio_svn::update : " << wc.path() << " at revision " << revnumber << " or " << revkind << endl ;

	apr_pool_t *subpool = svn_pool_create (pool);
	KURL dest = wc;
	dest.setProtocol( "file" );
	QString target = dest.path();
	recordCurrentURL( dest );
	
	//find the requested revision
	svn_opt_revision_t rev;
	svn_opt_revision_t endrev;

	if ( revnumber != -1 ) {
		rev.value.number = revnumber;
		rev.kind = svn_opt_revision_number;
	} else if ( !revkind.isNull() )
		svn_opt_parse_revision(&rev,&endrev,revkind.utf8(),subpool);

	svn_error_t *err = svn_client_update (NULL /*rev at which it was actually updated*/, svn_path_canonicalize( target.utf8(), subpool ), &rev, true, &ctx, subpool);
	if ( err ) {
		error( KIO::ERR_SLAVE_DEFINED, err->message );
		svn_pool_destroy( subpool );
		return;
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::checkout( const KURL& repos, const KURL& wc, int revnumber, const QString& revkind ) {
	kdDebug() << "kio_svn::checkout : " << repos.url() << " into " << wc.path() << " at revision " << revnumber << " or " << revkind << endl ;

	apr_pool_t *subpool = svn_pool_create (pool);
	KURL nurl = repos;
	KURL dest = wc;
	nurl.setProtocol( chooseProtocol( repos.protocol() ) );
	dest.setProtocol( "file" );
	QString target = nurl.url();
	recordCurrentURL( nurl );
	QString dpath = dest.path();
	
	//find the requested revision
	svn_opt_revision_t rev;
	svn_opt_revision_t endrev;
	if ( revnumber != -1 ) {
		rev.value.number = revnumber;
		rev.kind = svn_opt_revision_number;
	} else if ( !revkind.isNull() )
		svn_opt_parse_revision(&rev,&endrev,revkind.utf8(),subpool);

	svn_error_t *err = svn_client_checkout (NULL/* rev actually checkedout */, svn_path_canonicalize( target.utf8(), subpool ), svn_path_canonicalize ( dpath.utf8(), subpool ), &rev, true, &ctx, subpool);
	if ( err ) {
		error( KIO::ERR_SLAVE_DEFINED, err->message );
		svn_pool_destroy( subpool );
		return;
	}

	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::commit(const KURL& wc) {
	kdDebug() << "kio_svnProtocol::commit() : " << wc.url() << endl;
	
	apr_pool_t *subpool = svn_pool_create (pool);
	svn_client_commit_info_t *commit_info = NULL;
	bool nonrecursive = false;

	KURL nurl = wc;
	nurl.setProtocol( "file" );
	QString target = nurl.url();
	recordCurrentURL( nurl );
	apr_array_header_t *targets = apr_array_make(subpool, 2, sizeof(const char *));
	(*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) = apr_pstrdup( subpool, nurl.path().utf8() );

	svn_error_t *err = svn_client_commit(&commit_info,targets,nonrecursive,&ctx,subpool);
	if ( err ) {
		error( KIO::ERR_SLAVE_DEFINED, err->message );
		svn_pool_destroy( subpool );
		return;
	}
	
	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::add(const KURL& wc) {
	kdDebug() << "kio_svnProtocol::add() : " << wc.url() << endl;
	
	apr_pool_t *subpool = svn_pool_create (pool);
	bool nonrecursive = false;

	KURL nurl = wc;
	nurl.setProtocol( "file" );
	QString target = nurl.url();
	recordCurrentURL( nurl );
	svn_error_t *err = svn_client_add(nurl.path().utf8(),nonrecursive,&ctx,subpool);
	if ( err ) {
		error( KIO::ERR_SLAVE_DEFINED, err->message );
		svn_pool_destroy( subpool );
		return;
	}
	
	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::wc_delete(const KURL& wc) {
	kdDebug() << "kio_svnProtocol::wc_delete() : " << wc.url() << endl;
	
	apr_pool_t *subpool = svn_pool_create (pool);
	svn_client_commit_info_t *commit_info = NULL;
	bool nonrecursive = false;

	KURL nurl = wc;
	nurl.setProtocol( "file" );
	QString target = nurl.url();
	recordCurrentURL( nurl );
	apr_array_header_t *targets = apr_array_make(subpool, 2, sizeof(const char *));
	(*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) = apr_pstrdup( subpool, nurl.path().utf8() );

	svn_error_t *err = svn_client_delete(&commit_info,targets,nonrecursive,&ctx,subpool);

	if ( err ) {
		error( KIO::ERR_SLAVE_DEFINED, err->message );
		svn_pool_destroy( subpool );
		return;
	}
	
	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::wc_revert(const KURL& wc) {
	kdDebug() << "kio_svnProtocol::revert() : " << wc.url() << endl;
	
	apr_pool_t *subpool = svn_pool_create (pool);
	bool nonrecursive = false;

	KURL nurl = wc;
	nurl.setProtocol( "file" );
	QString target = nurl.url();
	recordCurrentURL( nurl );

	apr_array_header_t *targets = apr_array_make(subpool, 2, sizeof(const char *));
	(*(( const char ** )apr_array_push(( apr_array_header_t* )targets)) ) = apr_pstrdup( subpool, nurl.path().utf8() );

	svn_error_t *err = svn_client_revert(targets,nonrecursive,&ctx,subpool);
	if ( err ) {
		error( KIO::ERR_SLAVE_DEFINED, err->message );
		svn_pool_destroy( subpool );
		return;
	}
	
	finished();
	svn_pool_destroy (subpool);
}

void kio_svnProtocol::wc_status(const KURL& wc, bool checkRepos, bool fullRecurse, bool getAll, int revnumber, const QString& revkind) {
	kdDebug() << "kio_svnProtocol::status() : " << wc.url() << endl;
	
	apr_pool_t *subpool = svn_pool_create (pool);

	KURL nurl = wc;
	nurl.setProtocol( "file" );
	QString target = nurl.url();
	recordCurrentURL( nurl );

	svn_opt_revision_t rev;
	svn_opt_revision_t endrev;
	if ( revnumber != -1 ) {
		rev.value.number = revnumber;
		rev.kind = svn_opt_revision_number;
	} else if ( !revkind.isNull() )
		svn_opt_parse_revision(&rev,&endrev,revkind.utf8(),subpool);

	svn_revnum_t result_rev;
	svn_error_t *err = svn_client_status(&result_rev,nurl.path().utf8(),&rev,kio_svnProtocol::status,this,fullRecurse,getAll,checkRepos,false,&ctx,subpool);
	if ( err ) {
		error( KIO::ERR_SLAVE_DEFINED, err->message );
		svn_pool_destroy( subpool );
		return;
	}
	
	finished();
	svn_pool_destroy (subpool);
}

QString kio_svnProtocol::makeSvnURL ( const KURL& url ) const {
	QString kproto = url.protocol();
	KURL tpURL = url;
	QString svnUrl;
	if ( kproto == "svn+http" ) {
		kdDebug() << "http:/" << url.url() << endl;
		tpURL.setProtocol("http");
		svnUrl = tpURL.url();
		return svnUrl;
	}
	else if ( kproto == "svn+https" ) {
		kdDebug() << "https:/" << url.url() << endl;
		tpURL.setProtocol("https");
		svnUrl = tpURL.url();
		return svnUrl;
	}
	else if ( kproto == "svn+ssh" ) {
		kdDebug() << "svn+ssh:/" << url.url() << endl;
		tpURL.setProtocol("svn+ssh");
		svnUrl = tpURL.url();
		return svnUrl;
	}
	else if ( kproto == "svn" ) {
		kdDebug() << "svn:/" << url.url() << endl;
		tpURL.setProtocol("svn");
		svnUrl = tpURL.url();
		return svnUrl;
	}
	else if ( kproto == "svn+file" ) {
		kdDebug() << "file:/" << url.url() << endl;
		tpURL.setProtocol("file");
		svnUrl = tpURL.url();
		//hack : add one more / after file:/
		int idx = svnUrl.find("/");
		svnUrl.insert( idx, "/" );
		kdDebug() << "SvnURL: " << svnUrl << endl;
		return svnUrl;
	}
	return kproto;
}

QString kio_svnProtocol::chooseProtocol ( const QString& kproto ) const {
	if ( kproto == "svn+http" ) return QString( "http" );
	else if ( kproto == "svn+https" ) return QString( "https" );
	else if ( kproto == "svn+ssh" ) return QString( "svn+ssh" );
	else if ( kproto == "svn" ) return QString( "svn" );
	else if ( kproto == "svn+file" ) return QString( "file" );
	return kproto;
}

svn_error_t *kio_svnProtocol::trustSSLPrompt(svn_auth_cred_ssl_server_trust_t **cred_p, void *, const char */*realm*/, apr_uint32_t /*failures*/, const svn_auth_ssl_server_cert_info_t */*cert_info*/, svn_boolean_t /*may_save*/, apr_pool_t *pool) {
	//when ksvnd is ready make it prompt for the SSL certificate ... XXX
	*cred_p = (svn_auth_cred_ssl_server_trust_t*)apr_pcalloc (pool, sizeof (**cred_p));
	(*cred_p)->may_save = FALSE;
	return SVN_NO_ERROR;
}

svn_error_t *kio_svnProtocol::clientCertSSLPrompt(svn_auth_cred_ssl_client_cert_t **/*cred_p*/, void *, const char */*realm*/, svn_boolean_t /*may_save*/, apr_pool_t */*pool*/) {
	//when ksvnd is ready make it prompt for the SSL certificate ... XXX
/*	*cred_p = apr_palloc (pool, sizeof(**cred_p));
	(*cred_p)->cert_file = cert_file;*/
	return SVN_NO_ERROR;
}

svn_error_t *kio_svnProtocol::clientCertPasswdPrompt(svn_auth_cred_ssl_client_cert_pw_t **/*cred_p*/, void *, const char */*realm*/, svn_boolean_t /*may_save*/, apr_pool_t */*pool*/) {
	//when ksvnd is ready make it prompt for the SSL certificate password ... XXX
	return SVN_NO_ERROR;
}

svn_error_t *kio_svnProtocol::commitLogPrompt( const char **log_msg, const char **/*file*/, apr_array_header_t *commit_items, void *baton, apr_pool_t *pool ) {
	QCString replyType;
	QByteArray params;
	QByteArray reply;
	QString result;
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
		kdDebug() << " Commiting items : " << list << endl;
		slist << list;
	}

	QDataStream stream(params, IO_WriteOnly);
	stream << slist.join("\n");	

	if ( !p->dcopClient()->call( "kded","ksvnd","commitDialog(QString)", params, replyType, reply ) ) {
		kdWarning() << "Communication with KDED:KSvnd failed" << endl;
		return SVN_NO_ERROR;
	}

	if ( replyType != "QString" ) {
		kdWarning() << "Unexpected reply type" << endl;
		return SVN_NO_ERROR;
	}
	
	QDataStream stream2 ( reply, IO_ReadOnly );
	stream2 >> result;
	
	if ( result == QString::null ) { //cancelled
		*log_msg = NULL;
		return SVN_NO_ERROR;
	}
		
	message = svn_stringbuf_create( result.utf8(), pool );
	*log_msg = message->data;

	return SVN_NO_ERROR;
}

void kio_svnProtocol::notify(void *baton, const char *path, svn_wc_notify_action_t action, svn_node_kind_t kind, const char *mime_type, svn_wc_notify_state_t content_state, svn_wc_notify_state_t prop_state, svn_revnum_t revision) {
	kdDebug() << "NOTIFY : " << path << " updated at revision " << revision << " action : " << action << ", kind : " << kind << " , content_state : " << content_state << ", prop_state : " << prop_state << endl;

	QByteArray params;
	kio_svnProtocol *p = ( kio_svnProtocol* )baton;

	QDataStream stream(params, IO_WriteOnly);
	stream << QString::fromUtf8( path ) << action << kind << mime_type << content_state << prop_state << revision;

	if ( !p->dcopClient()->send( "kded","ksvnd","notify(QString,int,int,QString,int,int,long int)", params ) ) {
		kdWarning() << "Communication with KDED:KSvnd failed" << endl;
		return;
	}
}

void kio_svnProtocol::status(void *baton, const char *path, svn_wc_status_t *status) {
	kdDebug() << "STATUS : " << path << ", wc text status : " << status->text_status 
									 << ", wc prop status : " << status->prop_status
									 << ", repos text status : " << status->repos_text_status
									 << ", repos prop status : " << status->repos_prop_status 
									 << endl;

	QByteArray params;
	kio_svnProtocol *p = ( kio_svnProtocol* )baton;

	QDataStream stream(params, IO_WriteOnly);
	stream << QString::fromUtf8( path ) << status->text_status << status->prop_status << status->repos_text_status << status->repos_prop_status;

	if ( !p->dcopClient()->send( "kded","ksvnd","status(QString,int,int,int,int)", params ) ) {
		kdWarning() << "Communication with KDED:KSvnd failed" << endl;
		return;
	}
}

extern "C"
{
	int kdemain(int argc, char **argv)    {
		KInstance instance( "kio_svn" );

		kdDebug(7101) << "*** Starting kio_svn " << endl;

		if (argc != 4) {
			kdDebug(7101) << "Usage: kio_svn  protocol domain-socket1 domain-socket2" << endl;
			exit(-1);
		}

		kio_svnProtocol slave(argv[2], argv[3]);
		slave.dispatchLoop();

		kdDebug(7101) << "*** kio_svn Done" << endl;
		return 0;
	}
}

