#ifndef KDEVPLATFORM_PLUGIN_FILETEMPLATESPLUGIN_H
#define KDEVPLATFORM_PLUGIN_FILETEMPLATESPLUGIN_H

#include <interfaces/iplugin.h>
#include <interfaces/itemplateprovider.h>
#include <QVariantList>

class KUrl;
namespace KDevelop
{
class TemplatesModel;
class IToolViewFactory;
}


class FileTemplatesPlugin : public KDevelop::IPlugin, public KDevelop::ITemplateProvider
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ITemplateProvider)

public:
    enum TemplateType
    {
        NoTemplate,
        FileTemplate,
        ProjectTemplate
    };

    FileTemplatesPlugin(QObject* parent, const QVariantList& args);
    virtual ~FileTemplatesPlugin();
    virtual void unload();

    virtual KDevelop::ContextMenuExtension contextMenuExtension (KDevelop::Context* context);

    virtual QString name() const;
    virtual QIcon icon() const;
    virtual QAbstractItemModel* templatesModel() const;
    virtual QString knsConfigurationFile() const;
    virtual QStringList supportedMimeTypes() const;

    virtual void reload();
    virtual void loadTemplate(const QString& fileName);

    TemplateType determineTemplateType(const KUrl& url);

private:
    KDevelop::TemplatesModel* m_model;
    KDevelop::IToolViewFactory* m_toolView;

public slots:
    void createFromTemplate();
    void previewTemplate();
};

#endif // KDEVPLATFORM_PLUGIN_FILETEMPLATESPLUGIN_H
