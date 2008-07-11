/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "hgplugin.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <klocalizedstring.h>

#include <icore.h>

#include <dvcsjob.h>

#include "hgexecutor.h"

K_PLUGIN_FACTORY(KDevHgFactory, registerPlugin<HgPlugin>(); )
K_EXPORT_PLUGIN(KDevHgFactory("kdevhg"))

HgPlugin::HgPlugin( QObject *parent, const QVariantList & )
    : DistributedVersionControlPlugin(parent, KDevHgFactory::componentData())
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBasicVersionControl )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IDistributedVersionControl )

    core()->uiController()->addToolView(i18n("Mercurial"), DistributedVersionControlPlugin::d->m_factory);

    setXMLFile("kdevhg.rc");
    setupActions();

    DistributedVersionControlPlugin::d->m_exec = new HgExecutor(this);
}

HgPlugin::~HgPlugin()
{
    delete DistributedVersionControlPlugin::d;
}

QString HgPlugin::name() const
{
        return i18n("Mercurial");
}

// #include "hgplugin.moc"
