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
    FileTemplatesPlugin (QObject* parent, const QVariantList& args);
    virtual ~FileTemplatesPlugin();
    virtual void unload();
    
    virtual void reload();
    virtual void loadTemplate (const QString& fileName);
    virtual QStringList supportedMimeTypes();
    virtual QString knsConfigurationFile();
    virtual QAbstractItemModel* templatesModel();
    virtual QIcon icon();
    virtual QString name();
    
private:
    KDevelop::TemplatesModel* m_model;
    KDevelop::IToolViewFactory* m_toolView;
};

#endif // FILETEMPLATESPLUGIN_H
