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

#include "pathsmodel.h"

#include <interfaces/iproject.h>
#include <util/path.h>

#include <QFileDialog>

QString languageStandard(const QString& arguments)
{
    int idx = arguments.indexOf("-std=");
    if(idx == -1){
        return QStringLiteral("c++11");
    }

    idx += 5;
    int end = arguments.indexOf(' ', idx) != -1 ? arguments.indexOf(' ', idx) : arguments.size();
    return arguments.mid(idx, end - idx);
}

ConfigWidget::ConfigWidget(QWidget* parent, KDevelop::IProject* project)
    : QWidget(parent)
    , m_ui(new Ui::ConfigWidget())
    , m_project(project)
{
    const auto projectFolder = project->path().toLocalFile();

    m_ui->setupUi(this);
    m_model = new PathsModel(this);
    m_model->setProjectPath(project->path());

    m_ui->pathView->setModel(m_model);
    auto paths = ClangSettingsManager::self()->readPaths(project);
    if (paths.isEmpty()) {
        paths.append({ClangSettingsManager::self()->parserSettings(projectFolder, project), QStringLiteral(".")});
    }
    m_model->setPaths(paths);

    m_ui->pathView->setCurrentIndex(m_model->index(0));

    connect(m_ui->pathView, &QListView::activated, this, &ConfigWidget::itemActivated);
    connect(m_ui->parserOptions, &QLineEdit::textEdited, this, &ConfigWidget::textEdited);

    connect(m_model,&PathsModel::changed, this, &ConfigWidget::changed);
    connect(m_model,&PathsModel::rowsInserted, this, &ConfigWidget::itemActivated);
    connect(m_model,&PathsModel::rowsRemoved, this, &ConfigWidget::itemActivated);

    m_ui->parserOptions->setText(ClangSettingsManager::self()->parserSettings(projectFolder, m_project).parserOptions);

    QAction* delDefAction = new QAction(i18n("Delete path"), this);
    delDefAction->setShortcut(QKeySequence(Qt::Key_Delete));
    delDefAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    delDefAction->setIcon(QIcon::fromTheme("list-remove"));
    m_ui->pathView->addAction(delDefAction);
    m_ui->pathView->setContextMenuPolicy(Qt::ActionsContextMenu);
    connect(delDefAction, &QAction::triggered, this, &ConfigWidget::deletePath);

    connect(m_ui->addPath, &QPushButton::clicked, this, &ConfigWidget::addPath);
    connect(m_ui->removePath, &QPushButton::clicked, this, &ConfigWidget::deletePath);

    m_ui->languageStandards->setCurrentText(languageStandard(m_ui->parserOptions->text()));
    connect(m_ui->languageStandards, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::activated), this, &ConfigWidget::languageStandardChanged);
}

void ConfigWidget::itemActivated()
{
    const auto selection = m_ui->pathView->selectionModel()->currentIndex();
    if(!selection.isValid()){
        m_ui->parserOptions->clear();
        return;
    }

    auto settings = m_model->data(selection, PathsModel::ParserOptionsRole).value<ParserSettings>();

    m_ui->parserOptions->setText(settings.parserOptions);
    m_ui->languageStandards->setCurrentText(languageStandard(m_ui->parserOptions->text()));
}

void ConfigWidget::writeSettings()
{
    ClangSettingsManager::self()->writePaths(m_project, m_model->paths());
}

void ConfigWidget::deletePath()
{
    const auto selection = m_ui->pathView->selectionModel()->currentIndex();

    m_model->removeRow(selection.row());
}

void ConfigWidget::addPath()
{
    QFileDialog dlg(this, "", m_project->path().toLocalFile());
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setOption(QFileDialog::ReadOnly, true);
    if(dlg.exec() == QDialog::Rejected){
        return;
    }

    m_model->addPath(dlg.directoryUrl().toLocalFile());
    m_ui->pathView->setCurrentIndex(m_model->index(m_model->rowCount() - 1));
}

void ConfigWidget::textEdited()
{
    const auto parserOptions = m_ui->parserOptions->text();
    const auto index = m_ui->pathView->selectionModel()->currentIndex();

    m_model->setData(index, QVariant::fromValue(ParserSettings {parserOptions}), PathsModel::ParserOptionsRole);

    m_ui->languageStandards->setCurrentText(languageStandard(m_ui->parserOptions->text()));
}

void ConfigWidget::languageStandardChanged(const QString& standard)
{
    auto text = m_ui->parserOptions->text();

    auto currentStandard = languageStandard(text);

    m_ui->parserOptions->setText(text.replace(currentStandard, standard));

    textEdited();
}

void ConfigWidget::defaults()
{
    ParserSettingsEntry entry;
    entry.path = QStringLiteral(".");
    entry.settings = ClangSettingsManager::self()->defaultParserSettings();
    m_model->setPaths({entry});

    m_ui->parserOptions->setText(entry.settings.parserOptions);
    m_ui->languageStandards->setCurrentText(languageStandard(m_ui->parserOptions->text()));
}
