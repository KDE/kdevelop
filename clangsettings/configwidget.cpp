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

#include "configwidget.h"

#include "ui_configwidget.h"

#include <QFileSystemModel>

#include <interfaces/iproject.h>
#include <util/path.h>

namespace
{
int indexOfPath(const QList<ParserSettingsEntry>& paths, const QString& path)
{
    for (int i = 0; i < paths.size(); i++) {
        if (paths[i].path == path) {
            return i;
        }
    }

    return -1;
}

QString parserOptions(const QList<ParserSettingsEntry>& paths, const QString& path, KDevelop::IProject* project)
{
    int idx = indexOfPath(paths, path);
    if (idx != -1) {
        return paths[idx].settings.parserOptions;
    }

    return ClangSettingsManager::self()->parserSettings(path, project).parserOptions;
}

}

ConfigWidget::ConfigWidget(QWidget* parent, KDevelop::IProject* project)
    : QWidget(parent)
    , m_ui(new Ui::ConfigWidget())
    , m_project(project)
    , m_paths(ClangSettingsManager::self()->readPaths(project))
{
    const auto parentFolder = project->path().parent().toLocalFile();
    const auto projectFolder = project->path().toLocalFile();

    m_ui->setupUi(this);
    auto model = new QFileSystemModel(this);
    model->setRootPath(projectFolder);
    model->setReadOnly(true);

    m_ui->pathView->setModel(model);
    m_ui->pathView->setRootIndex(model->index(parentFolder));
    m_ui->pathView->setSortingEnabled(false);

    for (int i = model->columnCount() - 1; i >= 1; i--) {
        m_ui->pathView->setColumnHidden(i, true);
    }

    // TODO: hide somehow all other non project folders in the parent directory
    m_ui->pathView->setCurrentIndex(model->index(projectFolder));

    connect(m_ui->pathView, &QTreeView::activated, this, &ConfigWidget::itemActivated);
    connect(m_ui->parserOptions, &QLineEdit::textEdited, this, &ConfigWidget::textEdited);

    m_ui->parserOptions->setText(ClangSettingsManager::self()->parserSettings(projectFolder, m_project).parserOptions);
}

void ConfigWidget::itemActivated(const QModelIndex& index)
{
    auto model = static_cast<const QFileSystemModel*>(index.model());
    auto path = model->filePath(index);

    auto options = parserOptions(m_paths, path, m_project);
    m_ui->parserOptions->setText(options);
}

void ConfigWidget::textEdited()
{
    const auto parserOptions = m_ui->parserOptions->text();
    const auto index = m_ui->pathView->selectionModel()->currentIndex();
    auto model = static_cast<const QFileSystemModel*>(index.model());
    const auto path = model->filePath(index);

    if(path.isEmpty()){
        return;
    }

    auto currentSettings = ClangSettingsManager::self()->parserSettings(path, m_project);

    if(currentSettings.parserOptions == parserOptions){
        return;
    }

    int idx = indexOfPath(m_paths, path);
    if (idx == -1) {
        m_paths.append({{parserOptions}, path});
    } else {
        m_paths[idx] = {{parserOptions}, path};
    }
}

void ConfigWidget::writeSettings()
{
    ClangSettingsManager::self()->writePaths(m_project, m_paths);
}
