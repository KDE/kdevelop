#include "filetemplatesplugin.h"
#include "templatepreview.h"
#include "templateclassassistant.h"

#include <language/codegen/templatesmodel.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <project/projectmodel.h>

#include <KIcon>
#include <KDebug>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KAboutData>
#include <KActionCollection>
#include <KAction>
#include <QApplication>

#define debug() kDebug(debugArea())

using namespace KDevelop;

int debugArea()
{
    static int area = KDebug::registerArea("kdevfiletemplates");
    return area;
}

K_PLUGIN_FACTORY(FileTemplatesFactory, registerPlugin<FileTemplatesPlugin>();)
K_EXPORT_PLUGIN(FileTemplatesFactory(KAboutData("kdevfiletemplates", "kdevfiletemplates", ki18n("File Templates Configuration"), "0.1", ki18n("Support for managing file templates"), KAboutData::License_GPL)))

class TemplatePreviewFactory : public KDevelop::IToolViewFactory
{
    virtual QWidget* create(QWidget* parent = 0)
    {
        return new TemplatePreview(parent);
    }

    virtual QString id() const
    {
        return "org.kdevelop.TemplateFilePreview";
    }

    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::RightDockWidgetArea;
    }
};

FileTemplatesPlugin::FileTemplatesPlugin(QObject* parent, const QVariantList& args)
: IPlugin(FileTemplatesFactory::componentData(), parent)
{
    Q_UNUSED(args);
    KDEV_USE_EXTENSION_INTERFACE(ITemplateProvider)

    setXMLFile("kdevfiletemplates.rc");
    KAction* action = actionCollection()->addAction("new_from_template");
    action->setText( i18n( "New from Template" ) );
    action->setIcon( KIcon( "code-class" ) );
    action->setWhatsThis( i18n( "<b>Create new files from a template</b><br/>Allows you to create new source code files, such as classes or unit tests, using templates." ) );
    action->setStatusTip( i18n( "Create new files from a template" ) );
    connect (action, SIGNAL(triggered(bool)), SLOT(createFromTemplate()));

    m_model = new TemplatesModel(core()->self()->componentData(), this);
    m_model->setDescriptionResourceType("filetemplate_descriptions");
    m_model->setTemplateResourceType("filetemplates");
    m_model->refresh();

    m_toolView = new TemplatePreviewFactory;
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
    if (context->type() != Context::ProjectItemContext)
    {
        return IPlugin::contextMenuExtension(context);
    }

    ProjectItemContext* projectContext = dynamic_cast<ProjectItemContext*>(context);
    QList<ProjectBaseItem*> items = projectContext->items();
    if (items.size() != 1)
    {
        return IPlugin::contextMenuExtension(context);
    }

    ContextMenuExtension ext;
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
        KAction* action = new KAction(i18n("Create from Template"), this);
        action->setIcon(KIcon("code-class"));
        action->setData(url);
        connect(action, SIGNAL(triggered(bool)), SLOT(createFromTemplate()));
        ext.addAction(ContextMenuExtension::FileGroup, action);
    }

    return ext;
}

QString FileTemplatesPlugin::name() const
{
    return i18n("File Templates");
}

QIcon FileTemplatesPlugin::icon() const
{
    return KIcon("code-class");
}

QAbstractItemModel* FileTemplatesPlugin::templatesModel() const
{
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
    m_model->refresh();
}

void FileTemplatesPlugin::loadTemplate(const QString& fileName)
{
    m_model->loadTemplateFile(fileName);
}

void FileTemplatesPlugin::createFromTemplate()
{
    KUrl baseUrl;
    if (QAction* action = qobject_cast<QAction*>(sender()))
    {
        baseUrl = action->data().value<KUrl>();
    }
    TemplateClassAssistant assistant(QApplication::activeWindow(), baseUrl);
    assistant.exec();
}
