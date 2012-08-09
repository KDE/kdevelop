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

#include <QToolButton>

#include <KDebug>
#include <KLineEdit>
#include <KAction>

#include "ui_configwidget.h"
#include "projectpathsmodel.h"
#include "includesmodel.h"
#include "definesmodel.h"
#include <util/environmentgrouplist.h>
#include <interfaces/iproject.h>


ConfigWidget::ConfigWidget( QWidget* parent )
    : QWidget ( parent ), ui( new Ui::ConfigWidget )
    , pathsModel( new ProjectPathsModel( this ) )
    , includesModel( new IncludesModel( this ) )
    , definesModel( new DefinesModel( this ) )
{
    ui->setupUi( this );
    KDevelop::EnvironmentGroupList l( KGlobal::config() );
    ui->actionEnvironment->addItems( l.groups() );

    ui->addPath->setIcon(KIcon( "list-add" ));
    ui->removePath->setIcon(KIcon( "list-remove" ));
    ui->savePath->setIcon(KIcon( "dialog-ok" ));
    connect( ui->addPath, SIGNAL(clicked(bool)), SLOT(addProjectPath()) );
    connect( ui->removePath, SIGNAL(clicked(bool)), SLOT(deleteProjectPath()) );
    connect( ui->savePath, SIGNAL(clicked(bool)), SLOT(saveProjectPath()));

    ui->addIncludePath->setIcon(KIcon( "list-add" ));
    ui->saveIncludePath->setIcon(KIcon( "dialog-ok" ));
    connect( ui->addIncludePath, SIGNAL(clicked(bool)), SLOT(addIncludePath()) );
    connect( ui->saveIncludePath, SIGNAL(clicked(bool)), SLOT(saveIncludePath()) );

    {
        QToolButton* removeEntry = new QToolButton( ui->languageParameters );
        removeEntry->setIcon(KIcon( "list-remove" ));
        removeEntry->setText( i18n("Remove selection") );
        removeEntry->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
        connect( removeEntry, SIGNAL(clicked(bool)) , SIGNAL(deleteLanguageParametersEntry()) );
        ui->languageParameters->setCornerWidget( removeEntry, Qt::TopRightCorner );
        ui->languageParameters->updateGeometry();
    }

    ui->pathRequester->setMode( KFile::File | KFile::Directory );
    ui->includePathRequester->setMode( KFile::Directory );

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
    connect( ui->projectPaths, SIGNAL(currentIndexChanged(int)), SLOT(projectPathSelected(int)) );
    connect( ui->pathRequester, SIGNAL(textChanged(QString)), SLOT(projectPathEdited()) );
    connect( pathsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SIGNAL(changed()) );
    connect( pathsModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(changed()) );
    connect( pathsModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(changed()) );

    ui->includePaths->setModel( includesModel );
    connect( ui->includePaths->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(includePathSelected(QModelIndex)) );
    connect( ui->includePathRequester, SIGNAL(textChanged(QString)), SLOT(includePathEdited()) );
    connect( includesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(includesChanged()) );
    connect( includesModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(includesChanged())  );
    connect( includesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(includesChanged())  );

    ui->defines->setModel( definesModel );
    ui->defines->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
    connect( definesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(definesChanged()) );
    connect( definesModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(definesChanged()) );
    connect( definesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(definesChanged()) );

    connect( ui->languageParameters, SIGNAL(currentChanged(int)), SLOT(languageParametersTabSelected(int)) );

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

    connect( this, SIGNAL(changed()), SLOT(verify()) );
}

CustomBuildSystemConfig ConfigWidget::config() const
{
    CustomBuildSystemConfig c;
    c.buildDir = ui->buildDir->url();
    c.tools = m_tools;
    c.projectPaths = pathsModel->paths();
    return c;
}

void ConfigWidget::loadConfig( CustomBuildSystemConfig cfg )
{
    bool b = blockSignals( true );
    clear();
    ui->buildDir->setUrl( cfg.buildDir );
    pathsModel->setPaths( cfg.projectPaths );
    m_tools = cfg.tools;
    blockSignals( b );
    ui->projectPaths->setCurrentIndex(0); // at least a project root item is present
    projectPathSelected(0);
    ui->languageParameters->setCurrentIndex(0);
    languageParametersTabSelected(0);
    changeAction( ui->buildAction->currentIndex() );
    m_tools = cfg.tools;
    verify();
}

void ConfigWidget::setTool(const CustomBuildSystemTool& tool)
{
    bool b = ui->enableAction->blockSignals( true );
    ui->enableAction->setChecked( tool.enabled );
    ui->enableAction->blockSignals( b );

    ui->actionArguments->setText( tool.arguments );
    ui->actionArguments->setEnabled( tool.enabled );
    ui->actionExecutable->setUrl( tool.executable );
    ui->actionExecutable->setEnabled( tool.enabled );
    ui->actionEnvironment->setCurrentProfile( tool.envGrp );
    ui->actionEnvironment->setEnabled( tool.enabled );
    ui->execLabel->setEnabled( tool.enabled );
    ui->argLabel->setEnabled( tool.enabled );
    ui->envLabel->setEnabled( tool.enabled );
}

void ConfigWidget::changeAction( int idx )
{
    if (idx < 0 || idx >= m_tools.count() ) {
        CustomBuildSystemTool emptyTool;
        emptyTool.type = CustomBuildSystemTool::Build;
        emptyTool.enabled = false;
        setTool(emptyTool);
    } else {
        CustomBuildSystemTool& selectedTool = m_tools[idx];
        setTool(selectedTool);
    }
}

void ConfigWidget::toggleActionEnablement( bool enable )
{
    m_tools[ ui->buildAction->currentIndex() ].enabled = enable;
    emit changed();
}

void ConfigWidget::actionArgumentsEdited( const QString& txt )
{
    m_tools[ ui->buildAction->currentIndex() ].arguments = txt;
    emit changed();
}

void ConfigWidget::actionEnvironmentChanged( int )
{
    m_tools[ ui->buildAction->currentIndex() ].envGrp = ui->actionEnvironment->currentProfile();
    emit changed();
}

void ConfigWidget::actionExecutableChanged( const KUrl& url )
{
    m_tools[ ui->buildAction->currentIndex() ].executable = url.toLocalFile();
    emit changed();
}

void ConfigWidget::actionExecutableChanged(const QString& txt )
{
    m_tools[ ui->buildAction->currentIndex() ].executable = txt;
    emit changed();
}

void ConfigWidget::definesChanged()
{
    QModelIndex idx = currentPathIndex();
    if( idx.isValid() ) {
        bool b = pathsModel->setData( idx, definesModel->defines(), ProjectPathsModel::DefinesDataRole );
        if( b ) {
            emit changed();
        }
    }
}

void ConfigWidget::includesChanged()
{
    QModelIndex idx = currentPathIndex();
    if( idx.isValid() ) {
        bool b = pathsModel->setData( idx, includesModel->includes(), ProjectPathsModel::IncludesDataRole );
        if( b ) {
            emit changed();
        }
    }
    includePathSelected( ui->includePaths->currentIndex() );
}

void ConfigWidget::includePathSelected( const QModelIndex& selected )
{
    if( selected.isValid() ) {
        ui->includePathRequester->setUrl( selected.data().toString() );
    } else {
        ui->includePathRequester->clear();
    }

    ui->addIncludePath->setEnabled( false );
    ui->saveIncludePath->setEnabled( false );
}

void ConfigWidget::includePathEdited()
{
    bool hasAnyPaths = (includesModel->rowCount() != 0);
    ui->addIncludePath->setEnabled( true );
    ui->saveIncludePath->setEnabled( hasAnyPaths );
}

void ConfigWidget::projectPathSelected( int index )
{
    if( index >= 0 ) {
        QModelIndex modelIndex = pathsModel->index(index, 0);
        includesModel->setIncludes( modelIndex.data( ProjectPathsModel::IncludesDataRole ).toStringList() );
        definesModel->setDefines( modelIndex.data( ProjectPathsModel::DefinesDataRole ).toHash() );
        ui->pathRequester->setUrl( modelIndex.data( Qt::EditRole ).toString() );
    } else {
        includesModel->setIncludes( QStringList() );
        definesModel->setDefines( QHash<QString,QVariant>() );
        ui->pathRequester->clear();
    }

    bool hasAnyPaths = (ui->projectPaths->count() != 0);
    bool isEditable = hasAnyPaths && pathsModel->flags( currentPathIndex() ).testFlag( Qt::ItemIsEditable );

    ui->removePath->setEnabled( isEditable );
    ui->addPath->setEnabled( false );
    ui->savePath->setEnabled( false );
}

void ConfigWidget::projectPathEdited()
{
    bool hasAnyPaths = (ui->projectPaths->count() != 0);
    bool isEditable = hasAnyPaths && pathsModel->flags( currentPathIndex() ).testFlag( Qt::ItemIsEditable );

    ui->addPath->setEnabled( true );
    ui->savePath->setEnabled( isEditable );
}

void ConfigWidget::clear()
{
    pathsModel->setPaths( QList<CustomBuildSystemProjectPathConfig>() );
    includesModel->setIncludes( QStringList() );
    definesModel->setDefines( QHash<QString,QVariant>() );
    ui->buildAction->setCurrentIndex( int( CustomBuildSystemTool::Build ) );
    changeAction( ui->buildAction->currentIndex() );
    ui->buildDir->setText("");
}

void ConfigWidget::deleteDefine()
{
    QModelIndexList selection = ui->defines->selectionModel()->selectedRows();
    foreach( const QModelIndex& row, selection ) {
        definesModel->removeRow( row.row() );
    }
}

void ConfigWidget::addIncludePath()
{
    commitIncludePathRequester( true );
}

void ConfigWidget::saveIncludePath()
{
    commitIncludePathRequester( false );
}

void ConfigWidget::deleteIncludePath()
{
    includesModel->removeRow( ui->includePaths->currentIndex().row() );
}

void ConfigWidget::languageParametersTabSelected( int index )
{
    disconnect( this, SIGNAL(deleteLanguageParametersEntry()) );
    disconnect( this, SLOT(languageParametersItemsChanged(QModelIndex,QModelIndex)) );

    QAbstractItemModel* model = 0;

    switch(index) {
    // Include pathes
    case 0:
        connect( this, SIGNAL(deleteLanguageParametersEntry()), SLOT(deleteIncludePath()) );
        includePathSelected( QModelIndex() );
        model = includesModel;
        break;

    // Defines
    case 1:
        connect( this, SIGNAL(deleteLanguageParametersEntry()), SLOT(deleteDefine()) );
        model = definesModel;
        break;

    default:
        break;
    }

    if( model ) {
        connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), SLOT(languageParametersItemsChanged(QModelIndex, QModelIndex)));
        configureItemRemoveButton( model );
    }
}

void ConfigWidget::languageParametersItemsChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    if( const QAbstractItemModel* model = topLeft.model()) {
        Q_ASSERT( model == bottomRight.model() );
        configureItemRemoveButton( model );
    }
}

void ConfigWidget::configureItemRemoveButton( const QAbstractItemModel* model )
{
    if( QWidget* removeEntry = ui->languageParameters->cornerWidget() ) {
        removeEntry->setEnabled( model->rowCount() != 0 );
    }
}


void ConfigWidget::addProjectPath()
{
    commitProjectPathRequester( true );
}

void ConfigWidget::saveProjectPath()
{
    commitProjectPathRequester( false );
}

void ConfigWidget::deleteProjectPath()
{
    pathsModel->removeRow( ui->projectPaths->currentIndex() );
}

QModelIndex ConfigWidget::currentPathIndex()
{
    int currentPath = ui->projectPaths->currentIndex();
    return (currentPath < 0) ? QModelIndex() : ui->projectPaths->model()->index( currentPath, 0 );
}

void ConfigWidget::setProject(KDevelop::IProject* w_project)
{
    pathsModel->setProject( w_project );
    ui->pathRequester->setStartDir( w_project->folder() );
    ui->includePathRequester->setStartDir( w_project->folder() );
}

void ConfigWidget::verify() {
    Q_ASSERT( ui->projectPaths->count() == pathsModel->rowCount() );

    bool hasAnyPaths = (ui->projectPaths->count() != 0);
    bool pathSelected = currentPathIndex().isValid();
    Q_ASSERT( !hasAnyPaths || pathSelected );

    ui->languageParameters->setEnabled( hasAnyPaths );
}

void ConfigWidget::commitProjectPathRequester( bool add )
{
    QModelIndex pathIndex = currentPathIndex();

    // Do not allow to edit inexistent records
    // Instead, add a new record in all these cases
    if( !pathIndex.isValid() )
        add = true;

    KUrl savedUrl = ui->pathRequester->url();

    // Refuse to write an invalid or empty URL
    if( !savedUrl.isValid() || savedUrl.isEmpty() )
        return;

    // Further actions depend on previous mode
    if( add ) {
        pathsModel->addPath( savedUrl );
    } else {
        pathsModel->setData( pathIndex, savedUrl.url(), Qt::EditRole );
    }

    // The item may got inserted or not due to duplicate detection, we don't know
    // Attempt a search to find it
    for( int i = 0; i < pathsModel->rowCount(); ++i ) {
        if( pathsModel->data( pathsModel->index(i), ProjectPathsModel::FullUrlDataRole ).value<KUrl>() == savedUrl ) {
            ui->projectPaths->setCurrentIndex( i );
            projectPathSelected( i );
            break;
        }
    }
}

void ConfigWidget::commitIncludePathRequester( bool add )
{
    QModelIndex pathIndex = ui->includePaths->currentIndex();

    if( !pathIndex.isValid() ) {
        add = true;
    }

    KUrl savedUrl = ui->includePathRequester->url();
    savedUrl.cleanPath();
    QString savedPath = savedUrl.toLocalFile( KUrl::RemoveTrailingSlash );

    if( add ) {
        includesModel->addInclude( savedPath );
    } else {
        includesModel->setData( pathIndex, savedPath, Qt::EditRole );
    }

    for( int i = 0; i < includesModel->rowCount(); ++i ) {
        QModelIndex index = includesModel->index(i);
        if( index.data( Qt::EditRole ).toString() == savedPath ) {
            ui->includePaths->setCurrentIndex( index );
            break;
        }
    }
}

#include "configwidget.moc"

