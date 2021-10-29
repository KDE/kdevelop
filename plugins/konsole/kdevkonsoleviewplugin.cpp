/*
    SPDX-FileCopyrightText: 2003, 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdevkonsoleviewplugin.h"

#include <KLocalizedString>
#include <KService>

#include <interfaces/iuicontroller.h>
#include <interfaces/icore.h>

#include "kdevkonsoleview.h"
#include "debug.h"

QObject* createKonsoleView( QWidget*, QObject* op, const QVariantList& args)
{
    KService::Ptr service = KService::serviceByDesktopName(QStringLiteral("konsolepart"));
    KPluginFactory* factory = nullptr;
    if (service) {
        factory = KPluginLoader(*service.data()).factory();
    }
    if (!factory) {
        qCWarning(PLUGIN_KONSOLE) << "Failed to load 'konsolepart' plugin";
    }
    return new KDevKonsoleViewPlugin(factory, op, args);
}

K_PLUGIN_FACTORY_WITH_JSON(KonsoleViewFactory, "kdevkonsoleview.json", registerPlugin<KDevKonsoleViewPlugin>( QString(), &createKonsoleView );)

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

KDevKonsoleViewPlugin::KDevKonsoleViewPlugin( KPluginFactory* konsoleFactory, QObject *parent, const QVariantList & )
    : KDevelop::IPlugin( QStringLiteral("kdevkonsoleview"), parent )
    , m_konsoleFactory(konsoleFactory)
    , m_viewFactory(konsoleFactory ? new KDevKonsoleViewFactory(this) : nullptr)
{
    if(!m_viewFactory) {
      setErrorDescription(i18n("Failed to load 'konsolepart' plugin"));
    } else {
        core()->uiController()->addToolView(i18nc("@title:window", "Terminal"), m_viewFactory);
    }
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

