//
// C++ Implementation: kdevversioncontrolfeedback
//
// Description:
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kdevversioncontrol.h>
#include "kdevvcsfileinfoprovider.h"

///////////////////////////////////////////////////////////////////////////////
// struct KDevVCSFileInfo::Private
///////////////////////////////////////////////////////////////////////////////

struct KDevVCSFileInfo::Private
{
    Private() {}
    Private( QString aFileName, QString aRevision, QString aTimestamp, QString aTag )
        : fileName(aFileName), revision(aRevision), timestamp(aTimestamp), tag(aTag) {}

    QString fileName;
    QString revision;
    QString timestamp;
    QString tag;
};

///////////////////////////////////////////////////////////////////////////////
// struct KDevVCSFileInfo::Private
///////////////////////////////////////////////////////////////////////////////

KDevVCSFileInfo::KDevVCSFileInfo()
    : d(new Private)
{
}

///////////////////////////////////////////////////////////////////////////////

KDevVCSFileInfo::KDevVCSFileInfo(
    QString aFileName, QString aRevision, QString aTimestamp, QString aTag )
    : d(new Private( aFileName, aRevision, aTimestamp, aTag ))
{
}

///////////////////////////////////////////////////////////////////////////////

KDevVCSFileInfo::~KDevVCSFileInfo()
{
    delete d;
}

QString KDevVCSFileInfo::fileName() const
{
    return d->fileName;
}

QString KDevVCSFileInfo::revision() const
{
    return d->revision;
}

QString KDevVCSFileInfo::timestamp() const
{
    return d->timestamp;
}

QString KDevVCSFileInfo::tag() const
{
    return d->tag;
}

bool KDevVCSFileInfo::isModified() const
{
    return false;
}

bool KDevVCSFileInfo::isRegisteredInRepository() const
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// struct KDevVCSFileInfoProvider::Private
///////////////////////////////////////////////////////////////////////////////

struct KDevVCSFileInfoProvider::Private
{
    Private( KDevVersionControl *owner ) : m_owner( owner ) {}

    KDevVersionControl *m_owner;
};

///////////////////////////////////////////////////////////////////////////////
// class KDevVCSFileInfoProvider
///////////////////////////////////////////////////////////////////////////////

KDevVCSFileInfoProvider::KDevVCSFileInfoProvider( KDevVersionControl *parent )
    : d(new Private( parent ))
{
}

///////////////////////////////////////////////////////////////////////////////

KDevVCSFileInfoProvider::~KDevVCSFileInfoProvider()
{
    delete d;
}


