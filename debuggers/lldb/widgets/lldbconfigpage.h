/*
 * LLDB Debugger Support
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
    LldbConfigPage( QWidget* parent = 0 );
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
