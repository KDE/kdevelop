/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
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

#include <kpluginfactory.h>
#include <kpluginloader.h>

#include "ui_cmakebuildsettings.h"
#include "cmakebuilddircreator.h"
#include "cmakeconfig.h"
#include <KUrl>
#include <QFile>

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

    addConfig( CMakeSettings::self(), w );

    connect(m_prefsUi->buildDirs, SIGNAL(currentIndexChanged(const QString& )),
            this, SLOT(buildDirChanged( const QString & )));
    connect(m_prefsUi->cacheList, SIGNAL(clicked ( const QModelIndex & ) ),
            this, SLOT(listSelectionChanged ( const QModelIndex & )));
    connect(m_prefsUi->showInternal, SIGNAL( stateChanged ( int ) ),
            this, SLOT(showInternal ( int )));
    connect(m_prefsUi->addBuildDir, SIGNAL(pressed()),
            this, SLOT(createBuildDir()));
    connect(m_prefsUi->showAdvanced, SIGNAL(toggled(bool)), this, SLOT(showAdvanced(bool)));
    
    showInternal(m_prefsUi->showInternal->checkState());
    //TODO: This will break when the kdevelop project file is not inside the source dir
    m_srcFolder=KUrl(args[0].toString());
    m_srcFolder=m_srcFolder.upUrl();

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
    kDebug(9032) << "********loading";

    m_prefsUi->buildDirs->addItems(CMakeSettings::buildDirs());
    m_prefsUi->buildDirs->setCurrentIndex( m_prefsUi->buildDirs->findText( CMakeSettings::currentBuildDir().toLocalFile() ) );

//     QString cmDir=group.readEntry("CMakeDirectory");
//     m_prefsUi->kcfg_cmakeDir->setUrl(KUrl(cmDir));
//     kDebug(9032) << "cmakedir" << cmDir;
}

void CMakePreferences::save()
{
    kDebug(9032) << "*******saving";
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
    
    item = CMakeSettings::self()->findItem("cmakeBin");
    item->setProperty( qVariantFromValue<KUrl>( KUrl( m_currentModel->value("CMAKE_COMMAND") ) ) );
    
    item = CMakeSettings::self()->findItem("currentInstallDir");
    item->setProperty( qVariantFromValue<KUrl>( KUrl( m_currentModel->value("CMAKE_INSTALL_PREFIX") ) ) );
    m_currentModel->writeDown();
    
    kDebug(9032) << "doing real save from ProjectKCModule";
    ProjectKCModule<CMakeSettings>::save();
    CMakeSettings::self()->writeConfig();
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
        m_prefsUi->cacheList->setEnabled(true);
        connect(m_currentModel, SIGNAL( itemChanged ( QStandardItem * ) ),
                this, SLOT( cacheEdited( QStandardItem * ) ));
        
        showInternal(m_prefsUi->showInternal->checkState());
    }
    else
    {
        kDebug(9032) << "did not exist " << file;
        if(m_currentModel)
            m_currentModel->clear();
        m_prefsUi->cacheList->setEnabled(false);
    }
}

void CMakePreferences::listSelectionChanged(const QModelIndex & index)
{
    kDebug(9032) << "item " << index << " selected";
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
    bool showAdv=(state==Qt::Unchecked);
    for(int i=0; i<m_currentModel->rowCount(); i++)
    {
        bool shown=m_currentModel->isInternal(i);
        if(showAdv && !shown)
            shown=m_currentModel->isAdvanced(i);
        m_prefsUi->cacheList->setRowHidden(i, QModelIndex(), shown);
    }
}

void CMakePreferences::buildDirChanged(const QString &str)
{
    updateCache(str);
    kDebug(9032) << "Emitting changed signal for cmake kcm";
    emit changed(true);
}

void CMakePreferences::createBuildDir()
{
    CMakeBuildDirCreator bdCreator(m_srcFolder.toLocalFile(), this);
    //TODO: if there is information, use it to initialize the dialog
    if(bdCreator.exec()) {
        m_prefsUi->buildDirs->addItem(bdCreator.buildFolder().toLocalFile());
        kDebug(9032) << "Emitting changed signal for cmake kcm";
        emit changed(true);
    }
    //TODO: Save it for next runs
}

void CMakePreferences::showAdvanced(bool v)
{
    kDebug(9032) << "toggle pressed: " << v;
    m_prefsUi->advancedBox->setHidden(!v);
}

#include "cmakepreferences.moc"


