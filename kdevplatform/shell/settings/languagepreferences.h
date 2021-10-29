/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_LANGUAGEPREFERENCESPREFERENCES_H
#define KDEVPLATFORM_LANGUAGEPREFERENCESPREFERENCES_H

#include <interfaces/configpage.h>

namespace Ui
{
class LanguagePreferences;
}

namespace KDevelop
{

class LanguagePreferences : public ConfigPage
{
    Q_OBJECT
public:
    explicit LanguagePreferences(QWidget* parent);
    ~LanguagePreferences() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    void apply() override;

    void notifySettingsChanged();

private:
    Ui::LanguagePreferences *preferencesDialog;
};

}
#endif
