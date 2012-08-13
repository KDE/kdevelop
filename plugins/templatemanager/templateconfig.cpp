#include "templateconfig.h"
#include "templatepage.h"
#include "ui_templateconfig.h"

#include <interfaces/iplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/itemplateprovider.h>

#include <KPluginFactory>

K_PLUGIN_FACTORY(TemplateConfigFactory, registerPlugin<TemplateConfig>();)
K_EXPORT_PLUGIN(TemplateConfigFactory("kdevtemplates_config"))

TemplateConfig::TemplateConfig(QWidget* parent, const QVariantList& args)
: KCModule(TemplateConfigFactory::componentData(), parent, args)
{
    ui = new Ui::TemplateConfig;
    ui->setupUi(this);

    foreach (KDevelop::IPlugin* plugin, KDevelop::ICore::self()->pluginController()->allPluginsForExtension("org.kdevelop.ITemplateProvider"))
    {
        if (KDevelop::ITemplateProvider* provider = plugin->extension<KDevelop::ITemplateProvider>())
        {
            ui->ktabwidget->addTab(new TemplatePage(provider), provider->icon(), provider->name());
        }
    }
}

TemplateConfig::~TemplateConfig()
{
    delete ui;
}

