/************************************************************************
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>         *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include "projectpathswidget.h"

#include <QFileDialog>
#include <QPointer>
#include <QRegExp>

#include <interfaces/iproject.h>

#include "../compilerprovider/compilerprovider.h"
#include "../compilerprovider/settingsmanager.h"

#include "ui_projectpathswidget.h"
#include "ui_batchedit.h"
#include "projectpathsmodel.h"
#include <debug.h>

#include <KMessageBox>
#include <KLocalizedString>


using namespace KDevelop;

namespace
{
enum PageType {
    IncludesPage,
    DefinesPage,
    ParserArgumentsPage
};
}

ProjectPathsWidget::ProjectPathsWidget( QWidget* parent )
    : QWidget(parent),
      ui(new Ui::ProjectPathsWidget),
      pathsModel(new ProjectPathsModel(this))
{
    ui->setupUi( this );

    // hack taken from kurlrequester, make the buttons a bit less in height so they better match the url-requester
    ui->addPath->setFixedHeight( ui->projectPaths->sizeHint().height() );
    ui->removePath->setFixedHeight( ui->projectPaths->sizeHint().height() );

    connect( ui->addPath, &QPushButton::clicked, this, &ProjectPathsWidget::addProjectPath );
    connect( ui->removePath, &QPushButton::clicked, this, &ProjectPathsWidget::deleteProjectPath );
    connect( ui->batchEdit, &QPushButton::clicked, this, &ProjectPathsWidget::batchEdit );

    ui->projectPaths->setModel( pathsModel );
    connect( ui->projectPaths, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &ProjectPathsWidget::projectPathSelected );
    connect( pathsModel, &ProjectPathsModel::dataChanged, this, &ProjectPathsWidget::changed );
    connect( pathsModel, &ProjectPathsModel::rowsInserted, this, &ProjectPathsWidget::changed );
    connect( pathsModel, &ProjectPathsModel::rowsRemoved, this, &ProjectPathsWidget::changed );
    connect( ui->compiler, QOverload<const QString&>::of(&QComboBox::activated), this, &ProjectPathsWidget::changed );
    connect( ui->compiler, QOverload<const QString&>::of(&QComboBox::activated), this, &ProjectPathsWidget::changeCompilerForPath );

    connect( ui->includesWidget, QOverload<const QStringList&>::of(&IncludesWidget::includesChanged), this, &ProjectPathsWidget::includesChanged );
    connect( ui->definesWidget, QOverload<const KDevelop::Defines&>::of(&DefinesWidget::definesChanged), this, &ProjectPathsWidget::definesChanged );

    connect(ui->languageParameters, &QTabWidget::currentChanged, this, &ProjectPathsWidget::tabChanged);

    connect(ui->parserWidget, &ParserWidget::changed, this, &ProjectPathsWidget::parserArgumentsChanged);

    tabChanged(IncludesPage);
}

ProjectPathsWidget::~ProjectPathsWidget()
{
}

QVector<ConfigEntry> ProjectPathsWidget::paths() const
{
    return pathsModel->paths();
}

void ProjectPathsWidget::setPaths( const QVector<ConfigEntry>& paths )
{
    bool b = blockSignals( true );
    clear();
    pathsModel->setPaths( paths );
    blockSignals( b );
    ui->projectPaths->setCurrentIndex(0); // at least a project root item is present
    ui->languageParameters->setCurrentIndex(0);

    // Set compilers
    ui->compiler->clear();
    auto settings = SettingsManager::globalInstance();
    auto compilers = settings->provider()->compilers();
    for (int i = 0 ; i < compilers.count(); ++i) {
        Q_ASSERT(compilers[i]);
        if (!compilers[i]) {
            continue;
        }
        ui->compiler->addItem(compilers[i]->name());
        QVariant val; val.setValue(compilers[i]);
        ui->compiler->setItemData(i, val);
    }

    projectPathSelected(0);
    updateEnablements();
}

void ProjectPathsWidget::definesChanged( const Defines& defines )
{
    qCDebug(DEFINESANDINCLUDES) << "defines changed";
    updatePathsModel( QVariant::fromValue(defines), ProjectPathsModel::DefinesDataRole );
}

void ProjectPathsWidget::includesChanged( const QStringList& includes )
{
    qCDebug(DEFINESANDINCLUDES) << "includes changed";
    updatePathsModel( includes, ProjectPathsModel::IncludesDataRole );
}

void ProjectPathsWidget::parserArgumentsChanged()
{
    updatePathsModel(QVariant::fromValue(ui->parserWidget->parserArguments()), ProjectPathsModel::ParserArgumentsRole);
}

void ProjectPathsWidget::updatePathsModel(const QVariant& newData, int role)
{
    QModelIndex idx = pathsModel->index( ui->projectPaths->currentIndex(), 0, QModelIndex() );
    if( idx.isValid() ) {
        bool b = pathsModel->setData( idx, newData, role );
        if( b ) {
            emit changed();
        }
    }
}

void ProjectPathsWidget::projectPathSelected( int index )
{
    if( index < 0 && pathsModel->rowCount() > 0 ) {
        index = 0;
    }
    Q_ASSERT(index >= 0);
    const QModelIndex midx = pathsModel->index( index, 0 );
    ui->includesWidget->setIncludes( pathsModel->data( midx, ProjectPathsModel::IncludesDataRole ).toStringList() );
    ui->definesWidget->setDefines( pathsModel->data( midx, ProjectPathsModel::DefinesDataRole ).value<Defines>() );

    Q_ASSERT(pathsModel->data(midx, ProjectPathsModel::CompilerDataRole).value<CompilerPointer>());

    ui->compiler->setCurrentText(pathsModel->data(midx, ProjectPathsModel::CompilerDataRole).value<CompilerPointer>()->name());

    ui->parserWidget->setParserArguments(pathsModel->data(midx, ProjectPathsModel::ParserArgumentsRole).value<ParserArguments>());

    updateEnablements();
}

void ProjectPathsWidget::clear()
{
    bool sigDisabled = ui->projectPaths->blockSignals( true );
    pathsModel->setPaths({});
    ui->includesWidget->clear();
    ui->definesWidget->clear();
    updateEnablements();
    ui->projectPaths->blockSignals( sigDisabled );
}

void ProjectPathsWidget::addProjectPath()
{
    const QUrl directory = pathsModel->data(pathsModel->index(0, 0), ProjectPathsModel::FullUrlDataRole).toUrl();
    QPointer<QFileDialog> dlg = new QFileDialog(this, i18n("Select Project Path"), directory.toLocalFile());
    dlg->setFileMode(QFileDialog::Directory);
    dlg->setOption(QFileDialog::ShowDirsOnly);
    if (dlg->exec()) {
        pathsModel->addPath(dlg->selectedUrls().value(0));
        ui->projectPaths->setCurrentIndex(pathsModel->rowCount() - 1);
        updateEnablements();
    }
    delete dlg;
}

void ProjectPathsWidget::deleteProjectPath()
{
    const QModelIndex idx = pathsModel->index( ui->projectPaths->currentIndex(), 0 );
    if( KMessageBox::questionYesNo( this, i18n("Are you sure you want to remove the configuration for the path '%1'?", pathsModel->data( idx, Qt::DisplayRole ).toString() ), QStringLiteral("Remove Path Configuration") ) == KMessageBox::Yes ) {
        pathsModel->removeRows( ui->projectPaths->currentIndex(), 1 );
    }
    updateEnablements();
}

void ProjectPathsWidget::setProject(KDevelop::IProject* w_project)
{
    pathsModel->setProject( w_project );
    ui->includesWidget->setProject( w_project );
}

void ProjectPathsWidget::updateEnablements() {
    // Disable removal of the project root entry which is always first in the list
    ui->removePath->setEnabled( ui->projectPaths->currentIndex() > 0 );
}

void ProjectPathsWidget::batchEdit()
{
    Ui::BatchEdit be;
    QPointer<QDialog> dialog = new QDialog(this);
    be.setupUi(dialog);

    const int index = qMax(ui->projectPaths->currentIndex(), 0);

    const QModelIndex midx = pathsModel->index(index, 0);

    if (!midx.isValid()) {
        return;
    }

    bool includesTab = ui->languageParameters->currentIndex() == 0;
    if (includesTab) {
        auto includes = pathsModel->data(midx, ProjectPathsModel::IncludesDataRole).toStringList();
        be.textEdit->setPlainText(includes.join(QLatin1Char('\n')));

        dialog->setWindowTitle(i18n("Edit include directories/files"));
    } else {
        auto defines = pathsModel->data(midx, ProjectPathsModel::DefinesDataRole).value<Defines>();

        for (auto it = defines.constBegin(); it != defines.constEnd(); it++) {
            be.textEdit->appendPlainText(it.key() + QLatin1Char('=') + it.value());
        }

        dialog->setWindowTitle(i18n("Edit defined macros"));
    }

    if (dialog->exec() != QDialog::Accepted) {
        delete dialog;
        return;
    }

    if (includesTab) {
        auto includes = be.textEdit->toPlainText().split(QLatin1Char('\n'), QString::SkipEmptyParts);
        for (auto& s : includes) {
            s = s.trimmed();
        }

        pathsModel->setData(midx, includes, ProjectPathsModel::IncludesDataRole);
    } else {
        auto list = be.textEdit->toPlainText().split(QLatin1Char('\n'), QString::SkipEmptyParts);
        Defines defines;

        for (auto& d : list) {
            //This matches: a=b, a=, a
            QRegExp r(QStringLiteral("^([^=]+)(=(.*))?$"));

            if (!r.exactMatch(d)) {
                continue;
            }
            defines[r.cap(1).trimmed()] = r.cap(3).trimmed();
        }

        pathsModel->setData(midx, QVariant::fromValue(defines), ProjectPathsModel::DefinesDataRole);
    }

    projectPathSelected(index);
    delete dialog;
}

void ProjectPathsWidget::setCurrentCompiler(const QString& name)
{
    for (int i = 0 ; i < ui->compiler->count(); ++i) {
        if(ui->compiler->itemText(i) == name)
        {
            ui->compiler->setCurrentIndex(i);
        }
    }
}

CompilerPointer ProjectPathsWidget::currentCompiler() const
{
    return ui->compiler->itemData(ui->compiler->currentIndex()).value<CompilerPointer>();
}

void ProjectPathsWidget::tabChanged(int idx)
{
    if (idx == ParserArgumentsPage) {
        ui->batchEdit->setVisible(false);
        ui->compilerBox->setVisible(true);
        ui->configureLabel->setText(i18n("Configure C/C++ parser"));
    } else {
        ui->batchEdit->setVisible(true);
        ui->compilerBox->setVisible(false);
        ui->configureLabel->setText(i18n("Configure which macros and include directories/files will be added to the parser during project parsing:"));
    }
}

void ProjectPathsWidget::changeCompilerForPath()
{
    for (int idx = 0; idx < pathsModel->rowCount(); idx++) {
        const QModelIndex midx = pathsModel->index(idx, 0);
        if (pathsModel->data(midx, Qt::DisplayRole) == ui->projectPaths->currentText()) {
            pathsModel->setData(midx, QVariant::fromValue(currentCompiler()), ProjectPathsModel::CompilerDataRole);
            break;
        }
    }
}
