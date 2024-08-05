/*
    SPDX-FileCopyrightText: 2003, 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdevkonsoleviewplugin.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <KService>

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

KDevKonsoleViewPlugin::KDevKonsoleViewPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevkonsoleview"), parent, metaData)
{
    const auto factory = KPluginFactory::loadFactory(KPluginMetaData{QStringLiteral("kf6/parts/konsolepart")});
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

KPluginFactory* KDevKonsoleViewPlugin::konsoleFactory() const
{
    return m_konsoleFactory;
}

KDevKonsoleViewPlugin::~KDevKonsoleViewPlugin()
{
}

#include "kdevkonsoleviewplugin.moc"
#include "moc_kdevkonsoleviewplugin.cpp"
