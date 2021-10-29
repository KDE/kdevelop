/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_BGPREFERENCES_H
#define KDEVPLATFORM_BGPREFERENCES_H

#include <interfaces/configpage.h>

namespace Ui
{
class BGPreferences;
}

namespace KDevelop
{

class BGPreferences : public ConfigPage
{
    Q_OBJECT
public:
    explicit BGPreferences(QWidget* parent);
    ~BGPreferences() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    void apply() override;
    void reset() override;

private:
    Ui::BGPreferences *preferencesDialog;

};

}
#endif
