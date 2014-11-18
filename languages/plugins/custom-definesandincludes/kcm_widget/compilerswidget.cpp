/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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

#include "compilerswidget.h"

#include <QAction>
#include <KLocalizedString>
#include <QFileDialog>
#include <QKeySequence>
#include <QMenu>
#include <QSignalMapper>

#include "ui_compilerswidget.h"
#include "compilersmodel.h"
#include "debugarea.h"

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>

#include "../compilerprovider/icompilerprovider.h"

namespace
{
ICompilerProvider* compilerProvider()
{
    auto compilerProvider = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ICompilerProvider");
    if (!compilerProvider || !compilerProvider->extension<ICompilerProvider>()) {
        return {};
    }

    return compilerProvider->extension<ICompilerProvider>();
}
}

CompilersWidget::CompilersWidget(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::CompilersWidget)
    , m_compilersModel(new CompilersModel(this))
{
    m_ui->setupUi(this);
    m_ui->compilers->setModel(m_compilersModel);
    m_ui->compilers->header()->setSectionResizeMode(QHeaderView::Stretch);

    m_addMenu = new QMenu(m_ui->addButton);
    m_mapper = new QSignalMapper(m_addMenu);
    connect(m_mapper, static_cast<void(QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped), this, &CompilersWidget::addCompiler);

    m_addMenu->clear();

    if (auto cp = compilerProvider()) {
        foreach (const auto& factory, cp->compilerFactories()) {
            QAction* action = new QAction(m_addMenu);
            action->setText(factory->name());
            connect(action, &QAction::triggered, m_mapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
            m_mapper->setMapping(action, factory->name());
            m_addMenu->addAction(action);
        }
        m_ui->addButton->setMenu(m_addMenu);
    }

    connect(m_ui->removeButton, &QPushButton::clicked, this, &CompilersWidget::deleteCompiler);

    auto delAction = new QAction( i18n("Delete compiler"), this );
    delAction->setShortcut( QKeySequence( "Del" ) );
    delAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    m_ui->compilers->addAction( delAction );
    connect( delAction, &QAction::triggered, this, &CompilersWidget::deleteCompiler );

    connect(m_ui->compilers->selectionModel(), &QItemSelectionModel::currentChanged, this, &CompilersWidget::compilerSelected);

    connect(m_ui->compilerName, &QLineEdit::editingFinished, this, &CompilersWidget::compilerEdited);
    connect(m_ui->compilerPath, &QLineEdit::editingFinished, this, &CompilersWidget::compilerEdited);

    connect(m_ui->compilerSelector, &QPushButton::clicked, this, &CompilersWidget::selectCompilerPathDialog);

    connect(m_compilersModel, &CompilersModel::compilerChanged, this, &CompilersWidget::compilerChanged);

    enableItems(false);
}

void CompilersWidget::setCompilers(const QVector< CompilerPointer >& compilers)
{
    m_compilersModel->setCompilers(compilers);
}

void CompilersWidget::clear()
{
    m_compilersModel->setCompilers({});
}

void CompilersWidget::deleteCompiler()
{
    definesAndIncludesDebug() << "Deleting compiler";
    QModelIndexList selection = m_ui->compilers->selectionModel()->selectedIndexes();
    foreach (const QModelIndex& row, selection) {
        if (row.column() == 1) {
            //Don't remove the same compiler twice
            continue;
        }
        m_compilersModel->removeRows(row.row(), 1, row.parent());
    }
}

void CompilersWidget::addCompiler(const QString& factoryName)
{
   foreach (const auto& factory, compilerProvider()->compilerFactories()) {
        if (factoryName == factory->name()) {
            //add compiler without any information, the user will fill the data in later
            auto compilerIndex = m_compilersModel->addCompiler(factory->createCompiler(QString(), QString()));

            m_ui->compilers->selectionModel()->select(compilerIndex, QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            compilerSelected(compilerIndex);
            break;
        }
    }
}

QVector< CompilerPointer > CompilersWidget::compilers() const
{
    return m_compilersModel->compilers();
}

void CompilersWidget::compilerSelected(const QModelIndex& index)
{
    auto compiler = index.data(CompilersModel::CompilerDataRole);
    if (compiler.value<CompilerPointer>()) {
        m_ui->compilerName->setText(compiler.value<CompilerPointer>()->name());
        m_ui->compilerPath->setText(compiler.value<CompilerPointer>()->path());
        enableItems(true);
    } else {
        enableItems(false);
    }
}

void CompilersWidget::compilerEdited()
{
    auto indexes = m_ui->compilers->selectionModel()->selectedIndexes();
    Q_ASSERT(!indexes.isEmpty());

    auto compiler = indexes.first().data(CompilersModel::CompilerDataRole);
    if (!compiler.value<CompilerPointer>()) {
        return;
    }

    compiler.value<CompilerPointer>()->setName(m_ui->compilerName->text());
    compiler.value<CompilerPointer>()->setPath(m_ui->compilerPath->text());

    m_compilersModel->updateCompiler(m_ui->compilers->selectionModel()->selection());
}

void CompilersWidget::selectCompilerPathDialog()
{
    const QString compilerPath = QFileDialog::getOpenFileName(this, tr("Select path to compiler"));
    if (compilerPath.isEmpty())
        return;

    m_ui->compilerPath->setText(compilerPath);
    compilerEdited();
}

void CompilersWidget::enableItems(bool enable)
{
    m_ui->compilerName->setEnabled(enable);
    m_ui->compilerPath->setEnabled(enable);
    m_ui->compilerSelector->setEnabled(enable);

    if(!enable) {
        m_ui->compilerName->clear();
        m_ui->compilerPath->clear();
    }
}
