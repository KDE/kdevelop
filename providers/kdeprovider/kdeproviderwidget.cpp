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
#include <KPushButton>
#include <KConfigDialog>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include "ui_kdeconfig.h"
#include "kdeconfig.h"

using namespace KDevelop;

struct Source
{
    enum VcsType { Git, SVN };
    Source(const QString& anIcon, const QString& aName, VcsType aType, const QString& aUrl)
    : name(aName), icon(anIcon), type(aType), url(aUrl) {}
    
    QString name;
    QString icon;
    VcsType type;
    QString url;
    
    QString plugin() const { return type==SVN ? "kdevsubversion" : "kdevgit"; }
    
    static VcsLocation svnLocation(const Source& s)
    {
        QString svnPrefix=KDEProviderSettings::self()->svnPrefix();
        QString path = QString(s.url).replace("%PREFIX", svnPrefix);
        return VcsLocation(path);
    }
    VcsLocation location() const { return type==SVN ? svnLocation(*this) : VcsLocation(KUrl(url)); }
};

namespace
{

const uint nOfKDEProjects = 15;
Source kdeProjects[nOfKDEProjects] = {
    //SVN - kde modules
    Source("applications-system", "kdebase", Source::SVN, "%PREFIX/trunk/KDE/kdebase"),
    Source("", "kdebindings", Source::SVN, "%PREFIX/trunk/KDE/kdebindings"),
    Source("applications-education-university", "kdeedu", Source::SVN, "%PREFIX/trunk/KDE/kdeedu"),
    Source("applications-games", "kdegames", Source::SVN, "%PREFIX/trunk/KDE/kdegames"),
    Source("applications-graphics", "kdegraphics", Source::SVN, "%PREFIX/trunk/KDE/kdegraphics"),
    Source("", "kdelibs", Source::SVN, "%PREFIX/trunk/KDE/kdelibs"),
    Source("applications-multimedia", "kdemultimedia", Source::SVN, "%PREFIX/trunk/KDE/kdemultimedia"),
    Source("applications-internet", "kdenetwork", Source::SVN, "%PREFIX/trunk/KDE/kdenetwork"),
    Source("kontact", "kdepim", Source::SVN, "%PREFIX/trunk/KDE/kdepim"),
    Source("", "kdepimlibs", Source::SVN, "%PREFIX/trunk/KDE/kdepimlibs"),
    Source("plasma", "kdeplasma-addons", Source::SVN, "%PREFIX/trunk/KDE/kdeplasma-addons"),
    Source("applications-development", "kdesdk", Source::SVN, "%PREFIX/trunk/KDE/kdesdk"),
    Source("", "kdesupport", Source::SVN, "%PREFIX/trunk/kdesupport"),
    
    //GIT
    Source("kdevelop", "kdevplatform", Source::Git, "git@git.kde.org:kdevplatform"),
    Source("kdevelop", "kdevelop", Source::Git, "git@git.kde.org:kdevelop")
};
}

KDEProviderWidget::KDEProviderWidget(QWidget* parent)
    : IProjectProviderWidget(parent)
{
    setLayout(new QHBoxLayout(this));
    m_projects = new QComboBox(this);
    for(uint i=0; i<nOfKDEProjects; ++i) {
        const Source& s = kdeProjects[i];
        m_projects->addItem(KIcon(s.icon), s.name);
    }
    layout()->addWidget(m_projects);
    
    QPushButton* settings=new QPushButton(KIcon("configure"), i18n("Settings"), this);
    settings->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    connect(settings, SIGNAL(clicked()), SLOT(showSettings()));
    
    layout()->addWidget(settings);
    
    m_dialog = new KConfigDialog(this, "settings", KDEProviderSettings::self());
    m_dialog->setFaceType(KPageDialog::Auto);
    QWidget* page = new QWidget(m_dialog);
    Ui::KDEConfig().setupUi(page);
    
    m_dialog->addPage(page, i18n("General") );
    connect(m_dialog, SIGNAL(settingsChanged(const QString&)), this, SLOT(loadSettings()));
}

VcsJob* KDEProviderWidget::createWorkingCopy(const KUrl& destinationDirectory)
{
    int pos = m_projects->currentIndex();
    if(pos<0)
        return 0;
    
    const Source& s = kdeProjects[pos];
    IPlugin* plugin = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IBasicVersionControl", s.plugin());
    IBasicVersionControl* vcIface = plugin->extension<IBasicVersionControl>();
    VcsJob* ret = vcIface->createWorkingCopy(s.location(), destinationDirectory);
    
    return ret;
}

void KDEProviderWidget::showSettings()
{
    if(KConfigDialog::showDialog("kdesettings"))
        return;
    
    m_dialog->show();
}
