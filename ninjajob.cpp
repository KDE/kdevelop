/* This file is part of KDevelop
    Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ninjajob.h"
#include <KProcess>
#include <KUrl>
#include <KDebug>
#include <KLocalizedString>
#include <outputview/outputmodel.h>
#include <outputview/outputdelegate.h>
#include <util/commandexecutor.h>

NinjaJob::NinjaJob(const KUrl& dir, const QStringList& arguments, QObject* parent)
    : OutputJob(parent, Verbose)
    , m_lastLine(false)
    , m_item(0)
{
    Q_ASSERT(!dir.isRelative() && !dir.isEmpty());
    setToolTitle("Ninja");
    setCapabilities(Killable);
    setStandardToolView( KDevelop::IOutputView::BuildView );
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
    setObjectName("ninja "+arguments.join(" "));
    setDelegate(new KDevelop::OutputDelegate);
 
    m_process = new KDevelop::CommandExecutor("ninja", this);
    m_process->setArguments( arguments );
    m_process->setWorkingDirectory(dir.toLocalFile(KUrl::RemoveTrailingSlash));
    
    m_model = new KDevelop::OutputModel(dir, this);
    setModel( m_model );
    m_model->setFilteringStrategy(KDevelop::OutputModel::CompilerFilter);

    connect(m_process, SIGNAL(receivedStandardError(QStringList)),
            SLOT(appendLines(QStringList)) );
    connect(m_process, SIGNAL(receivedStandardOutput(QStringList)),
            SLOT(appendLines(QStringList)) );
    
    connect( m_process, SIGNAL(failed(QProcess::ProcessError)), this, SLOT(slotFailed(QProcess::ProcessError)) );
    connect( m_process, SIGNAL(completed()), this, SLOT(slotCompleted()) );
}

void NinjaJob::signalWhenFinished(const QByteArray& signal, KDevelop::ProjectBaseItem* item)
{
    m_signal = signal;
    m_item = item;
    connect(this, SIGNAL(finished(KJob*)), SLOT(emitProjectBuilderSignal(KJob*)));
}

void NinjaJob::emitProjectBuilderSignal(KJob* job)
{
    Q_ASSERT(!m_signal.isEmpty());
    if(job->error()==0)
        QMetaObject::invokeMethod(parent(), m_signal, Q_ARG(KDevelop::ProjectBaseItem*, m_item));
    else
        QMetaObject::invokeMethod(parent(), "failed", Q_ARG(KDevelop::ProjectBaseItem*, m_item));
}

void NinjaJob::start()
{
    startOutput();
    kDebug() << "Executing ninja" << m_process->arguments();
    m_model->appendLine( m_process->workingDirectory() + "> " + m_process->command() + " " + m_process->arguments().join(" ") );
    m_process->start();
}

bool NinjaJob::doKill()
{
    m_process->kill();
    m_model->appendLine( i18n("*** Aborted ***") );
    return true;
}

void NinjaJob::slotCompleted()
{
    emitResult();
    m_model->appendLine( i18n("*** Finished ***") );
}

void NinjaJob::slotFailed(QProcess::ProcessError error)
{
    Q_UNUSED( error )
    setError(Failed);
    // FIXME need more detail
    setErrorText(i18n("Ninja failed to compile %1", m_process->workingDirectory()));
    m_model->appendLine( i18n("*** Failed ***") );
    emitResult();
}

void NinjaJob::appendLines(const QStringList& lines)
{
    if(lines.isEmpty())
        return;
    
    QStringList ret(lines);
    bool prev = false;
    for(QStringList::iterator it=ret.end(); it!=ret.begin(); ) {
        --it;
        bool curr = it->startsWith('[');
        if(prev && curr)
            it = ret.erase(it);
        prev = curr;
    }
    
    if(m_lastLine && ret.first().startsWith('['))
        m_model->removeLastLines(1);
    m_lastLine = ret.last().startsWith('[');
    m_model->appendLines(ret);
}
