/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "openwithplugin.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QMimeDatabase>
#include <QMimeType>
#include <QVariantList>

#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageBox_KDevCompat>
#include <KApplicationTrader>
#include <KParts/MainWindow>
#include <KPluginFactory>
#include <KService>
#include <KOpenWithDialog>
#include <KParts/PartLoader>
#include <KIO/ApplicationLauncherJob>
#include <kio_version.h>
#if KIO_VERSION < QT_VERSION_CHECK(5, 98, 0)
#include <KIO/JobUiDelegate>
#else
#include <KIO/JobUiDelegateFactory>
#endif

#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(KDevOpenWithFactory, "kdevopenwith.json", registerPlugin<OpenWithPlugin>();)

namespace {

bool sortActions(QAction* left, QAction* right)
{
    return left->text() < right->text();
}

bool isTextEditor(const KPluginMetaData& md)
{
    return md.value(QLatin1String("ServiceTypes"), QStringList()).contains( QStringLiteral("KTextEditor/Document") );
}

QString defaultForMimeType(const QString& mimeType)
{
    KConfigGroup config = KSharedConfig::openConfig()->group("Open With Defaults");
    if (config.hasKey(mimeType)) {
        QString storageId = config.readEntry(mimeType, QString());
        if (!storageId.isEmpty() && KService::serviceByStorageId(storageId)) {
            return storageId;
        }
    }
    return QString();
}

bool canOpenDefault(const QString& mimeType)
{
    if (defaultForMimeType(mimeType).isEmpty() && mimeType == QLatin1String("inode/directory")) {
        // potentially happens in non-kde environments apparently, see https://git.reviewboard.kde.org/r/122373
        return KApplicationTrader::preferredService(mimeType);
    } else {
        return true;
    }
}
}

OpenWithPlugin::OpenWithPlugin ( QObject* parent, const QVariantList& )
    : IPlugin ( QStringLiteral("kdevopenwith"), parent )
{
}

OpenWithPlugin::~OpenWithPlugin()
{
}

KDevelop::ContextMenuExtension OpenWithPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    // do not recurse
    if (context->hasType(KDevelop::Context::OpenWithContext)) {
        return ContextMenuExtension();
    }

    m_urls.clear();

    auto* filectx = dynamic_cast<FileContext*>( context );
    auto* projctx = dynamic_cast<ProjectItemContext*>( context );
    if ( filectx && filectx->urls().count() > 0 ) {
        m_urls = filectx->urls();
    } else if ( projctx && projctx->items().count() > 0 ) {
        // For now, let's handle *either* files only *or* directories only
        const auto items = projctx->items();
        const int wantedType = items.at(0)->type();
        for (ProjectBaseItem* item : items) {
            if (wantedType == ProjectBaseItem::File && item->file()) {
                m_urls << item->file()->path().toUrl();
            } else if ((wantedType == ProjectBaseItem::Folder || wantedType == ProjectBaseItem::BuildFolder) && item->folder()) {
                m_urls << item->folder()->path().toUrl();
            }
        }
    }

    if (m_urls.isEmpty()) {
        return KDevelop::ContextMenuExtension();
    }

    // Ok, lets fetch the mimetype for the !!first!! url and the relevant services
    // TODO: Think about possible alternatives to using the mimetype of the first url.
    QMimeType mimetype = QMimeDatabase().mimeTypeForUrl(m_urls.first());
    m_mimeType = mimetype.name();

    QList<QAction*> partActions = relevantParts(parent);
    QList<QAction*> appActions = relevantApps(parent);

    OpenWithContext subContext(m_urls, mimetype);
    const QList<ContextMenuExtension> extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( &subContext, parent);
    for (const ContextMenuExtension& ext : extensions) {
        appActions += ext.actions(ContextMenuExtension::OpenExternalGroup);
        partActions += ext.actions(ContextMenuExtension::OpenEmbeddedGroup);
    }

    {
        auto other = new QAction(i18nc("@item:menu", "Other..."), parent);
        connect(other, &QAction::triggered, this, [this] {
            auto dialog = new KOpenWithDialog(m_urls, ICore::self()->uiController()->activeMainWindow());
            if (dialog->exec() == QDialog::Accepted && dialog->service()) {
                openService(dialog->service());
            }
        });
        appActions << other;
    }

    // Now setup a menu with actions for each part and app
    auto* menu = new QMenu(i18nc("@title:menu", "Open With"), parent);
    auto documentOpenIcon = QIcon::fromTheme( QStringLiteral("document-open") );
    menu->setIcon( documentOpenIcon );

    if (!partActions.isEmpty()) {
        menu->addSection(i18nc("@title:menu", "Embedded Editors"));
        menu->addActions( partActions );
    }
    if (!appActions.isEmpty()) {
        menu->addSection(i18nc("@title:menu", "External Applications"));
        menu->addActions( appActions );
    }

    KDevelop::ContextMenuExtension ext;

    if (canOpenDefault(m_mimeType)) {
        auto* openAction = new QAction(i18nc("@action:inmenu", "Open"), parent);
        openAction->setIcon( documentOpenIcon );
        connect( openAction, &QAction::triggered, this, &OpenWithPlugin::openDefault );
        ext.addAction( KDevelop::ContextMenuExtension::FileGroup, openAction );
    }

    ext.addAction(KDevelop::ContextMenuExtension::FileGroup, menu->menuAction());
    return ext;
}

QList<QAction*> OpenWithPlugin::relevantApps(QWidget* parent)
{
    const auto pref = KApplicationTrader::preferredService(m_mimeType);

    const KService::List list = KApplicationTrader::queryByMimeType(m_mimeType, [] (auto /*x*/) { return true; });
    QList<QAction*> actions;
    QAction* standardAction = nullptr;
    const QString defaultId = defaultForMimeType(m_mimeType);
    for (auto& svc : list) {
        auto* act = new QAction(svc->name(), parent);
        act->setIcon( QIcon::fromTheme( svc->icon() ) );
        const QString sid = svc->storageId();
        connect(act, &QAction::triggered, this, [this, sid]() { open(sid); } );
        actions << act;
        if ( svc->storageId() == pref->storageId() ) {
            standardAction = act;
        }
    }
    std::sort(actions.begin(), actions.end(), sortActions);
    if (standardAction) {
        actions.removeOne(standardAction);
        actions.prepend(standardAction);
    }
    return actions;
}

QList<QAction*> OpenWithPlugin::relevantParts(QWidget* parent)
{
    const auto list = KParts::PartLoader::partsForMimeType(m_mimeType);

    QList<QAction*> actions;
    QAction* standardAction = nullptr;
    const QString defaultId = defaultForMimeType(m_mimeType);
    for (auto& svc : list) {
        auto* act = new QAction(isTextEditor(svc) ? i18nc("@item:inmenu", "Default Editor") : svc.name(), parent);
        act->setIcon( QIcon::fromTheme( svc.iconName() ) );
        if (svc.pluginId() == defaultId || (defaultId.isEmpty() && isTextEditor(svc))) {
            QFont font = act->font();
            font.setBold(true);
            act->setFont(font);
        }
        const QString sid = svc.pluginId();
        connect(act, &QAction::triggered, this, [this, sid]() { open(sid); } );
        actions << act;
        if ( isTextEditor(svc) ) {
            standardAction = act;
        } else if ( !standardAction ) {
            standardAction = act;
        }
    }
    std::sort(actions.begin(), actions.end(), sortActions);
    if (standardAction) {
        actions.removeOne(standardAction);
        actions.prepend(standardAction);
    }
    return actions;
}

void OpenWithPlugin::openDefault()
{
    //  check preferred handler
    const QString defaultId = defaultForMimeType(m_mimeType);
    if (!defaultId.isEmpty()) {
        open(defaultId);
        return;
    }

    // default handlers
    if (m_mimeType == QLatin1String("inode/directory")) {
        KService::Ptr service = KApplicationTrader::preferredService(m_mimeType);
        auto* job = new KIO::ApplicationLauncherJob(service);
        job->setUrls(m_urls);
#if KIO_VERSION < QT_VERSION_CHECK(5, 98, 0)
        job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled,
#else
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled,
#endif
                                                  ICore::self()->uiController()->activeMainWindow()));
        job->start();
    } else {
        for (const QUrl& u : qAsConst(m_urls)) {
            ICore::self()->documentController()->openDocument( u );
        }
    }
}

void OpenWithPlugin::open( const QString& storageid )
{
    openService(KService::serviceByStorageId( storageid ));
}

void OpenWithPlugin::openService(const KService::Ptr& service)
{
    if (service->isApplication()) {
        auto* job = new KIO::ApplicationLauncherJob(service);
        job->setUrls(m_urls);
#if KIO_VERSION < QT_VERSION_CHECK(5, 98, 0)
        job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled,
#else
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled,
#endif
                                                  ICore::self()->uiController()->activeMainWindow()));
        job->start();
    } else {
        QString prefName = service->desktopEntryName();
        if (isTextEditor(KPluginMetaData(service->storageId()))) {
            // If the user chose a KTE part, lets make sure we're creating a TextDocument instead of
            // a PartDocument by passing no preferredpart to the documentcontroller
            // TODO: Solve this rather inside DocumentController
            prefName.clear();
        }
        for (const QUrl& u : qAsConst(m_urls)) {
            ICore::self()->documentController()->openDocument( u, prefName );
        }
    }

    KConfigGroup config = KSharedConfig::openConfig()->group("Open With Defaults");
    if (service->storageId() != config.readEntry(m_mimeType, QString())) {
        int setDefault = KMessageBox::questionTwoActions(
            qApp->activeWindow(),
            i18nc("%1: mime type name, %2: app/part name", "Do you want to open all '%1' files by default with %2?",
                  m_mimeType, service->name()),
            i18nc("@title:window", "Set as Default?"),
            KGuiItem(i18nc("@action:button", "Set as Default"), QStringLiteral("dialog-ok")),
            KGuiItem(i18nc("@action:button", "Do Not Set"), QStringLiteral("dialog-cancel")),
            QStringLiteral("OpenWith-%1").arg(m_mimeType));
        if (setDefault == KMessageBox::PrimaryAction) {
            config.writeEntry(m_mimeType, service->storageId());
        }
    }
}

void OpenWithPlugin::openFilesInternal( const QList<QUrl>& files )
{
    if (files.isEmpty()) {
        return;
    }

    m_urls = files;
    m_mimeType = QMimeDatabase().mimeTypeForUrl(m_urls.first()).name();
    openDefault();
}

#include "openwithplugin.moc"
#include "moc_openwithplugin.cpp"
