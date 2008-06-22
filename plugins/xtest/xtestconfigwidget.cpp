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


#include "xtestconfig.h"
#include "xtestconfigwidget.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <QFile>
#include <kdebug.h>

K_PLUGIN_FACTORY(XTestConfigFactory, registerPlugin<XTestConfigWidget>(); )
K_EXPORT_PLUGIN(XTestConfigFactory("kcm_kdevxtest_config"))

XTestConfigWidget::XTestConfigWidget(QWidget* parent, const QVariantList& args)
    : ProjectKCModule<XTestConfig>(XTestConfigFactory::componentData(), parent, args)
{
    setupUi(this);
    kDebug() << "0===> CONSTRUCTING";

    kcfg_testRegistration->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
    addConfig(XTestConfig::self(), this);
    load();
}

XTestConfigWidget::~XTestConfigWidget()
{
}

#include "xtestconfigwidget.moc"
