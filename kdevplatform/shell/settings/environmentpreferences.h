/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
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

