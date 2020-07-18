/*
 *   KDevelop outline view
 *   Copyright 2010, 2015 Alex Richardson <alex.richardson@gmx.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "outlineviewplugin.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include <interfaces/iuicontroller.h>
#include <interfaces/icore.h>

#include <debug.h>
#include "outlinewidget.h"

K_PLUGIN_FACTORY_WITH_JSON(KDevOutlineViewFactory, "kdevoutlineview.json", registerPlugin<OutlineViewPlugin>();)

using namespace KDevelop;

class OutlineViewFactory: public KDevelop::IToolViewFactory {
public:
    explicit OutlineViewFactory(OutlineViewPlugin *plugin) : m_plugin(plugin) {}

    QWidget* create(QWidget *parent = nullptr) override
    {
        return new OutlineWidget(parent, m_plugin);
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        return Qt::RightDockWidgetArea;
    }

    QString id() const override
    {
        return QStringLiteral("org.kdevelop.OutlineView");
    }

private:
    OutlineViewPlugin *m_plugin;
};

OutlineViewPlugin::OutlineViewPlugin(QObject *parent, const QVariantList&)
        : KDevelop::IPlugin(QStringLiteral("kdevoutlineview"), parent)
        , m_factory(new OutlineViewFactory(this))
{
    core()->uiController()->addToolView(i18nc("@title:window", "Outline"), m_factory);
}

OutlineViewPlugin::~OutlineViewPlugin()
{
}

void OutlineViewPlugin::unload()
{
    core()->uiController()->removeToolView(m_factory);
}

#include "outlineviewplugin.moc"
