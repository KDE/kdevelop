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

    FileTemplatesPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);
    ~FileTemplatesPlugin() override;
    void unload() override;

    KDevelop::ContextMenuExtension contextMenuExtension (KDevelop::Context* context, QWidget* parent) override;

    QString name() const override;
    QIcon icon() const override;
    QAbstractItemModel* templatesModel() const override;
    QString knsConfigurationFile() const override;
    QStringList supportedMimeTypes() const override;

    void reload() override;
    void loadTemplate(const QString& fileName) override;

    TemplateType determineTemplateType(const QUrl& url);

private:
    // created lazily
    mutable KDevelop::TemplatesModel* m_model = nullptr;
    KDevelop::IToolViewFactory* m_toolView;

public Q_SLOTS:
    void createFromTemplate();
    void previewTemplate();
};

#endif // KDEVPLATFORM_PLUGIN_FILETEMPLATESPLUGIN_H
