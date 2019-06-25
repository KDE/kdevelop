/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_PROJECTTESTJOB_H
#define KDEVPLATFORM_PROJECTTESTJOB_H

#include <KJob>

#include "utilexport.h"

namespace KDevelop {

class IProject;
class ProjectTestJobPrivate;

/**
 * A combined result of a project test job
 *
 * @sa ProjectTestJob
 *
 **/
struct KDEVPLATFORMUTIL_EXPORT ProjectTestResult
{
    ProjectTestResult()
    {}
    /**
     * The total number of test suites launched in this job
     **/
    int total = 0;
    /**
     * The number of passed test suites in this job.
     **/
    int passed = 0;
    /**
     * The number of failed test suites in this job.
     **/
    int failed = 0;
    /**
     * The number of errors in this job.
     **/
    int error = 0;
};

/**
 * @brief A job that tests an entire project and reports the total result
 *
 * Launches all test suites in the specified project without raising the output window.
 * Instead of providing individual test results, it combines and simplifies them.
 *
 **/
class KDEVPLATFORMUTIL_EXPORT ProjectTestJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Create a project test job
     *
     * @param project The project to be tested
     * @param parent This job's parent object, or 0 for no parent.
     *
     **/
    explicit ProjectTestJob(IProject* project, QObject* parent = nullptr);

    /**
     * Destructor
     *
     **/
    ~ProjectTestJob() override;

    /**
     * Start this job.
     **/
    void start() override;

    /**
     * @brief The result of this job
     *
     * This function only returns a correct result after all the tests are completed.
     * It is therefore best to call this after the KJob::result() signal is emitted.
     *
     * @sa ProjectTestResult
     **/
    ProjectTestResult testResult();

protected:
    bool doKill() override;

private:
    friend class ProjectTestJobPrivate;
    const QScopedPointer<class ProjectTestJobPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProjectTestJob)
};

}

Q_DECLARE_TYPEINFO(KDevelop::ProjectTestResult, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_PROJECTTESTJOB_H
