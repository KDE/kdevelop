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
#include "cmakeconfig.h"
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <interfaces/iruncontroller.h>
#include <KStandardDirs>

K_PLUGIN_FACTORY(CMakePreferencesFactory, registerPlugin<CMakePreferences>(); )
K_EXPORT_PLUGIN(CMakePreferencesFactory("kcm_kdevcmake_settings"))

CMakePreferences::CMakePreferences(QWidget* parent, const QVariantList& args)
    : ProjectKCModule<CMakeSettings>(CMakePreferencesFactory::componentData(), parent, args), m_currentModel(0)
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
    addConfig( CMakeSettings::self(), w );

    connect(m_prefsUi->buildDirs, SIGNAL(currentIndexChanged(const QString& )),
            this, SLOT(buildDirChanged( const QString & )));
    connect(m_prefsUi->showInternal, SIGNAL( stateChanged ( int ) ),
            this, SLOT(showInternal ( int )));
    connect(m_prefsUi->addBuildDir, SIGNAL(pressed()), this, SLOT(createBuildDir()));
    connect(m_prefsUi->removeBuildDir, SIGNAL(pressed()), this, SLOT(removeBuildDir()));
    connect(m_prefsUi->showAdvanced, SIGNAL(toggled(bool)), this, SLOT(showAdvanced(bool)));
    
    showInternal(m_prefsUi->showInternal->checkState());
    m_subprojFolder=KUrl(args[1].toString()).upUrl();
    
    kDebug(9042) << "Source folder: " << m_srcFolder << args[1].toString();
//     foreach(const QVariant &v, args)
//     {
//         kDebug(9042) << "arg: " << v.toString();
//     }

    m_prefsUi->showAdvanced->setChecked(false);
    showAdvanced(false);
    load();
}

CMakePreferences::~CMakePreferences()
{}

void CMakePreferences::load()
{
    ProjectKCModule<CMakeSettings>::load();
    CMakeSettings::self()->readConfig();

    kDebug(9042) << "********loading";
    m_prefsUi->buildDirs->clear();
    m_prefsUi->buildDirs->addItems(CMakeSettings::buildDirs());
    m_prefsUi->buildDirs->setCurrentIndex( m_prefsUi->buildDirs->findText( CMakeSettings::currentBuildDir().toLocalFile() ) );
    
    m_srcFolder=m_subprojFolder;
    m_srcFolder.cd(CMakeSettings::projectRootRelative());

    if(m_prefsUi->buildDirs->count()==0)
    {
        m_prefsUi->removeBuildDir->setEnabled(false);
    }
//     QString cmDir=group.readEntry("CMakeDirectory");
//     m_prefsUi->kcfg_cmakeDir->setUrl(KUrl(cmDir));
//     kDebug(9032) << "cmakedir" << cmDir;
}

void CMakePreferences::save()
{
    kDebug(9042) << "*******saving";
    QStringList bDirs;
    int count=m_prefsUi->buildDirs->model()->rowCount();
    for(int i=0; i<count; i++)
    {
        bDirs += m_prefsUi->buildDirs->itemText(i);
    }

    KConfigSkeletonItem* item = CMakeSettings::self()->findItem("buildDirs");
    item->setProperty( QVariant( bDirs ) );
    
    item = CMakeSettings::self()->findItem("currentBuildDir");
    item->setProperty( qVariantFromValue<KUrl>( KUrl( m_prefsUi->buildDirs->currentText() ) ) );
    
    KUrl cmakeCmd;
    KUrl installPrefix;
    QString buildType;
    
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
    
    item = CMakeSettings::self()->findItem("cmakeBin");
    item->setProperty(qVariantFromValue<KUrl>(cmakeCmd));
    
    item = CMakeSettings::self()->findItem("currentInstallDir");
    item->setProperty( qVariantFromValue<KUrl>(installPrefix));
    
    item = CMakeSettings::self()->findItem("currentBuildType");
    item->setProperty( qVariantFromValue<QString>(buildType));
    
    kDebug(9042) << "doing real save from ProjectKCModule";
    ProjectKCModule<CMakeSettings>::save();
    CMakeSettings::self()->writeConfig();
    
    //We run cmake on the builddir to generate it 
    if (needReconfiguring) {
        configure();
    }
}

void CMakePreferences::defaults()
{
    ProjectKCModule<CMakeSettings>::defaults();
//     kDebug(9032) << "*********defaults!";
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
    else
    {
        disconnect(m_prefsUi->cacheList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, 0);
        m_currentModel->deleteLater();
        m_currentModel=0;
        m_prefsUi->cacheList->setModel(0);
        m_prefsUi->cacheList->setEnabled(false);
        emit changed(true);
    }
}

void CMakePreferences::listSelectionChanged(const QModelIndex & index, const QModelIndex& )
{
    kDebug(9042) << "item " << index << " selected";
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
    kDebug(9042) << "builddir Changed" << str;
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
        kDebug(9042) << "Emitting changed signal for cmake kcm";
        emit changed(true);
    }
    //TODO: Save it for next runs
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
    //TODO: Why doesn't this use the project() member of the kcmodule? That should give the
    // project for which the dialog was opened.
    KDevelop::IProject* p=KDevelop::ICore::self()->projectController()->findProjectForUrl(m_subprojFolder);
    KDevelop::ProjectFolderItem* it=p->projectItem();
    KDevelop::IProjectBuilder *b=p->buildSystemManager()->builder(it);
    KJob* job=b->configure(p);
    
    KDevelop::ICore::self()->runController()->registerJob(job);
}

void CMakePreferences::showAdvanced(bool v)
{
    kDebug(9042) << "toggle pressed: " << v;
    m_prefsUi->advancedBox->setHidden(!v);
}


#include "cmakepreferences.moc"
