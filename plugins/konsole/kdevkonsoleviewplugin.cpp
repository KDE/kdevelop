/*
    SPDX-FileCopyrightText: 2003, 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdevkonsoleviewplugin.h"

#include <KLocalizedString>
#include <KService>
#include <KPluginFactory>
#include <KParts/PartLoader>

#include <interfaces/iuicontroller.h>
#include <interfaces/icore.h>

#include "kdevkonsoleview.h"
#include "debug.h"

K_PLUGIN_FACTORY_WITH_JSON(KonsoleViewFactory, "kdevkonsoleview.json", registerPlugin<KDevKonsoleViewPlugin>();)

class KDevKonsoleViewFactory: public KDevelop::IToolViewFactory{
public:
    explicit KDevKonsoleViewFactory(KDevKonsoleViewPlugin *plugin):
        mplugin(plugin) {}
    QWidget* create(QWidget *parent = nullptr) override
    {
        return new KDevKonsoleView(mplugin, parent);
    }
    Qt::DockWidgetArea defaultPosition() const override
    {
        return Qt::BottomDockWidgetArea;
    }
    QString id() const override
    {
        return QStringLiteral("org.kdevelop.KonsoleView");
    }
private:
    KDevKonsoleViewPlugin *mplugin;
};

KDevKonsoleViewPlugin::KDevKonsoleViewPlugin(QObject* parent, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevkonsoleview"), parent)
    , m_viewFactory(new KDevKonsoleViewFactory(this))
{
    // TODO KF6: check whether constructing a KPluginMetaData from this plugin ID still works
    // consider passing some path or prefix instead of the plugin ID to increase robustness.
    const auto factory = KPluginFactory::loadFactory(KPluginMetaData(QStringLiteral("konsolepart")));
    if (!factory) {
        qCWarning(PLUGIN_KONSOLE) << "Failed to load 'konsolepart' plugin:" << factory.errorText;
        setErrorDescription(i18n("Failed to load 'konsolepart' plugin: %1", factory.errorString));
        return;
    }

    m_konsoleFactory = factory.plugin;
    m_viewFactory = new KDevKonsoleViewFactory(this);

    core()->uiController()->addToolView(i18nc("@title:window", "Terminal"), m_viewFactory);
}

void KDevKonsoleViewPlugin::unload()
{
    if (m_viewFactory) {
        core()->uiController()->removeToolView(m_viewFactory);
    }
}

KDevKonsoleViewPlugin::~KDevKonsoleViewPlugin()
{
}

#include "kdevkonsoleviewplugin.moc"
#include "moc_kdevkonsoleviewplugin.cpp"
