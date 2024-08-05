/*
    SPDX-FileCopyrightText: 2002 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2002 Falk Brettschneider <falkbr@kdevelop.org>
    SPDX-FileCopyrightText: 2003 Julian Rockey <linux@jrockey.com>
    SPDX-FileCopyrightText: 2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
    SPDX-FileCopyrightText: 2003 Mario Scalas <mario.scalas@libero.it>
    SPDX-FileCopyrightText: 2003-2004, 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iplugin.h"

#include <memory>

#include <KActionCollection>
#include <KMainWindow>
#include <KPluginMetaData>
#include <KXmlGuiWindow>
#include <KXMLGUIFactory>

#include "icore.h"
#include "iprojectcontroller.h"
#include "iproject.h"
#include "contextmenuextension.h"

namespace KDevelop
{

class IPluginPrivate
{
public:
    explicit IPluginPrivate(IPlugin *q)
        : q(q)
    {}

    ~IPluginPrivate()
    {
    }

    void guiClientAdded(KXMLGUIClient *client)
    {
        if (client != q)
            return;

        q->initializeGuiState();
        updateState();
    }

    void updateState()
    {
        const int projectCount =
            ICore::self()->projectController()->projectCount();

        IPlugin::ReverseStateChange reverse = IPlugin::StateNoReverse;
        if (! projectCount)
            reverse = IPlugin::StateReverse;

        q->stateChanged(QStringLiteral("has_project"), reverse);
    }

    IPlugin *q;
    ICore *core;
    QString m_errorDescription;
};

IPlugin::IPlugin(const QString& componentName, QObject* parent, const KPluginMetaData& metaData)
    : QObject(parent)
    , KXMLGUIClient()
    , d_ptr(new IPluginPrivate(this))
{
    Q_D(IPlugin);

    // The following cast is safe, there's no component in KDevPlatform that
    // creates plugins except the plugincontroller. The controller passes
    // Core::self() as parent to KServiceTypeTrader::createInstanceFromQuery
    // so we know the parent is always a Core* pointer.
    // This is the only way to pass the Core pointer to the plugin during its
    // creation so plugins have access to ICore during their creation.
    Q_ASSERT(qobject_cast<KDevelop::ICore*>(parent));
    d->core = static_cast<KDevelop::ICore*>(parent);

    setComponentName(componentName, metaData.name());

    auto clientAdded = [this] (KXMLGUIClient* client) {
        Q_D(IPlugin);
        d->guiClientAdded(client);
    };
    const auto mainWindows = KMainWindow::memberList();
    for (KMainWindow* mw : mainWindows) {
        auto* guiWindow = qobject_cast<KXmlGuiWindow*>(mw);
        if (! guiWindow)
            continue;

        connect(guiWindow->guiFactory(), &KXMLGUIFactory::clientAdded,
                this, clientAdded);
    }

    auto updateState = [this] {
        Q_D(IPlugin);
        d->updateState();
    };
    connect(ICore::self()->projectController(), &IProjectController::projectOpened,
            this, updateState);
    connect(ICore::self()->projectController(), &IProjectController::projectClosed,
            this, updateState);
}

IPlugin::~IPlugin() = default;

void IPlugin::unload()
{
}

ICore *IPlugin::core() const
{
    Q_D(const IPlugin);

    return d->core;
}

}

KDevelop::ContextMenuExtension KDevelop::IPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    Q_UNUSED(context);
    Q_UNUSED(parent);
    return KDevelop::ContextMenuExtension();
}

void KDevelop::IPlugin::initializeGuiState()
{ }

class CustomXmlGUIClient : public KXMLGUIClient
{
public:
    explicit CustomXmlGUIClient(const KDevelop::IPlugin* plugin)
    {
        // TODO KF5: Get rid off this
        setComponentName(plugin->componentName(), plugin->actionCollection()->componentDisplayName());
    }
    using KXMLGUIClient::setXMLFile;
};

KXMLGUIClient* KDevelop::IPlugin::createGUIForMainWindow(Sublime::MainWindow* window)
{
    auto ret = std::make_unique<CustomXmlGUIClient>(this);

    QString file;
    createActionsForMainWindow(window, file, *ret->actionCollection());
    if (ret->actionCollection()->isEmpty()) {
        return nullptr;
    }

    Q_ASSERT(!file.isEmpty()); //A file must have been set
    ret->setXMLFile(file);
    return ret.release();
}

void KDevelop::IPlugin::createActionsForMainWindow( Sublime::MainWindow* /*window*/, QString& /*xmlFile*/, KActionCollection& /*actions*/ )
{
}

bool KDevelop::IPlugin::hasError() const
{
    Q_D(const IPlugin);

    return !d->m_errorDescription.isEmpty();
}

void KDevelop::IPlugin::setErrorDescription(const QString& description)
{
    Q_D(IPlugin);

    d->m_errorDescription = description;
}


QString KDevelop::IPlugin::errorDescription() const
{
    Q_D(const IPlugin);

    return d->m_errorDescription;
}

int KDevelop::IPlugin::configPages() const
{
    return 0;
}

KDevelop::ConfigPage* KDevelop::IPlugin::configPage (int, QWidget*)
{
    return nullptr;
}

int KDevelop::IPlugin::perProjectConfigPages() const
{
    return 0;
}

KDevelop::ConfigPage* KDevelop::IPlugin::perProjectConfigPage(int, const ProjectConfigOptions&, QWidget*)
{
    return nullptr;
}
#include "moc_iplugin.cpp"
