/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007-2008 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakepreferences.h"

#include <interfaces/icore.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KUrl>
#include <KMessageBox>
#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <QFile>
#include <QHeaderView>

#include "ui_cmakebuildsettings.h"
#include "cmakecachedelegate.h"
#include "cmakebuilddirchooser.h"
#include "cmakeutils.h"
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <interfaces/iruncontroller.h>
#include <KStandardDirs>

K_PLUGIN_FACTORY(CMakePreferencesFactory, registerPlugin<CMakePreferences>(); )
K_EXPORT_PLUGIN(CMakePreferencesFactory("kcm_kdevcmake_settings"))

CMakePreferences::CMakePreferences(QWidget* parent, const QVariantList& args)
    : KCModule(CMakePreferencesFactory::componentData(), parent, args) , m_currentModel(0)
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    m_prefsUi = new Ui::CMakeBuildSettings;
    m_prefsUi->setupUi( w );
    l->addWidget( w );

    m_prefsUi->addBuildDir->setIcon(KIcon( "list-add" ));
    m_prefsUi->removeBuildDir->setIcon(KIcon( "list-remove" ));
    
    m_prefsUi->addBuildDir->setText(QString());
    m_prefsUi->removeBuildDir->setText(QString());
    m_prefsUi->cacheList->setItemDelegate(new CMakeCacheDelegate(m_prefsUi->cacheList));
    m_prefsUi->cacheList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_prefsUi->cacheList->horizontalHeader()->setStretchLastSection(true);
    m_prefsUi->cacheList->verticalHeader()->hide();

    connect(m_prefsUi->buildDirs, SIGNAL(currentIndexChanged(const QString& )),
            this, SLOT(buildDirChanged( const QString & )));
    connect(m_prefsUi->showInternal, SIGNAL( stateChanged ( int ) ),
            this, SLOT(showInternal ( int )));
    connect(m_prefsUi->addBuildDir, SIGNAL(pressed()), this, SLOT(createBuildDir()));
    connect(m_prefsUi->removeBuildDir, SIGNAL(pressed()), this, SLOT(removeBuildDir()));
    connect(m_prefsUi->showAdvanced, SIGNAL(toggled(bool)), this, SLOT(showAdvanced(bool)));
    
    showInternal(m_prefsUi->showInternal->checkState());
    m_subprojFolder=KUrl(args[1].toString()).upUrl();
    m_project = KDevelop::ICore::self()->projectController()->findProjectForUrl(m_subprojFolder);

    m_prefsUi->showAdvanced->setChecked(false);
    showAdvanced(false);
    load();
}

CMakePreferences::~CMakePreferences()
{}

void CMakePreferences::load()
{
    m_prefsUi->buildDirs->clear();
    m_prefsUi->buildDirs->addItems(CMake::allBuildDirs(m_project));
    m_prefsUi->buildDirs->setCurrentIndex( m_prefsUi->buildDirs->findText( CMake::currentBuildDir(m_project).toLocalFile() ) );
    
    m_srcFolder=m_subprojFolder;
    m_srcFolder.cd(CMake::projectRootRelative(m_project));

    if(m_prefsUi->buildDirs->count()==0)
    {
        m_prefsUi->removeBuildDir->setEnabled(false);
    }
}

void CMakePreferences::save()
{
    QStringList bDirs;
    int count=m_prefsUi->buildDirs->model()->rowCount();
    for(int i=0; i<count; i++)
    {
        bDirs += m_prefsUi->buildDirs->itemText(i);
    }
    CMake::setAllBuildDirs(m_project, bDirs);

    KUrl currentBuildDir( m_prefsUi->buildDirs->currentText() );
    CMake::setCurrentBuildDir(m_project, currentBuildDir);

    KUrl cmakeCmd;
    KUrl installPrefix;
    QString buildType;
    QString extraConfigArgs;
    bool needReconfiguring = true;

    if(m_currentModel)
    {
        cmakeCmd=m_currentModel->value("CMAKE_COMMAND");
        installPrefix=m_currentModel->value("CMAKE_INSTALL_PREFIX");
        buildType=m_currentModel->value("CMAKE_BUILD_TYPE");
        if (!m_currentModel->writeDown()) {
            KMessageBox::error(this, i18n("Could not write CMake settings to file '%1'.\nCheck that you have write access to it", m_currentModel->filePath().pathOrUrl()));
            needReconfiguring = false;
        }
    }
    else if (m_newBuildFolderSettings.contains(currentBuildDir))
    {
        BuildFolderSettings settings = m_newBuildFolderSettings[currentBuildDir];
        cmakeCmd = settings.cmakeBinary;
        installPrefix = settings.installDir;
        buildType = settings.buildType;
        extraConfigArgs = settings.extraArguments;
    }

    CMake::setCurrentCMakeBinary(m_project, cmakeCmd);
    CMake::setCurrentInstallDir(m_project, installPrefix);
    CMake::setCurrentBuildType(m_project, buildType);
    CMake::setCurrentExtraArguments(m_project, extraConfigArgs);

    //We run cmake on the builddir to generate it 
    if (needReconfiguring) {
        configure();
    }
}

void CMakePreferences::updateCache(const KUrl& newBuildDir)
{
    KUrl file(newBuildDir);
    file.addPath("CMakeCache.txt");
    if(QFile::exists(file.toLocalFile()))
    {
        m_currentModel=new CMakeCacheModel(this, file);
        m_prefsUi->cacheList->setModel(m_currentModel);
        m_prefsUi->cacheList->hideColumn(1);
        m_prefsUi->cacheList->hideColumn(3);
        m_prefsUi->cacheList->hideColumn(4);
        m_prefsUi->cacheList->resizeColumnToContents(0);
        m_prefsUi->cacheList->setEnabled(true);
        connect(m_currentModel, SIGNAL( itemChanged ( QStandardItem * ) ),
                this, SLOT( cacheEdited( QStandardItem * ) ));
        connect(m_prefsUi->cacheList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(listSelectionChanged (QModelIndex,QModelIndex)));
        
        foreach(const QModelIndex &idx, m_currentModel->persistentIndices())
        {
            m_prefsUi->cacheList->openPersistentEditor(idx);
        }
        
        showInternal(m_prefsUi->showInternal->checkState());
    }
    else if (m_currentModel)
    {
        m_currentModel->clear();
        delete m_currentModel;
        m_currentModel=0;
        m_prefsUi->cacheList->setEnabled(false);
    }
}

void CMakePreferences::listSelectionChanged(const QModelIndex & index, const QModelIndex& )
{
    QModelIndex idx = index.sibling(index.row(), 3);
    QModelIndex idxType = index.sibling(index.row(), 1);
    QString comment=QString("%1. %2")
            .arg(m_currentModel->itemFromIndex(idxType)->text())
            .arg(m_currentModel->itemFromIndex(idx)->text());
    m_prefsUi->commentText->setText(comment);
}

void CMakePreferences::showInternal(int state)
{
    if(!m_currentModel)
        return;

    bool showAdv=(state == Qt::Checked);
    for(int i=0; i<m_currentModel->rowCount(); i++)
    {
        bool hidden=m_currentModel->isInternal(i) || (!showAdv && m_currentModel->isAdvanced(i));
        m_prefsUi->cacheList->setRowHidden(i, hidden);
    }
}

void CMakePreferences::buildDirChanged(const QString &str)
{
    updateCache(str);
    emit changed(true);
}

void CMakePreferences::createBuildDir()
{
    CMakeBuildDirChooser bdCreator;
    bdCreator.setSourceFolder( m_srcFolder );
    
    QStringList used;
    for(int i=0; i<m_prefsUi->buildDirs->count(); i++)
    {
        used += m_prefsUi->buildDirs->itemText(i);
        
        //Comparisons are with TrailingSlash-less paths, remove it just in case
        if(used.last().endsWith('/'))
            used.last().chop(1);
    }
    bdCreator.setAlreadyUsed(used);
    bdCreator.setCMakeBinary(KStandardDirs::findExe("cmake"));
    
    if(bdCreator.exec())
    {
        m_prefsUi->buildDirs->addItem(bdCreator.buildFolder().toLocalFile(KUrl::RemoveTrailingSlash));
        m_prefsUi->removeBuildDir->setEnabled(true);
        KUrl buildFolder = bdCreator.buildFolder();
        m_newBuildFolderSettings[buildFolder].buildType = bdCreator.buildType();
        m_newBuildFolderSettings[buildFolder].cmakeBinary = bdCreator.cmakeBinary();
        m_newBuildFolderSettings[buildFolder].extraArguments = bdCreator.extraArguments();
        m_newBuildFolderSettings[buildFolder].installDir = bdCreator.installPrefix();
        emit changed(true);
    }
}

void CMakePreferences::removeBuildDir()
{
    QString removed=m_prefsUi->buildDirs->currentText();
    int curr=m_prefsUi->buildDirs->currentIndex();
    if(curr>=0)
    {
        m_prefsUi->buildDirs->removeItem(curr);
    }
    if(m_prefsUi->buildDirs->count()==0)
        m_prefsUi->removeBuildDir->setEnabled(false);
    
    if(QDir(removed).exists())
    {
        int ret=KMessageBox::warningYesNo(this,
                i18n("The %1 directory is about to be removed in KDevelop's list.\n"
                    "Do you want KDevelop to remove it in the file system as well?", removed));
        if(ret==KMessageBox::Yes)
        {
            bool correct=KIO::NetAccess::del(KUrl(removed), this);
            if(!correct)
                KMessageBox::error(this, i18n("Could not remove: %1.\n", removed));
        }
    }
    emit changed(true);
}

void CMakePreferences::configure()
{
    KDevelop::ProjectFolderItem* it = m_project->projectItem();
    KDevelop::IProjectBuilder *b = m_project->buildSystemManager()->builder(it);
    KJob* job=b->configure(m_project);
    m_configureJobFolder = CMake::currentBuildDir(m_project);
    KDevelop::ICore::self()->runController()->registerJob(job);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(configureFinished(KJob*)));
}

void CMakePreferences::configureFinished(KJob*)
{
    if (KUrl(m_prefsUi->buildDirs->currentText()) == m_configureJobFolder)
        updateCache(m_configureJobFolder);
}

void CMakePreferences::showAdvanced(bool v)
{
    m_prefsUi->advancedBox->setHidden(!v);
}


#include "cmakepreferences.moc"
