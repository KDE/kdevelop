#ifndef TEMPLATECONFIG_H
#define TEMPLATECONFIG_H

#include <kcmodule.h>

namespace Ui
{
    class TemplateConfig;
}

class TemplateConfig : public KCModule
{
    Q_OBJECT
public:
    explicit TemplateConfig (QWidget* parent = 0, const QVariantList& args = QVariantList());
    virtual ~TemplateConfig();

private:
    Ui::TemplateConfig* ui;
};

#endif // TEMPLATECONFIG_H
