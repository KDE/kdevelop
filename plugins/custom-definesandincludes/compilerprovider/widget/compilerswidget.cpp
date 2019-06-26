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
#include <QKeySequence>
#include <QMenu>
#include <QSignalBlocker>

#include "ui_compilerswidget.h"
#include "compilersmodel.h"
#include "../compilerprovider/settingsmanager.h"
#include "../compilerprovider/compilerprovider.h"
#include <debug.h>

using namespace KDevelop;

CompilersWidget::CompilersWidget(QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
    , m_ui(new Ui::CompilersWidget)
    , m_compilersModel(new CompilersModel(this))
{
    m_ui->setupUi(this);
    m_ui->compilers->setModel(m_compilersModel);
    m_ui->compilers->header()->setSectionResizeMode(QHeaderView::Stretch);

    m_addMenu = new QMenu(m_ui->addButton);

    m_addMenu->clear();

    auto settings = SettingsManager::globalInstance();
    auto provider = settings->provider();
    const auto compilerFactories = provider->compilerFactories();
    for (const auto& factory : compilerFactories) {
        auto* action = new QAction(m_addMenu);
        const QString fname = factory->name();
        action->setText(fname);
        connect(action, &QAction::triggered, this, [this, fname]() { addCompiler(fname); });
        m_addMenu->addAction(action);
    }
    m_ui->addButton->setMenu(m_addMenu);

    connect(m_ui->removeButton, &QPushButton::clicked, this, &CompilersWidget::deleteCompiler);

    auto delAction = new QAction( i18n("Delete compiler"), this );
    delAction->setShortcut( QKeySequence( QStringLiteral("Del") ) );
    delAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    m_ui->compilers->addAction( delAction );
    connect( delAction, &QAction::triggered, this, &CompilersWidget::deleteCompiler );

    connect(m_ui->compilers->selectionModel(), &QItemSelectionModel::currentChanged, this, &CompilersWidget::compilerSelected);

    connect(m_ui->compilerName, &QLineEdit::textEdited, this, &CompilersWidget::compilerEdited);

    connect(m_ui->compilerPath, &KUrlRequester::textEdited, this, &CompilersWidget::compilerEdited);

    connect(m_compilersModel, &CompilersModel::compilerChanged, this, &CompilersWidget::compilerChanged);

    enableItems(false);
}

CompilersWidget::~CompilersWidget()
{
}

void CompilersWidget::setCompilers(const QVector< CompilerPointer >& compilers)
{
    m_compilersModel->setCompilers(compilers);
    m_ui->compilers->expandAll();
}

void CompilersWidget::clear()
{
    m_compilersModel->setCompilers({});
}

void CompilersWidget::deleteCompiler()
{
    qCDebug(DEFINESANDINCLUDES) << "Deleting compiler";
    auto selectionModel = m_ui->compilers->selectionModel();
    const auto selectedRowsBefore = selectionModel->selectedIndexes();
    for (const QModelIndex& row : selectedRowsBefore) {
        if (row.column() == 1) {
            //Don't remove the same compiler twice
            continue;
        }

        if(m_compilersModel->removeRows(row.row(), 1, row.parent())) {
            auto selectedCompiler = selectionModel->selectedIndexes();
            compilerSelected(selectedCompiler.isEmpty() ? QModelIndex() : selectedCompiler.first());
        }
    }

    emit changed();
}

void CompilersWidget::addCompiler(const QString& factoryName)
{
    auto settings = SettingsManager::globalInstance();
    auto provider = settings->provider();
    const auto compilerFactories = provider->compilerFactories();
    for (const auto& factory : compilerFactories) {
        if (factoryName == factory->name()) {
            //add compiler without any information, the user will fill the data in later
            auto compilerIndex = m_compilersModel->addCompiler(factory->createCompiler(QString(), QString()));

            m_ui->compilers->selectionModel()->select(compilerIndex, QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            compilerSelected(compilerIndex);
            m_ui->compilers->scrollTo(compilerIndex);
            m_ui->compilerName->setFocus(Qt::OtherFocusReason);
            break;
        }
    }

    emit changed();
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
        
        //NOTE: there is a bug in kLineEdit, which causes textEdited signal to be
        // spuriously emitted on calling setText(). See bug report here:
        // https://bugs.kde.org/show_bug.cgi?id=388798
        // The resulting spurious call of compilerEdited then fails with an assert.
        //Work around this bug until it is fixed upstream by disabling signals here
        const QSignalBlocker blocker(m_ui->compilerPath);
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

    emit changed();
}

void CompilersWidget::enableItems(bool enable)
{
    m_ui->compilerName->setEnabled(enable);
    m_ui->compilerPath->setEnabled(enable);

    if(!enable) {
        m_ui->compilerName->clear();
        
        //NOTE: this is to work around the 
        //spurious signal bug in kLineEdit
        const QSignalBlocker blocker(m_ui->compilerPath);
        m_ui->compilerPath->clear();
    }
}

void CompilersWidget::reset()
{
    auto settings = SettingsManager::globalInstance();
    setCompilers(settings->provider()->compilers());
}

void CompilersWidget::apply()
{
    auto settings = SettingsManager::globalInstance();
    auto provider = settings->provider();

    settings->writeUserDefinedCompilers(compilers());

    const auto& providerCompilers = provider->compilers();
    const auto& widgetCompilers = compilers();
    for (auto& compiler: providerCompilers) {
        if (!widgetCompilers.contains(compiler)) {
            provider->unregisterCompiler(compiler);
        }
    }

    for (auto& compiler: widgetCompilers) {
        if (!providerCompilers.contains(compiler)) {
            provider->registerCompiler(compiler);
        }
    }
}

void CompilersWidget::defaults()
{
}

QString CompilersWidget::name() const
{
    return i18n("C/C++ Compilers");
}

QString CompilersWidget::fullName() const
{
    return i18n("Configure C/C++ Compilers");
}

QIcon CompilersWidget::icon() const
{
    return QIcon::fromTheme(QStringLiteral("kdevelop"));
}

KDevelop::ConfigPage::ConfigPageType CompilersWidget::configPageType() const
{
    return ConfigPage::LanguageConfigPage;
}
