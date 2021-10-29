/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_UIPREFERENCES_H
#define KDEVPLATFORM_UIPREFERENCES_H

#include "configpage.h"

namespace Ui{
    class UiConfig;
}

/**
 @author Andreas Pakulat <apaku@gmx.de>
*/

class UiPreferences : public KDevelop::ConfigPage
{
    Q_OBJECT
public:
    explicit UiPreferences(QWidget* parent = nullptr);

    ~UiPreferences() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public Q_SLOTS:
    // need to customize behaviour
    void apply() override;
private:
    Ui::UiConfig* m_uiconfigUi;
};

#endif

