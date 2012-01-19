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

#include "configwidget.h"

#include <KDebug>
#include <KLineEdit>
#include <KAction>

#include "ui_configwidget.h"
#include "projectpathsmodel.h"
#include "includesmodel.h"
#include "definesmodel.h"
#include <util/environmentgrouplist.h>


ConfigWidget::ConfigWidget( QWidget* parent )
    : QWidget ( parent ), ui( new Ui::ConfigWidget )
    , pathsModel( new ProjectPathsModel( this ) )
    , includesModel( new IncludesModel( this ) )
    , definesModel( new DefinesModel( this ) )
{
    ui->setupUi( this );
    KDevelop::EnvironmentGroupList l( KGlobal::config() );
    ui->actionEnvironment->addItems( l.groups() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Build, i18n("Build"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Configure, i18n("Configure"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Install, i18n("Install"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Clean, i18n("Clean"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Prune, i18n("Prune"), QVariant() );

    connect( ui->buildAction, SIGNAL(activated(int)), SLOT(changeAction(int)) );

    connect( ui->enableAction, SIGNAL(toggled(bool)), SLOT(toggleActionEnablement(bool)) );
    connect( ui->actionArguments, SIGNAL(textEdited(QString)), SLOT(actionArgumentsEdited(QString)) );
    connect( ui->actionEnvironment, SIGNAL(activated(int)), SLOT(actionEnvironmentChanged(int)) );
    connect( ui->actionExecutable, SIGNAL(urlSelected(KUrl)), SLOT(actionExecutableChanged(KUrl)) );
    connect( ui->actionExecutable->lineEdit(), SIGNAL(textEdited(QString)), SLOT(actionExecutableChanged(QString)) );

    ui->projectPaths->setModel( pathsModel );
    connect( ui->projectPaths->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(projectPathSelected(QItemSelection,QItemSelection)) );
    connect( pathsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SIGNAL(changed()) );
    connect( pathsModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(changed()) );
    connect( pathsModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(changed()) );

    ui->includePaths->setModel( includesModel );
    connect( includesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(includesChanged()) );
    connect( includesModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(includesChanged())  );
    connect( includesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(includesChanged())  );

    ui->defines->setModel( definesModel );
    ui->defines->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
    connect( definesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(definesChanged()) );
    connect( definesModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(definesChanged()) );
    connect( definesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(definesChanged()) );

    ui->switchIncludeDefines->setCurrentIndex( 0 );
    ui->stackedWidget->setCurrentIndex( 0 );

    KAction* delPathAction = new KAction( i18n("Delete Project Path"), this );
    delPathAction->setShortcut( KShortcut( "Del" ) );
    delPathAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    ui->projectPaths->addAction( delPathAction );
    connect( delPathAction, SIGNAL(triggered()), SLOT(deleteProjectPath()) );

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

CustomBuildSystemConfig ConfigWidget::config() const
{
    CustomBuildSystemConfig c;
    c.buildDir = ui->buildDir->url();
    for( int i = 0; i < ui->buildAction->count(); i++ ) {
        QVariant data = ui->buildAction->itemData( i );
        CustomBuildSystemTool t;
        if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
            t = data.value<CustomBuildSystemTool>();
        }
        Q_ASSERT( t.type != CustomBuildSystemTool::Undefined );
        c.tools[ CustomBuildSystemTool::ActionType( i ) ] = t;
    }
    c.projectPaths = pathsModel->paths();
    return c;
}

void ConfigWidget::loadConfig( CustomBuildSystemConfig cfg )
{
    bool b = blockSignals( true );
    ui->buildDir->setUrl( cfg.buildDir );
    fillTools( cfg.tools );
    pathsModel->setPaths( cfg.projectPaths );
    blockSignals( b );
}

void ConfigWidget::fillTools(const QHash< CustomBuildSystemTool::ActionType, CustomBuildSystemTool>& tools)
{
    for( int i = 0; i < ui->buildAction->count(); i++ ) {
        QHash< CustomBuildSystemTool::ActionType, CustomBuildSystemTool>::const_iterator it = tools.find( CustomBuildSystemTool::ActionType( i ) );
        CustomBuildSystemTool t;
        t.type = CustomBuildSystemTool::ActionType( i );
        t.enabled = false;
        if( it != tools.end() ) {
            t = *it;
        }
        Q_ASSERT( t.type != CustomBuildSystemTool::Undefined );
        Q_ASSERT( t.type == CustomBuildSystemTool::ActionType( i ) );
        ui->buildAction->setItemData( i, QVariant::fromValue<CustomBuildSystemTool>( t ) );
    }
    ui->buildAction->setCurrentIndex( CustomBuildSystemTool::Build );
    changeAction( ui->buildAction->currentIndex() );
}

void ConfigWidget::changeAction( int idx )
{
    Q_ASSERT( idx >= 0 && idx < ui->buildAction->count() );

    QVariant data = ui->buildAction->itemData( idx );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        bool b = ui->enableAction->blockSignals( true );
        ui->enableAction->setChecked( t.enabled );
        ui->enableAction->blockSignals( b );

        ui->actionArguments->setText( t.arguments );
        ui->actionArguments->setEnabled( t.enabled );
        ui->actionExecutable->setUrl( t.executable );
        ui->actionExecutable->setEnabled( t.enabled );
        ui->actionEnvironment->setCurrentIndex( ui->actionEnvironment->findText( t.envGrp ) );
        ui->actionEnvironment->setEnabled( t.enabled );
        ui->execLabel->setEnabled( t.enabled );
        ui->argLabel->setEnabled( t.enabled );
        ui->envLabel->setEnabled( t.enabled );
    }
}

void ConfigWidget::toggleActionEnablement( bool enable )
{
    QVariant data = ui->buildAction->itemData( ui->buildAction->currentIndex() );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        t.enabled = enable;
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant::fromValue( t ) );
        emit changed();
    }
}

void ConfigWidget::actionArgumentsEdited( const QString& txt )
{
    QVariant data = ui->buildAction->itemData( ui->buildAction->currentIndex() );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        t.arguments = txt;
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant::fromValue( t ) );
        emit changed();
    }
}

void ConfigWidget::actionEnvironmentChanged( int )
{
    QVariant data = ui->buildAction->itemData( ui->buildAction->currentIndex() );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        t.envGrp = ui->actionEnvironment->currentProfile();
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant::fromValue( t ) );
        emit changed();
    }
}

void ConfigWidget::actionExecutableChanged( const KUrl& url )
{
    QVariant data = ui->buildAction->itemData( ui->buildAction->currentIndex() );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        t.executable = url;
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant::fromValue( t ) );
        emit changed();
    }
}

void ConfigWidget::actionExecutableChanged(const QString& txt )
{
    actionExecutableChanged( KUrl( txt ) );
}

void ConfigWidget::definesChanged()
{
    QList<QModelIndex> idx = ui->projectPaths->selectionModel()->selectedRows();
    if( !idx.isEmpty() ) {
        bool b = pathsModel->setData( idx.first(), definesModel->defines(), ProjectPathsModel::SetDefinesRole );
        if( b ) {
            emit changed();
        }
    }
}

void ConfigWidget::includesChanged()
{
    QList<QModelIndex> idx = ui->projectPaths->selectionModel()->selectedRows();
    if( !idx.isEmpty() ) {
        bool b = pathsModel->setData( idx.first(), includesModel->includes(), ProjectPathsModel::SetIncludesRole );
        if( b ) {
            emit changed();
        }
    }
}

void ConfigWidget::projectPathSelected( const QItemSelection& selected, const QItemSelection& )
{
    bool enable = !( selected.isEmpty() || selected.indexes().first().row() == pathsModel->rowCount() - 1 );
    ui->includePaths->setEnabled( enable );
    ui->defines->setEnabled( enable );
    ui->switchIncludeDefines->setEnabled( enable );

    if( enable ) {
        includesModel->setIncludes( pathsModel->data( selected.indexes().first(), ProjectPathsModel::IncludesDataRole ).toStringList() );
        definesModel->setDefines( pathsModel->data( selected.indexes().first(), ProjectPathsModel::DefinesDataRole ).toHash() );
    } else {
        includesModel->setIncludes( QStringList() );
        definesModel->setDefines( QHash<QString,QVariant>() );
    }
}

void ConfigWidget::clear()
{
    pathsModel->setPaths( QList<CustomBuildSystemProjectPathConfig>() );
    includesModel->setIncludes( QStringList() );
    definesModel->setDefines( QHash<QString,QVariant>() );
    for( int i = 0; i < ui->buildAction->count(); i++ ) {
        CustomBuildSystemTool t;
        t.type = CustomBuildSystemTool::ActionType( i );
        ui->buildAction->setItemData( i, QVariant::fromValue( t ) );
    }
    ui->buildAction->setCurrentIndex( int( CustomBuildSystemTool::Build ) );
    changeAction( ui->buildAction->currentIndex() );
    ui->buildDir->setText("");
}

void ConfigWidget::deleteDefine()
{
    QModelIndex idx = ui->defines->currentIndex();
    definesModel->removeRows( idx.row(), 1, QModelIndex() );
}

void ConfigWidget::deleteIncludePath()
{
    QModelIndex idx = ui->includePaths->currentIndex();
    includesModel->removeRows( idx.row(), 1, QModelIndex() );
}

void ConfigWidget::deleteProjectPath()
{
    QModelIndex idx = ui->projectPaths->currentIndex();
    pathsModel->removeRows( idx.row(), 1, QModelIndex() );
}

#include "configwidget.moc"

