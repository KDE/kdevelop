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
#include <KParts/PartLoader>
#include <KPluginFactory>
#include <KOpenWithDialog>
#include <KIO/ApplicationLauncherJob>
#include <kio_version.h>
#include <KIO/JobUiDelegateFactory>

#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>

#include <utility>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(KDevOpenWithFactory, "kdevopenwith.json", registerPlugin<OpenWithPlugin>();)

namespace {
constexpr QLatin1String partIdConfigEntryValuePrefix{"PART-ID:", 8};

bool isTextPart(const QString& pluginId)
{
    return pluginId == QLatin1String("katepart");
}

bool isDirectory(const QString& mimeType)
{
    return mimeType == QLatin1String("inode/directory");
}

KConfigGroup defaultsConfig()
{
    return KSharedConfig::openConfig()->group(QStringLiteral("Open With Defaults"));
}

bool sortActions(QAction* left, QAction* right)
{
    return left->text() < right->text();
}

QList<QAction*> sortedActions(QList<QAction*> actions, int sortOffset)
{
    if (!actions.isEmpty()) {
        Q_ASSERT(actions.size() >= sortOffset);
        std::sort(actions.begin() + sortOffset, actions.end(), sortActions);
    }
    return actions;
}

QAction* createAction(const QString& name, const QString& iconName, QWidget* parent, bool isDefault)
{
    auto* action = new QAction(QIcon::fromTheme(iconName), name, parent);
    if (isDefault) {
        QFont font = action->font();
        font.setBold(true);
        action->setFont(font);
    }
    return action;
}
} // unnamed namespace

using OpenWithUtils::FileOpener;

FileOpener::FileOpener(const KService::Ptr& service)
    : FileOpener(false, service->storageId())
{
    m_service = service;
    Q_ASSERT(service->isApplication());
}

FileOpener FileOpener::fromPartId(const QString& partId)
{
    return FileOpener{true, partId};
}

FileOpener FileOpener::fromConfigEntryValue(const QString& value)
{
    if (value.startsWith(partIdConfigEntryValuePrefix)) {
        // TODO: can the part ID be validated efficiently here? A matching plugin
        // should be available and support the MIME type key of the config entry.
        return FileOpener::fromPartId(value.mid(partIdConfigEntryValuePrefix.size()));
    }

    if (!value.isEmpty()) {
        auto service = KService::serviceByStorageId(value);
        if (service && service->isApplication()) {
            FileOpener opener{false, value};
            opener.m_service = std::move(service);
            return opener;
        }
    }

    return FileOpener{false, QString()};
}

QString FileOpener::toConfigEntryValue() const
{
    Q_ASSERT(isValid());
    if (m_isPart) {
        return partIdConfigEntryValuePrefix + m_id;
    }
    return m_id;
}

bool FileOpener::isValid() const
{
    return !m_id.isEmpty();
}

bool FileOpener::isPart() const
{
    Q_ASSERT(isValid());
    return m_isPart;
}

const QString& FileOpener::id() const
{
    Q_ASSERT(isValid());
    return m_id;
}

const KService::Ptr& FileOpener::service() const
{
    Q_ASSERT(isValid());
    Q_ASSERT(!isPart());
    Q_ASSERT(m_service);
    return m_service;
}

FileOpener::FileOpener(bool isPart, const QString& id)
    : m_isPart{isPart}
    , m_id{id}
{
}

bool OpenWithUtils::operator==(const FileOpener& a, const FileOpener& b)
{
    Q_ASSERT(a.isValid());
    Q_ASSERT(b.isValid());
    return a.isPart() == b.isPart() && a.id() == b.id();
}

bool OpenWithPlugin::canOpenDefault() const
{
    if (!m_defaultOpener.isValid() && isDirectory(m_mimeType)) {
        // potentially happens in non-kde environments apparently, see https://git.reviewboard.kde.org/r/122373
        return KApplicationTrader::preferredService(m_mimeType);
    } else {
        return true;
    }
}

OpenWithPlugin::OpenWithPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : IPlugin(QStringLiteral("kdevopenwith"), parent, metaData)
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

    auto mimetype = updateMimeTypeForUrls();

    auto partActions = actionsForParts(parent);
    auto appActions = actionsForApplications(parent);

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
                openApplication(dialog->service());
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

    if (canOpenDefault()) {
        auto* openAction = new QAction(i18nc("@action:inmenu", "Open"), parent);
        openAction->setIcon( documentOpenIcon );
        connect( openAction, &QAction::triggered, this, &OpenWithPlugin::openDefault );
        ext.addAction( KDevelop::ContextMenuExtension::FileGroup, openAction );
    }

    ext.addAction(KDevelop::ContextMenuExtension::FileGroup, menu->menuAction());
    return ext;
}

QList<QAction*> OpenWithPlugin::actionsForParts(QWidget* parent)
{
    if (isDirectory(m_mimeType)) {
        // Don't return any parts for directories, KDevelop can only open files, not folders, thus
        // we wouldn't ever actually do anything with the parts shown to the user here.
        // Note that we can open a folder in an external application just fine though.
        return {};
    }

    const auto parts = KParts::PartLoader::partsForMimeType(m_mimeType);
    QList<QAction*> actions;
    actions.reserve(parts.size());

    int textEditorActionPos = -1;
    for (const auto& part : parts) {
        const auto pluginId = part.pluginId();
        const auto isTextEditor = isTextPart(pluginId);

        if (isTextEditor) {
            textEditorActionPos = actions.size();
        }

        const bool isDefault =
            m_defaultOpener.isValid() ? FileOpener::fromPartId(pluginId) == m_defaultOpener : isTextEditor;
        auto* action = createAction(isTextEditor ? i18nc("@item:inmenu", "Default Editor") : part.name(),
                                    part.iconName(), parent, isDefault);
        connect(action, &QAction::triggered, this, [this, action, pluginId]() {
            openPart(pluginId, action->text());
        });
        actions << action;
    }

    // partsForMimeType has the preferred part at the first position, let's keep it there
    int sortOffset = 1;
    if (textEditorActionPos > 0) {
        // move the text editor action up front
        actions.move(textEditorActionPos, 0);
        // keep the user-preferred mime at the 2nd pos
        sortOffset++;
    }

    return sortedActions(std::move(actions), sortOffset);
}

QList<QAction*> OpenWithPlugin::actionsForApplications(QWidget* parent)
{
    const auto services = KApplicationTrader::queryByMimeType(m_mimeType);
    QList<QAction*> actions;
    actions.reserve(services.size());

    for (const auto& service : services) {
        const bool isDefault = m_defaultOpener.isValid() && service == m_defaultOpener;
        auto* action = createAction(service->name(), service->icon(), parent, isDefault);
        connect(action, &QAction::triggered, this, [this, service]() {
            openApplication(service);
        });
        actions << action;
    }

    // queryByMimeType returns the preferred service in the first position, let's keep it there
    const auto sortOffset = 1;
    return sortedActions(std::move(actions), sortOffset);
}

void OpenWithPlugin::openDefault() const
{
    //  check preferred handler
    if (m_defaultOpener.isValid()) {
        if (m_defaultOpener.isPart()) {
            delegateToParts(m_defaultOpener.id());
        } else {
            delegateToExternalApplication(m_defaultOpener.service());
        }
        return;
    }

    // default handlers
    if (isDirectory(m_mimeType)) {
        KService::Ptr service = KApplicationTrader::preferredService(m_mimeType);
        delegateToExternalApplication(service);
    } else {
        for (const QUrl& u : std::as_const(m_urls)) {
            ICore::self()->documentController()->openDocument( u );
        }
    }
}

void OpenWithPlugin::delegateToParts(const QString& pluginId) const
{
    auto prefName = pluginId;
    if (isTextPart(pluginId)) {
        // If the user chose a KTE part, lets make sure we're creating a TextDocument instead of
        // a PartDocument by passing no preferredpart to the documentcontroller
        // TODO: Solve this rather inside DocumentController
        prefName.clear();
    }
    for (const QUrl& u : std::as_const(m_urls)) {
        ICore::self()->documentController()->openDocument(u, prefName);
    }
}

void OpenWithPlugin::openPart(const QString& pluginId, const QString& name)
{
    delegateToParts(pluginId);
    rememberDefaultChoice(FileOpener::fromPartId(pluginId), name);
}

void OpenWithPlugin::delegateToExternalApplication(const KService::Ptr& service) const
{
    Q_ASSERT(service->isApplication());

    auto* job = new KIO::ApplicationLauncherJob(service);
    job->setUrls(m_urls);
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled,
                                                       ICore::self()->uiController()->activeMainWindow()));
    job->start();
}

void OpenWithPlugin::openApplication(const KService::Ptr& service)
{
    delegateToExternalApplication(service);
    rememberDefaultChoice(service, service->name());
}

void OpenWithPlugin::openFilesInternal( const QList<QUrl>& files )
{
    if (files.isEmpty()) {
        return;
    }

    m_urls = files;
    updateMimeTypeForUrls();
    openDefault();
}

void OpenWithPlugin::rememberDefaultChoice(const FileOpener& opener, const QString& name)
{
    if (m_defaultOpener.isValid() && opener == m_defaultOpener) {
        return;
    }

    const auto setDefault = KMessageBox::questionTwoActions(
        qApp->activeWindow(),
        i18nc("%1: mime type name, %2: app/part name", "Do you want to open all '%1' files by default with %2?",
              m_mimeType, name),
        i18nc("@title:window", "Set as Default?"),
        KGuiItem(i18nc("@action:button", "Set as Default"), QStringLiteral("dialog-ok")),
        KGuiItem(i18nc("@action:button", "Do Not Set"), QStringLiteral("dialog-cancel")),
        QStringLiteral("OpenWith-%1").arg(m_mimeType));

    if (setDefault == KMessageBox::PrimaryAction) {
        m_defaultOpener = opener;
        defaultsConfig().writeEntry(m_mimeType, m_defaultOpener.toConfigEntryValue());
    }
}

QMimeType OpenWithPlugin::updateMimeTypeForUrls()
{
    // Fetch the MIME type of the !!first!! URL.
    // TODO: think about possible alternatives to using the MIME type of the first URL.
    auto mimeType = QMimeDatabase().mimeTypeForUrl(m_urls.first());
    m_mimeType = mimeType.name();

    const auto config = defaultsConfig();
    m_defaultOpener = FileOpener::fromConfigEntryValue(config.readEntry(m_mimeType, QString()));

    return mimeType;
}

#include "openwithplugin.moc"
#include "moc_openwithplugin.cpp"
