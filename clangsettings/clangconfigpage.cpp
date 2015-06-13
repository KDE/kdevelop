/*
 * This file is part of KDevelop
 *
 * Copyright 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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

#include "clangconfigpage.h"

#include "configwidget.h"

#include <QVBoxLayout>
#include <QIcon>

ClangConfigPage::ClangConfigPage(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
    : ProjectConfigPage<ClangProjectConfig>(plugin, options, parent)
{
    m_configWidget = new ConfigWidget(this, project());
    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_configWidget);

    connect(m_configWidget, &ConfigWidget::changed, this, &ClangConfigPage::changed);
}

ClangConfigPage::~ClangConfigPage()
{}

void ClangConfigPage::defaults()
{
    ProjectConfigPage::defaults();
    m_configWidget->defaults();
}

void ClangConfigPage::apply()
{
     ProjectConfigPage::apply();
     configWidget->writeSettings();
}

QString ClangConfigPage::name() const
{
    return i18n("Clang command-line options");
}

QString ClangConfigPage::fullName() const
{
    return i18n("Configure Clang command-line options");
}

QIcon ClangConfigPage::icon() const
{
    return QIcon::fromTheme("kdevelop");
}
