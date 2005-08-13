/**

  	 Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>

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
	 the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
	 Boston, MA 02110-1301, USA.
	 */

#ifndef SVNFILEINFOPROVIDER_H
#define SVNFILEINFOPROVIDER_H

#include <qmap.h>

#include <kdevversioncontrol.h>
#include <kio/job.h>
#include <dcopclient.h>
#include <dcopobject.h>
#include <subversion_part.h>

/**
Provider for SVN file information

@author Mickael Marchand
*/
class SVNFileInfoProvider : public KDevVCSFileInfoProvider, public DCOPObject/*, virtual public DCOPClient*/
{
    Q_OBJECT
	K_DCOP

public:
    SVNFileInfoProvider( subversionPart *parent, const char *name = 0);
    virtual ~SVNFileInfoProvider();

// -- Sync interface
    virtual const VCSFileInfoMap *status( const QString &dirPath ) ;

// -- Async interface for requesting data
    virtual bool requestStatus( const QString &dirPath, void *callerData );

	QString projectDirectory() const;

k_dcop:
	void slotStatus( const QString& , int, int, int, int, long int ) ;

public slots:
	void slotResult( KIO::Job * );

private:
    mutable void *m_savedCallerData;
	mutable QString m_previousDirPath;
	mutable VCSFileInfoMap *m_cachedDirEntries;
	KIO::SimpleJob *job;
	subversionPart *m_part;
};

#endif
