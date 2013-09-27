/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "dummybsm.h"

#include <interfaces/iplugincontroller.h>
#include <QDebug>

K_PLUGIN_FACTORY(DummyBSMFactory, registerPlugin<DummyBSM>(); )

DummyBSM::DummyBSM(QObject *parent, const QVariantList& args, const KDevelop::Path::List& cf)
    : KDevelop::IPlugin( DummyBSMFactory::componentData(), parent ), m_controlledFiles(cf)
{
    Q_UNUSED( args );
}

KDevelop::ProjectFolderItem* DummyBSM::import(KDevelop::IProject *project )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    m_folder=new KDevelop::ProjectFolderItem(project, KDevelop::Path("/"), 0);
    m_target=new KDevelop::ProjectTargetItem(project, "standard", m_folder);
    qDebug() << "importing" << m_controlledFiles;
    foreach(const KDevelop::Path& header, m_controlledFiles)
    {
        KDevelop::Path cpp(header.parent(), header.lastPathSegment().replace(".h", ".cpp"));
        new KDevelop::ProjectFileItem(project, header, m_target);
        new KDevelop::ProjectFileItem(project, cpp, m_target);
        qDebug() << "adding file to dummy" << header << cpp;
    }
    return m_folder;
}

QList<KDevelop::ProjectTargetItem*> DummyBSM::targets() const
{
    return QList<KDevelop::ProjectTargetItem*>() << m_target;
}

QList<KDevelop::ProjectTargetItem*> DummyBSM::targets(KDevelop::ProjectFolderItem *folder) const
{
    QList<KDevelop::ProjectTargetItem*> ret;
    if(folder==m_folder)
        ret << m_target;
    return ret;
}

#include "dummybsm.moc"
