#ifndef KDEVPLATFORM_PLUGIN_FILETEMPLATESPLUGIN_H
#define KDEVPLATFORM_PLUGIN_FILETEMPLATESPLUGIN_H

#include <interfaces/iplugin.h>
#include <interfaces/itemplateprovider.h>
#include <QVariantList>

class QUrl;
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
    virtual void unload() override;

    virtual KDevelop::ContextMenuExtension contextMenuExtension (KDevelop::Context* context) override;

    virtual QString name() const override;
    virtual QIcon icon() const override;
    virtual QAbstractItemModel* templatesModel() override;
    virtual QString knsConfigurationFile() const override;
    virtual QStringList supportedMimeTypes() const override;

    virtual void reload() override;
    virtual void loadTemplate(const QString& fileName) override;

    TemplateType determineTemplateType(const QUrl& url);

private:
    KDevelop::TemplatesModel* m_model;
    KDevelop::IToolViewFactory* m_toolView;

public slots:
    void createFromTemplate();
    void previewTemplate();
};

#endif // KDEVPLATFORM_PLUGIN_FILETEMPLATESPLUGIN_H
