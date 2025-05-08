/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CPPCHECK_GLOBAL_CONFIG_PAGE_H
#define CPPCHECK_GLOBAL_CONFIG_PAGE_H

#include <interfaces/configpage.h>

namespace cppcheck
{

class GlobalConfigPage: public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    GlobalConfigPage(KDevelop::IPlugin* plugin, QWidget* parent);
    ~GlobalConfigPage() override;

    KDevelop::ConfigPage::ConfigPageType configPageType() const override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

protected:
    bool needsResetDuringInitialization() const override
    {
        return false;
    }
};

}

#endif
