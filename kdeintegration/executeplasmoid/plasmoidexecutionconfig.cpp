/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>
    Copyright 2009 Niko Sams <niko.sams@gmail.com>

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
#include "plasmoidexecutionconfig.h"
#include "plasmoidexecutionjob.h"
#include "debug.h"

#include <KLocalizedString>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iuicontroller.h>
#include <project/projectmodel.h>
#include <project/builderjob.h>
#include <serialization/indexedstring.h>
#include <util/kdevstringhandler.h>
#include <util/executecompositejob.h>
#include <util/path.h>

#include <KMessageBox>
#include <KParts/MainWindow>
#include <KConfigGroup>
#include <QMenu>
#include <QLineEdit>

Q_DECLARE_METATYPE(KDevelop::IProject*);

QIcon PlasmoidExecutionConfig::icon() const
{
    return QIcon::fromTheme("system-run");
}

QStringList readProcess(QProcess* p)
{
    QStringList ret;
    while(!p->atEnd()) {
        QByteArray line = p->readLine();
        int nameEnd=line.indexOf(' ');
        if(nameEnd>0) {
            ret += line.left(nameEnd);
        }
    }
    return ret;
}

PlasmoidExecutionConfig::PlasmoidExecutionConfig( QWidget* parent )
    : LaunchConfigurationPage( parent )
{
    setupUi(this);
    connect( identifier->lineEdit(), SIGNAL(textEdited(QString)), SIGNAL(changed()) );

    QProcess pPlasmoids;
    pPlasmoids.start("plasmoidviewer", QStringList("--list"), QIODevice::ReadOnly);

    QProcess pThemes;
    pThemes.start("plasmoidviewer", QStringList("--list-themes"), QIODevice::ReadOnly);
    pThemes.waitForFinished();
    pPlasmoids.waitForFinished();

    foreach(const QString& plasmoid, readProcess(&pPlasmoids)) {
        identifier->addItem(plasmoid);
    }

    themes->addItem(QString());
    foreach(const QString& theme, readProcess(&pThemes)) {
        themes->addItem(theme);
    }

    addDependency->setIcon( QIcon::fromTheme("list-add") );
    removeDependency->setIcon( QIcon::fromTheme("list-remove") );
    moveDepUp->setIcon( QIcon::fromTheme("go-up") );
    moveDepDown->setIcon( QIcon::fromTheme("go-down") );
    browseProject->setIcon(QIcon::fromTheme("folder-document"));

    connect( addDependency, SIGNAL(clicked(bool)), SIGNAL(changed()) );
    connect( removeDependency, SIGNAL(clicked(bool)), SIGNAL(changed()) );
    connect( moveDepDown, SIGNAL(clicked(bool)), SIGNAL(changed()) );
    connect( moveDepUp, SIGNAL(clicked(bool)), SIGNAL(changed()) );
    connect( browseProject, SIGNAL(clicked(bool)), targetDependency, SLOT(selectItemDialog()));
    connect( addDependency, SIGNAL(clicked(bool)), SLOT(addDep()) );
    connect( removeDependency, SIGNAL(clicked(bool)), SLOT(removeDep()) );
    connect( moveDepDown, SIGNAL(clicked(bool)), SLOT(moveDependencyDown()) );
    connect( moveDepUp, SIGNAL(clicked(bool)), SLOT(moveDependencyUp()) );
    connect( dependencies->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(checkActions(QItemSelection,QItemSelection)) );
    connect( targetDependency, SIGNAL(textChanged(QString)), SLOT(depEdited(QString)));
}

void PlasmoidExecutionConfig::saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project ) const
{
    Q_UNUSED( project );
    cfg.writeEntry("PlasmoidIdentifier", identifier->lineEdit()->text());
    QStringList args;
    args += "--formfactor";
    args += formFactor->currentText();
    if(!themes->currentText().isEmpty()) {
        args += "--theme";
        args += themes->currentText();
    }
    cfg.writeEntry("Arguments", args);

    QVariantList deps;
    for( int i = 0; i < dependencies->count(); i++ )
    {
        deps << dependencies->item( i )->data( Qt::UserRole );
    }
    cfg.writeEntry( "Dependencies", KDevelop::qvariantToString( QVariant( deps ) ) );
}

void PlasmoidExecutionConfig::loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* )
{
    bool b = blockSignals( true );
    identifier->lineEdit()->setText(cfg.readEntry("PlasmoidIdentifier", ""));
    blockSignals( b );

    QStringList arguments = cfg.readEntry("Arguments", QStringList());
    int idxFormFactor = arguments.indexOf("--formfactor")+1;
    if(idxFormFactor>0)
        formFactor->setCurrentIndex(formFactor->findText(arguments[idxFormFactor]));

    int idxTheme = arguments.indexOf("--theme")+1;
    if(idxTheme>0)
        themes->setCurrentIndex(themes->findText(arguments[idxTheme]));

    QVariantList deps = KDevelop::stringToQVariant( cfg.readEntry( "Dependencies", QString() ) ).toList();
    QStringList strDeps;
    foreach( const QVariant& dep, deps ) {
        QStringList deplist = dep.toStringList();
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        KDevelop::ProjectBaseItem* pitem=model->itemFromIndex(model->pathToIndex(deplist));
        QIcon icon;
        if(pitem)
            icon=QIcon(pitem->iconName());

        QListWidgetItem* item = new QListWidgetItem(icon, KDevelop::joinWithEscaping( deplist, '/', '\\' ), dependencies );
        item->setData( Qt::UserRole, dep );
    }
}

QString PlasmoidExecutionConfig::title() const
{
    return i18n("Configure Plasmoid Execution");
}

QList< KDevelop::LaunchConfigurationPageFactory* > PlasmoidLauncher::configPages() const
{
    return QList<KDevelop::LaunchConfigurationPageFactory*>();
}

QString PlasmoidLauncher::description() const
{
    return i18n("Display a plasmoid");
}

QString PlasmoidLauncher::id()
{
    return "PlasmoidLauncher";
}

QString PlasmoidLauncher::name() const
{
    return i18n("Plasmoid Launcher");
}

PlasmoidLauncher::PlasmoidLauncher(ExecutePlasmoidPlugin* plugin)
    : m_plugin(plugin)
{
}

KJob* PlasmoidLauncher::start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg)
{
    Q_ASSERT(cfg);
    if( !cfg )
    {
        return 0;
    }

    if( launchMode == "execute" )
    {
        KJob* depsJob = dependencies(cfg);
        QList<KJob*> jobs;
        if(depsJob)
            jobs << depsJob;
        jobs << new PlasmoidExecutionJob(m_plugin, cfg);

        return new KDevelop::ExecuteCompositeJob( KDevelop::ICore::self()->runController(), jobs );
    }
    qWarning() << "Unknown launch mode " << launchMode << "for config:" << cfg->name();
    return 0;
}

KJob* PlasmoidLauncher::calculateDependencies(KDevelop::ILaunchConfiguration* cfg)
{
    QVariantList deps = KDevelop::stringToQVariant( cfg->config().readEntry( "Dependencies", QString() ) ).toList();
    if( !deps.isEmpty() )
    {
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        QList<KDevelop::ProjectBaseItem*> items;
        foreach( const QVariant& dep, deps )
        {
            KDevelop::ProjectBaseItem* item = model->itemFromIndex( model->pathToIndex( dep.toStringList() ) );
            if( item )
            {
                items << item;
            }
            else
            {
                KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(),
                                   i18n("Couldn't resolve the dependency: %1", dep.toString()));
            }
        }
        KDevelop::BuilderJob* job = new KDevelop::BuilderJob;
        job->addItems( KDevelop::BuilderJob::Install, items );
        job->updateJobName();
        return job;
    }
    return 0;
}

KJob* PlasmoidLauncher::dependencies(KDevelop::ILaunchConfiguration* cfg)
{
    return calculateDependencies(cfg);
}


QStringList PlasmoidLauncher::supportedModes() const
{
    return QStringList() << "execute";
}

KDevelop::LaunchConfigurationPage* PlasmoidPageFactory::createWidget(QWidget* parent)
{
    return new PlasmoidExecutionConfig( parent );
}

PlasmoidPageFactory::PlasmoidPageFactory()
{}

PlasmoidExecutionConfigType::PlasmoidExecutionConfigType()
{
    factoryList.append( new PlasmoidPageFactory );
}

PlasmoidExecutionConfigType::~PlasmoidExecutionConfigType()
{
    qDeleteAll(factoryList);
    factoryList.clear();
}

QString PlasmoidExecutionConfigType::name() const
{
    return i18n("Plasmoid Launcher");
}

QList<KDevelop::LaunchConfigurationPageFactory*> PlasmoidExecutionConfigType::configPages() const
{
    return factoryList;
}

QString PlasmoidExecutionConfigType::typeId()
{
    return "PlasmoidLauncherType";
}

QIcon PlasmoidExecutionConfigType::icon() const
{
    return QIcon::fromTheme("plasma");
}

static bool canLaunchMetadataFile(const KDevelop::Path &path)
{
    KConfig cfg(path.toLocalFile(), KConfig::SimpleConfig);
    KConfigGroup group(&cfg, "Desktop Entry");
    QStringList services = group.readEntry("ServiceTypes", group.readEntry("X-KDE-ServiceTypes", QStringList()));
    return services.contains("Plasma/Applet");
}

//don't bother, nobody uses this interface
bool PlasmoidExecutionConfigType::canLaunch(const QUrl& ) const
{
    return false;
}

bool PlasmoidExecutionConfigType::canLaunch(KDevelop::ProjectBaseItem* item) const
{
    KDevelop::ProjectFolderItem* folder = item->folder();
    if(folder && folder->hasFileOrFolder("metadata.desktop")) {
        return canLaunchMetadataFile(KDevelop::Path(folder->path(), "metadata.desktop"));
    }
    return false;
}

void PlasmoidExecutionConfigType::configureLaunchFromItem(KConfigGroup config, KDevelop::ProjectBaseItem* item) const
{
    config.writeEntry("PlasmoidIdentifier", item->url().toLocalFile());
}

void PlasmoidExecutionConfigType::configureLaunchFromCmdLineArguments(KConfigGroup /*config*/, const QStringList &/*args*/) const
{}

QMenu* PlasmoidExecutionConfigType::launcherSuggestions()
{
    QList<QAction*> found;
    QList<KDevelop::IProject*> projects = KDevelop::ICore::self()->projectController()->projects();
    foreach(KDevelop::IProject* p, projects) {
        QSet<KDevelop::IndexedString> files = p->fileSet();
        foreach(const KDevelop::IndexedString& file, files) {
            KDevelop::Path path(file.str());
            if (path.lastPathSegment() == "metadata.desktop" && canLaunchMetadataFile(path)) {
                path = path.parent();
                QString relUrl = p->path().relativePath(path);
                QAction* action = new QAction(relUrl, this);
                action->setProperty("url", relUrl);
                action->setProperty("project", qVariantFromValue<KDevelop::IProject*>(p));
                connect(action, SIGNAL(triggered(bool)), SLOT(suggestionTriggered()));
                found.append(action);
            }
        }
    }

    QMenu *m = 0;
    if(!found.isEmpty()) {
        m = new QMenu(i18n("Plasmoids"));
        m->addActions(found);
    }
    return m;
}

void PlasmoidExecutionConfigType::suggestionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    KDevelop::IProject* p = action->property("project").value<KDevelop::IProject*>();
    QString relUrl = action->property("url").toString();

    QPair<QString,QString> launcher = qMakePair( launchers().at( 0 )->supportedModes().at(0), launchers().at( 0 )->id() );

    QString name = relUrl.mid(relUrl.lastIndexOf('/')+1);
    KDevelop::ILaunchConfiguration* config = KDevelop::ICore::self()->runController()->createLaunchConfiguration(this, launcher, p, name);
    KConfigGroup cfg = config->config();
    cfg.writeEntry("PlasmoidIdentifier", relUrl);
    emit signalAddLaunchConfiguration(config);
}

///// slightly copied from nativeapp

void PlasmoidExecutionConfig::depEdited(const QString& str)
{
    int pos;
    QString tmp = str;
    addDependency->setEnabled( !str.isEmpty()
                               && ( !targetDependency->validator()
                               || targetDependency->validator()->validate( tmp, pos ) == QValidator::Acceptable ) );
}

void PlasmoidExecutionConfig::addDep()
{
    QIcon icon;
    KDevelop::ProjectBaseItem* pitem = targetDependency->currentItem();
    if(pitem)
        icon= QIcon(pitem->iconName());

    QListWidgetItem* item = new QListWidgetItem(icon, targetDependency->text(), dependencies);
    item->setData( Qt::UserRole, targetDependency->itemPath() );
    targetDependency->setText("");
    addDependency->setEnabled( false );
    dependencies->selectionModel()->clearSelection();
    item->setSelected(true);
//     dependencies->selectionModel()->select( dependencies->model()->index( dependencies->model()->rowCount() - 1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectCurrent );
}

void PlasmoidExecutionConfig::removeDep()
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

void PlasmoidExecutionConfig::checkActions(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED( deselected );
    qCDebug(EXECUTEPLASMOID) << "checkActions";
    if( !selected.indexes().isEmpty() )
    {
        qCDebug(EXECUTEPLASMOID) << "have selection";
        Q_ASSERT( selected.indexes().count() == 1 );
        QModelIndex idx = selected.indexes().at( 0 );
        qCDebug(EXECUTEPLASMOID) << "index" << idx;
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

void PlasmoidExecutionConfig::moveDependencyDown()
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

void PlasmoidExecutionConfig::moveDependencyUp()
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
