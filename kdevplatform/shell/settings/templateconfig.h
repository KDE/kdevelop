/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEMPLATECONFIG_H
#define KDEVPLATFORM_TEMPLATECONFIG_H

#include <interfaces/configpage.h>

namespace Ui
{
    class TemplateConfig;
}

class TemplateConfig : public KDevelop::ConfigPage
{
    Q_OBJECT
public:
    explicit TemplateConfig(QWidget* parent = nullptr);
    ~TemplateConfig() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    void reset() override;
    void apply() override;
    void defaults() override;
private:
    Ui::TemplateConfig* ui;
};

#endif // KDEVPLATFORM_PLUGIN_TEMPLATECONFIG_H
