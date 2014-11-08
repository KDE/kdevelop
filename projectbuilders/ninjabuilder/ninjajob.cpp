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
#include "kdevninjabuilderplugin.h"

#include <KProcess>
#include <QUrl>
#include <KLocalizedString>
#include <KConfigGroup>
#include <interfaces/iproject.h>
#include <outputview/outputmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <QFile>

NinjaJob::NinjaJob(KDevelop::ProjectBaseItem* item, const QStringList& arguments, const QByteArray& signal, KDevNinjaBuilderPlugin* parent)
    : OutputExecuteJob(parent)
    , m_isInstalling(false)
    , m_idx(item->index())
    , m_signal(signal)
    , m_plugin(parent)
{
    setToolTitle(i18n("Ninja"));
    setCapabilities(Killable);
    setStandardToolView( KDevelop::IOutputView::BuildView );
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
    setFilteringStrategy( KDevelop::OutputModel::CompilerFilter );
    setProperties( NeedWorkingDirectory | PortableMessages | DisplayStderr | IsBuilderHint | PostProcessOutput );

    *this << ninjaBinary();
    *this << arguments;

    QStringList targets;
    foreach( const QString& arg, arguments ) {
        if( !arg.startsWith( '-' ) ) {
            targets << arg;
        }
    }
    QString title;
    if( !targets.isEmpty() )
        title = i18n("Ninja (%1): %2", item->text(), targets.join(" "));
    else
        title = i18n("Ninja (%1)", item->text());
    setJobName( title );

    connect(this, &NinjaJob::finished, this, &NinjaJob::emitProjectBuilderSignal);
}

void NinjaJob::setIsInstalling( bool isInstalling )
{
    m_isInstalling = isInstalling;
}

QString NinjaJob::ninjaBinary()
{
    QString path = QStandardPaths::findExecutable("ninja-build");
    if (path.isEmpty()) {
        path = QStandardPaths::findExecutable("ninja");
    }
    return path;
}

QUrl NinjaJob::workingDirectory() const
{
    KDevelop::ProjectBaseItem* it = item();
    if(!it)
        return QUrl();
    KDevelop::IBuildSystemManager* bsm = it->project()->buildSystemManager();
    KDevelop::Path workingDir = bsm->buildDirectory( it );
    while( !QFile::exists( workingDir.toLocalFile() + "build.ninja" ) ) {
        KDevelop::Path upWorkingDir = workingDir.parent();
        if( !upWorkingDir.isValid() || upWorkingDir == workingDir ) {
            return bsm->buildDirectory( it->project()->projectItem() ).toUrl();
        }
        workingDir = upWorkingDir;
    }
    return workingDir.toUrl();
}

QStringList NinjaJob::privilegedExecutionCommand() const
{
    KDevelop::ProjectBaseItem* it = item();
    if(!it)
        return QStringList();
    KSharedConfig::Ptr configPtr = it->project()->projectConfiguration();
    KConfigGroup builderGroup( configPtr, "NinjaBuilder" );

    bool runAsRoot = builderGroup.readEntry( "Install As Root", false );
    if ( runAsRoot && m_isInstalling )
    {
        int suCommand = builderGroup.readEntry( "Su Command", 0 );
        QStringList arguments;
        QString suCommandName;
        switch( suCommand ) {
            case 1:
                return QStringList() << "kdesudo" << "-t";

            case 2:
                return QStringList() << "sudo";

            default:
                return QStringList() << "kdesu" << "-t";
        }
    }
    return QStringList();
}

void NinjaJob::emitProjectBuilderSignal(KJob* job)
{
    Q_ASSERT(!m_signal.isEmpty());

    if (!m_plugin)
        return;

    KDevelop::ProjectBaseItem* it = item();
    if(!it)
        return;

    if(job->error()==0)
        QMetaObject::invokeMethod(m_plugin, m_signal, Q_ARG(KDevelop::ProjectBaseItem*, it));
    else
        QMetaObject::invokeMethod(m_plugin, "failed", Q_ARG(KDevelop::ProjectBaseItem*, it));
}

void NinjaJob::postProcessStderr( const QStringList& lines )
{
    appendLines( lines );
}

void NinjaJob::postProcessStdout( const QStringList& lines )
{
    appendLines( lines );
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
        if((prev && curr) || it->endsWith("] "))
            it = ret.erase(it);
        prev = curr;
    }

    model()->appendLines(ret);
}

KDevelop::ProjectBaseItem* NinjaJob::item() const
{
    return KDevelop::ICore::self()->projectController()->projectModel()->itemFromIndex(m_idx);
}
