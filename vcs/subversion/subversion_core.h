/**

  	 Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>

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

#ifndef __SUBVERSION_CORE_H__
#define __SUBVERSION_CORE_H__

#include <qobject.h>
#include <qwidget.h>
#include <kio/job.h>
#include <kurl.h>
#include <ktempdir.h>
#include "subversion_fileinfo.h"
#include "subversion_global.h"

class KDevProject;
class subversionPart;
class subversionWidget;
class KApplication;
class SvnBlameHolder;
class SvnLogHolder;
class SvnLogViewWidget;

// class subversionCore : public QObject, public DCOPObject
class subversionCore : public QObject {
  Q_OBJECT
//   K_DCOP

public:

  subversionCore(subversionPart *part);
  ~subversionCore();
	subversionWidget *processWidget() const;
// 	SvnLogViewWidget *processWidget() const;
	void update( const KURL::List&);
	void commit( const KURL::List&, bool recurse, bool keeplocks );
	void svnLog( const KURL::List& list,
			int revstart, QString revKindStart, int revend, QString revKindEnd, 
			bool discorverChangedPath, bool strictNodeHistory );
	void blame( const KURL &url, SvnGlobal::UrlMode mode, int revstart, QString revKindStart, int revend, QString revKindEnd );
	void add( const KURL::List&);
	void del( const KURL::List&);
	void diff( const KURL::List&, const QString& where);
	void diffAsync( const KURL &pathOrUrl1, const KURL &pathOrUrl2,
					int rev1, QString revKind1, int rev2, QString revKind2,
					bool recurse, bool pegdiff = false );
	void revert( const KURL::List&);
	void resolve( const KURL::List&);
	void checkout();
	void switchTree( const KURL &path, const KURL &repositUrl,
					 int revNum, const QString &revKind, bool recurse );
	void switchRelocate( const KURL &path, const KURL &currentUrl, const KURL &newUrl, bool recurse );
	void svnCopy( const KURL &src, int srcRev, const QString &srcRevKind, const KURL &dest );
	void merge( const KURL &src1, int rev1, QString revKind1, const KURL &src2, int rev2, QString revKind2, const KURL &wc_path,
				   bool recurse, bool ignore_ancestry, bool force, bool dry_run );
    // This is blocking function. But the GUI is not blocked.
    // Information will be pulled solely from the working copy.Thus no network connections will be made.
    // Parameter holderMap is the map to be filled out by this method. Callers should preallocate this object.
    // Return true on success. Otherwise return false.
    bool clientInfo( KURL path_or_url, bool recurse, QMap< KURL, SvnGlobal::SvnInfoHolder> &holderMap );
	void createNewProject( const QString& dirName, const KURL& importURL, bool init );
    KDevVCSFileInfoProvider *fileInfoProvider() const;
	
	void initProcessDlg( KIO::Job *job, const QString &src, const QString &dest );
// k_dcop:
// 	void notification( const QString&, int,int, const QString&, int,int ,long int, const QString& );

private slots:
	void slotEndCheckout( KIO::Job * job );
	void slotResult( KIO::Job * job );
	void slotLogResult( KIO::Job * job );
	void slotBlameResult( KIO::Job * job );
	void slotDiffResult( KIO::Job * job );

signals:
	void checkoutFinished( QString dir );

private:
	QGuardedPtr<subversionWidget> m_widget;
	subversionPart *m_part;
	QString wcPath;
	SVNFileInfoProvider *m_fileInfoProvider;
	QStringList diffresult; //for diff commands ;)
	// be nice about tmp diff files: delete all of them when exiting.
	KTempDir* diffTmpDir;

};

#endif
