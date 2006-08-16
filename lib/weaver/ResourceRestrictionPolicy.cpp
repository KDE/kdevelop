/* -*- C++ -*-

   This file implements the ResourceRestrictionPolicy class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id: Job.h 32 2005-08-17 08:38:01Z mirko $
*/

#include <QList>
#include <QMutex>
#include <QMutexLocker>

#include "DebuggingAids.h"
#include "ResourceRestrictionPolicy.h"

class Job;

using namespace ThreadWeaver;

class ResourceRestrictionPolicy::Private
{
public:
    Private ( int theCap )
        : cap ( theCap)
    {}

    int cap;
    QList<Job*> customers;
    QMutex mutex;
};

ResourceRestrictionPolicy::ResourceRestrictionPolicy ( int cap)
    : QueuePolicy ()
    , d (new Private (cap))
{
}

ResourceRestrictionPolicy::~ResourceRestrictionPolicy()
{
    delete d;
}

void ResourceRestrictionPolicy::setCap (int cap)
{
    QMutexLocker l ( & d->mutex );
    d->cap = cap;
}

bool ResourceRestrictionPolicy::canRun( Job* job )
{
    QMutexLocker l ( & d->mutex );
    if ( d->customers.size() < d->cap )
    {
        d->customers.append( job );
        return true;
    } else {
        return false;
    }
}

void ResourceRestrictionPolicy::free ( Job* job )
{
    QMutexLocker l ( & d->mutex );
    int position = d->customers.indexOf (job);

    if (position != -1)
    {
        debug ( 4, "ResourceRestrictionPolicy::free: job %p done.\n", job );
        d->customers.removeAt (position);
    }
}

void ResourceRestrictionPolicy::release ( Job* job )
{
    free (job);
}

void ResourceRestrictionPolicy::destructed ( Job* job )
{
    free (job);
}
