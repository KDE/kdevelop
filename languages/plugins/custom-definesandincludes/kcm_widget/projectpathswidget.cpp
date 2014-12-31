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

#include <kfiledialog.h>
#include <kmessagebox.h>

#include <QRegExp>

#include <util/environmentgrouplist.h>
#include <interfaces/iproject.h>
#include <KLocalizedString>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>

#include "../compilerprovider/compilerprovider.h"

#include "compilerswidget.h"

#include "ui_projectpathswidget.h"
#include "ui_batchedit.h"
#include "projectpathsmodel.h"
#include "debugarea.h"

using namespace KDevelop;

ProjectPathsWidget::ProjectPathsWidget( QWidget* parent )
    : QWidget(parent),
      ui(new Ui::ProjectPathsWidget),
      pathsModel(new ProjectPathsModel(this))
{
    ui->setupUi( this );

    ui->addPath->setIcon(QIcon::fromTheme("list-add"));
    ui->removePath->setIcon(QIcon::fromTheme("list-remove"));

    // hack taken from kurlrequester, make the buttons a bit less in height so they better match the url-requester
    ui->addPath->setFixedHeight( ui->projectPaths->sizeHint().height() );
    ui->removePath->setFixedHeight( ui->projectPaths->sizeHint().height() );

    connect( ui->addPath, &QPushButton::clicked, this, &ProjectPathsWidget::addProjectPath );
    connect( ui->removePath, &QPushButton::clicked, this, &ProjectPathsWidget::deleteProjectPath );
    connect( ui->batchEdit, &QPushButton::clicked, this, &ProjectPathsWidget::batchEdit );

    ui->projectPaths->setModel( pathsModel );
    connect( ui->projectPaths, static_cast<void(KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &ProjectPathsWidget::projectPathSelected );
    connect( pathsModel, &ProjectPathsModel::dataChanged, this, &ProjectPathsWidget::changed );
    connect( pathsModel, &ProjectPathsModel::rowsInserted, this, &ProjectPathsWidget::changed );
    connect( pathsModel, &ProjectPathsModel::rowsRemoved, this, &ProjectPathsWidget::changed );
    connect( ui->compiler, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::activated), this, &ProjectPathsWidget::changed );

    connect(ui->compilersWidget, &CompilersWidget::compilerChanged, this, &ProjectPathsWidget::changed);

    connect( ui->includesWidget, static_cast<void(IncludesWidget::*)(const QStringList&)>(&IncludesWidget::includesChanged), this, &ProjectPathsWidget::includesChanged );
    connect( ui->definesWidget, static_cast<void(DefinesWidget::*)(const KDevelop::Defines&)>(&DefinesWidget::definesChanged), this, &ProjectPathsWidget::definesChanged );

    connect(ui->compilersWidget, &CompilersWidget::compilerChanged,
            this, &ProjectPathsWidget::compilerChanged);

    connect(ui->languageParameters, &QTabWidget::currentChanged, this, &ProjectPathsWidget::tabChanged);
}

QList<ConfigEntry> ProjectPathsWidget::paths() const
{
    return pathsModel->paths();
}

void ProjectPathsWidget::setPaths( const QList<ConfigEntry>& paths )
{
    bool b = blockSignals( true );
    clear();
    pathsModel->setPaths( paths );
    blockSignals( b );
    ui->projectPaths->setCurrentIndex(0); // at least a project root item is present
    projectPathSelected(0);
    ui->languageParameters->setCurrentIndex(0);
    updateEnablements();
}

void ProjectPathsWidget::definesChanged( const Defines& defines )
{
    definesAndIncludesDebug() << "defines changed";
    updatePathsModel( QVariant::fromValue(defines), ProjectPathsModel::DefinesDataRole );
}

void ProjectPathsWidget::includesChanged( const QStringList& includes )
{
    definesAndIncludesDebug() << "includes changed";
    updatePathsModel( includes, ProjectPathsModel::IncludesDataRole );
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
    updateEnablements();
}

void ProjectPathsWidget::clear()
{
    bool sigDisabled = ui->projectPaths->blockSignals( true );
    pathsModel->setPaths( QList<ConfigEntry>() );
    ui->includesWidget->clear();
    ui->definesWidget->clear();
    updateEnablements();
    ui->projectPaths->blockSignals( sigDisabled );
}

void ProjectPathsWidget::addProjectPath()
{
    KFileDialog dlg(pathsModel->data(pathsModel->index(0, 0), ProjectPathsModel::FullUrlDataRole).value<QUrl>(), "", this);
    dlg.setMode( KFile::LocalOnly | KFile::ExistingOnly | KFile::File | KFile::Directory );
    dlg.exec();
    pathsModel->addPath(dlg.selectedUrl());
    ui->projectPaths->setCurrentIndex(pathsModel->rowCount() - 1);
    updateEnablements();
}

void ProjectPathsWidget::deleteProjectPath()
{
    const QModelIndex idx = pathsModel->index( ui->projectPaths->currentIndex(), 0 );
    if( KMessageBox::questionYesNo( this, i18n("Are you sure you want to remove the configuration for the path '%1'?", pathsModel->data( idx, Qt::DisplayRole ).toString() ), "Remove Path Configuration" ) == KMessageBox::Yes ) {
        pathsModel->removeRows( ui->projectPaths->currentIndex(), 1 );
    }
    updateEnablements();
}

void ProjectPathsWidget::setProject(KDevelop::IProject* w_project)
{
    m_project = w_project;
    pathsModel->setProject( m_project );
    ui->includesWidget->setProject( m_project );
}

void ProjectPathsWidget::updateEnablements() {
    // Disable removal of the project root entry which is always first in the list
    ui->removePath->setEnabled( ui->projectPaths->currentIndex() > 0 );
}

void ProjectPathsWidget::batchEdit()
{
    Ui::BatchEdit be;
    QDialog dialog(this);
    be.setupUi(&dialog);

    const int index = qMax(ui->projectPaths->currentIndex(), 0);

    const QModelIndex midx = pathsModel->index(index, 0);

    if (!midx.isValid()) {
        return;
    }

    bool includesTab = ui->languageParameters->currentIndex() == 0;
    if (includesTab) {
        auto includes = pathsModel->data(midx, ProjectPathsModel::IncludesDataRole).toStringList();
        be.textEdit->setPlainText(includes.join("\n"));

        dialog.setWindowTitle(i18n("Edit include directories/files"));
    } else {
        auto defines = pathsModel->data(midx, ProjectPathsModel::DefinesDataRole).value<Defines>();

        for (auto it = defines.constBegin(); it != defines.constEnd(); it++) {
            be.textEdit->append(it.key() + "=" + it.value());
        }

        dialog.setWindowTitle(i18n("Edit defined macros"));
    }

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    if (includesTab) {
        auto includes = be.textEdit->toPlainText().split('\n', QString::SkipEmptyParts);
        for (auto& s : includes) {
            s = s.trimmed();
        }

        pathsModel->setData(midx, includes, ProjectPathsModel::IncludesDataRole);
    } else {
        auto list = be.textEdit->toPlainText().split('\n', QString::SkipEmptyParts);
        Defines defines;

        for (auto& d : list) {
            //This matches: a=b, a=, a
            QRegExp r("^([^=]+)(=(.*))?$");

            if (!r.exactMatch(d)) {
                continue;
            }
            defines[r.cap(1).trimmed()] = r.cap(3).trimmed();
        }

        pathsModel->setData(midx, QVariant::fromValue(defines), ProjectPathsModel::DefinesDataRole);
    }

    projectPathSelected(index);
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

void ProjectPathsWidget::setCompilers(const QVector< CompilerPointer >& compilers, bool updateCompilersModel)
{
    ui->compiler->clear();
    for (int i = 0 ; i < compilers.count(); ++i) {
        Q_ASSERT(compilers[i]);
        if (!compilers[i]) {
            continue;
        }
        ui->compiler->addItem(compilers[i]->name());
        QVariant val; val.setValue(compilers[i]);
        ui->compiler->setItemData(i, val);
    }

    if (updateCompilersModel) {
        ui->compilersWidget->setCompilers(compilers);
    }
}

QVector< CompilerPointer > ProjectPathsWidget::compilers() const
{
    return ui->compilersWidget->compilers();
}

void ProjectPathsWidget::compilerChanged()
{
    auto current = currentCompiler()->name();
    setCompilers(ui->compilersWidget->compilers(), false);
    setCurrentCompiler(current);
}

void ProjectPathsWidget::tabChanged(int idx)
{
    if (idx==2) {
        ui->batchEdit->setVisible(false);
        ui->compilerBox->setVisible(true);
    } else {
        ui->batchEdit->setVisible(true);
        ui->compilerBox->setVisible(false);
    }
}
