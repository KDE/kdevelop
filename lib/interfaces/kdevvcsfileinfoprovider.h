//
// C++ Interface: kdevversioncontrolfeedback
//
// Description:
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KDEVVCSFILEINFOPROVIDER_H
#define KDEVVCSFILEINFOPROVIDER_H

/**
This is a basic interface for providing information about the file
state in respect to repository.

@author KDevelop Authors
*/

#include <qvaluelist.h>

/**
*
*/
struct KDevVCSFileInfo
{
    KDevVCSFileInfo();
    KDevVCSFileInfo( QString aFileName, QString aRevision, QString aTimestamp, QString aTag );
    virtual ~KDevVCSFileInfo();

    QString fileName() const;
    QString revision() const;
    QString timestamp() const;
    QString tag() const;

    // Reimplements these two methods (they return false by default)
    virtual bool isModified() const;
    virtual bool isRegisteredInRepository() const;

protected:
    struct Private;
    Private *d;

private:
    // Verboten!
    KDevVCSFileInfo( const KDevVCSFileInfo & );
    KDevVCSFileInfo &operator=( const KDevVCSFileInfo & );
};

/**
*
*/
typedef QValueList<KDevVCSFileInfo> KDevVCSFileInfoList;

class KDevVersionControl;
class KURL;

/**
*  A basic interface for providing info on file registered in a version control repository repository
*/
class KDevVCSFileInfoProvider
{

public:
    /**
    * Constructor
    */
    KDevVCSFileInfoProvider( KDevVersionControl *parent );
    /**
    * Destructor
    */
    virtual ~KDevVCSFileInfoProvider();

    /**
    * @return the version control which owns this version control
    */
    KDevVersionControl *owner() const;

    /**
    * @param filePath absolute path of the fileName
    * @return file info for a single file.
    */
    virtual KDevVCSFileInfo fileInfo( const KURL &fileUrl ) = 0;

    virtual KDevVCSFileInfo dirInfo( const KURL &dirUrl ) = 0;

private:
    struct Private;
    Private *d;

private:
    // Verboten!
    KDevVCSFileInfoProvider( const KDevVCSFileInfoProvider & );
    KDevVCSFileInfoProvider &operator=( const KDevVCSFileInfoProvider & );
};

#endif // KDEVVCSFILEINFOPROVIDER_H
