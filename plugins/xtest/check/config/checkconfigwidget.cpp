/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */


#include "checkconfig.h"
#include "checkconfigwidget.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <QFile>
#include <KDebug>

K_PLUGIN_FACTORY(CheckConfigFactory, registerPlugin<CheckConfigWidget>(); )
K_EXPORT_PLUGIN(CheckConfigFactory("kcm_kdevcheck_config"))

CheckConfigWidget::CheckConfigWidget(QWidget* parent, const QVariantList& args)
    : ProjectKCModule<CheckConfig>(CheckConfigFactory::componentData(), parent, args)
{
    setupUi(this);
    kDebug() << "0===> CONSTRUCTING";

    kcfg_testRegistration->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
    addConfig(CheckConfig::self(), this);
    load();
}

CheckConfigWidget::~CheckConfigWidget()
{
}

#include "checkconfigwidget.moc"
