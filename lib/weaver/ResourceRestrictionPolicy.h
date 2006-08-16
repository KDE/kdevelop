/* -*- C++ -*-

   This file declares the ResourceRestrictionPolicy class.

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

#ifndef RESOURCE_RESTRICTION_POLICY_H
#define RESOURCE_RESTRICTION_POLICY_H

#include "QueuePolicy.h"

namespace ThreadWeaver {


    /** ResourceRestrictionPolicy is used to limit the number of concurrent accesses to the same resource.

        If a set of Jobs accesses a resource that can be overloaded, this may
        degrade application performance. For example, loading too many files
        from the hard disc at the same time may lead to longer load times.
        ResourceRestrictionPolicy can be used to cap the number of
        accesses. Resource restriction policies are shared between the
        affected jobs. All jobs that share a resurce restriction policy have
        to acquire permission from the policy before they can run. In this
        way, resource restrictions can be compared to semaphores, only that
        they require no locking at the thread level.

        The SMIV example uses a resource restriction to limit the number of
        images files that are loaded from the disk at the same time.

    */

    class ResourceRestrictionPolicy : public QueuePolicy
    {
    public:
      explicit ResourceRestrictionPolicy ( int cap = 0);
      ~ResourceRestrictionPolicy();

      void setCap (int newcap);
      bool canRun( Job* );
      void free (Job*);
      void release (Job*);
      void destructed (Job*);

    private:
      class Private;
      Private* d;
    };

}

#endif // RESOURCE_RESTRICTION_POLICY_H
