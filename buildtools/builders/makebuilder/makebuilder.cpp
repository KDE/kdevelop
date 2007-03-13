/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

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

#include <config.h>

#include <QtCore/QStringList>

#include "makebuilder.h"
#include <projectmodel.h>

#include <iproject.h>
#include <icore.h>
#include <iplugincontroller.h>
#include <ioutputview.h>
#include <QtDesigner/QExtensionFactory>

#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#define MAKE_COMMAND "make"

typedef KGenericFactory<MakeBuilder> MakeBuilderFactory ;
K_EXPORT_COMPONENT_FACTORY(kdevmakebuilder, MakeBuilderFactory("kdevmakebuilder"))

KDEV_ADD_EXTENSION_FACTORY( IMakeBuilder, MakeBuilder )
KDEV_ADD_EXTENSION_FACTORY_NS( KDevelop, IProjectBuilder, MakeBuilder )

MakeBuilder::MakeBuilder(QObject *parent, const QStringList &)
    : KDevelop::IPlugin(MakeBuilderFactory::componentData(), parent)
{
    IPlugin* i = core()->pluginController()->pluginForExtension("IOutputView");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {
            connect(i, SIGNAL(commandFinished(const QStringList &)),
                this, SLOT(commandFinished(const QStringList &)));
            connect(i, SIGNAL(commandFailed(const QStringList &)),
                this, SLOT(commandFailed(const QStringList &)));
        }
    }
}

MakeBuilder::~MakeBuilder()
{
}

bool MakeBuilder::build( KDevelop::ProjectBaseItem *dom )
{
    kDebug(9038) << "Building with make" << endl;
    if( dom->type() != KDevelop::ProjectBaseItem::Project )
        return false;
    KDevelop::ProjectItem* item = static_cast<KDevelop::ProjectItem*>(dom);
    IPlugin* i = core()->pluginController()->pluginForExtension("IOutputView");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {
            QStringList cmd = buildCommand(item);
            m_queue << QPair<QStringList, KDevelop::ProjectBaseItem*>( cmd, dom );
            kDebug(9038) << "Starting build: " << cmd << endl;
            view->queueCommand( item->url(), cmd, QStringList() );
            return true;
        }
    }
    return false;

}

bool MakeBuilder::clean( KDevelop::ProjectBaseItem *dom )
{
    Q_UNUSED(dom);
    return false;
}

void MakeBuilder::commandFinished(const QStringList &command)
{
    if( !m_queue.isEmpty() )
    {
        QPair< QStringList, KDevelop::ProjectBaseItem* > pair = m_queue.front();

        if( pair.first == command )
        {
            m_queue.pop_front();
            emit built( pair.second );
        }
    }
}

void MakeBuilder::commandFailed(const QStringList &command)
{
    if( !m_queue.isEmpty() )
    {
        QPair<QStringList, KDevelop::ProjectBaseItem*> pair = m_queue.front();
        if( pair.first == command )
        {
            m_queue.pop_front();
            emit failed(pair.second);
        }
    }
}

QStringList MakeBuilder::buildCommand( KDevelop::ProjectBaseItem *item,
                                       const QString &target )
{
    //FIXME Get this from the new project file format
//     QDomDocument &dom = *KDevApi::self()->projectDom();

    QStringList cmdline;
//     QString cmdline = DomUtil::readEntry(dom, makeTool);
//     int prio = DomUtil::readIntEntry(dom, priority);
//     QString nice;
//     if (prio != 0) {
//         nice = QString("nice -n%1 ").arg(prio);
//     }

    if (cmdline.isEmpty())
        cmdline << MAKE_COMMAND;

//     if (!DomUtil::readBoolEntry(dom, abortOnError))
//         cmdline += " -k";
//     int jobs = DomUtil::readIntEntry(dom, numberOfJobs);
//     if (jobs != 0) {
//         cmdline += " -j";
//         cmdline += QString::number(jobs);
//     }
//     if (DomUtil::readBoolEntry(dom, dontAct))
//         cmdline += " -n";

    cmdline << target;

//     cmdline.prepend(nice);
//     cmdline.prepend(makeEnvironment());

    Q_ASSERT(item->folder());

//     cmdline.prepend("&&");
//     cmdline.prepend( item->folder()->text() );
//     cmdline.prepend("cd");

    return cmdline;
}

QStringList MakeBuilder::extensions() const
{
    return QStringList() << "IMakeBuilder" << "IProjectBuilder";
}


void MakeBuilder::registerExtensions()
{
    extensionManager()->registerExtensions( new MakeBuilderIMakeBuilderFactory(
    extensionManager() ), Q_TYPEID( IMakeBuilder ) );
    extensionManager()->registerExtensions( new MakeBuilderIProjectBuilderFactory(
    extensionManager() ), Q_TYPEID( KDevelop::IProjectBuilder ) );
}
void MakeBuilder::unregisterExtensions()
{
    extensionManager()->unregisterExtensions( new MakeBuilderIMakeBuilderFactory(
    extensionManager() ), Q_TYPEID( IMakeBuilder ) );
    extensionManager()->unregisterExtensions( new MakeBuilderIProjectBuilderFactory(
    extensionManager() ), Q_TYPEID( KDevelop::IProjectBuilder ) );
}


#include "makebuilder.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
