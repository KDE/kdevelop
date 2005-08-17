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

#ifndef _svn_H_
#define _svn_H_

#include <qstring.h>
#include <q3cstring.h>

#include <kurl.h>
#include <kio/global.h>
#include <kio/slavebase.h>
#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_auth.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_config.h>
#include <sys/stat.h>
#include <q3valuelist.h>
#include <subversion-1/svn_wc.h>

class Q3CString;

typedef struct kbaton {
	svn_stream_t *target_stream;
	svn_stringbuf_t *target_string;
	svn_stream_t *string_stream;
} kbaton;

typedef struct kio_svn_callback_baton_t {
	const char* base_dir;
	apr_hash_t *config;
	apr_pool_t *pool;
} kio_svn_callback_baton_t;


class kio_svnProtocol : public KIO::SlaveBase
{
	public:
		kio_svnProtocol(const Q3CString &pool_socket, const Q3CString &app_socket);
		virtual ~kio_svnProtocol();
		virtual void special( const QByteArray& data );
		virtual void get(const KURL& url);
		virtual void listDir(const KURL& url);
		virtual void stat(const KURL& url);
		virtual void mkdir(const KURL& url, int permissions);
		virtual void del( const KURL& url, bool isfile );
		virtual void copy(const KURL & src, const KURL& dest, int permissions, bool overwrite);
		virtual void rename(const KURL& src, const KURL& dest, bool overwrite);
		void checkout( const KURL& repos, const KURL& wc, int revnumber, const QString& revkind );
		void update( const KURL& wc, int revnumber, const QString& revkind );
		void commit( const KURL& wc );
		void add( const KURL& wc );
		//these work using the working copy
		void wc_delete( const KURL& wc );
		void wc_revert( const KURL& wc );
		void wc_status(const KURL& wc, bool checkRepos=false, bool fullRecurse=true, bool getAll=true, int revnumber=-1, const QString& revkind="HEAD");

		static svn_error_t* checkAuth(svn_auth_cred_simple_t **cred, void *baton, const char *realm, const char *username, svn_boolean_t may_save, apr_pool_t *pool);
		static svn_error_t *trustSSLPrompt(svn_auth_cred_ssl_server_trust_t **cred_p, void *, const char *realm, apr_uint32_t failures, const svn_auth_ssl_server_cert_info_t *cert_info, svn_boolean_t may_save, apr_pool_t *pool);
		static svn_error_t *clientCertSSLPrompt(svn_auth_cred_ssl_client_cert_t **cred_p, void *, const char *realm, svn_boolean_t may_save, apr_pool_t *pool);
		static svn_error_t *clientCertPasswdPrompt(svn_auth_cred_ssl_client_cert_pw_t **cred_p, void *, const char *realm, svn_boolean_t may_save, apr_pool_t *pool);
		static svn_error_t *commitLogPrompt( const char **log_msg, const char **tmp_file, apr_array_header_t *commit_items, void *baton, apr_pool_t *pool );
		static void notify(void *baton, const char *path, svn_wc_notify_action_t action, svn_node_kind_t kind, const char *mime_type, svn_wc_notify_state_t content_state, svn_wc_notify_state_t prop_state, svn_revnum_t revision);
		static void status(void *baton, const char *path, svn_wc_status_t *status);

		QString chooseProtocol ( const QString& kproto ) const; 
		QString makeSvnURL ( const KURL& url ) const;
			
		void recordCurrentURL(const KURL& url);
		KURL myURL;
		svn_client_ctx_t ctx;
		KIO::AuthInfo info;

		enum SVN_METHOD { 
			SVN_CHECKOUT=1, //KURL repository, KURL workingcopy, int revnumber=-1, QString revkind(HEAD, ...) //revnumber==-1 => use of revkind
			SVN_UPDATE=2, // KURL wc (svn:///tmp/test, int revnumber=-1, QString revkind(HEAD, ...) // revnumber==-1 => use of revkind
			SVN_COMMIT=3, 
			SVN_LOG=4, 
			SVN_IMPORT=5,
			SVN_ADD=6,
			SVN_DEL=7,
			SVN_REVERT=8,
			SVN_STATUS=9
		};

	private:
		bool createUDSEntry( const QString& filename, const QString& user, long int size, bool isdir, time_t mtime, KIO::UDSEntry& entry);
		apr_pool_t *pool;
};

#endif
