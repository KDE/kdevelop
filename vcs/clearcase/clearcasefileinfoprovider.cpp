//
// C++ Implementation: clearcasefileinfoprovider
//
// Description:
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "clearcasefileinfoprovider.h"
#include "clearcasepart.h"
#include "kdevversioncontrol.h"

#include <kprocess.h>
#include <qapplication.h>

ClearcaseFileinfoProvider::ClearcaseFileinfoProvider(ClearcasePart *parent)
	: KDevVCSFileInfoProvider( (KDevVersionControl*) parent, "clearcasefileinfoprovider")
{
    vcsInfo_ = NULL;

    connect(parent, SIGNAL(statusReady(const VCSFileInfoMap&, void*)), SIGNAL(triggerUpdate(const VCSFileInfoMap&, void*)));

    kdevVCS_ = parent;
}


ClearcaseFileinfoProvider::~ClearcaseFileinfoProvider()
{
}


const VCSFileInfoMap* ClearcaseFileinfoProvider::status( const QString &dirPath ) {

    if (curDirPath_ == dirPath) return vcsInfo_;

    curDirPath_ = dirPath;

    if (vcsInfo_ != NULL) delete vcsInfo_;
    
    vcsInfo_ = ccManipulator_.retreiveFilesInfos(dirPath);

    return vcsInfo_;
}

bool ClearcaseFileinfoProvider::requestStatus( const QString &dirPath, void *callerData ) {
 	
    VCSFileInfoMap* vcsDirInfos = ccManipulator_.retreiveFilesInfos(dirPath);

    // update the file tree view
    emit statusReady(*vcsDirInfos, callerData);

    delete vcsDirInfos;
    return true;
}



QStringList ClearcaseFileinfoProvider::registeredEntryList() const
{
    QStringList l;
    return l;
}

