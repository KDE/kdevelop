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

#include <QtCore/QStringList>
#include <QtCore/QProcess>

#include <KDebug>
#include <kshell.h>
#include <kglobal.h>
#include <klocale.h>
#include <kprocess.h>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <util/environmentgrouplist.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <util/processlinemaker.h>
#include <interfaces/iproject.h>

#include "makeoutputdelegate.h"
#include "makeoutputmodel.h"
#include "makebuilder.h"

using namespace KDevelop;

MakeJob::MakeJob(MakeBuilder* builder, KDevelop::ProjectBaseItem* item, CommandType c,  const QString& overrideTarget )
    : OutputJob(builder)
    , m_builder(builder)
    , m_item(item)
    , m_command(c)
    , m_overrideTarget(overrideTarget)
    , m_lineMaker(0)
    , m_process(0)
    , m_killed(false)
    , firstError(false)
{
    setCapabilities(Killable);

    QString title;
    if( !m_overrideTarget.isEmpty() )
        title = i18n("Make: %1", m_overrideTarget);
    else
        title = i18n("Make: %1", m_item->text());

    setTitle(title);
    setObjectName(title);
}

void MakeJob::start()
{
    kDebug(9037) << "Building with make" << m_command << m_overrideTarget;

    if( m_item->type() == KDevelop::ProjectBaseItem::File ) {
        setError(IncorrectItemError);
        setErrorText("Internal error: cannot build a file item");
        return emitResult();
    }

    KUrl buildDir = computeBuildDir(m_item);
    if( !buildDir.isValid() ) {
        setError(InvalidBuildDirectoryError);
        setErrorText(i18n("Invalid build directory '%1'", buildDir.prettyUrl()));
        return emitResult();
    }

    QStringList cmd = computeBuildCommand();
    if( cmd.isEmpty() ) {
        setError(BuildCommandError);
        setErrorText(i18n("Could not create build command for target '%1'", m_overrideTarget));
        return emitResult();
    }

    setStandardToolView(IOutputView::BuildView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);

    setModel(new MakeOutputModel(), IOutputView::TakeOwnership);
    setDelegate(m_builder->delegate());

    startOutput();

    model()->addLine( buildDir.toLocalFile() + "> " + cmd.join(" ") );

    QString command = cmd.first();
    cmd.pop_front();

    m_process = new KProcess(this);
    m_process->setOutputChannelMode( KProcess::MergedChannels );
    m_lineMaker = new ProcessLineMaker( m_process );
    connect( m_lineMaker, SIGNAL(receivedStdoutLines( const QStringList& ) ),
             this, SLOT( addStandardOutput( const QStringList& ) ) );
    connect( m_process, SIGNAL( error( QProcess::ProcessError ) ),
             this, SLOT( procError( QProcess::ProcessError ) ) );
    connect( m_process, SIGNAL( finished( int, QProcess::ExitStatus ) ),
             this, SLOT( procFinished( int, QProcess::ExitStatus ) ) );

    Q_FOREACH( QString s, environmentVars().keys() )
    {
        m_process->setEnv( s, environmentVars()[s] );
    }

    m_process->setWorkingDirectory( buildDir.toLocalFile() );
    m_process->setProgram( command, cmd );
    kDebug(9037) << "Starting build:" << cmd << "Build directory" << buildDir;
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

const QString & MakeJob::customTarget() const
{
    return m_overrideTarget;
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
//     QString cmdline = DomUtil::readEntry(dom, makeTool);
//     int prio = DomUtil::readIntEntry(dom, priority);
//     QString nice;
//     if (prio != 0) {
//         nice = QString("nice -n%1 ").arg(prio);
//     }

    KSharedConfig::Ptr configPtr = m_item->project()->projectConfiguration();
    KConfigGroup builderGroup( configPtr, "MakeBuilder" );

    QString makeBin = builderGroup.readEntry("Make Binary", "make");
    cmdline << makeBin;

    if( ! builderGroup.readEntry("Abort on First Error", true) )
    {
        cmdline << "-k";
    }
    if( builderGroup.readEntry("Run Multiple Jobs", false ) )
    {
        int jobnumber = builderGroup.readEntry("Number Of Jobs", 1);
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
        foreach(const QString option, KShell::splitArgs( extraOptions ) )
            cmdline << option;
    }

    if( m_overrideTarget.isEmpty() )
    {
        if( m_item->type() == KDevelop::ProjectBaseItem::Target )
        {
            KDevelop::ProjectTargetItem* targetItem = static_cast<KDevelop::ProjectTargetItem*>(m_item);
            cmdline << targetItem->text();
        }
        else if( m_item->type() == KDevelop::ProjectBaseItem::BuildFolder )
        {
            QString target = builderGroup.readEntry("Default Target", QString());
            if( !target.isEmpty() )
                cmdline << target;
        }
    }else
    {
        cmdline << m_overrideTarget;
    }

    return cmdline;
}

QMap<QString, QString> MakeJob::environmentVars() const
{
    KSharedConfig::Ptr configPtr = m_item->project()->projectConfiguration();
    KConfigGroup builderGroup( configPtr, "MakeBuilder" );
    QString defaultProfile = builderGroup.readEntry(
            "Default Make Environment Profile", "default" );

    QStringList procDefaultList = QProcess::systemEnvironment();
    QMap<QString, QString> retMap;
    foreach( QString _line, procDefaultList )
    {
        QString varName = _line.section( '=', 0, 0 );
        QString varValue = _line.section( '=', 1 );
        retMap.insert( varName, varValue );
    }
    if( defaultProfile.isEmpty() )
        return retMap;

    const KDevelop::EnvironmentGroupList l(configPtr);
    const QMap<QString, QString> userMap = l.variables( defaultProfile );

    for( QMap<QString, QString>::const_iterator it = userMap.begin();
         it != userMap.end(); ++it )
    {
        retMap.insert( it.key(), it.value() );
    }

    return retMap;
}

void MakeJob::addStandardOutput( const QStringList& lines )
{
    model()->addLines( lines );
}

void MakeJob::procError( QProcess::ProcessError error )
{
    //This slot might be called twice for a given process, once when a real error 
    //occurs and then again when the process has exited due to the error
    //via procFinished() slot. So make sure we run the code only once.
    if( error() )
    {
        return;
    } 
    Q_UNUSED(error)
    m_lineMaker->flushBuffers();

    if (!m_killed) {
        setError(FailedError);
        setErrorText(i18n("Job failed"));
        model()->addLine( i18n("*** Failed ***") );

    } else {
        model()->addLine( i18n("*** Aborted ***") );
    }
    emitResult();
}
void MakeJob::procFinished( int code, QProcess::ExitStatus status )
{
    Q_UNUSED(code)
    m_lineMaker->flushBuffers();
    if( code==0 && status == QProcess::NormalExit )
    {
        model()->addLine( i18n("*** Finished ***") );
        emitResult();
    }
    else
    {
        procError(QProcess::UnknownError);
    }
}

bool MakeJob::doKill()
{
    m_killed = true;
    m_process->kill();
    return true;
}

MakeOutputModel* MakeJob::model() const
{
    return dynamic_cast<MakeOutputModel*>( OutputJob::model() );
}

void MakeJob::setItem( ProjectBaseItem* item )
{
    m_item = item;
}


#include "makejob.moc"
