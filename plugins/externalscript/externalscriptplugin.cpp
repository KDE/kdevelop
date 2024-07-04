/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "externalscriptplugin.h"

#include "externalscriptview.h"
#include "externalscriptitem.h"
#include "externalscriptjob.h"
#include <debug.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <interfaces/isession.h>

#include <outputview/outputjob.h>

#include <project/projectmodel.h>
#include <util/path.h>

#include <language/interfaces/editorcontext.h>

#include <KPluginFactory>
#include <KProcess>
#include <KLocalizedString>

#include <QAction>
#include <QStandardItemModel>
#include <QDBusConnection>
#include <QMenu>

K_PLUGIN_FACTORY_WITH_JSON(ExternalScriptFactory, "kdevexternalscript.json", registerPlugin<ExternalScriptPlugin>(); )

class ExternalScriptViewFactory
    : public KDevelop::IToolViewFactory
{
public:
    explicit ExternalScriptViewFactory(ExternalScriptPlugin* plugin) : m_plugin(plugin) {}

    QWidget* create(QWidget* parent = nullptr) override
    {
        return new ExternalScriptView(m_plugin, parent);
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        return Qt::RightDockWidgetArea;
    }

    QString id() const override
    {
        return QStringLiteral("org.kdevelop.ExternalScriptView");
    }

private:
    ExternalScriptPlugin* m_plugin;
};

// We extend ExternalScriptJob so that it deletes the temporarily created item on destruction
class ExternalScriptJobOwningItem
    : public ExternalScriptJob
{
    Q_OBJECT

public:
    ExternalScriptJobOwningItem(ExternalScriptItem* item, const QUrl& url,
                                ExternalScriptPlugin* parent) : ExternalScriptJob(item, url, parent)
        , m_item(item)
    {
    }
    ~ExternalScriptJobOwningItem() override
    {
        delete m_item;
    }

private:
    ExternalScriptItem* m_item;
};

ExternalScriptPlugin* ExternalScriptPlugin::m_self = nullptr;

ExternalScriptPlugin::ExternalScriptPlugin(QObject* parent, const KPluginMetaData& metaData,
                                           const QVariantList& /*args*/)
    : IPlugin(QStringLiteral("kdevexternalscript"), parent, metaData)
    , m_model(new QStandardItemModel(this))
    , m_factory(new ExternalScriptViewFactory(this))
{
    Q_ASSERT(!m_self);
    m_self = this;

    QDBusConnection::sessionBus().registerObject(QStringLiteral(
                                                     "/org/kdevelop/ExternalScriptPlugin"), this,
                                                 QDBusConnection::ExportScriptableSlots);

    setXMLFile(QStringLiteral("kdevexternalscript.rc"));

    //BEGIN load config
    KConfigGroup config = getConfig();
    const auto groups = config.groupList();
    for (const QString& group : groups) {
        KConfigGroup script = config.group(group);
        if (script.hasKey("name") && script.hasKey("command")) {
            auto* item = new ExternalScriptItem;
            item->setKey(script.name());
            item->setText(script.readEntry("name"));
            item->setCommand(script.readEntry("command"));
            item->setInputMode(static_cast<ExternalScriptItem::InputMode>(script.readEntry("inputMode", 0u)));
            item->setOutputMode(static_cast<ExternalScriptItem::OutputMode>(script.readEntry("outputMode", 0u)));
            item->setErrorMode(static_cast<ExternalScriptItem::ErrorMode>(script.readEntry("errorMode", 0u)));
            item->setSaveMode(static_cast<ExternalScriptItem::SaveMode>(script.readEntry("saveMode", 0u)));
            item->setFilterMode(script.readEntry("filterMode", 0u));
            item->action()->setShortcut(QKeySequence(script.readEntry("shortcuts")));
            item->setShowOutput(script.readEntry("showOutput", true));
            m_model->appendRow(item);
        }
    }

    //END load config

    core()->uiController()->addToolView(i18n("External Scripts"), m_factory);

    connect(m_model, &QStandardItemModel::rowsAboutToBeRemoved,
            this, &ExternalScriptPlugin::rowsAboutToBeRemoved);
    connect(m_model, &QStandardItemModel::rowsInserted,
            this, &ExternalScriptPlugin::rowsInserted);

    const bool firstUse = config.readEntry("firstUse", true);
    if (firstUse) {
        // some example scripts
        auto* item = new ExternalScriptItem;
        item->setText(i18n("Quick Compile"));
        item->setCommand(QStringLiteral("g++ -o %b %f && ./%b"));
        m_model->appendRow(item);

    #ifndef Q_OS_WIN
        item = new ExternalScriptItem;
        item->setText(i18n("Sort Selection"));
        item->setCommand(QStringLiteral("sort"));
        item->setInputMode(ExternalScriptItem::InputSelectionOrDocument);
        item->setOutputMode(ExternalScriptItem::OutputReplaceSelectionOrDocument);
        item->setShowOutput(false);
        m_model->appendRow(item);

        item = new ExternalScriptItem;
        item->setText(i18n("Google Selection"));
        item->setCommand(QStringLiteral("xdg-open \"https://www.google.com/search?q=%s\""));
        item->setShowOutput(false);
        m_model->appendRow(item);

        item = new ExternalScriptItem;
        item->setText(i18n("Paste to Hastebin"));
        item->setCommand(QStringLiteral(
                             "a=$(cat); curl -X POST -s -d \"$a\" https://hastebin.com/documents | awk -F '\"' '{print \"https://hastebin.com/\"$4}' | xargs xdg-open ;"));
        item->setInputMode(ExternalScriptItem::InputSelectionOrDocument);
        item->setShowOutput(false);
        m_model->appendRow(item);
    #endif

        config.writeEntry("firstUse", false);
        config.sync();
    }
}

ExternalScriptPlugin* ExternalScriptPlugin::self()
{
    return m_self;
}

ExternalScriptPlugin::~ExternalScriptPlugin()
{
    m_self = nullptr;
}

KDevelop::ContextMenuExtension ExternalScriptPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    m_urls.clear();

    int folderCount = 0;

    if (context->type() == KDevelop::Context::FileContext) {
        auto* filectx = static_cast<KDevelop::FileContext*>(context);
        m_urls = filectx->urls();
    } else if (context->type() == KDevelop::Context::ProjectItemContext) {
        auto* projctx = static_cast<KDevelop::ProjectItemContext*>(context);
        const auto items = projctx->items();
        for (KDevelop::ProjectBaseItem* item : items) {
            if (item->file()) {
                m_urls << item->file()->path().toUrl();
            } else if (item->folder()) {
                m_urls << item->folder()->path().toUrl();
                folderCount++;
            }
        }
    } else if (context->type() == KDevelop::Context::EditorContext) {
        auto* econtext = static_cast<KDevelop::EditorContext*>(context);
        m_urls << econtext->url();
    }

    if (!m_urls.isEmpty()) {
        KDevelop::ContextMenuExtension ext;
        QMenu* menu = nullptr;

        for (int row = 0; row < m_model->rowCount(); ++row) {
            auto* item = dynamic_cast<ExternalScriptItem*>(m_model->item(row));
            Q_ASSERT(item);

            if (context->type() != KDevelop::Context::EditorContext) {
                // filter scripts that depend on an opened document
                // if the context menu was not requested inside the editor
                if (item->performParameterReplacement() && item->command().contains(QLatin1String("%s"))) {
                    continue;
                } else if (item->inputMode() == ExternalScriptItem::InputSelectionOrNone) {
                    continue;
                }
            }

            if (folderCount == m_urls.count()) {
                // when only folders filter items that don't have %d parameter (or another parameter)
                if (item->performParameterReplacement() &&
                    (!item->command().contains(QLatin1String("%d")) ||
                     item->command().contains(QLatin1String("%s")) ||
                     item->command().contains(QLatin1String("%u")) ||
                     item->command().contains(QLatin1String("%f")) ||
                     item->command().contains(QLatin1String("%b")) ||
                     item->command().contains(QLatin1String("%n"))
                    )
                ) {
                    continue;
                }
            }

            if (!menu) {
                menu = new QMenu(i18nc("@title:menu", "External Scripts"), parent);
            }

            auto* scriptAction = new QAction(item->text(), menu);
            scriptAction->setData(QVariant::fromValue<ExternalScriptItem*>(item));
            connect(scriptAction, &QAction::triggered, this, &ExternalScriptPlugin::executeScriptFromContextMenu);
            menu->addAction(scriptAction);
        }

        if (menu) {
            ext.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, menu->menuAction());
        }

        return ext;
    }

    return KDevelop::IPlugin::contextMenuExtension(context, parent);
}

void ExternalScriptPlugin::unload()
{
    core()->uiController()->removeToolView(m_factory);
    KDevelop::IPlugin::unload();
}

KConfigGroup ExternalScriptPlugin::getConfig() const
{
    return KSharedConfig::openConfig()->group(QStringLiteral("External Scripts"));
}

QStandardItemModel* ExternalScriptPlugin::model() const
{
    return m_model;
}

void ExternalScriptPlugin::execute(ExternalScriptItem* item, const QUrl& url) const
{
    auto* job = new ExternalScriptJob(item, url, const_cast<ExternalScriptPlugin*>(this));

    KDevelop::ICore::self()->runController()->registerJob(job);
}

void ExternalScriptPlugin::execute(ExternalScriptItem* item) const
{
    auto document = KDevelop::ICore::self()->documentController()->activeDocument();
    execute(item, document ? document->url() : QUrl());
}

bool ExternalScriptPlugin::executeCommand(const QString& command, const QString& workingDirectory) const
{
    auto* item = new ExternalScriptItem;
    item->setCommand(command);
    item->setWorkingDirectory(workingDirectory);
    item->setPerformParameterReplacement(false);
    qCDebug(PLUGIN_EXTERNALSCRIPT) << "executing command " << command << " in dir " << workingDirectory <<
        " as external script";
    auto* job =
        new ExternalScriptJobOwningItem(item, QUrl(), const_cast<ExternalScriptPlugin*>(this));
    // When a command is executed, for example through the terminal, we don't want the command output to be risen
    job->setVerbosity(KDevelop::OutputJob::Silent);

    KDevelop::ICore::self()->runController()->registerJob(job);
    return true;
}

QString ExternalScriptPlugin::executeCommandSync(const QString& command, const QString& workingDirectory) const
{
    qCDebug(PLUGIN_EXTERNALSCRIPT) << "executing command " << command << " in working-dir " << workingDirectory;
    KProcess process;
    process.setWorkingDirectory(workingDirectory);
    process.setShellCommand(command);
    process.setOutputChannelMode(KProcess::OnlyStdoutChannel);
    process.execute();
    return QString::fromLocal8Bit(process.readAll());
}

void ExternalScriptPlugin::executeScriptFromActionData() const
{
    auto* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);

    auto* item = action->data().value<ExternalScriptItem*>();
    Q_ASSERT(item);

    execute(item);
}

void ExternalScriptPlugin::executeScriptFromContextMenu() const
{
    auto* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);

    auto* item = action->data().value<ExternalScriptItem*>();
    Q_ASSERT(item);

    for (const QUrl& url : m_urls) {
        // An empty URL here probably means that the user triggered an external script action from an untitled
        // document's context menu. In this case EditorContext::url() returns an untitled KTextEditor::Document's
        // URL, which is empty. An empty QUrl is relative, so attempting to open an empty URL makes the assertion
        // !inputUrl.isRelative() in DocumentControllerPrivate::openDocumentInternal() fail.
        // Therefore, do not attempt to open a document for an empty URL. In the discussed scenario the untitled
        // document is already open and active. Thus the triggered external script action can work correctly.
        if (!url.isEmpty()) {
            KDevelop::ICore::self()->documentController()->openDocument(url);
        }
        execute(item, url);
    }
}

void ExternalScriptPlugin::rowsInserted(const QModelIndex& /*parent*/, int start, int end)
{
    setupKeys(start, end);
    for (int row = start; row <= end; ++row) {
        saveItemForRow(row);
    }
}

void ExternalScriptPlugin::rowsAboutToBeRemoved(const QModelIndex& /*parent*/, int start, int end)
{
    KConfigGroup config = getConfig();
    for (int row = start; row <= end; ++row) {
        const ExternalScriptItem* const item = static_cast<ExternalScriptItem*>(m_model->item(row));
        KConfigGroup child = config.group(item->key());
        qCDebug(PLUGIN_EXTERNALSCRIPT) << "removing config group:" << child.name();
        child.deleteGroup();
    }

    config.sync();
}

void ExternalScriptPlugin::saveItem(const ExternalScriptItem* item)
{
    const QModelIndex index = m_model->indexFromItem(item);
    Q_ASSERT(index.isValid());

    getConfig().group(item->key()).deleteGroup(); // delete the previous group
    setupKeys(index.row(), index.row());
    saveItemForRow(index.row()); // save the new group
}

void ExternalScriptPlugin::saveItemForRow(int row)
{
    const QModelIndex idx = m_model->index(row, 0);
    Q_ASSERT(idx.isValid());

    auto* item = dynamic_cast<ExternalScriptItem*>(m_model->item(row));
    Q_ASSERT(item);

    qCDebug(PLUGIN_EXTERNALSCRIPT) << "save extern script:" << item << idx;
    KConfigGroup config = getConfig().group(item->key());
    config.writeEntry("name", item->text());
    config.writeEntry("command", item->command());
    config.writeEntry("inputMode", ( uint ) item->inputMode());
    config.writeEntry("outputMode", ( uint ) item->outputMode());
    config.writeEntry("errorMode", ( uint ) item->errorMode());
    config.writeEntry("saveMode", ( uint ) item->saveMode());
    config.writeEntry("shortcuts", item->action()->shortcut().toString());
    config.writeEntry("showOutput", item->showOutput());
    config.writeEntry("filterMode", item->filterMode());
    config.sync();
}

void ExternalScriptPlugin::setupKeys(int start, int end)
{
    QStringList keys = getConfig().groupList();

    for (int row = start; row <= end; ++row) {
        auto* const item = static_cast<ExternalScriptItem*>(m_model->item(row));

        int nextSuffix = 2;
        QString keyCandidate = item->text();
        for (; keys.contains(keyCandidate); ++nextSuffix) {
            keyCandidate = item->text() + QString::number(nextSuffix);
        }

        qCDebug(PLUGIN_EXTERNALSCRIPT) << "set key" << keyCandidate << "for" << item << item->command();
        item->setKey(keyCandidate);
        keys.push_back(keyCandidate);
    }
}

#include "externalscriptplugin.moc"
#include "moc_externalscriptplugin.cpp"
