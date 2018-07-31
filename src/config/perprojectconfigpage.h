/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CLANGTIDY_PERPROJECTCONFIGPAGE_H_
#define CLANGTIDY_PERPROJECTCONFIGPAGE_H_

#include <QItemSelectionModel>
#include <interfaces/configpage.h>

#include "config/configgroup.h"

class QIcon;
class QStringListModel;

namespace KDevelop
{
class IProject;
}

namespace ClangTidy
{
class Plugin;

namespace Ui
{
    class PerProjectConfig;
}
/**
 * \class
 * \brief Implements the clang-tidy's configuration project for the current
 * project.
 */
class PerProjectConfigPage : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    PerProjectConfigPage(KDevelop::IProject* project, QWidget* parent);
    ~PerProjectConfigPage() override;

    ConfigPageType configPageType() const override;
    QString name() const override;
    QIcon icon() const override;
    void setList(const QStringList& list);
    void setActiveChecksReceptorList(QStringList* list);

public Q_SLOTS:
    void apply() override;
    void defaults() override;
    void reset() override;

private:
    KDevelop::IProject* m_project;
    Ui::PerProjectConfig* ui;
    ConfigGroup m_config;
    QStringList* m_activeChecksReceptor;
    QStringList m_underlineAvailChecks;
    QStringListModel* m_availableChecksModel;
    QItemSelectionModel* m_selectedItemModel;
};
}

#endif /* CLANGTIDY_PERPROJECTCONFIGPAGE_H_ */
