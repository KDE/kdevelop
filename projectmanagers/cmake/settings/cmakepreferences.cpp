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

#include <KLocalizedString>
#include <KMessageBox>
#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <QFile>
#include <QDir>
#include <QHeaderView>

#include "ui_cmakebuildsettings.h"
#include "cmakecachedelegate.h"
#include "cmakebuilddirchooser.h"
#include "../debug.h"
#include <cmakeutils.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <interfaces/iruncontroller.h>
#include <KAboutData>
#include <util/environmentgrouplist.h>
#include <KLocalizedString>
#include <QStandardPaths>

CMakePreferences::CMakePreferences(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
    : KDevelop::ConfigPage(plugin, nullptr, parent), m_project(options.project), m_currentModel(0)
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    m_prefsUi = new Ui::CMakeBuildSettings;
    m_prefsUi->setupUi( w );
    l->addWidget( w );

    m_prefsUi->addBuildDir->setIcon(QIcon::fromTheme( "list-add" ));
    m_prefsUi->removeBuildDir->setIcon(QIcon::fromTheme( "list-remove" ));

    m_prefsUi->addBuildDir->setText(QString());
    m_prefsUi->removeBuildDir->setText(QString());
    m_prefsUi->cacheList->setItemDelegate(new CMakeCacheDelegate(m_prefsUi->cacheList));
    m_prefsUi->cacheList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_prefsUi->cacheList->horizontalHeader()->setStretchLastSection(true);
    m_prefsUi->cacheList->verticalHeader()->hide();

    connect(m_prefsUi->buildDirs, static_cast<void(KComboBox::*)(int)>(&KComboBox::currentIndexChanged),
            this, &CMakePreferences::buildDirChanged);
    connect(m_prefsUi->showInternal, &QCheckBox::stateChanged,
            this, &CMakePreferences::showInternal);
    connect(m_prefsUi->addBuildDir, &QPushButton::pressed, this, &CMakePreferences::createBuildDir);
    connect(m_prefsUi->removeBuildDir, &QPushButton::pressed, this, &CMakePreferences::removeBuildDir);
    connect(m_prefsUi->showAdvanced, &QPushButton::toggled, this, &CMakePreferences::showAdvanced);
    connect(m_prefsUi->environment, &KDevelop::EnvironmentSelectionWidget::currentProfileChanged, this, static_cast<void(CMakePreferences::*)()>(&CMakePreferences::changed));

    showInternal(m_prefsUi->showInternal->checkState());
    m_subprojFolder = KDevelop::Path(options.projectTempFile).parent();

    qCDebug(CMAKE) << "Source folder: " << m_srcFolder << options.projectTempFile;
//     foreach(const QVariant &v, args)
//     {
//         qCDebug(CMAKE) << "arg: " << v.toString();
//     }

    m_prefsUi->configureEnvironment->setSelectionWidget(m_prefsUi->environment);

    m_prefsUi->showAdvanced->setChecked(false);
    showAdvanced(false);
    reset(); // load the initial values
}

CMakePreferences::~CMakePreferences()
{
    CMake::removeOverrideBuildDirIndex(m_project);
    delete m_prefsUi;
}

void CMakePreferences::reset()
{
    qCDebug(CMAKE) << "********loading";
    m_prefsUi->buildDirs->clear();
    m_prefsUi->buildDirs->addItems( CMake::allBuildDirs(m_project) );
    CMake::removeOverrideBuildDirIndex(m_project); // addItems() triggers buildDirChanged(), compensate for it
    m_prefsUi->buildDirs->setCurrentIndex( CMake::currentBuildDirIndex(m_project) );
    m_prefsUi->environment->setCurrentProfile( CMake::currentEnvironment(m_project) );

    m_srcFolder = m_project->path();

    m_prefsUi->removeBuildDir->setEnabled(m_prefsUi->buildDirs->count()!=0);
//     QString cmDir=group.readEntry("CMakeDirectory");
//     m_prefsUi->kcfg_cmakeDir->setUrl(QUrl(cmDir));
//     qCDebug(CMAKE) << "cmakedir" << cmDir;
}

void CMakePreferences::apply()
{
    qCDebug(CMAKE) << "*******saving";

    // the build directory list is incrementally maintained through createBuildDir() and removeBuildDir().
    // We won't rewrite it here based on the data from m_prefsUi->buildDirs.
    CMake::removeOverrideBuildDirIndex( m_project, true ); // save current selection
    int savedBuildDir = CMake::currentBuildDirIndex(m_project);
    if (savedBuildDir < 0) {
        // no build directory exists: skip any writing to config file as well as configuring
        return;
    }

    CMake::setCurrentEnvironment( m_project, m_prefsUi->environment->currentProfile() );

    qCDebug(CMAKE) << "writing to cmake config: using builddir " << CMake::currentBuildDirIndex(m_project);
    qCDebug(CMAKE) << "writing to cmake config: builddir path " << CMake::currentBuildDir(m_project);
    qCDebug(CMAKE) << "writing to cmake config: installdir " << CMake::currentInstallDir(m_project);
    qCDebug(CMAKE) << "writing to cmake config: build type " << CMake::currentBuildType(m_project);
    qCDebug(CMAKE) << "writing to cmake config: cmake binary " << CMake::currentCMakeBinary(m_project);
    qCDebug(CMAKE) << "writing to cmake config: environment " << CMake::currentEnvironment(m_project);

    //We run cmake on the builddir to generate it
    configure();
}

void CMakePreferences::defaults()
{
    // do nothing
}

void CMakePreferences::configureCacheView()
{
    // Sets up the cache view after model re-creation/reset.
    // Emits changed(false) because model re-creation probably means
    // mass programmatical invocation of itemChanged(), which invokes changed(true) - which is not what we want.
    m_prefsUi->cacheList->setModel(m_currentModel);
    m_prefsUi->cacheList->hideColumn(1);
    m_prefsUi->cacheList->hideColumn(3);
    m_prefsUi->cacheList->hideColumn(4);
    m_prefsUi->cacheList->horizontalHeader()->resizeSection(0, 200);

    if( m_currentModel ) {
        m_prefsUi->cacheList->setEnabled( true );
        foreach(const QModelIndex & idx, m_currentModel->persistentIndices()) {
            m_prefsUi->cacheList->openPersistentEditor(idx);
        }
    } else {
        m_prefsUi->cacheList->setEnabled( false );
    }

    showInternal(m_prefsUi->showInternal->checkState());
}

void CMakePreferences::updateCache(const KDevelop::Path &newBuildDir)
{
    KDevelop::Path file(newBuildDir, "CMakeCache.txt");
    if(QFile::exists(file.toLocalFile()))
    {
        m_currentModel->deleteLater();
        m_currentModel=new CMakeCacheModel(this, file);
        configureCacheView();
        connect(m_currentModel, &CMakeCacheModel::itemChanged,
                this, &CMakePreferences::cacheEdited);
        connect(m_currentModel, &CMakeCacheModel::modelReset,
                this, &CMakePreferences::configureCacheView);
        connect(m_prefsUi->cacheList->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &CMakePreferences::listSelectionChanged);
    }
    else
    {
        disconnect(m_prefsUi->cacheList->selectionModel(), &QItemSelectionModel::currentChanged, this, 0);
        m_currentModel->deleteLater();
        m_currentModel=0;
        configureCacheView();
    }

    if( !m_currentModel )
        emit changed();
}

void CMakePreferences::listSelectionChanged(const QModelIndex & index, const QModelIndex& )
{
    qCDebug(CMAKE) << "item " << index << " selected";
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

void CMakePreferences::buildDirChanged(int index)
{
    CMake::setOverrideBuildDirIndex( m_project, index );
    const KDevelop::Path buildDir = CMake::currentBuildDir(m_project);
    m_prefsUi->environment->setCurrentProfile( CMake::currentEnvironment( m_project ) );
    updateCache(buildDir);
    qCDebug(CMAKE) << "builddir Changed" << buildDir;
    emit changed();
}

void CMakePreferences::cacheUpdated()
{
    const KDevelop::Path buildDir = CMake::currentBuildDir(m_project);
    updateCache(buildDir);
    qCDebug(CMAKE) << "cache updated for" << buildDir;
}

void CMakePreferences::createBuildDir()
{
    CMakeBuildDirChooser bdCreator;
    bdCreator.setSourceFolder( m_srcFolder );

    // NOTE: (on removing the trailing slashes)
    // Generally, we have no clue about how shall a trailing slash look in the current system.
    // Moreover, the slash may be a part of the filename.
    // It may be '/' or '\', so maybe should we rely on CMake::allBuildDirs() for returning well-formed pathes?
    QStringList used = CMake::allBuildDirs( m_project );
    bdCreator.setAlreadyUsed(used);
    bdCreator.setCMakeBinary(KDevelop::Path(QStandardPaths::findExecutable("cmake")));

    if(bdCreator.exec())
    {
        QString newbuilddir = bdCreator.buildFolder().toLocalFile();
        m_prefsUi->buildDirs->addItem(newbuilddir);

        int buildDirCount = m_prefsUi->buildDirs->count();
        int addedBuildDirIndex = buildDirCount - 1;
        m_prefsUi->buildDirs->setCurrentIndex(addedBuildDirIndex);
        m_prefsUi->removeBuildDir->setEnabled(true);

        // Initialize the kconfig items with the values from the dialog, this ensures the settings
        // end up in the config file once the changes are saved
        qCDebug(CMAKE) << "adding to cmake config: new builddir index" << addedBuildDirIndex;
        qCDebug(CMAKE) << "adding to cmake config: builddir path " << bdCreator.buildFolder();
        qCDebug(CMAKE) << "adding to cmake config: installdir " << bdCreator.installPrefix();
        qCDebug(CMAKE) << "adding to cmake config: extra args" << bdCreator.extraArguments();
        qCDebug(CMAKE) << "adding to cmake config: build type " << bdCreator.buildType();
        qCDebug(CMAKE) << "adding to cmake config: cmake binary " << bdCreator.cmakeBinary();
        qCDebug(CMAKE) << "adding to cmake config: environment empty";
        CMake::setBuildDirCount( m_project, buildDirCount );
        CMake::setCurrentBuildDir( m_project, bdCreator.buildFolder() );
        CMake::setCurrentInstallDir( m_project, bdCreator.installPrefix() );
        CMake::setCurrentExtraArguments( m_project, bdCreator.extraArguments() );
        CMake::setCurrentBuildType( m_project, bdCreator.buildType() );
        CMake::setCurrentCMakeBinary( m_project, bdCreator.cmakeBinary() );
        CMake::setCurrentEnvironment( m_project, QString() );

        qCDebug(CMAKE) << "Emitting changed signal for cmake kcm";
        emit changed();
    }
    //TODO: Save it for next runs
}

void CMakePreferences::removeBuildDir()
{
    int curr=m_prefsUi->buildDirs->currentIndex();
    if(curr < 0)
        return;

    KDevelop::Path removedPath = CMake::currentBuildDir( m_project );
    QString removed = removedPath.toLocalFile();
    if(QDir(removed).exists())
    {
        int ret=KMessageBox::warningYesNo(this,
                i18n("The %1 directory is about to be removed in KDevelop's list.\n"
                    "Do you want KDevelop to remove it in the file system as well?", removed));
        if(ret==KMessageBox::Yes)
        {
            bool correct=KIO::NetAccess::del(removedPath.toUrl(), this);
            if(!correct)
                KMessageBox::error(this, i18n("Could not remove: %1.\n", removed));
        }
    }

    qCDebug(CMAKE) << "removing from cmake config: using builddir " << curr;
    qCDebug(CMAKE) << "removing from cmake config: builddir path " << removedPath;
    qCDebug(CMAKE) << "removing from cmake config: installdir " << CMake::currentInstallDir( m_project );
    qCDebug(CMAKE) << "removing from cmake config: extra args" << CMake::currentExtraArguments( m_project );
    qCDebug(CMAKE) << "removing from cmake config: buildtype " << CMake::currentBuildType( m_project );
    qCDebug(CMAKE) << "removing from cmake config: cmake binary " << CMake::currentCMakeBinary( m_project );
    qCDebug(CMAKE) << "removing from cmake config: environment " << CMake::currentEnvironment( m_project );

    CMake::removeBuildDirConfig(m_project);
    m_prefsUi->buildDirs->removeItem( curr ); // this triggers buildDirChanged()
    if(m_prefsUi->buildDirs->count()==0)
        m_prefsUi->removeBuildDir->setEnabled(false);

    emit changed();
}

void CMakePreferences::configure()
{
    KDevelop::IProjectBuilder *b=m_project->buildSystemManager()->builder();
    KJob* job=b->configure(m_project);
    if( m_currentModel ) {
        QVariantMap map = m_currentModel->changedValues();
        job->setProperty("extraCMakeCacheValues", map);
        connect(job, &KJob::finished, m_currentModel, &CMakeCacheModel::reset);
    } else {
        connect(job, &KJob::finished, this, &CMakePreferences::cacheUpdated);
    }

    connect(job, &KJob::finished, m_project, &KDevelop::IProject::reloadModel);
    KDevelop::ICore::self()->runController()->registerJob(job);
}

void CMakePreferences::showAdvanced(bool v)
{
    qCDebug(CMAKE) << "toggle pressed: " << v;
    m_prefsUi->advancedBox->setHidden(!v);
}


QString CMakePreferences::name() const
{
    return i18n("CMake");
}

QString CMakePreferences::fullName() const
{
    return i18n("Configure CMake settings");
}

QIcon CMakePreferences::icon() const
{
    return QIcon::fromTheme("cmake");
}

#include "cmakepreferences.moc"
