/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#include <kgenericfactory.h>

#include "ui_cmakebuildsettings.h"
#include "cmakeconfig.h"
#include <KUrl>

typedef KGenericFactory<CMakePreferences> CMakePreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdevcmake_settings, CMakePreferencesFactory( "kcm_kdevcmake_settings" )  )

CMakePreferences::CMakePreferences(QWidget* parent, const QStringList& args)
    : ProjectKCModule<CMakeSettings>(CMakePreferencesFactory::componentData(), parent, args)
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    m_prefsUi = new Ui::CMakeBuildSettings;
    m_prefsUi->setupUi( w );
    l->addWidget( w );

    addConfig( CMakeSettings::self(), w );

    load();
    connect(m_prefsUi->kcfg_buildFolder, SIGNAL(textChanged(const QString& )),
            this, SLOT(buildDirChanged(const QString&)));
    connect(m_prefsUi->cacheList, SIGNAL(clicked ( const QModelIndex & ) ),
            this, SLOT(listSelectionChanged ( const QModelIndex & )));
    connect(m_prefsUi->showInternal, SIGNAL( stateChanged ( int ) ),
            this, SLOT(showInternal ( int )));
}

CMakePreferences::~CMakePreferences()
{
}

void CMakePreferences::load()
{
    ProjectKCModule<CMakeSettings>::load();
    kDebug(9032) << "********loading";
    buildDirChanged(m_prefsUi->kcfg_buildFolder->text());
}

void CMakePreferences::save()
{
    ProjectKCModule<CMakeSettings>::save();
    kDebug(9032) << "*******saving";
}

void CMakePreferences::defaults()
{
    ProjectKCModule<CMakeSettings>::defaults();
    kDebug(9032) << "*********defaults!";
}

void CMakePreferences::buildDirChanged(const QString& newBuildDir)
{
    KUrl file(newBuildDir);
    file.addPath("CMakeCache.txt");
    if(QFile::exists(file.toLocalFile()))
    {
        m_prefsUi->cacheList->setEnabled(true);
        m_currentModel=new CMakeCacheModel(this, file);
        m_prefsUi->cacheList->setModel(m_currentModel);
        m_prefsUi->cacheList->hideColumn(3);
        m_prefsUi->cacheList->setEnabled(true);
    }
    else
    {
        kDebug(9032) << "did not exist " << file;
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
    for(int i=0; i<m_currentModel->rowCount(); i++)
    {
        QStandardItem* it = m_currentModel->item(i, 4);
        if(!it)
            continue;
        if(!it->text().isEmpty() && state==Qt::Unchecked)
            m_prefsUi->cacheList->setRowHidden(i, QModelIndex(), true);
        else
            m_prefsUi->cacheList->setRowHidden(i, QModelIndex(), false);
    }
}


#include "cmakepreferences.moc"

//kate: space-indent on; indent-width 4; replace-tabs on;
