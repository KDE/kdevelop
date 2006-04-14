//
// C++ Interface: clearcasefileinfoprovider
//
// Description:
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CLEARCASEFILEINFOPROVIDER_H
#define CLEARCASEFILEINFOPROVIDER_H

#include "clearcasepart.h"
#include "clearcasemanipulator.h"


/**
@author KDevelop Authors
*/
class ClearcaseFileinfoProvider : public KDevVCSFileInfoProvider
{
  Q_OBJECT
public:
    ClearcaseFileinfoProvider(ClearcasePart *parent);

    virtual ~ClearcaseFileinfoProvider();

    // -- Sync interface
    const VCSFileInfoMap *status( const QString &dirPath ) ;

    // -- Async interface for requesting data
    bool requestStatus( const QString &dirPath, void *callerData );


private:

    QStringList registeredEntryList() const;


private:
    ClearcaseManipulator ccManipulator_;
    QString curDirPath_;
    VCSFileInfoMap* vcsInfo_;
    ClearcasePart* kdevVCS_;

};

#endif
