/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVCLAZY_GLOBAL_CONFIG_PAGE_H
#define KDEVCLAZY_GLOBAL_CONFIG_PAGE_H

#include <interfaces/configpage.h>

// plugin
#include "checksdb.h"


namespace Clazy
{
namespace Ui {
class GlobalConfigPage;
}
class CheckSetSelectionManager;

class GlobalConfigPage: public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    GlobalConfigPage(CheckSetSelectionManager* checkSetSelectionManager,
                     const QSharedPointer<const ChecksDB>& db,
                     KDevelop::IPlugin* plugin, QWidget* parent);
    ~GlobalConfigPage() override = default;

    KDevelop::ConfigPage::ConfigPageType configPageType() const override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    void apply() override;
    void defaults() override;
    void reset() override;

protected:
    bool needsResetDuringInitialization() const override
    {
        return false;
    }

private:
    Ui::GlobalConfigPage* ui;
    CheckSetSelectionManager* const m_checkSetSelectionManager;
};

}

#endif
