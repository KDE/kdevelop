/* -*- C++ -*-

   This file implements the DependencyPolicy class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: DebuggingAids.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#ifndef DEPENDENCYPOLICY_H
#define DEPENDENCYPOLICY_H

#include "QueuePolicy.h"

#include <QtCore/QList>

namespace ThreadWeaver {

    /** DependencyPolicy implements the way dependencies between Jobs are handled.

        To declare that Job B can only be executed when Job A is finished,
        call addDependency.

        Be aware of circular dependencies. All dependencies on a Job will be
        removed if the Job object is destructed.

        JobSequence uses dependencies to implement the ordered execution of
        the sequence elements.
    */
    class DependencyPolicy : public QueuePolicy
    {
    public:
      /** Destructor. */
      ~DependencyPolicy();

        /** Add jobA as a dependency of jobB.
            jobA will only be executed after jobB has been successfully processed.
            @param jobA the depending job
            @param jobB the job jobA depends on
        */
        void addDependency( Job* jobA,  Job* jobB );

        /** Remove dependency.

            The dependency of jobA on jobB is removed. If no dependencies are
            left for jobA, canRun will return true.

            Returns false if the given object is not dependency of this job.

	    This function is inefficient, and should be used only to abort
	    execution of a job.

	    @param jobA the depending job
	    @param jobB the job jobA depends on
	    @return true if dependency existed, false otherwise
        */
        bool removeDependency( Job* jobA, Job* jobB );

        /** Resolve all dependencies.
            This method is called after the Job has been finished, or
            when it is deleted without being executed (performed by the
            destructor).
            The method will remove all entries stating that another Job
            depends on this one.
        */
        void resolveDependencies ( Job* );

        /** Retrieve a list of dependencies of this job. */
        QList<Job*> getDependencies( Job* ) const;

        static DependencyPolicy& instance();

        bool canRun( Job* );

        void free( Job* );

        void release( Job* );

        void destructed( Job* );

    protected:
        /** Query whether the job has an unresolved dependency.
            In case it does, the policy will return false from canRun().
        */
        bool hasUnresolvedDependencies( Job* );

        DependencyPolicy();

    private:
        class Private;
        Private* d;

    public:
        /** This method should be useful for debugging purposes. */
        void dumpJobDependencies();
    };

}

#endif
