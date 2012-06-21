/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
    Copyright 2008 Hamish Rodda <rodda@kde.org>

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

#include "makejob.h"

#include <QtCore/QFileInfo>

#include <KDebug>
#include <KShell>
#include <KGlobal>
#include <KProcess>

#include <util/environmentgrouplist.h>
#include <util/processlinemaker.h>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <outputview/outputdelegate.h>
#include <outputview/outputmodel.h>

#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include "makebuilder.h"

using namespace KDevelop;

MakeJob::MakeJob(MakeBuilder* builder, KDevelop::ProjectBaseItem* item,
                 CommandType c,  const QStringList& overrideTargets,
                 const MakeVariables& variables )
    : OutputJob(builder)
    , m_builder(builder)
    , m_item(item)
    , m_command(c)
    , m_overrideTargets(overrideTargets)
    , m_variables(variables)
    , m_lineMaker(0)
    , m_process(0)
    , m_killed(false)
    , firstError(false)
{
    setCapabilities(Killable);

    QString title;
    if( !m_overrideTargets.isEmpty() )
        title = i18n("Make: %1", m_overrideTargets.join(" "));
    else
        title = i18n("Make: %1", m_item->text());

    setTitle(title);
    setObjectName(title);
}

MakeJob::~MakeJob()
{
    if(!m_killed && m_process && m_process->state() != KProcess::NotRunning)
    {
        m_process->kill();
        m_process->waitForFinished();
    }
    Q_ASSERT(!m_process || m_process->state() == KProcess::NotRunning);
}

void MakeJob::start()
{
    kDebug(9037) << "Building with make" << m_command << m_overrideTargets.join(" ");
    if (!m_item)
    {
        setError(ItemNoLongerValidError);
        setErrorText(i18n("Build item no longer available"));
        return emitResult();
    }

    if( m_item->type() == KDevelop::ProjectBaseItem::File ) {
        setError(IncorrectItemError);
        setErrorText(i18n("Internal error: cannot build a file item"));
        return emitResult();
    }

    KUrl buildDir = computeBuildDir(m_item);
    if( !buildDir.isValid() ) {
        setError(InvalidBuildDirectoryError);
        setErrorText(i18n("Invalid build directory '%1'", buildDir.prettyUrl()));
        return emitResult();
    }
    else if( !buildDir.isLocalFile() ) {
        setError(InvalidBuildDirectoryError);
        setErrorText(i18n("'%1' is not a local path", buildDir.prettyUrl()));
        return emitResult();
    }
    else if ( !QFileInfo(buildDir.toLocalFile()).isDir() ) {
        setError(InvalidBuildDirectoryError);
        setErrorText(i18n("'%1' is not a directory", buildDir.prettyUrl()));
        return emitResult();
    }

    QStringList cmd = computeBuildCommand();
    if( cmd.isEmpty() ) {
        setError(BuildCommandError);
        setErrorText(i18n("Could not create build command for targets '%1'", m_overrideTargets.join(" ")));
        return emitResult();
    }

    setStandardToolView(IOutputView::BuildView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);

    setModel(new KDevelop::OutputModel(buildDir), IOutputView::TakeOwnership);
    model()->setFilteringStrategy(KDevelop::OutputModel::CompilerFilter);
    setDelegate(m_builder->delegate());

    startOutput();

    model()->appendLine( buildDir.toLocalFile() + "> " + cmd.join(" ") );

    QString command = cmd.first();
    cmd.pop_front();

    m_process = new KProcess(this);
    m_process->setOutputChannelMode( KProcess::MergedChannels );
    m_lineMaker = new ProcessLineMaker( m_process );
    connect( m_lineMaker, SIGNAL(receivedStdoutLines(QStringList)),
             this, SLOT(addStandardOutput(QStringList)) );
    connect( m_process, SIGNAL(error(QProcess::ProcessError)),
             this, SLOT(procError(QProcess::ProcessError)) );
    connect( m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
             this, SLOT(procFinished(int,QProcess::ExitStatus)) );

    m_process->setEnvironment( environmentVars() );
    m_process->setWorkingDirectory( buildDir.toLocalFile() );
    m_process->setProgram( command, cmd );
    kDebug(9037) << "Starting build:" << command << cmd << "Build directory" << buildDir;
    m_process->start();
}

KDevelop::ProjectBaseItem * MakeJob::item() const
{
    return m_item;
}

MakeJob::CommandType MakeJob::commandType()
{
    return m_command;
}

QStringList MakeJob::customTargets() const
{
    return m_overrideTargets;
}

KUrl MakeJob::computeBuildDir(KDevelop::ProjectBaseItem* item) const
{
    KUrl buildDir;
    KDevelop::IBuildSystemManager *bldMan = item->project()->buildSystemManager();
    if( bldMan )
        buildDir = bldMan->buildDirectory( item ); // the correct build dir
    else
    {
        switch( item->type() )
        {
            case KDevelop::ProjectBaseItem::Folder:
            case KDevelop::ProjectBaseItem::BuildFolder:
                return static_cast<KDevelop::ProjectFolderItem*>(item)->url();
                break;
            case KDevelop::ProjectBaseItem::Target:
            case KDevelop::ProjectBaseItem::File:
                buildDir = computeBuildDir( static_cast<KDevelop::ProjectBaseItem*>( item->parent() ) );
                break;
        }
    }
    return buildDir;
}

QStringList MakeJob::computeBuildCommand() const
{
    QStringList cmdline;

    KSharedConfig::Ptr configPtr = m_item->project()->projectConfiguration();
    KConfigGroup builderGroup( configPtr, "MakeBuilder" );

#ifdef _MSC_VER
    QString makeBin = builderGroup.readEntry("Make Binary", "nmake");
#else
    QString makeBin = builderGroup.readEntry("Make Binary", "make");
#endif
    cmdline << makeBin;

    if( ! builderGroup.readEntry("Abort on First Error", true) )
    {
        cmdline << "-k";
    }

    int jobnumber = builderGroup.readEntry("Number Of Jobs", 1);
    if(jobnumber>1) {
        QString jobNumberArg = QString("-j%1").arg(jobnumber);
        cmdline << jobNumberArg;
    }

    if( builderGroup.readEntry("Display Only", false) )
    {
        cmdline << "-n";
    }

    QString extraOptions = builderGroup.readEntry("Additional Options", QString());
    if( ! extraOptions.isEmpty() )
    {
        foreach(const QString& option, KShell::splitArgs( extraOptions ) )
            cmdline << option;
    }

    MakeVariables::const_iterator it = m_variables.constBegin();
    while ( it != m_variables.constEnd() )
    {
        cmdline += QString("%1=%2").arg(it->first).arg(it->second);
        ++it;
    }

    if( m_overrideTargets.isEmpty() )
    {
        QString target;
        switch (m_item->type()) {
            case KDevelop::ProjectBaseItem::Target:
            case KDevelop::ProjectBaseItem::ExecutableTarget:
            case KDevelop::ProjectBaseItem::LibraryTarget:
                Q_ASSERT(m_item->target());
                cmdline << m_item->target()->text();
                break;
            case KDevelop::ProjectBaseItem::BuildFolder:
                target = builderGroup.readEntry("Default Target", QString());
                if( !target.isEmpty() )
                    cmdline << target;
                break;
            default: break;
        }
    }else
    {
        cmdline += m_overrideTargets;
    }

    bool runAsRoot = builderGroup.readEntry("Install As Root", false);
    if (runAsRoot && m_command == InstallCommand)
    {
        int suCommand = builderGroup.readEntry("Su Command", 0);
        QStringList arguments;
        QString suCommandName;
        if (suCommand == 1) {
          suCommandName = "kdesudo";
          arguments << "-t" << "--" << cmdline;
        } else if (suCommand == 2) {
          suCommandName = "sudo";
          arguments << cmdline;
        } else { //default
          suCommandName = "kdesu";
          arguments << "-t" << "--" << cmdline;
        }
        cmdline = QStringList() << suCommandName << arguments;
    }

    return cmdline;
}

QStringList MakeJob::environmentVars() const
{
    KSharedConfig::Ptr configPtr = m_item->project()->projectConfiguration();
    KConfigGroup builderGroup( configPtr, "MakeBuilder" );
    QString defaultProfile = builderGroup.readEntry(
            "Default Make Environment Profile", "default" );

    const KDevelop::EnvironmentGroupList l(KGlobal::config());
    QStringList env = QProcess::systemEnvironment();
    QStringList::iterator it, end = env.end();
    for( it = env.begin(); it != end; it++ ) {
        if( (*it).startsWith("LC_MESSAGES") || (*it).startsWith("LC_ALL") ) {
            env.erase( it );
        }
    }
    env.append( "LC_MESSAGES=C" );
    return l.createEnvironment( defaultProfile, env );
}

void MakeJob::addStandardOutput( const QStringList& lines )
{
    model()->appendLines( lines );
}

void MakeJob::procError( QProcess::ProcessError err )
{
    //This slot might be called twice for a given process, once when a real error 
    //occurs and then again when the process has exited due to the error
    //via procFinished() slot. So make sure we run the code only once.
    if( error() )
    {
        return;
    } 
    Q_UNUSED(err)
    m_lineMaker->flushBuffers();

    if (!m_killed) {
        setError(FailedError);
        setErrorText(i18n("Job failed"));
        model()->appendLine( i18n("*** Failed ***") );

    }
    emitResult();
}

void MakeJob::procFinished( int code, QProcess::ExitStatus status )
{
    Q_UNUSED(code)
    m_lineMaker->flushBuffers();
    if( code==0 && status == QProcess::NormalExit )
    {
        model()->appendLine( i18n("*** Finished ***") );
        emitResult();
    }
    else
    {
        procError(QProcess::UnknownError);
    }
}

bool MakeJob::doKill()
{
    model()->appendLine( i18n("*** Aborted ***") );
    m_killed = true;
    m_process->kill();
    m_process->waitForFinished();
    return true;
}

KDevelop::OutputModel* MakeJob::model() const
{
    return dynamic_cast<KDevelop::OutputModel*>( OutputJob::model() );
}

void MakeJob::setItem( ProjectBaseItem* item )
{
    m_item = item;
}

#include "makejob.moc"
