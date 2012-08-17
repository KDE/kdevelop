#ifndef FILETEMPLATESPLUGIN_H
#define FILETEMPLATESPLUGIN_H

#include <interfaces/iplugin.h>
#include <interfaces/itemplateprovider.h>
#include <QVariantList>

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
    FileTemplatesPlugin(QObject* parent, const QVariantList& args);
    virtual ~FileTemplatesPlugin();
    virtual void unload();

    virtual QString name() const;
    virtual QIcon icon() const;
    virtual QAbstractItemModel* templatesModel() const;
    virtual QString knsConfigurationFile() const;
    virtual QStringList supportedMimeTypes() const;

    virtual void reload();
    virtual void loadTemplate(const QString& fileName);

private:
    KDevelop::TemplatesModel* m_model;
    KDevelop::IToolViewFactory* m_toolView;

public slots:
    void createFromTemplate();
};

#endif // FILETEMPLATESPLUGIN_H
