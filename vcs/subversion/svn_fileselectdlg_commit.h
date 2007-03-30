/*
 *  Copyright (C) 2007 Dukju Ahn (dukjuahn@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef _svnfileselectdlgcommit_
#define _svnfileselectdlgcommit_

#include "svn_commitdlgbase.h"
#include <qdialog.h>
#include <kurl.h>

class QString;
class KURL;
class QListView;
class subversionPart;

class SVNFileSelectDlgCommit : public SvnCommitDlgBase{
	
Q_OBJECT
		
public:
	SVNFileSelectDlgCommit( KURL::List&, subversionPart* part, QWidget* parent = 0 );
	~SVNFileSelectDlgCommit();
	void insertItem( QString status, KURL url );
	KURL::List checkedUrls();
	bool recursive();
	bool keepLocks();
	
public slots:
	int exec();	
protected:
	QListView* listView();
	subversionPart *m_part;
};

#endif
