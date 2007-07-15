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
#include <qcstring.h>

#include <kurl.h>
#include <kio/global.h>
#include <kio/slavebase.h>
#include <subversion-1/svn_pools.h>
#include <subversion-1/svn_auth.h>
#include <subversion-1/svn_client.h>
#include <subversion-1/svn_config.h>
#include <sys/stat.h>
#include <qvaluelist.h>
#include <subversion-1/svn_wc.h>
#include "subversion_global.h"

class QCString;
class kio_svnProtocol;

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

typedef struct notify_baton {
  svn_boolean_t received_some_change;
  svn_boolean_t is_checkout;
  svn_boolean_t is_export;
  svn_boolean_t suppress_final_line;
  svn_boolean_t sent_first_txdelta;
  svn_boolean_t in_external;
  svn_boolean_t had_print_error; /* Used to not keep printing error messages
                                    when we've already had one print error. */
  apr_pool_t *pool; /* this pool is cleared after every notification,
                       so don't keep anything here! */
  kio_svnProtocol *master;
} notify_baton;


class kio_svnProtocol : public KIO::SlaveBase
{
	public:
		kio_svnProtocol(const QCString &pool_socket, const QCString &app_socket);
		virtual ~kio_svnProtocol();
		virtual void special( const QByteArray& data );
		virtual void get(const KURL& url);
		virtual void listDir(const KURL& url);
		virtual void stat(const KURL& url);
		virtual void mkdir(const KURL& url, int permissions);
		virtual void mkdir(const KURL::List& list, int permissions);
		virtual void del( const KURL& url, bool isfile );
// 		virtual void copy(const KURL & src, const KURL& dest, int permissions, bool overwrite);
		virtual void rename(const KURL& src, const KURL& dest, bool overwrite);
		void checkout( const KURL& repos, const KURL& wc, int revnumber, const QString& revkind );
		void import( const KURL& repos, const KURL& wc );
		void svn_switch( const KURL& wc, const KURL& url, int revnumber, const QString& revkind, bool recurse);
		void svn_switch_relocate( const KURL &wc, const KURL &origUrl, const KURL &newUrl,
				                  bool recurse );
		void svn_diff( const KURL& url1, const KURL& url2, int rev1, int rev2, const QString& revkind1, const QString& revkind2, bool recurse, bool pegdiff);
		//TODO fix with svn 1.2 : support a KURL::List -> svn_client_update2()
		void update( const KURL::List &list, int revnumber, const QString& revkind );
		void commit( const KURL::List& wc );
		void commit2( bool recurse, bool keeplocks, const KURL::List& wc );
		void blame( KURL url, SvnGlobal::UrlMode mode, /*int pegRev, QString pegRevKind,*/ int startRev, QString startRevKind, int endRev, QString endRevKind );
		static svn_error_t* blameReceiver( void *baton, apr_int64_t line_no, svn_revnum_t rev, const char *author, const char *date, const char *line, apr_pool_t *pool );
		void svn_log( int revstart, const QString &revkindstart, int revend, const QString &revkindend, bool discorverChangedPath, bool strictNodeHistory, const KURL::List& targets );
		static svn_error_t* receiveLogMessage(void *baton, apr_hash_t *changed_paths, svn_revnum_t revision, const char *author, const char *date, const char *message, apr_pool_t *pool );
		void add( const KURL::List& wcList );
		//these work using the working copy
		void wc_resolve( const KURL& wc, bool recurse = true );
		void wc_delete( const KURL::List& wc );
		void wc_revert( const KURL::List& wc );
		void wc_status(const KURL& wc, bool checkRepos=false, bool fullRecurse=true, bool getAll=true, int revnumber=-1, const QString& revkind="HEAD");
		void wc_status2(const KURL& wc, bool checkRepos=false, bool fullRecurse=true, bool getAll=true, bool noIgnore=false, int revnumber=-1, const QString& revkind="WORKING");
        void svn_info( KURL pathOrUrl, int pegRev, QString pegRevKind, int rev, QString revKind, bool recurse );
        static svn_error_t* infoReceiver( void *baton, const char *path, const svn_info_t *info, apr_pool_t *pool);
		void svn_copy( const KURL &srcUrl, int srcRev, const QString &srcRevKind, const KURL &destUrl );
		void svn_merge( const KURL &src1, int rev1, QString revKind1, const KURL &src2, int rev2, QString revKind2,
						const KURL &target_wc,
						bool recurse, bool ignore_ancestry, bool force, bool dry_run );

		static svn_error_t* checkAuth(svn_auth_cred_simple_t **cred, void *baton, const char *realm, const char *username, svn_boolean_t may_save, apr_pool_t *pool);
		static svn_error_t *trustSSLPrompt(svn_auth_cred_ssl_server_trust_t **cred_p, void *, const char *realm, apr_uint32_t failures, const svn_auth_ssl_server_cert_info_t *cert_info, svn_boolean_t may_save, apr_pool_t *pool);
		static svn_error_t *clientCertSSLPrompt(svn_auth_cred_ssl_client_cert_t **cred_p, void *, const char *realm, svn_boolean_t may_save, apr_pool_t *pool);
		static svn_error_t *clientCertPasswdPrompt(svn_auth_cred_ssl_client_cert_pw_t **cred_p, void *, const char *realm, svn_boolean_t may_save, apr_pool_t *pool);
		static svn_error_t *commitLogPrompt( const char **log_msg, const char **tmp_file, apr_array_header_t *commit_items, void *baton, apr_pool_t *pool );
		static void notify(void *baton, const char *path, svn_wc_notify_action_t action, svn_node_kind_t kind, const char *mime_type, svn_wc_notify_state_t content_state, svn_wc_notify_state_t prop_state, svn_revnum_t revision);
		static void status(void *baton, const char *path, svn_wc_status_t *status);
		static void progressCallback( apr_off_t progress, apr_off_t total, void *baton, apr_pool_t *pool);

		QString chooseProtocol ( const QString& kproto ) const; 
		QString makeSvnURL ( const KURL& url ) const;
		void initNotifier(bool is_checkout, bool is_export, bool suppress_final_line, apr_pool_t *spool);
			
		void recordCurrentURL(const KURL& url);
		void popupMessage( const QString& message );
		int counter() { return m_counter; }
		void incCounter() { m_counter++; }
		svn_opt_revision_t createRevision( int revision, const QString& revkind, apr_pool_t *pool );

		KURL myURL;
		svn_client_ctx_t *ctx;
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
			SVN_STATUS=9,
			SVN_MKDIR=10,
			SVN_RESOLVE=11,
			SVN_SWITCH=12,
			SVN_DIFF=13,
			SVN_BLAME=14,
            SVN_INFO = 15,
			SVN_SWITCH_RELOCATE = 16,
			SVN_COPY = 17,
			SVN_MERGE = 18,
			SVN_COMMIT_2=103,
			SVN_STATUS_2=109
                    
		};

	private:
		bool createUDSEntry( const QString& filename, const QString& user, long long int size, bool isdir, time_t mtime, KIO::UDSEntry& entry);
		apr_pool_t *pool;
		unsigned long int m_counter;
};

#endif
