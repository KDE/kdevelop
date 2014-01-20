/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "nativeappconfig.h"

#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/ilaunchconfiguration.h>

#include <project/projectmodel.h>

#include "nativeappjob.h"
#include <interfaces/iproject.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/builderjob.h>
#include <kmessagebox.h>
#include <interfaces/iuicontroller.h>
#include <util/executecompositejob.h>
#include <kparts/mainwindow.h>
#include <interfaces/iplugincontroller.h>

#include "executeplugin.h"
#include <util/kdevstringhandler.h>
#include <util/environmentgrouplist.h>
#include <project/projectitemlineedit.h>
#include "projecttargetscombobox.h"
#include <QMenu>

#include <KCModuleProxy>
#include <KConfigDialog>
#include <KShell>
#include <kcmoduleinfo.h>

QIcon NativeAppConfigPage::icon() const
{
    return QIcon::fromTheme("system-run");
}

static KDevelop::ProjectBaseItem* itemForPath(const QStringList& path, KDevelop::ProjectModel* model)
{
    return model->itemFromIndex(model->pathToIndex(path));
}

//TODO: Make sure to auto-add the executable target to the dependencies when its used.

void NativeAppConfigPage::loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* project ) 
{
    bool b = blockSignals( true );
    projectTarget->setBaseItem( project ? project->projectItem() : 0, true);
    projectTarget->setCurrentItemPath( cfg.readEntry( ExecutePlugin::projectTargetEntry, QStringList() ) );

    KUrl exe = cfg.readEntry( ExecutePlugin::executableEntry, QUrl());
    if( !exe.isEmpty() || project ){
        executablePath->setUrl( !exe.isEmpty() ? exe : project->folder() );
    }else{
        KDevelop::IProjectController* pc = KDevelop::ICore::self()->projectController();
        if( pc ){
            executablePath->setUrl( pc->projects().count() ? pc->projects().first()->folder() : KUrl() );
        }
    }

    //executablePath->setFilter("application/x-executable");

    executableRadio->setChecked( true );
    if ( !cfg.readEntry( ExecutePlugin::isExecutableEntry, false ) && projectTarget->count() ){
        projectTargetRadio->setChecked( true );
    }

    arguments->setClearButtonShown( true );
    arguments->setText( cfg.readEntry( ExecutePlugin::argumentsEntry, "" ) );
    workingDirectory->setUrl( cfg.readEntry( ExecutePlugin::workingDirEntry, QUrl() ) );
    environment->setCurrentProfile( cfg.readEntry( ExecutePlugin::environmentGroupEntry, QString() ) );
    runInTerminal->setChecked( cfg.readEntry( ExecutePlugin::useTerminalEntry, false ) );
    terminal->setEditText( cfg.readEntry( ExecutePlugin::terminalEntry, terminal->itemText(0) ) );
    QVariantList deps = KDevelop::stringToQVariant( cfg.readEntry( ExecutePlugin::dependencyEntry, QString() ) ).toList();
    QStringList strDeps;
    foreach( const QVariant& dep, deps ) {
        QStringList deplist = dep.toStringList();
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        KDevelop::ProjectBaseItem* pitem=itemForPath(deplist, model);
        QIcon icon;
        if(pitem)
            icon=QIcon::fromTheme(pitem->iconName());
        
        QListWidgetItem* item = new QListWidgetItem(icon, KDevelop::joinWithEscaping( deplist, '/', '\\' ), dependencies );
        item->setData( Qt::UserRole, dep );
    }
    dependencyAction->setCurrentIndex( dependencyAction->findData( cfg.readEntry( ExecutePlugin::dependencyActionEntry, "Nothing" ) ) );
    blockSignals( b );
}

NativeAppConfigPage::NativeAppConfigPage( QWidget* parent ) 
    : LaunchConfigurationPage( parent )
{
    setupUi(this);    
    //Setup data info for combobox
    dependencyAction->setItemData(0, "Nothing" );
    dependencyAction->setItemData(1, "Build" );
    dependencyAction->setItemData(2, "Install" );
    dependencyAction->setItemData(3, "SudoInstall" );

    addDependency->setIcon( QIcon::fromTheme("list-add") );
    removeDependency->setIcon( QIcon::fromTheme("list-remove") );
    moveDepUp->setIcon( QIcon::fromTheme("go-up") );
    moveDepDown->setIcon( QIcon::fromTheme("go-down") );
    browseProject->setIcon(QIcon::fromTheme("folder-document"));
    
    //Set workingdirectory widget to ask for directories rather than files
    workingDirectory->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);

    configureEnvironment->setSelectionWidget(environment);

    //connect signals to changed signal
    connect( projectTarget, SIGNAL(currentIndexChanged(QString)), SIGNAL(changed()) );
    connect( projectTargetRadio, SIGNAL(toggled(bool)), SIGNAL(changed()) );
    connect( executableRadio, SIGNAL(toggled(bool)), SIGNAL(changed()) );
    connect( executablePath->lineEdit(), SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( executablePath, SIGNAL(urlSelected(KUrl)), SIGNAL(changed()) );
    connect( arguments, SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( workingDirectory, SIGNAL(urlSelected(KUrl)), SIGNAL(changed()) );
    connect( workingDirectory->lineEdit(), SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( environment, SIGNAL(currentProfileChanged(QString)), SIGNAL(changed()) );
    connect( addDependency, SIGNAL(clicked(bool)), SLOT(addDep()) );
    connect( addDependency, SIGNAL(clicked(bool)), SIGNAL(changed()) );
    connect( removeDependency, SIGNAL(clicked(bool)), SIGNAL(changed()) );
    connect( removeDependency, SIGNAL(clicked(bool)), SLOT(removeDep()) );
    connect( moveDepDown, SIGNAL(clicked(bool)), SIGNAL(changed()) );
    connect( moveDepUp, SIGNAL(clicked(bool)), SIGNAL(changed()) );
    connect( moveDepDown, SIGNAL(clicked(bool)), SLOT(moveDependencyDown()) );
    connect( moveDepUp, SIGNAL(clicked(bool)), SLOT(moveDependencyUp()) );
    connect( dependencies->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(checkActions(QItemSelection,QItemSelection)) );
    connect( dependencyAction, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );
    connect( runInTerminal, SIGNAL(toggled(bool)), SIGNAL(changed()) );
    connect( terminal, SIGNAL(editTextChanged(QString)), SIGNAL(changed()) );
    connect( terminal, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );
    connect( dependencyAction, SIGNAL(currentIndexChanged(int)), SLOT(activateDeps(int)) );
    connect( targetDependency, SIGNAL(textChanged(QString)), SLOT(depEdited(QString)));
    connect( browseProject, SIGNAL(clicked(bool)), SLOT(selectItemDialog()));
}


void NativeAppConfigPage::depEdited( const QString& str )
{
    int pos;
    QString tmp = str;
    addDependency->setEnabled( !str.isEmpty() 
                               && ( !targetDependency->validator() 
                               || targetDependency->validator()->validate( tmp, pos ) == QValidator::Acceptable ) );
}

void NativeAppConfigPage::activateDeps( int idx )
{
    browseProject->setEnabled( dependencyAction->itemData( idx ).toString() != "Nothing" );
    dependencies->setEnabled( dependencyAction->itemData( idx ).toString() != "Nothing" );
    targetDependency->setEnabled( dependencyAction->itemData( idx ).toString() != "Nothing" );
}

void NativeAppConfigPage::checkActions( const QItemSelection& selected, const QItemSelection& unselected )
{
    Q_UNUSED( unselected );
    kDebug() << "checkActions";
    if( !selected.indexes().isEmpty() )
    {
        kDebug() << "have selection";
        Q_ASSERT( selected.indexes().count() == 1 );
        QModelIndex idx = selected.indexes().at( 0 );
        kDebug() << "index" << idx;
        moveDepUp->setEnabled( idx.row() > 0 );
        moveDepDown->setEnabled( idx.row() < dependencies->count() - 1 );
        removeDependency->setEnabled( true );
    } else 
    {
        removeDependency->setEnabled( false );
        moveDepUp->setEnabled( false );
        moveDepDown->setEnabled( false );
    }
}

void NativeAppConfigPage::moveDependencyDown()
{
    QList<QListWidgetItem*> list = dependencies->selectedItems();
    if( !list.isEmpty() )
    {
        Q_ASSERT( list.count() == 1 );
        QListWidgetItem* item = list.at( 0 );
        int row = dependencies->row( item );
        dependencies->takeItem( row );
        dependencies->insertItem( row+1, item );
        dependencies->selectionModel()->select( dependencies->model()->index( row+1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectCurrent );
    }
}

void NativeAppConfigPage::moveDependencyUp()
{
    
    QList<QListWidgetItem*> list = dependencies->selectedItems();
    if( !list.isEmpty() )
    {
        Q_ASSERT( list.count() == 1 );
        QListWidgetItem* item = list.at( 0 );
        int row = dependencies->row( item );
        dependencies->takeItem( row );
        dependencies->insertItem( row-1, item );
        dependencies->selectionModel()->select( dependencies->model()->index( row-1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectCurrent );
    }
}

void NativeAppConfigPage::addDep()
{
    QIcon icon;
    KDevelop::ProjectBaseItem* pitem = targetDependency->currentItem();
    if(pitem)
        icon = QIcon::fromTheme(pitem->iconName());

    QListWidgetItem* item = new QListWidgetItem(icon, targetDependency->text(), dependencies);
    item->setData( Qt::UserRole, targetDependency->itemPath() );
    targetDependency->setText("");
    addDependency->setEnabled( false );
    dependencies->selectionModel()->clearSelection();
    item->setSelected(true);
//     dependencies->selectionModel()->select( dependencies->model()->index( dependencies->model()->rowCount() - 1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectCurrent );
}

void NativeAppConfigPage::selectItemDialog()
{
    if(targetDependency->selectItemDialog()) {
        addDep();
    }
}

void NativeAppConfigPage::removeDep()
{
    QList<QListWidgetItem*> list = dependencies->selectedItems();
    if( !list.isEmpty() )
    {
        Q_ASSERT( list.count() == 1 );
        int row = dependencies->row( list.at(0) );
        delete dependencies->takeItem( row );
        
        dependencies->selectionModel()->select( dependencies->model()->index( row - 1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectCurrent );
    }
}

void NativeAppConfigPage::saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project ) const
{
    Q_UNUSED( project );
    cfg.writeEntry( ExecutePlugin::isExecutableEntry, executableRadio->isChecked() );
    cfg.writeEntry( ExecutePlugin::executableEntry, executablePath->url() );
    cfg.writeEntry( ExecutePlugin::projectTargetEntry, projectTarget->currentItemPath() );
    cfg.writeEntry( ExecutePlugin::argumentsEntry, arguments->text() );
    cfg.writeEntry( ExecutePlugin::workingDirEntry, workingDirectory->url() );
    cfg.writeEntry( ExecutePlugin::environmentGroupEntry, environment->currentProfile() );
    cfg.writeEntry( ExecutePlugin::useTerminalEntry, runInTerminal->isChecked() );
    cfg.writeEntry( ExecutePlugin::terminalEntry, terminal->currentText() );
    cfg.writeEntry( ExecutePlugin::dependencyActionEntry, dependencyAction->itemData( dependencyAction->currentIndex() ).toString() );
    QVariantList deps;
    for( int i = 0; i < dependencies->count(); i++ )
    {
        deps << dependencies->item( i )->data( Qt::UserRole );
    }
    cfg.writeEntry( ExecutePlugin::dependencyEntry, KDevelop::qvariantToString( QVariant( deps ) ) );
}

QString NativeAppConfigPage::title() const 
{
    return i18n("Configure Native Application");
}

QList< KDevelop::LaunchConfigurationPageFactory* > NativeAppLauncher::configPages() const 
{
    return QList<KDevelop::LaunchConfigurationPageFactory*>();
}

QString NativeAppLauncher::description() const
{
    return "Executes Native Applications";
}

QString NativeAppLauncher::id() 
{
    return "nativeAppLauncher";
}

QString NativeAppLauncher::name() const 
{
    return i18n("Native Application");
}

NativeAppLauncher::NativeAppLauncher()
{
}

KJob* NativeAppLauncher::start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg)
{
    Q_ASSERT(cfg);
    if( !cfg )
    {
        return 0;
    }
    if( launchMode == "execute" )
    {
        IExecutePlugin* iface = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IExecutePlugin", "kdevexecute")->extension<IExecutePlugin>();
        Q_ASSERT(iface);
        KJob* depjob = iface->dependecyJob( cfg );
        QList<KJob*> l;
        if( depjob )
        {
            l << depjob;
        }
        l << new NativeAppJob( KDevelop::ICore::self()->runController(), cfg );
        return new KDevelop::ExecuteCompositeJob( KDevelop::ICore::self()->runController(), l );
        
    }
    kWarning() << "Unknown launch mode " << launchMode << "for config:" << cfg->name();
    return 0;
}

QStringList NativeAppLauncher::supportedModes() const
{
    return QStringList() << "execute";
}

KDevelop::LaunchConfigurationPage* NativeAppPageFactory::createWidget(QWidget* parent)
{
    return new NativeAppConfigPage( parent );
}

NativeAppPageFactory::NativeAppPageFactory()
{
}

NativeAppConfigType::NativeAppConfigType() 
{
    factoryList.append( new NativeAppPageFactory() );
}

NativeAppConfigType::~NativeAppConfigType()
{
    qDeleteAll(factoryList);
    factoryList.clear();
}

QString NativeAppConfigType::name() const
{
    return i18n("Compiled Binary");
}


QList<KDevelop::LaunchConfigurationPageFactory*> NativeAppConfigType::configPages() const 
{
    return factoryList;
}

QString NativeAppConfigType::id() const 
{
    return ExecutePlugin::_nativeAppConfigTypeId;
}

QIcon NativeAppConfigType::icon() const
{
    return QIcon::fromTheme("application-x-executable");
}

bool NativeAppConfigType::canLaunch ( KDevelop::ProjectBaseItem* item ) const
{
    if( item->target() && item->target()->executable() ) {
        return canLaunch( item->target()->executable()->builtUrl() );
    }
    return false;
}

bool NativeAppConfigType::canLaunch ( const KUrl& file ) const
{
    return ( file.isLocalFile() && QFileInfo( file.toLocalFile() ).isExecutable() );
}

void NativeAppConfigType::configureLaunchFromItem ( KConfigGroup cfg, KDevelop::ProjectBaseItem* item ) const
{
    cfg.writeEntry( ExecutePlugin::isExecutableEntry, false );
    KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
    cfg.writeEntry( ExecutePlugin::projectTargetEntry, model->pathFromIndex( model->indexFromItem( item ) ) );
    cfg.writeEntry( ExecutePlugin::workingDirEntry, QUrl(item->executable()->builtUrl().upUrl()) );
    cfg.sync();
}

void NativeAppConfigType::configureLaunchFromCmdLineArguments ( KConfigGroup cfg, const QStringList& args ) const
{
    cfg.writeEntry( ExecutePlugin::isExecutableEntry, true );
    cfg.writeEntry( ExecutePlugin::executableEntry, args.first() );
    QStringList a(args);
    a.removeFirst();
    cfg.writeEntry( ExecutePlugin::argumentsEntry, KShell::joinArgs(a) );
    cfg.sync();
}

QList<KDevelop::ProjectTargetItem*> targetsInFolder(KDevelop::ProjectFolderItem* folder)
{
    QList<KDevelop::ProjectTargetItem*> ret;
    foreach(KDevelop::ProjectFolderItem* f, folder->folderList())
        ret += targetsInFolder(f);
    
    ret += folder->targetList();
    return ret;
}

bool actionLess(QAction* a, QAction* b)
{
    return a->text() < b->text();
}

bool menuLess(QMenu* a, QMenu* b)
{
    return a->title() < b->title();
}

QMenu* NativeAppConfigType::launcherSuggestions()
{
    QMenu* ret = new QMenu(i18n("Project Executables"));
    
    KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
    QList<KDevelop::IProject*> projects = KDevelop::ICore::self()->projectController()->projects();
    
    foreach(KDevelop::IProject* project, projects) {
        if(project->projectFileManager()->features() & KDevelop::IProjectFileManager::Targets) {
            QList<KDevelop::ProjectTargetItem*> targets=targetsInFolder(project->projectItem());
            QHash<KDevelop::ProjectBaseItem*, QList<QAction*> > targetsContainer;
            QMenu* projectMenu = ret->addMenu(QIcon::fromTheme("project-development"), project->name());
            foreach(KDevelop::ProjectTargetItem* target, targets) {
                if(target->executable()) {
                    QStringList path = model->pathFromIndex(target->index());
                    if(!path.isEmpty()){
                        QAction* act = new QAction(projectMenu);
                        act->setData(KDevelop::joinWithEscaping(path, '/','\\'));
                        act->setProperty("name", target->text());
                        path.removeFirst();
                        act->setText(path.join("/"));
                        act->setIcon(QIcon::fromTheme("system-run"));
                        connect(act, SIGNAL(triggered(bool)), SLOT(suggestionTriggered()));
                        targetsContainer[target->parent()].append(act);
                    }
                }
            }

            QList<QAction*> separateActions;
            QList<QMenu*> submenus;
            foreach(KDevelop::ProjectBaseItem* folder, targetsContainer.keys()) {
                QList<QAction*> actions = targetsContainer.value(folder);
                if(actions.size()==1 || !folder->parent()) {
                    separateActions += actions.first();
                } else {
                    foreach(QAction* a, actions) {
                        a->setText(a->property("name").toString());
                    }
                    QStringList path = model->pathFromIndex(folder->index());
                    path.removeFirst();
                    QMenu* submenu = new QMenu(path.join("/"));
                    submenu->addActions(actions);
                    submenus += submenu;
                }
            }
            qSort(separateActions.begin(), separateActions.end(), actionLess);
            qSort(submenus.begin(), submenus.end(), menuLess);
            foreach(QMenu* m, submenus)
                projectMenu->addMenu(m);
            projectMenu->addActions(separateActions);

            projectMenu->setEnabled(!projectMenu->isEmpty());
        }
    }
    
    return ret;
}

void NativeAppConfigType::suggestionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
    KDevelop::ProjectTargetItem* pitem = dynamic_cast<KDevelop::ProjectTargetItem*>(itemForPath(KDevelop::splitWithEscaping(action->data().toString(),'/', '\\'), model));
    if(pitem) {
        QPair<QString,QString> launcher = qMakePair( launchers().at( 0 )->supportedModes().at(0), launchers().at( 0 )->id() );
        KDevelop::IProject* p = pitem->project();
        
        KDevelop::ILaunchConfiguration* config = KDevelop::ICore::self()->runController()->createLaunchConfiguration(this, launcher, p, pitem->text());
        KConfigGroup cfg = config->config();
        
        QStringList splitPath = model->pathFromIndex(pitem->index());
//         QString path = KDevelop::joinWithEscaping(splitPath,'/','\\');
        cfg.writeEntry( ExecutePlugin::projectTargetEntry, splitPath );
        cfg.writeEntry( ExecutePlugin::dependencyEntry, KDevelop::qvariantToString( QVariantList() << splitPath ) );
        cfg.writeEntry( ExecutePlugin::dependencyActionEntry, "Build" );
        cfg.sync();
        
        emit signalAddLaunchConfiguration(config);
    }
}

#include "nativeappconfig.moc"
