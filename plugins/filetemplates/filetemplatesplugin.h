#ifndef KDEVPLATFORM_PLUGIN_FILETEMPLATESPLUGIN_H
#define KDEVPLATFORM_PLUGIN_FILETEMPLATESPLUGIN_H

#include "filetemplateprovider.h"

#include <interfaces/iplugin.h>
#include <QVariantList>

class QUrl;
namespace KDevelop
{
class IToolViewFactory;
}

class FileTemplatesPlugin : public KDevelop::IPlugin, public FileTemplateProvider
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

    TemplateType determineTemplateType(const QUrl& url);

private:
    KDevelop::IToolViewFactory* m_toolView;

public Q_SLOTS:
    void createFromTemplate();
    void previewTemplate();
};

#endif // KDEVPLATFORM_PLUGIN_FILETEMPLATESPLUGIN_H
