/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CVSFILEINFOPROVIDER_H
#define CVSFILEINFOPROVIDER_H

#include <qmap.h>

#include <kdevversioncontrol.h>
#include "cvsservicedcopIface.h"
#include "cvsdir.h"
#include "bufferedstringreader.h"

class CvsServicePart;
class CvsService_stub;
class CvsJob_stub;

/**
Provider for CVS file information

@author Mario Scalas
*/
class CVSFileInfoProvider : public KDevVCSFileInfoProvider, virtual public CVSServiceDCOPIface
{
    Q_OBJECT
public:
    CVSFileInfoProvider( CvsServicePart *parent, CvsService_stub *cvsService );
    virtual ~CVSFileInfoProvider();

// -- Sync interface
    virtual const VCSFileInfoMap *status( const QString &dirPath ) const;

// -- Async interface for requesting data
    virtual bool requestStatus( const QString &dirPath, void *callerData );

private:
    // DCOP Iface
    virtual void slotJobExited( bool normalExit, int exitStatus );
    virtual void slotReceivedOutput( QString someOutput );
    virtual void slotReceivedErrors( QString someErrors );

    QString projectDirectory() const;

    static VCSFileInfoMap *parse( QStringList stringStream );

    static VCSFileInfo::FileState String2EnumState( QString stateAsString );

    static void printOutFileInfoMap( const VCSFileInfoMap &map );

    BufferedStringReader m_bufferedReader;
    QStringList m_statusLines;

    mutable void *m_savedCallerData;
    mutable CvsJob_stub *m_requestStatusJob;
    CvsService_stub *m_cvsService;

    //! Caching
    mutable QString m_previousDirPath;
    mutable VCSFileInfoMap *m_cachedDirEntries;
};

#endif
