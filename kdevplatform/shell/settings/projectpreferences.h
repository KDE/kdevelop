/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROJECTPREFERENCES_H
#define KDEVPLATFORM_PROJECTPREFERENCES_H

#include <interfaces/configpage.h>

namespace Ui
{
class ProjectPreferences;
}

namespace KDevelop
{

class ProjectPreferences : public ConfigPage
{
    Q_OBJECT
public:
    explicit ProjectPreferences(QWidget *parent);
    ~ProjectPreferences() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    void apply() override;

private Q_SLOTS:
    void slotSettingsChanged();

private:
    Ui::ProjectPreferences *preferencesDialog;

};

}
#endif
