/* This file is part of KDevelop
 C opyright 2012 Ivan Shapoval*ov <intelfx100@gmail.com>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_OUTPUTEXECUTEJOB_H
#define KDEVPLATFORM_OUTPUTEXECUTEJOB_H

#include "outputjob.h"
#include "outputmodel.h"
#include <QProcess>
#include <QUrl>

class KProcess;

namespace KDevelop
{

class ProcessLineMaker;
class OutputExecuteJobPrivate;

class KDEVPLATFORMOUTPUTVIEW_EXPORT OutputExecuteJob : public OutputJob
{
    Q_OBJECT

public:
    enum JobStatus
    {
        JobRunning = 0,    /**< The job is running */
        JobSucceeded = 1,  /**< The job has succeeded */
        JobCanceled = 2,   /**< The job has been cancelled */
        JobFailed = 3,     /**< The job has failed */
        JobNotStarted = 4  /**< The job hasn't been started so far */
    };

    enum
    {
        InvalidWorkingDirectoryError = OutputJob::UserDefinedError,
        UserDefinedError
    };

    enum JobProperty
    {
        AppendProcessString   = 0x001, /**< Whether to append a process string to the user-specified job name */
        NeedWorkingDirectory  = 0x002, /**< Whether to require a non-empty working directory to be provided */
        CheckWorkingDirectory = 0x004, /**< Whether to check that the working directory actually exists (and not to create it if needed) */
        PortableMessages      = 0x008, /**< Whether to set LC_MESSAGES=C in the process' environment */
        DisplayStdout         = 0x010, /**< Whether to pass process' stdout to the output model */
        AccumulateStdout      = 0x020, /**< Whether to save process' stdout to the internal byte-array buffer for further consumption */
        DisplayStderr         = 0x040, /**< Whether to pass process' stderr to the output model */
        AccumulateStderr      = 0x080, /**< Whether to save process' stderr to the internal byte-array buffer for further consumption */
        NoSilentOutput        = 0x100, /**< Whether to call \ref startOutput() only if verbosity is \ref OutputJob::Verbose */
        PostProcessOutput     = 0x200, /**< Whether to connect line maker's signals to \ref postProcessStdout() and \ref postProcessStderr() */
        IsBuilderHint         = 0x400, /**< Whether to use builder-specific messages to talk to user (e. g. "build directory" instead of "working directory" */
    };
    Q_FLAGS(JobProperty JobProperties)
    Q_DECLARE_FLAGS(JobProperties, JobProperty)

    OutputExecuteJob( QObject* parent = 0, OutputJobVerbosity verbosity = OutputJob::Verbose );
    virtual ~OutputExecuteJob();

    /**
     * Get the job's status (associated with the process).
     *
     * @returns The job's status.
     * @see JobStatus
     */
    JobStatus status() const;

    /**
     * Get the job's output model.
     *
     * @returns The job's output model, downcasted to \ref OutputModel
     */
    OutputModel* model() const;

    /**
     * Returns a working directory for the job's process.
     *
     * @returns URL which has been set through \ref setWorkingDirectory(); empty URL if unset.
     */
    virtual QUrl workingDirectory() const;

    /**
     * Set a working directory for the job's process.
     * Effective if \ref workingDirectory() hasn't been overridden.
     *
     * @param directory a valid local directory URL, or an empty URL to unset.
     */
    void setWorkingDirectory( const QUrl& directory );

    /**
     * Get process' command line.
     *
     * @returns The command line for the process, with first element in list being the program path.
     */
    virtual QStringList commandLine() const;

    /**
     * Append an element to the command line argument list for this process.
     * If no executable is set yet, it will be set instead.
     * Effective if \ref commandLine() hasn't been overridden.
     *
     * @param argument the argument to add
     */
    OutputExecuteJob& operator<<( const QString& argument );

    /**
     * Append a list of elements to the command line argument list for this process.
     * If no executable is set yet, it will be set from the first argument in given list.
     * Effective if \ref commandLine() hasn't been overridden.
     *
     * @param arguments the arguments to add
     */
    OutputExecuteJob& operator<<( const QStringList& arguments );

    /**
     * Get the privilege escalation command ("su", "sudo", etc.) used for the job's process.
     *
     * @returns The privilege escalation command name and arguments; empty list if not set.
     */
    virtual QStringList privilegedExecutionCommand() const;

    /**
     * Set the privilege escalation command ("su", "sudo", etc.) which will be used for the job's process.
     * Effective if \ref privilegedExecutionCommand() hasn't been overridden.
     *
     * @param command The privilege escalation command's name and arguments; empty list to unset.
     * @see privilegedCommand
     */
    void setPrivilegedExecutionCommand( const QStringList& command );

    /**
     * A convenience function to set the job name.
     *
     * Calls \ref setTitle() and \ref setObjectName().
     *
     * @note If you need the command-line to be appended to the job name,
     * make sure that it is already configured upon calling this function.
     *
     * @param name The name to set; empty string to use default (process string).
     */
    void setJobName( const QString& name );

    /**
     * Set the filtering strategy for the output model.
     */
    void setFilteringStrategy( OutputModel::OutputFilterStrategy strategy );

    /**
     * Get the current properties of the job.
     *
     * @note Default-set properties are: \ref DisplayStdout.
     */
    virtual JobProperties properties() const;

    /**
     * Set properties of the job.
     * Effective if \ref properties() hasn't been overridden.
     *
     * @param properties Which flags to add to the job.
     * @param override Whether to assign instead of doing bitwise OR.
     * @see JobProperties, properties(), unsetProperties()
     */
    void setProperties( JobProperties properties, bool override = false );

    /**
     * Unset properties of the job.
     *
     * @param properties Which flags to remove from the job
     * @see JobProperties, properties(), setProperties()
     */
    void unsetProperties( JobProperties properties );

    /**
     * Add a variable to the job's process environment.
     *
     * The variables added with this method override ones from the system environment and
     * the global environment profile, but are overridden by "PortableMessages" property.
     *
     * @param name The name of a variable to add
     * @param value The value of a variable to add; empty string to unset.
     */
    void addEnvironmentOverride( const QString& name, const QString& value );

    /**
     * Remove a variable from the override set.
     *
     * @param name The name of a variable to remove.
     * @note This does not force a variable to empty value; this is to undo the overriding itself.
     */
    void removeEnvironmentOverride( const QString& name );

    /**
     * Get the global environment profile name for the job's process.
     *
     * @returns The environment profile name to use in the job's process; empty if unset.
     */
    virtual QString environmentProfile() const;

    /**
     * Set the environment profile name for the job's process.
     * Effective if \ref environmentProfile() hasn't been overridden.
     *
     * @param profile The name of profile to set.
     */
    void setEnvironmentProfile( const QString& profile );

    virtual void start();

protected:
    virtual bool doKill();

protected slots:
    // Redefine these functions if you want to post-process the output somehow
    // before it hits the output model.
    // Default implementations for either function call "model()->appendLines( lines );".
    // Do the same if you need the output to be visible.
    virtual void postProcessStdout( const QStringList& lines );
    virtual void postProcessStderr( const QStringList& lines );

    // Redefine these functions if you want to handle process' exit codes in a special manner.
    // One possible usage is in "cvs diff" job which returns 1 on success.
    virtual void childProcessExited( int exitCode, QProcess::ExitStatus exitStatus );
    virtual void childProcessError( QProcess::ProcessError processError );

private:
    OutputExecuteJobPrivate* d;

    Q_PRIVATE_SLOT(d, void childProcessStdout());
    Q_PRIVATE_SLOT(d, void childProcessStderr());
};

} // namespace KDevelop

Q_DECLARE_OPERATORS_FOR_FLAGS(KDevelop::OutputExecuteJob::JobProperties);

#endif // KDEVPLATFORM_OUTPUTEXECUTEJOB_H
