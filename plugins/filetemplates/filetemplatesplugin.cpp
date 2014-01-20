#include "filetemplatesplugin.h"
#include "templateclassassistant.h"
#include "templatepreviewtoolview.h"

#include <language/codegen/templatesmodel.h>
#include <language/interfaces/editorcontext.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/idocumentcontroller.h>
#include <project/projectmodel.h>

#include <KDebug>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KAboutData>
#include <KActionCollection>
#include <KConfigGroup>

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QIcon>

#define debug() kDebug(debugArea())

using namespace KDevelop;

int debugArea()
{
    static int area = KDebug::registerArea("kdevfiletemplates");
    return area;
}

// K_PLUGIN_FACTORY(FileTemplatesFactory, registerPlugin<FileTemplatesPlugin>();)
// K_EXPORT_PLUGIN(FileTemplatesFactory(KAboutData("kdevfiletemplates", "kdevfiletemplates", ki18n("File Templates Configuration"), "0.1", ki18n("Manages templates for source files"), KAboutData::License_GPL)))

class TemplatePreviewFactory : public KDevelop::IToolViewFactory
{
public:
    TemplatePreviewFactory(FileTemplatesPlugin* plugin)
    : KDevelop::IToolViewFactory()
    , m_plugin(plugin)
    {

    }

    virtual QWidget* create(QWidget* parent = 0)
    {
        return new TemplatePreviewToolView(m_plugin, parent);
    }

    virtual QString id() const
    {
        return "org.kdevelop.TemplateFilePreview";
    }

    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::RightDockWidgetArea;
    }

private:
    FileTemplatesPlugin* m_plugin;
};

FileTemplatesPlugin::FileTemplatesPlugin(QObject* parent, const QVariantList& args)
    : IPlugin("kdevfiletemplates", parent)
    , m_model(0)
{
    Q_UNUSED(args);
    KDEV_USE_EXTENSION_INTERFACE(ITemplateProvider)

    setXMLFile("kdevfiletemplates.rc");
    QAction* action = actionCollection()->addAction("new_from_template");
    action->setText( i18n( "New From Template" ) );
    action->setIcon( QIcon::fromTheme( "code-class" ) );
    action->setWhatsThis( i18n( "Allows you to create new source code files, such as classes or unit tests, using templates." ) );
    action->setStatusTip( i18n( "Create new files from a template" ) );
    connect (action, SIGNAL(triggered(bool)), SLOT(createFromTemplate()));

    m_toolView = new TemplatePreviewFactory(this);
    core()->uiController()->addToolView(i18n("Template Preview"), m_toolView);
}

FileTemplatesPlugin::~FileTemplatesPlugin()
{

}

void FileTemplatesPlugin::unload()
{
    core()->uiController()->removeToolView(m_toolView);
}

ContextMenuExtension FileTemplatesPlugin::contextMenuExtension (Context* context)
{
    ContextMenuExtension ext;
    KUrl fileUrl;

    if (context->type() == Context::ProjectItemContext)
    {
        ProjectItemContext* projectContext = dynamic_cast<ProjectItemContext*>(context);
        QList<ProjectBaseItem*> items = projectContext->items();
        if (items.size() != 1)
        {
            return ext;
        }

        KUrl url;
        ProjectBaseItem* item = items.first();
        if (item->folder())
        {
            url = item->url();
        }
        else if (item->target())
        {
            url = item->parent()->url();
        }
        if (url.isValid())
        {
            QAction* action = new QAction(i18n("Create From Template"), this);
            action->setIcon(QIcon::fromTheme("code-class"));
            action->setData(url);
            connect(action, SIGNAL(triggered(bool)), SLOT(createFromTemplate()));
            ext.addAction(ContextMenuExtension::FileGroup, action);
        }

        if (item->file())
        {
            fileUrl = item->url();
        }
    }
    else if (context->type() == Context::EditorContext)
    {
        KDevelop::EditorContext* editorContext = dynamic_cast<KDevelop::EditorContext*>(context);
        fileUrl = editorContext->url();
    }

    if (fileUrl.isValid() && determineTemplateType(fileUrl) != NoTemplate)
    {
        QAction* action = new QAction(i18n("Show Template Preview"), this);
        action->setIcon(QIcon::fromTheme("document-preview"));
        action->setData(fileUrl);
        connect(action, SIGNAL(triggered(bool)), SLOT(previewTemplate()));
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
    return QIcon::fromTheme("code-class");
}

QAbstractItemModel* FileTemplatesPlugin::templatesModel()
{
    if(!m_model) {
        m_model = new TemplatesModel("kdevfiletemplates", this);
    }
    return m_model;
}

QString FileTemplatesPlugin::knsConfigurationFile() const
{
    return "kdevfiletemplates.knsrc";
}

QStringList FileTemplatesPlugin::supportedMimeTypes() const
{
    QStringList types;
    types << "application/x-desktop";
    types << "application/x-bzip-compressed-tar";
    types << "application/zip";
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
    KUrl baseUrl;
    if (QAction* action = qobject_cast<QAction*>(sender()))
    {
        baseUrl = action->data().value<KUrl>();
    }
    if (!baseUrl.isValid()) {
        // fall-back to currently active document's parent directory
        IDocument* doc = ICore::self()->documentController()->activeDocument();
        if (doc && doc->url().isValid()) {
            baseUrl = doc->url().upUrl();
        }
    }
    TemplateClassAssistant* assistant = new TemplateClassAssistant(QApplication::activeWindow(), baseUrl);
    assistant->setAttribute(Qt::WA_DeleteOnClose);
    assistant->show();
}

FileTemplatesPlugin::TemplateType FileTemplatesPlugin::determineTemplateType(const KUrl& url)
{
    QDir dir(url.toLocalFile());

    /*
     * Search for a description file in the url's directory.
     * If it is not found there, try cascading up a maximum of 5 directories.
     */
    int level = 0;
    while (dir.cdUp() && level < 5)
    {
        QStringList filters;
        filters << "*.kdevtemplate" << "*.desktop";
        foreach (const QString& entry, dir.entryList(filters))
        {
            kDebug() << "Trying entry" << entry;
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
            if (entry.endsWith(".kdevtemplate"))
            {
                return ProjectTemplate;
            }

            KConfig* config = new KConfig(dir.absoluteFilePath(entry), KConfig::SimpleConfig);
            KConfigGroup group = config->group("General");

            kDebug() << "General group keys:" << group.keyList();

            if (!group.hasKey("Name") || !group.hasKey("Category"))
            {
                continue;
            }

            if (group.hasKey("Files"))
            {
                kDebug() << "Group has files " << group.readEntry("Files", QStringList());
                foreach (const QString& outputFile, group.readEntry("Files", QStringList()))
                {
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
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action || !action->data().value<KUrl>().isValid())
    {
        return;
    }
    TemplatePreviewToolView* preview = qobject_cast<TemplatePreviewToolView*>(core()->uiController()->findToolView(i18n("Template Preview"), m_toolView));
    if (!preview)
    {
        return;
    }

    core()->documentController()->activateDocument(core()->documentController()->openDocument(action->data().value<KUrl>()));
}
