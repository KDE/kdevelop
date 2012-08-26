/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
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

#include <QToolButton>

#include <KDebug>
#include <KLineEdit>
#include <KAction>
#include <kfiledialog.h>

#include "ui_projectpathswidget.h"
#include "projectpathsmodel.h"
#include "includesmodel.h"
#include "definesmodel.h"
#include <util/environmentgrouplist.h>
#include <interfaces/iproject.h>

extern int cbsDebugArea(); // from debugarea.cpp

ProjectPathsWidget::ProjectPathsWidget( QWidget* parent )
    : QWidget ( parent ), ui( new Ui::ProjectPathsWidget )
    , pathsModel( new ProjectPathsModel( this ) )
    , includesModel( new IncludesModel( this ) )
    , definesModel( new DefinesModel( this ) )
{
    ui->setupUi( this );

    // Hack to workaround broken setIcon(QIcon) overload in KPushButton, the function does not set the icon at all
    // So need to explicitly use the KIcon overload
    ui->addPath->setIcon(KIcon("list-add"));
    ui->replacePath->setIcon(KIcon("document-edit"));
    ui->removePath->setIcon(KIcon("list-remove"));
    ui->addIncludePath->setIcon(KIcon("list-add"));
    ui->removeIncludePath->setIcon(KIcon("list-remove"));
    connect( ui->addPath, SIGNAL(clicked(bool)), SLOT(addProjectPath()) );
    connect( ui->replacePath, SIGNAL(clicked(bool)), SLOT(replaceProjectPath()) );
    connect( ui->removePath, SIGNAL(clicked(bool)), SLOT(deleteProjectPath()) );
    
    connect( ui->addIncludePath, SIGNAL(clicked(bool)), SLOT(addIncludePath()) );
    connect( ui->removeIncludePath, SIGNAL(clicked(bool)), SLOT(deleteIncludePath()) );

    ui->includePathRequester->setMode( KFile::Directory );

    ui->projectPaths->setModel( pathsModel );
    connect( ui->projectPaths, SIGNAL(currentIndexChanged(int)), SLOT(projectPathSelected(int)) );
    connect( pathsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SIGNAL(changed()) );
    connect( pathsModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(changed()) );
    connect( pathsModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(changed()) );

    ui->includePaths->setModel( includesModel );
    connect( ui->includePaths->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(includePathSelected(QModelIndex)) );
    connect( ui->includePathRequester, SIGNAL(textChanged(QString)), SLOT(includePathEdited()) );
    connect( ui->includePathRequester, SIGNAL(urlSelected(KUrl)), SLOT(includePathUrlSelected(KUrl)) );
    connect( includesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(includesChanged()) );
    connect( includesModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(includesChanged())  );
    connect( includesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(includesChanged())  );

    ui->defines->setModel( definesModel );
    ui->defines->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
    connect( definesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(definesChanged()) );
    connect( definesModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(definesChanged()) );
    connect( definesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(definesChanged()) );

    KAction* delIncAction = new KAction( i18n("Delete Include Path"), this );
    delIncAction->setShortcut( KShortcut( "Del" ) );
    delIncAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    ui->includePaths->addAction( delIncAction );
    connect( delIncAction, SIGNAL(triggered()), SLOT(deleteIncludePath()) );

    KAction* delDefAction = new KAction( i18n("Delete Define"), this );
    delDefAction->setShortcut( KShortcut( "Del" ) );
    delDefAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    ui->defines->addAction( delDefAction );
    connect( delDefAction, SIGNAL(triggered()), SLOT(deleteDefine()) );
}

QList<CustomBuildSystemProjectPathConfig> ProjectPathsWidget::paths() const
{
    return pathsModel->paths();
}

void ProjectPathsWidget::setPaths( const QList<CustomBuildSystemProjectPathConfig>& paths )
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

void ProjectPathsWidget::definesChanged()
{
    kDebug(cbsDebugArea()) << "defines changed";
    updatePathsModel( definesModel->defines(), ProjectPathsModel::DefinesDataRole );
}

void ProjectPathsWidget::includesChanged()
{
    kDebug(cbsDebugArea()) << "includes changed";
    updatePathsModel( includesModel->includes(), ProjectPathsModel::IncludesDataRole );
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

void ProjectPathsWidget::includePathSelected( const QModelIndex& selected )
{
    kDebug(cbsDebugArea()) << "include path list entry selected:" << selected;
    updateEnablements();
}

void ProjectPathsWidget::includePathEdited()
{
    kDebug(cbsDebugArea()) << "include path edited:" << ui->includePathRequester->url();
    updateEnablements();
}

void ProjectPathsWidget::projectPathSelected( int index )
{
    kDebug(cbsDebugArea()) << "project path selected:" << index;
    updateEnablements();
}

void ProjectPathsWidget::clear()  
{
    pathsModel->setPaths( QList<CustomBuildSystemProjectPathConfig>() );
    includesModel->setIncludes( QStringList() );
    definesModel->setDefines( QHash<QString,QVariant>() );
    updateEnablements();
}

void ProjectPathsWidget::deleteDefine()
{
    kDebug(cbsDebugArea()) << "Deleting defines";
    QModelIndexList selection = ui->defines->selectionModel()->selectedRows();
    foreach( const QModelIndex& row, selection ) {
        definesModel->removeRow( row.row() );
    }
    updateEnablements();
}

void ProjectPathsWidget::addIncludePath()
{
    kDebug(cbsDebugArea()) << "adding include path" << ui->includePathRequester->url();
    updateEnablements();
}

void ProjectPathsWidget::deleteIncludePath()
{
    kDebug(cbsDebugArea()) << "deleting include path" << ui->includePaths->currentIndex();
    updateEnablements();
}

void ProjectPathsWidget::includePathUrlSelected(const KUrl& url)
{
    kDebug(cbsDebugArea()) << "include path url selected" << url;
    updateEnablements();
}

void ProjectPathsWidget::addProjectPath()
{
    kDebug(cbsDebugArea()) << "enabling project path editing" << pathsModel->data(pathsModel->index(0, 0), ProjectPathsModel::FullUrlDataRole).value<KUrl>();
    KFileDialog dlg(pathsModel->data(pathsModel->index(0, 0), ProjectPathsModel::FullUrlDataRole).value<KUrl>(), "", this);
    dlg.setMode( KFile::ExistingOnly | KFile::File | KFile::Directory );
    dlg.exec();
    kDebug(cbsDebugArea()) << "adding url:" << dlg.selectedUrl();
    pathsModel->addPath(dlg.selectedUrl());
    ui->projectPaths->setCurrentIndex(pathsModel->rowCount() - 1);
    kDebug(cbsDebugArea()) << "added url:" << pathsModel->rowCount();
    updateEnablements();
}

void ProjectPathsWidget::replaceProjectPath()
{
    KFileDialog dlg(pathsModel->data(pathsModel->index(0, 0), ProjectPathsModel::FullUrlDataRole).value<KUrl>(), "", this);
    dlg.setMode( KFile::ExistingOnly | KFile::File | KFile::Directory );
    dlg.exec();
    kDebug(cbsDebugArea()) << "adding url:" << dlg.selectedUrl();
    pathsModel->setData( pathsModel->index( ui->projectPaths->currentIndex(), 0 ), QVariant::fromValue<KUrl>(dlg.selectedUrl()), ProjectPathsModel::FullUrlDataRole );
    kDebug(cbsDebugArea()) << "added url:" << pathsModel->rowCount();
    updateEnablements();
}

void ProjectPathsWidget::deleteProjectPath()
{
    kDebug(cbsDebugArea()) << "deleting project path" << ui->projectPaths->currentIndex();
    updateEnablements();
}
void ProjectPathsWidget::setProject(KDevelop::IProject* w_project)
{
    kDebug(cbsDebugArea()) << "setting project:" << w_project->projectFileUrl() << w_project->folder();
    pathsModel->setProject( w_project );
    ui->includePathRequester->setStartDir( w_project->folder() );
}

void ProjectPathsWidget::updateEnablements() {
    // Disable removal of the project root entry which is always first in the list
    ui->removePath->setEnabled( ui->projectPaths->currentIndex() > 0 );
    ui->replacePath->setEnabled( ui->projectPaths->currentIndex() > 0 );
    ui->removeIncludePath->setEnabled( ui->includePaths->currentIndex().isValid() );
}

#include "projectpathswidget.moc"

