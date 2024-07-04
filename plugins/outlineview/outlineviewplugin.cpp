/*
    SPDX-FileCopyrightText: 2010, 2015 Alex Richardson <alex.richardson@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
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

OutlineViewPlugin::OutlineViewPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevoutlineview"), parent, metaData)
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
#include "moc_outlineviewplugin.cpp"
