/*
* GDB Debugger Support
*
* Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
* Copyright 2007 Hamish Rodda <rodda@kde.org>
* Copyright 2009 Andreas Pakulat <apaku@gmx.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as
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

#include "debugjob.h"
#include "debuggerplugin.h"
#include <interfaces/ilaunchconfiguration.h>
#include <util/environmentgrouplist.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <klocalizedstring.h>
#include <outputview/outputmodel.h>
#include <execute/iexecuteplugin.h>
#include "debugsession.h"
#include "debug.h"

#include <QFileInfo>
#include <KI18n/KLocalizedString>

using namespace GDBDebugger;
using namespace KDevelop;

DebugJob::DebugJob( GDBDebugger::CppDebuggerPlugin* p, KDevelop::ILaunchConfiguration* launchcfg, IExecutePlugin* execute, QObject* parent)
    : KDevelop::OutputJob(parent)
    , m_launchcfg( launchcfg )
    , m_execute( execute )
{
    setCapabilities(Killable);

    m_session = p->createSession();
    connect(m_session, &DebugSession::applicationStandardOutputLines, this, &DebugJob::stderrReceived);
    connect(m_session, &DebugSession::applicationStandardErrorLines, this, &DebugJob::stdoutReceived);
    connect(m_session, &DebugSession::finished, this, &DebugJob::done );

    if (launchcfg->project()) {
        setObjectName(i18nc("ProjectName: run configuration name", "%1: %2", launchcfg->project()->name(), launchcfg->name()));
    } else {
        setObjectName(launchcfg->name());
    }
}

void DebugJob::start()
{
    KConfigGroup grp = m_launchcfg->config();
    KDevelop::EnvironmentGroupList l(KSharedConfig::openConfig());
    Q_ASSERT(m_execute);
    QString err;
    QString executable = m_execute->executable( m_launchcfg, err ).toLocalFile();

    if( !err.isEmpty() )
    {
        setError( -1 );
        setErrorText( err );
        emitResult();
        return;
    }

    if(!QFileInfo(executable).isExecutable()){
        setError( -1 );
        setErrorText(QString("'%1' is not an executable").arg(executable));
        emitResult();
        return;
    }

    QStringList arguments = m_execute->arguments( m_launchcfg, err );
    if( !err.isEmpty() )
    {
        setError( -1 );
        setErrorText( err );
    }
    if( error() != 0 )
    {
        emitResult();
        return;
    }

    setStandardToolView(KDevelop::IOutputView::DebugView);
    setBehaviours(KDevelop::IOutputView::Behaviours(KDevelop::IOutputView::AllowUserClose) | KDevelop::IOutputView::AutoScroll);
    setModel( new KDevelop::OutputModel );
    setTitle(m_launchcfg->name());

    QString startWith = grp.readEntry(GDBDebugger::startWithEntry, QString("ApplicationOutput"));
    if (startWith == "GdbConsole") {
        setVerbosity(Silent);
    } else if (startWith == "FrameStack") {
        setVerbosity(Silent);
    } else {
        setVerbosity(Verbose);
    }

    startOutput();

    m_session->startProgram( m_launchcfg, m_execute );
}

bool DebugJob::doKill()
{
    qCDebug(DEBUGGERGDB);
    m_session->stopDebugger();
    return true;
}

void DebugJob::stderrReceived(const QStringList& l )
{
    if (KDevelop::OutputModel* m = model()) {
        m->appendLines( l );
    }
}

void DebugJob::stdoutReceived(const QStringList& l )
{
    if (KDevelop::OutputModel* m = model()) {
        m->appendLines( l );
    }
}

KDevelop::OutputModel* DebugJob::model()
{
    return dynamic_cast<KDevelop::OutputModel*>( KDevelop::OutputJob::model() );
}


void DebugJob::done()
{
    emitResult();
}


KillSessionJob::KillSessionJob(DebugSession *session, QObject* parent): KJob(parent), m_session(session)
{
    connect(m_session, &DebugSession::finished, this, &KillSessionJob::sessionFinished);
    setCapabilities(Killable);
}

void KillSessionJob::start()
{
    //NOOP
}

bool KillSessionJob::doKill()
{
    m_session->stopDebugger();
    return true;
}

void KillSessionJob::sessionFinished()
{
    emitResult();
}
