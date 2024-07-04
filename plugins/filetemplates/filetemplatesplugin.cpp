#include "filetemplatesplugin.h"
#include "templateclassassistant.h"
#include "templatepreviewtoolview.h"
#include "debug.h"

#include <language/codegen/templatesmodel.h>
#include <language/interfaces/editorcontext.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iselectioncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QIcon>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(FileTemplatesFactory, "kdevfiletemplates.json", registerPlugin<FileTemplatesPlugin>();)

class TemplatePreviewFactory : public KDevelop::IToolViewFactory
{
public:
    explicit TemplatePreviewFactory(FileTemplatesPlugin* plugin)
    : KDevelop::IToolViewFactory()
    , m_plugin(plugin)
    {

    }

    QWidget* create(QWidget* parent = nullptr) override
    {
        return new TemplatePreviewToolView(m_plugin, parent);
    }

    QString id() const override
    {
        return QStringLiteral("org.kdevelop.TemplateFilePreview");
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        return Qt::RightDockWidgetArea;
    }

private:
    FileTemplatesPlugin* m_plugin;
};

FileTemplatesPlugin::FileTemplatesPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args)
    : IPlugin(QStringLiteral("kdevfiletemplates"), parent, metaData)
{
    Q_UNUSED(args);

    setXMLFile(QStringLiteral("kdevfiletemplates.rc"));
    QAction* action = actionCollection()->addAction(QStringLiteral("new_from_template"));
    action->setText(i18nc("@action", "New from Template..."));
    action->setIcon( QIcon::fromTheme( QStringLiteral("code-class") ) );
    action->setWhatsThis(i18nc("@info:whatsthis", "Allows you to create new source code files, such as classes or unit tests, using templates." ) );
    action->setToolTip( i18nc("@info:tooltip",  "Create new files from a template" ) );
    connect (action, &QAction::triggered, this, &FileTemplatesPlugin::createFromTemplate);

    m_toolView = new TemplatePreviewFactory(this);
    core()->uiController()->addToolView(i18nc("@title:window", "Template Preview"), m_toolView);
}

FileTemplatesPlugin::~FileTemplatesPlugin()
{

}

void FileTemplatesPlugin::unload()
{
    core()->uiController()->removeToolView(m_toolView);
}

ContextMenuExtension FileTemplatesPlugin::contextMenuExtension(Context* context, QWidget* parent)
{
    ContextMenuExtension ext;
    QUrl fileUrl;

    if (context->type() == Context::ProjectItemContext)
    {
        auto* projectContext = static_cast<ProjectItemContext*>(context);
        QList<ProjectBaseItem*> items = projectContext->items();
        if (items.size() != 1)
        {
            return ext;
        }

        QUrl url;
        ProjectBaseItem* item = items.first();
        if (item->folder())
        {
            url = item->path().toUrl();
        }
        else if (item->target())
        {
            url = item->parent()->path().toUrl();
        }
        if (url.isValid())
        {
            auto* action = new QAction(i18nc("@action:inmenu", "Create from Template..."), parent);
            action->setIcon(QIcon::fromTheme(QStringLiteral("code-class")));
            action->setData(url);
            connect(action, &QAction::triggered, this, &FileTemplatesPlugin::createFromTemplate);
            ext.addAction(ContextMenuExtension::FileGroup, action);
        }

        if (item->file())
        {
            fileUrl = item->path().toUrl();
        }
    }
    else if (context->type() == Context::EditorContext)
    {
        auto* editorContext = static_cast<KDevelop::EditorContext*>(context);
        fileUrl = editorContext->url();
    }

    if (fileUrl.isValid() && determineTemplateType(fileUrl) != NoTemplate)
    {
        auto* action = new QAction(i18nc("@action:inmenu", "Show Template Preview"), parent);
        action->setIcon(QIcon::fromTheme(QStringLiteral("document-preview")));
        action->setData(fileUrl);
        connect(action, &QAction::triggered, this, &FileTemplatesPlugin::previewTemplate);
        ext.addAction(ContextMenuExtension::ExtensionGroup, action);
    }

    return ext;
}

QString FileTemplatesPlugin::name() const
{
    return i18n("File Templates");
}

QIcon FileTemplatesPlugin::icon() const
{
    return QIcon::fromTheme(QStringLiteral("code-class"));
}

QAbstractItemModel* FileTemplatesPlugin::templatesModel() const
{
    if(!m_model) {
        auto* self = const_cast<FileTemplatesPlugin*>(this);
        m_model = new TemplatesModel(QStringLiteral("kdevfiletemplates"), self);
    }
    return m_model;
}

QString FileTemplatesPlugin::knsConfigurationFile() const
{
    return QStringLiteral("kdevfiletemplates.knsrc");
}

QStringList FileTemplatesPlugin::supportedMimeTypes() const
{
    const QStringList types{
        QStringLiteral("application/x-desktop"),
        QStringLiteral("application/x-bzip-compressed-tar"),
        QStringLiteral("application/zip"),
    };
    return types;
}

void FileTemplatesPlugin::reload()
{
    templatesModel();
    m_model->refresh();
}

void FileTemplatesPlugin::loadTemplate(const QString& fileName)
{
    templatesModel();
    m_model->loadTemplateFile(fileName);
}

void FileTemplatesPlugin::createFromTemplate()
{
    QUrl baseUrl;
    if (auto* action = qobject_cast<QAction*>(sender()))
    {
        baseUrl = action->data().toUrl();
    }
    if (!baseUrl.isValid()) {
        // fall-back to currently active document's parent directory
        IDocument* doc = ICore::self()->documentController()->activeDocument();
        if (doc && doc->url().isValid()) {
            baseUrl = doc->url().adjusted(QUrl::RemoveFilename);
        }
    }
    if (!baseUrl.isValid()) {
        // fall-back to currently selected project's or item's base directory
        auto* projectContext = dynamic_cast<ProjectItemContext*>(ICore::self()->selectionController()->currentSelection());
        if (projectContext) {
            const QList<ProjectBaseItem*> items = projectContext->items();
            if (items.size() == 1) {
                ProjectBaseItem* item = items.at(0);
                if (item->folder()) {
                    baseUrl = item->path().toUrl();
                } else if (item->target()) {
                    baseUrl = item->parent()->path().toUrl();
                }
            }
        }
    }
    if (!baseUrl.isValid()) {
        // fall back to base directory of currently open project, if there is only one
        const QList<IProject*> projects = ICore::self()->projectController()->projects();
        if (projects.size() == 1) {
            baseUrl = projects.at(0)->path().toUrl();
        }
    }
    if (!baseUrl.isValid()) {
        // last resort
        baseUrl = ICore::self()->projectController()->projectsBaseDirectory();
    }
    auto* assistant = new TemplateClassAssistant(QApplication::activeWindow(), baseUrl);
    assistant->setAttribute(Qt::WA_DeleteOnClose);
    assistant->show();
}

FileTemplatesPlugin::TemplateType FileTemplatesPlugin::determineTemplateType(const QUrl& url)
{
    QDir dir(url.toLocalFile());

    /*
     * Search for a description file in the url's directory.
     * If it is not found there, try cascading up a maximum of 5 directories.
     */
    int level = 0;
    while (dir.cdUp() && level < 5)
    {
        const QStringList filters{QStringLiteral("*.kdevtemplate"), QStringLiteral("*.desktop")};
        const auto entries = dir.entryList(filters);
        for (const QString& entry : entries) {
            qCDebug(PLUGIN_FILETEMPLATES) << "Trying entry" << entry;
            /*
            * This logic is not perfect, but it works for most cases.
            *
            * Project template description files usually have the suffix
            * ".kdevtemplate", so those are easy to find. For project templates,
            * all the files in the directory are template files.
            *
            * On the other hand, file templates use the generic suffix ".desktop".
            * Fortunately, those explicitly list input and output files, so we
            * only match the explicitly listed files
            */
            if (entry.endsWith(QLatin1String(".kdevtemplate")))
            {
                return ProjectTemplate;
            }

            auto* config = new KConfig(dir.absoluteFilePath(entry), KConfig::SimpleConfig);
            KConfigGroup group = config->group(QStringLiteral("General"));

            qCDebug(PLUGIN_FILETEMPLATES) << "General group keys:" << group.keyList();

            if (!group.hasKey("Name") || !group.hasKey("Category"))
            {
                continue;
            }

            if (group.hasKey("Files"))
            {
                qCDebug(PLUGIN_FILETEMPLATES) << "Group has files " << group.readEntry("Files", QStringList());
                const auto outputFiles = group.readEntry("Files", QStringList());
                for (const QString& outputFile : outputFiles) {
                    if (dir.absoluteFilePath(config->group(outputFile).readEntry("File")) == url.toLocalFile())
                    {
                        return FileTemplate;
                    }
                }
            }

            if (group.hasKey("ShowFilesAfterGeneration"))
            {
                return ProjectTemplate;
            }
        }

        ++level;
    }

    return NoTemplate;
}

void FileTemplatesPlugin::previewTemplate()
{
    auto* action = qobject_cast<QAction*>(sender());
    if (!action || !action->data().toUrl().isValid())
    {
        return;
    }
    auto* preview = qobject_cast<TemplatePreviewToolView*>(core()->uiController()->findToolView(i18nc("@title:window", "Template Preview"), m_toolView));
    if (!preview)
    {
        return;
    }

    core()->documentController()->activateDocument(core()->documentController()->openDocument(action->data().toUrl()));
}

#include "filetemplatesplugin.moc"
#include "moc_filetemplatesplugin.cpp"
