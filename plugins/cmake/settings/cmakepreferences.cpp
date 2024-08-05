/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2007-2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakepreferences.h"

#include <interfaces/icore.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageBox_KDevCompat>
#include <KJobWidgets>
#include <KIO/DeleteJob>

#include <QFile>
#include <QDir>
#include <QHeaderView>
#include <QComboBox>

#include "ui_cmakebuildsettings.h"
#include "cmakecachedelegate.h"
#include "cmakebuilddirchooser.h"
#include "cmakebuilderconfig.h"
#include <debug.h>
#include <cmakeutils.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <interfaces/iruncontroller.h>

using namespace KDevelop;

CMakePreferences::CMakePreferences(IPlugin* plugin, const ProjectConfigOptions& options, QWidget* parent)
    : ConfigPage(plugin, nullptr, parent), m_project(options.project), m_currentModel(nullptr)
{
    m_prefsUi = new Ui::CMakeBuildSettings;
    m_prefsUi->setupUi(this);

    m_prefsUi->cacheList->setItemDelegate(new CMakeCacheDelegate(m_prefsUi->cacheList));
    m_prefsUi->cacheList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_prefsUi->cacheList->horizontalHeader()->setStretchLastSection(true);
    m_prefsUi->cacheList->verticalHeader()->hide();

    // configure the extraArguments widget to span the advanced box width but not
    // expand the dialog to the width of the longest element in the argument history.
    m_prefsUi->extraArguments->setMinimumWidth(m_prefsUi->extraArguments->minimumSizeHint().width());
    m_extraArgumentsHistory = new CMakeExtraArgumentsHistory(m_prefsUi->extraArguments);

    connect(m_prefsUi->buildDirs, QOverload<int>::of(&KComboBox::currentIndexChanged),
            this, &CMakePreferences::buildDirChanged);
    connect(m_prefsUi->showInternal, &QCheckBox::stateChanged,
            this, &CMakePreferences::showInternal);
    connect(m_prefsUi->addBuildDir, &QPushButton::pressed, this, &CMakePreferences::createBuildDir);
    connect(m_prefsUi->removeBuildDir, &QPushButton::pressed, this, &CMakePreferences::removeBuildDir);
    connect(m_prefsUi->showAdvanced, &QPushButton::toggled, this, &CMakePreferences::showAdvanced);
    connect(m_prefsUi->environment, &EnvironmentSelectionWidget::currentProfileChanged,
            this, &CMakePreferences::changed);
    connect(m_prefsUi->configureEnvironment, &EnvironmentConfigureButton::environmentConfigured,
            this, &CMakePreferences::changed);

    connect(m_prefsUi->installationPrefix, &KUrlRequester::textChanged,
            this, &CMakePreferences::changed);
    connect(m_prefsUi->buildType, &QComboBox::currentTextChanged,
            this, &CMakePreferences::changed);
    connect(m_prefsUi->extraArguments, &KComboBox::currentTextChanged,
            this, &CMakePreferences::changed);
    connect(m_prefsUi->extraArguments, &KComboBox::editTextChanged,
            this, &CMakePreferences::changed);
    connect(m_prefsUi->cMakeExecutable, &KUrlRequester::textChanged,
            this, &CMakePreferences::changed);

    showInternal(m_prefsUi->showInternal->checkState());
    m_subprojFolder = Path(options.projectTempFile).parent();

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
    delete m_extraArgumentsHistory;
    delete m_prefsUi;
}

void CMakePreferences::initAdvanced()
{
    m_prefsUi->environment->setCurrentProfile( CMake::currentEnvironment(m_project) );
    m_prefsUi->installationPrefix->setText(CMake::currentInstallDir(m_project).toLocalFile());
    m_prefsUi->installationPrefix->setMode(KFile::Directory);
    setBuildType(CMake::currentBuildType(m_project));
    m_prefsUi->extraArguments->setEditText(CMake::currentExtraArguments(m_project));
    m_prefsUi->cMakeExecutable->setText(CMake::currentCMakeExecutable(m_project).toLocalFile());
}

void CMakePreferences::setBuildType(const QString& buildType)
{
    if (m_prefsUi->buildType->currentText() == buildType)
        return;

    if (m_prefsUi->buildType->findText(buildType) == -1) {
        m_prefsUi->buildType->addItem(buildType);
    }
    m_prefsUi->buildType->setCurrentIndex(m_prefsUi->buildType->findText(buildType));
}

void CMakePreferences::reset()
{
    qCDebug(CMAKE) << "********loading";
    m_prefsUi->buildDirs->clear();
    m_prefsUi->buildDirs->addItems( CMake::allBuildDirs(m_project) );
    CMake::removeOverrideBuildDirIndex(m_project); // addItems() triggers buildDirChanged(), compensate for it
    m_prefsUi->buildDirs->setCurrentIndex( CMake::currentBuildDirIndex(m_project) );

    initAdvanced();

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

    CMake::setCurrentInstallDir( m_project, Path(m_prefsUi->installationPrefix->text()) );
    const QString buildType = m_prefsUi->buildType->currentText();
    CMake::setCurrentBuildType(m_project, buildType);
    CMake::setCurrentExtraArguments( m_project, m_prefsUi->extraArguments->currentText() );
    CMake::setCurrentCMakeExecutable( m_project, Path(m_prefsUi->cMakeExecutable->text()) );

    qCDebug(CMAKE) << "writing to cmake config: using builddir " << CMake::currentBuildDirIndex(m_project);
    qCDebug(CMAKE) << "writing to cmake config: builddir path " << CMake::currentBuildDir(m_project);
    qCDebug(CMAKE) << "writing to cmake config: installdir " << CMake::currentInstallDir(m_project);
    qCDebug(CMAKE) << "writing to cmake config: build type " << CMake::currentBuildType(m_project);
    qCDebug(CMAKE) << "writing to cmake config: cmake executable " << CMake::currentCMakeExecutable(m_project);
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
    m_prefsUi->cacheList->hideColumn(5);
    m_prefsUi->cacheList->horizontalHeader()->resizeSection(0, 200);

    if( m_currentModel ) {
        m_prefsUi->cacheList->setEnabled( true );
        const auto persistentIndices = m_currentModel->persistentIndices();
        for (const QModelIndex& idx : persistentIndices) {
            m_prefsUi->cacheList->openPersistentEditor(idx);
        }
    } else {
        m_prefsUi->cacheList->setEnabled( false );
    }

    showInternal(m_prefsUi->showInternal->checkState());
}

void CMakePreferences::updateCache(const Path &newBuildDir)
{
    const Path file = newBuildDir.isValid() ? Path(newBuildDir, QStringLiteral("CMakeCache.txt")) : Path();
    if(QFile::exists(file.toLocalFile()))
    {
        if (m_currentModel) {
            m_currentModel->deleteLater();
        }
        m_currentModel = new CMakeCacheModel(this, file);

        configureCacheView();
        connect(m_currentModel, &CMakeCacheModel::itemChanged,
                this, &CMakePreferences::cacheEdited);
        connect(m_currentModel, &CMakeCacheModel::modelReset,
                this, &CMakePreferences::configureCacheView);
        connect(m_prefsUi->cacheList->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &CMakePreferences::listSelectionChanged);
        connect(m_currentModel, &CMakeCacheModel::valueChanged, this,
                [this](const QString& name, const QString& value) {
                    if (name == QLatin1String("CMAKE_BUILD_TYPE")) {
                        setBuildType(value);
                    }
                });
        connect(m_prefsUi->buildType, &QComboBox::currentTextChanged, m_currentModel, [this](const QString& value) {
            if (!m_currentModel)
                return;
            const auto items = m_currentModel->findItems(QStringLiteral("CMAKE_BUILD_TYPE"));
            for (auto* item : items) {
                m_currentModel->setData(m_currentModel->index(item->row(), 2), value);
            }
        });
    }
    else
    {
        disconnect(m_prefsUi->cacheList->selectionModel(), &QItemSelectionModel::currentChanged, this, nullptr);
        if (m_currentModel) {
            m_currentModel->deleteLater();
            m_currentModel = nullptr;
        }
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
    QString comment=QStringLiteral("%1. %2")
            .arg(m_currentModel->itemFromIndex(idxType)->text(),
                 m_currentModel->itemFromIndex(idx)->text());
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
    const Path buildDir = CMake::currentBuildDir(m_project);
    initAdvanced();
    updateCache(buildDir);
    qCDebug(CMAKE) << "builddir Changed" << buildDir;
    emit changed();
}

void CMakePreferences::cacheUpdated()
{
    const Path buildDir = CMake::currentBuildDir(m_project);
    updateCache(buildDir);
    qCDebug(CMAKE) << "cache updated for" << buildDir;
}

void CMakePreferences::createBuildDir()
{
    CMakeBuildDirChooser bdCreator;
    bdCreator.setProject( m_project );

    // NOTE: (on removing the trailing slashes)
    // Generally, we have no clue about how shall a trailing slash look in the current system.
    // Moreover, the slash may be a part of the filename.
    // It may be '/' or '\', so maybe should we rely on CMake::allBuildDirs() for returning well-formed paths?
    QStringList used = CMake::allBuildDirs( m_project );
    bdCreator.setAlreadyUsed(used);
    bdCreator.setCMakeExecutable(Path(CMakeBuilderSettings::self()->cmakeExecutable().toLocalFile()));

    if(bdCreator.exec())
    {
        int addedBuildDirIndex = m_prefsUi->buildDirs->count();

        // Initialize the kconfig items with the values from the dialog, this ensures the settings
        // end up in the config file once the changes are saved
        qCDebug(CMAKE) << "adding to cmake config: new builddir index" << addedBuildDirIndex;
        qCDebug(CMAKE) << "adding to cmake config: builddir path " << bdCreator.buildFolder();
        qCDebug(CMAKE) << "adding to cmake config: installdir " << bdCreator.installPrefix();
        qCDebug(CMAKE) << "adding to cmake config: extra args" << bdCreator.extraArguments();
        qCDebug(CMAKE) << "adding to cmake config: build type " << bdCreator.buildType();
        qCDebug(CMAKE) << "adding to cmake config: cmake executable " << bdCreator.cmakeExecutable();
        qCDebug(CMAKE) << "adding to cmake config: environment empty";
        CMake::setOverrideBuildDirIndex( m_project, addedBuildDirIndex );
        CMake::setBuildDirCount( m_project, addedBuildDirIndex + 1 );
        CMake::setCurrentBuildDir( m_project, bdCreator.buildFolder() );
        CMake::setCurrentInstallDir( m_project, bdCreator.installPrefix() );
        CMake::setCurrentExtraArguments( m_project, bdCreator.extraArguments() );
        CMake::setCurrentBuildType( m_project, bdCreator.buildType() );
        CMake::setCurrentCMakeExecutable(m_project, bdCreator.cmakeExecutable());
        CMake::setCurrentEnvironment( m_project, QString() );

        QString newbuilddir = bdCreator.buildFolder().toLocalFile();
        m_prefsUi->buildDirs->addItem( newbuilddir );
        m_prefsUi->buildDirs->setCurrentIndex( addedBuildDirIndex );
        m_prefsUi->removeBuildDir->setEnabled( true );

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

    Path removedPath = CMake::currentBuildDir( m_project );
    QString removed = removedPath.toLocalFile();
    if(QDir(removed).exists())
    {
        int ret = KMessageBox::warningTwoActions(
            this,
            i18n("The %1 directory is about to be removed in KDevelop's list.\n"
                 "Do you want KDevelop to delete it in the file system as well?",
                 removed),
            {}, KStandardGuiItem::del(),
            KGuiItem(i18nc("@action:button", "Do Not Delete"), QStringLiteral("dialog-cancel")));
        if (ret == KMessageBox::PrimaryAction) {
            auto deleteJob = KIO::del(removedPath.toUrl());
            KJobWidgets::setWindow(deleteJob, this);
            if (!deleteJob->exec())
                KMessageBox::error(this, i18n("Could not remove: %1", removed));
        }
    }

    qCDebug(CMAKE) << "removing from cmake config: using builddir " << curr;
    qCDebug(CMAKE) << "removing from cmake config: builddir path " << removedPath;
    qCDebug(CMAKE) << "removing from cmake config: installdir " << CMake::currentInstallDir( m_project );
    qCDebug(CMAKE) << "removing from cmake config: extra args" << CMake::currentExtraArguments( m_project );
    qCDebug(CMAKE) << "removing from cmake config: buildtype " << CMake::currentBuildType( m_project );
    qCDebug(CMAKE) << "removing from cmake config: cmake executable " << CMake::currentCMakeExecutable(m_project);
    qCDebug(CMAKE) << "removing from cmake config: environment " << CMake::currentEnvironment( m_project );


    CMake::removeBuildDirConfig(m_project);
    m_prefsUi->buildDirs->removeItem( curr ); // this triggers buildDirChanged()
    if(m_prefsUi->buildDirs->count()==0)
        m_prefsUi->removeBuildDir->setEnabled(false);

    emit changed();
}

void CMakePreferences::configure()
{
    IProjectBuilder *b=m_project->buildSystemManager()->builder();
    KJob* job=b->configure(m_project);
    if( m_currentModel ) {
        QVariantMap map = m_currentModel->changedValues();
        job->setProperty("extraCMakeCacheValues", map);
        connect(job, &KJob::finished, m_currentModel, &CMakeCacheModel::reset);
    } else {
        connect(job, &KJob::finished, this, &CMakePreferences::cacheUpdated);
    }

    connect(job, &KJob::finished, m_project, &IProject::reloadModel);
    ICore::self()->runController()->registerJob(job);
}

void CMakePreferences::showAdvanced(bool v)
{
    qCDebug(CMAKE) << "toggle pressed: " << v;
    m_prefsUi->advancedBox->setHidden(!v);
}


QString CMakePreferences::name() const
{
    return i18nc("@title:tab", "CMake");
}

QString CMakePreferences::fullName() const
{
    return i18nc("@title:tab", "Configure CMake Settings");
}

QIcon CMakePreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("cmake"));
}

#include "moc_cmakepreferences.cpp"
