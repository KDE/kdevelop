#include "filetemplatesplugin.h"
#include "templatepreview.h"

#include <language/codegen/templatesmodel.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include <KIcon>
#include <KDebug>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KAboutData>

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

void FileTemplatesPlugin::reload()
{
    m_model->refresh();
}

void FileTemplatesPlugin::loadTemplate (const QString& fileName)
{
    m_model->loadTemplateFile(fileName);
}

QStringList FileTemplatesPlugin::supportedMimeTypes()
{
    QStringList types;
    types << "application/x-desktop";
    types << "application/x-bzip-compressed-tar";
    types << "application/zip";
    return types;
}

QString FileTemplatesPlugin::knsConfigurationFile()
{
    return "filetemplates.knsrc";
}

QAbstractItemModel* FileTemplatesPlugin::templatesModel()
{
    return m_model;
}

QIcon FileTemplatesPlugin::icon()
{
    return KIcon("code-class");
}

QString FileTemplatesPlugin::name()
{
    return i18n("File Templates");
}

