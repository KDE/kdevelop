/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "kdeprojectsmodel.h"
#include <KIcon>
#include <KUrl>
#include <QDebug>
#include <vcs/vcslocation.h>

using namespace KDevelop;

namespace
{
typedef QPair<QString, QString> QSP;
const uint nOfKDEProjects = 13;
Source kdeProjects[nOfKDEProjects] = {
    //SVN - kde modules
    Source("applications-system", "kdebase", Source::SVN, QSP("svn", "trunk/KDE/kdebase")),
    Source("", "kdebindings", Source::SVN, QSP("svn", "trunk/KDE/kdebindings")),
    Source("applications-education-university", "kdeedu", Source::SVN, QSP("svn", "trunk/KDE/kdeedu")),
    Source("applications-games", "kdegames", Source::SVN, QSP("svn", "trunk/KDE/kdegames")),
    Source("applications-graphics", "kdegraphics", Source::SVN, QSP("svn", "trunk/KDE/kdegraphics")),
    Source("", "kdelibs", Source::SVN, QSP("svn", "trunk/KDE/kdelibs")),
    Source("applications-multimedia", "kdemultimedia", Source::SVN, QSP("svn", "trunk/KDE/kdemultimedia")),
    Source("applications-internet", "kdenetwork", Source::SVN, QSP("svn", "trunk/KDE/kdenetwork")),
    Source("kontact", "kdepim", Source::SVN, QSP("svn", "trunk/KDE/kdepim")),
    Source("", "kdepimlibs", Source::SVN, QSP("svn", "trunk/KDE/kdepimlibs")),
    Source("plasma", "kdeplasma-addons", Source::SVN, QSP("svn", "trunk/KDE/kdeplasma-addons")),
    Source("applications-development", "kdesdk", Source::SVN, QSP("svn", "trunk/KDE/kdesdk")),
    Source("", "kdesupport", Source::SVN, QSP("svn", "trunk/kdesupport"))
};

}

SourceItem::SourceItem(const Source& s)
    : QStandardItem(KIcon(s.icon), s.name), m_s(s)
{
    Q_ASSERT(!m_s.name.isEmpty());
}

QVariant SourceItem::data(int role) const
{
    switch(role)
    {
        case KDEProjectsModel::VcsLocationRole:
            return QVariantMap(m_s.m_urls);
        case KDEProjectsModel::PluginRole:
            return m_s.plugin();
        default:
            return QStandardItem::data(role);
    }
}

Source::Source(const QString& anIcon, const QString& aName, Source::VcsType aType, const QPair<QString, QString>& aUrl)
    : name(aName), icon(anIcon), type(aType)
{
    m_urls[aUrl.first] = aUrl.second;
}

KDEProjectsModel::KDEProjectsModel(QObject* parent): QStandardItemModel(parent)
{
    for(uint i=0; i<nOfKDEProjects; ++i) {
        QStandardItem* item = new SourceItem(kdeProjects[i]);
        
        appendRow(item);
    }
}
