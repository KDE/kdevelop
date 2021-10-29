/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ENVIRONMENTPREFERENCES_H
#define KDEVPLATFORM_ENVIRONMENTPREFERENCES_H

#include <interfaces/configpage.h>

namespace KDevelop
{
class EnvironmentPreferencesPrivate;

class EnvironmentPreferences : public ConfigPage
{
    Q_OBJECT
public:
    explicit EnvironmentPreferences(const QString& preselectedProfileName = {}, QWidget* parent = nullptr);
    ~EnvironmentPreferences() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    void apply() override;
    void reset() override;
    void defaults() override;

private:
    const QScopedPointer<class EnvironmentPreferencesPrivate> d_ptr;
    Q_DECLARE_PRIVATE(EnvironmentPreferences)
};

}

#endif

