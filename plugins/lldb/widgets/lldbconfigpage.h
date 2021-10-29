/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDBCONFIGPAGE_H
#define LLDBCONFIGPAGE_H

#include <interfaces/launchconfigurationpage.h>

namespace Ui
{
class LldbConfigPage;
}

class LldbConfigPage : public KDevelop::LaunchConfigurationPage
{
    Q_OBJECT
public:
    explicit LldbConfigPage( QWidget* parent = nullptr );
    ~LldbConfigPage() override;

    QIcon icon() const override;
    QString title() const override;
    void loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject *proj = nullptr) override;
    void saveToConfiguration(KConfigGroup cfg, KDevelop::IProject *proj = nullptr) const override;

private:
    Ui::LldbConfigPage* ui;
};

class LldbConfigPageFactory : public KDevelop::LaunchConfigurationPageFactory
{
public:
    KDevelop::LaunchConfigurationPage* createWidget(QWidget* parent) override;
};


#endif // LLDBCONFIGPAGE_H
