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
#include "cmakeconfig.h"
#include <KUrl>

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

    load();
    connect(m_prefsUi->kcfg_buildFolder, SIGNAL(textChanged(const QString& )),
            this, SLOT(buildDirChanged( const QString & )));
    connect(m_prefsUi->cacheList, SIGNAL(clicked ( const QModelIndex & ) ),
            this, SLOT(listSelectionChanged ( const QModelIndex & )));
    connect(m_prefsUi->showInternal, SIGNAL( stateChanged ( int ) ),
            this, SLOT(showInternal ( int )));
    connect(m_currentModel, SIGNAL( itemChanged ( QStandardItem * ) ),
            this, SLOT( cacheEdited( QStandardItem * ) ));
}

CMakePreferences::~CMakePreferences()
{
}

void CMakePreferences::load()
{
    ProjectKCModule<CMakeSettings>::load();
//     kDebug(9032) << "********loading";
    updateCache(m_prefsUi->kcfg_buildFolder->url());
}

void CMakePreferences::save()
{
    ProjectKCModule<CMakeSettings>::save();
//     kDebug(9032) << "*******saving";
    m_currentModel->writeDown();
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
        m_prefsUi->cacheList->hideColumn(3);
        m_prefsUi->cacheList->hideColumn(4);
        m_prefsUi->cacheList->setEnabled(true);
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
    m_prefsUi->commentText->setText(m_currentModel->itemFromIndex(idx)->text());
}

void CMakePreferences::showInternal(int state)
{
    if(!m_currentModel)
        return;
    for(int i=m_currentModel->internal(); i<m_currentModel->rowCount(); i++)
    {
        if(state==Qt::Unchecked)
            m_prefsUi->cacheList->setRowHidden(i, QModelIndex(), true);
        else
            m_prefsUi->cacheList->setRowHidden(i, QModelIndex(), false);
    }
}

void CMakePreferences::buildDirChanged(const QString &)
{
    m_prefsUi->kcfg_buildFolder->url();
}

#include "cmakepreferences.moc"

//kate: space-indent on; indent-width 4; replace-tabs on;

