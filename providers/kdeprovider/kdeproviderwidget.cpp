/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include "kdeproviderwidget.h"
#include <QVBoxLayout>
#include <QComboBox>
#include <KIcon>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <vcs/interfaces/ibasicversioncontrol.h>

struct Source
{
    enum VcsType { Git, SVN };
    Source(const QString& anIcon, const QString& aName, VcsType aType, const QString& aUrl)
    : name(aName), icon(anIcon), type(aType), url(aUrl) {}
    
    QString name;
    QString icon;
    VcsType type;
    QString url;
};

using namespace KDevelop;

namespace
{

const uint nOfKDEProjects = 15;
Source kdeProjects[nOfKDEProjects] = {
    //SVN - kde modules
    Source("applications-system", "kdebase", Source::SVN, "%PREFIX/trunk/KDE/kdebase"),
    Source("kdebindings", "kdebindings", Source::SVN, "%PREFIX/trunk/KDE/kdebindings"),
    Source("applications-education-university", "kdeedu", Source::SVN, "%PREFIX/trunk/KDE/kdeedu"),
    Source("applications-games", "kdegames", Source::SVN, "%PREFIX/trunk/KDE/kdegames"),
    Source("applications-graphics", "kdegraphics", Source::SVN, "%PREFIX/trunk/KDE/kdegraphics"),
    Source("kdelibs", "kdelibs", Source::SVN, "%PREFIX/trunk/KDE/kdelibs"),
    Source("applications-multimedia", "kdemultimedia", Source::SVN, "%PREFIX/trunk/KDE/kdemultimedia"),
    Source("applications-internet", "kdenetwork", Source::SVN, "%PREFIX/trunk/KDE/kdenetwork"),
    Source("kontact", "kdepim", Source::SVN, "%PREFIX/trunk/KDE/kdepim"),
    Source("kdepimlibs", "kdepimlibs", Source::SVN, "%PREFIX/trunk/KDE/kdepimlibs"),
    Source("plasma", "kdeplasma-addons", Source::SVN, "%PREFIX/trunk/KDE/kdeplasma-addons"),
    Source("applications-development", "kdesdk", Source::SVN, "%PREFIX/trunk/KDE/kdesdk"),
    Source("kdesupport", "kdesupport", Source::SVN, "%PREFIX/trunk/kdesupport"),
    
    //GIT
    Source("kdevplatform", "kdevplatform", Source::Git, "git@gitorious.org:kdevplatform/kdevplatform.git"),
    Source("kdevelop", "kdevelop", Source::Git, "git@gitorious.org:kdevelop/kdevelop.git")
};
}

KDEProviderWidget::KDEProviderWidget(QWidget* parent)
    : IProjectProviderWidget(parent)
{
    setLayout(new QVBoxLayout(this));
    m_projects = new QComboBox(this);
    for(uint i=0; i<nOfKDEProjects; ++i) {
        const Source& s = kdeProjects[i];
        m_projects->addItem(KIcon(s.icon), s.name);
    }
    layout()->addWidget(m_projects);
}

VcsLocation svnLocation(const Source& s)
{
    QString svnPrefix="svn+ssh://apol@svn.kde.org/home/kde";
    QString path = QString(s.url).replace("%PREFIX", svnPrefix);
    return VcsLocation(path);
}

VcsJob* KDEProviderWidget::createWorkingCopy(const KUrl& destinationDirectory)
{
    int pos = m_projects->currentIndex();
    if(pos<0)
        return 0;
    
    VcsJob* ret=0;
    const Source& s = kdeProjects[pos];
    if(s.type==Source::SVN) {
        IPlugin* plugin = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IBasicVersionControl", "kdevsubversion");
        
        IBasicVersionControl* vcIface = plugin->extension<IBasicVersionControl>();
        ret = vcIface->createWorkingCopy(svnLocation(s), destinationDirectory);
    } else { //Git
        IPlugin* plugin = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IBasicVersionControl", "kdevgit");
        IBasicVersionControl* vcIface = plugin->extension<IBasicVersionControl>();
        
        ret = vcIface->createWorkingCopy(VcsLocation(KUrl(s.url)), destinationDirectory);
    }
    return ret;
}

