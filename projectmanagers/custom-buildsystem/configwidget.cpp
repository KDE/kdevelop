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
    , m_mode( UI_NORMAL )
{
    ui->setupUi( this );
    KDevelop::EnvironmentGroupList l( KGlobal::config() );
    ui->actionEnvironment->addItems( l.groups() );

    ui->addPath->setIcon(KIcon( "list-add" ));
    ui->removePath->setIcon(KIcon( "list-remove" ));
    ui->editPath->setIcon(KIcon( "document-edit" ));
    ui->cancelEditPath->setIcon(KIcon( "dialog-cancel" ));
    ui->savePath->setIcon(KIcon( "dialog-ok" ));
    connect( ui->addPath, SIGNAL(clicked(bool)), SLOT(addProjectPath()) );
    connect( ui->removePath, SIGNAL(clicked(bool)), SLOT(deleteProjectPath()) );
    connect( ui->savePath, SIGNAL(clicked(bool)), SLOT(saveProjectPath()));
    connect( ui->editPath, SIGNAL(clicked(bool)), SLOT(editProjectPath()) );
    connect( ui->cancelEditPath, SIGNAL(clicked(bool)), SLOT(cancelEditingProjectPath()) );
    configurePathButtons();

    ui->addIncludePath->setIcon(KIcon( "list-add" ));
    connect( ui->addIncludePath, SIGNAL(clicked(bool)), SLOT(addIncludePath()) );

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

    ui->pathRequester->comboBox()->setModel( pathsModel );
    connect( ui->pathRequester->comboBox(), SIGNAL(currentIndexChanged(int)), SLOT(projectPathSelected(int)) );
    connect( ui->pathRequester, SIGNAL(openFileDialog(KUrlRequester*)), SLOT(editProjectPath()) );
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

    ui->languageParameters->setCurrentIndex(0);
    languageParametersTabSelected(0);
    connect( ui->languageParameters, SIGNAL(currentChanged(int)), SLOT(languageParametersTabSelected(int)) );

    KAction* delPathAction = new KAction( i18n("Delete Project Path"), this );
    delPathAction->setShortcut( KShortcut( "Del" ) );
    delPathAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    ui->pathRequester->comboBox()->addAction( delPathAction );
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
    ui->pathRequester->comboBox()->setCurrentIndex(0); // at least a project root item is present
    projectPathSelected(0);
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
}

void ConfigWidget::projectPathSelected( int index )
{
    if( index >= 0 ) {
        QModelIndex modelIndex = pathsModel->index(index, 0);
        includesModel->setIncludes( modelIndex.data( ProjectPathsModel::IncludesDataRole ).toStringList() );
        definesModel->setDefines( modelIndex.data( ProjectPathsModel::DefinesDataRole ).toHash() );
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

void ConfigWidget::addIncludePath() {
    KUrl addedUrl = ui->includePathRequester->url();
    addedUrl.cleanPath();
    includesModel->addInclude( addedUrl.toLocalFile( KUrl::RemoveTrailingSlash ) );
}

void ConfigWidget::deleteIncludePath()
{
    includesModel->removeRow( ui->includePaths->currentIndex().row() );
}

void ConfigWidget::languageParametersTabSelected( int index )
{
    disconnect( this, SIGNAL(deleteLanguageParametersEntry()) );

    switch(index) {
    // Include pathes
    case 0:
        connect( this, SIGNAL(deleteLanguageParametersEntry()), SLOT(deleteIncludePath()) );
        break;

    // Defines
    case 1:
        connect( this, SIGNAL(deleteLanguageParametersEntry()), SLOT(deleteDefine()) );
        break;

    default:
        break;
    }
}

void ConfigWidget::addProjectPath()
{
    switch( m_mode ) {
    case UI_NORMAL:
        setUiMode( UI_ADDING );
        break;

    case UI_EDITING:
        // "Add" pressed in editing mode means we should save the data by adding a new item
        // instead of overwriting
        // So pretend that we were in adding mode, not editing
        m_mode = UI_ADDING;
        setUiMode( UI_NORMAL );
        break;

    case UI_ADDING:
    default:
        Q_ASSERT_X( 0, "ConfigWidget::addProjectPath()", "Invalid state for add button" );
        break;
    }
}

void ConfigWidget::editProjectPath()
{
    if( m_mode == UI_NORMAL )
        setUiMode( UI_EDITING );
}

void ConfigWidget::saveProjectPath()
{
    setUiMode( UI_NORMAL );
}

void ConfigWidget::cancelEditingProjectPath()
{
    if( m_mode != UI_NORMAL ) {
        m_mode = UI_NORMAL;
        setPathSelectorNonEditable();
        configurePathButtons();
    }
}

void ConfigWidget::deleteProjectPath()
{
    pathsModel->removeRow( ui->pathRequester->comboBox()->currentIndex() );
}

QModelIndex ConfigWidget::currentPathIndex()
{
    int currentPath = ui->pathRequester->comboBox()->currentIndex();
    return (currentPath < 0) ? QModelIndex() : ui->pathRequester->comboBox()->model()->index( currentPath, 0 );
}

void ConfigWidget::setProject(KDevelop::IProject* w_project)
{
    pathsModel->setProject( w_project );
    ui->pathRequester->setStartDir( w_project->folder() );
    ui->includePathRequester->setStartDir( w_project->folder() );
}

void ConfigWidget::verify() {
    Q_ASSERT( ui->pathRequester->comboBox()->count() == pathsModel->rowCount() );

    bool hasAnyPaths = (ui->pathRequester->comboBox()->count() != 0);
    bool pathSelected = (ui->pathRequester->comboBox()->currentIndex() != -1);
    Q_ASSERT( !hasAnyPaths || pathSelected );

    ui->languageParameters->setEnabled( hasAnyPaths );
    ui->removePath->setEnabled( hasAnyPaths );
}

void ConfigWidget::setPathSelectorEditable( QString editText )
{
    ui->pathRequester->comboBox()->setEditable( true );

    if( editText.isEmpty() )
        ui->pathRequester->comboBox()->clearEditText();
    else
        ui->pathRequester->comboBox()->setEditText( editText );
}

void ConfigWidget::setPathSelectorNonEditable()
{
    ui->pathRequester->comboBox()->setEditable( false );
}

void ConfigWidget::setUiMode( ConfigWidget::UiMode mode )
{
    if( m_mode == mode )
        return;

    QModelIndex pathIndex = currentPathIndex();

    // Do not allow to edit inexistent records
    // Instead, add a new record in all these cases
    if( mode == UI_EDITING && !pathIndex.isValid() )
        mode = UI_ADDING;

    switch( mode ) {
    case UI_ADDING:
        setPathSelectorEditable( QString() );

        break;

    case UI_EDITING:
        setPathSelectorEditable( pathsModel->data( currentPathIndex(), Qt::EditRole ).toString() );

        break;

    case UI_NORMAL: {
        KUrl savedUrl = ui->pathRequester->url();

        // Refuse to write an invalid or empty URL
        if( !savedUrl.isValid() || savedUrl.isEmpty() )
            return;

        // Further actions depend on previous mode
        switch( m_mode ) {
        case UI_ADDING:
            pathsModel->addPath( savedUrl );
            setPathSelectorNonEditable();
            break;

        case UI_EDITING:
            pathsModel->setData( pathIndex, savedUrl.url(), Qt::EditRole );
            setPathSelectorNonEditable();
            break;

        case UI_NORMAL:
        default:
            Q_ASSERT_X( 0, "ConfigWidget::setUiMode()", "Invalid transition" );
            break;
        }

        // The item may got inserted or not due to duplicate detection, we don't know
        // Attempt a search to find it
        for( int i = 0; i < pathsModel->rowCount(); ++i ) {
            if( pathsModel->data( pathsModel->index(i), ProjectPathsModel::FullUrlDataRole ).value<KUrl>() == savedUrl ) {
                ui->pathRequester->comboBox()->setCurrentIndex( i );
                projectPathSelected( i );
                break;
            }
        }

        break;
    } // case UI_NORMAL

    default:
        Q_ASSERT_X( 0, "ConfigWidget::setUiMode()", "Invalid target" );
        break;
    }

    m_mode = mode;
    configurePathButtons();
}

void ConfigWidget::configurePathButtons()
{
    bool editing = ( m_mode != UI_NORMAL );
    ui->cancelEditPath->setVisible( editing );
    ui->savePath->setVisible( editing );
    ui->removePath->setVisible( !editing );
    ui->editPath->setVisible( !editing );
    ui->addPath->setEnabled( m_mode != UI_ADDING );
}


#include "configwidget.moc"

