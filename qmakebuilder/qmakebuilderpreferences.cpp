/* KDevelop QMake Support
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#include "qmakebuilderpreferences.h"

#include <QVBoxLayout>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kurlrequester.h>
#include <KMessageBox>
#include <KIO/NetAccess>

#include "ui_qmakeconfig.h"
#include "../qmakebuilddirchooser.h"
#include "../qmakebuilddirchooserdialog.h"
#include "../qmakeconfig.h"
#include "qmakebuilderconfig.h"

#include <interfaces/iproject.h>

K_PLUGIN_FACTORY(QMakeBuilderPreferencesFactory, registerPlugin<QMakeBuilderPreferences>(); )
K_EXPORT_PLUGIN(QMakeBuilderPreferencesFactory("kcm_kdev_qmakebuilder"))


QMakeBuilderPreferences::QMakeBuilderPreferences(QWidget* parent, const QVariantList& args)
    : ProjectKCModule<QMakeBuilderSettings>( QMakeBuilderPreferencesFactory::componentData(), parent, args)
{
//     Q_ASSERT( args.count() > 0 );
//     QMakeBuilderSettings::instance( args.first() );
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget;

    m_prefsUi = new Ui::QMakeConfig;
    m_prefsUi->setupUi( w );

    // display icons instead of text
    m_prefsUi->addButton->setIcon(KIcon( "list-add" ));
    m_prefsUi->addButton->setText(QString());
    m_prefsUi->removeButton->setIcon(KIcon( "list-remove" ));
    m_prefsUi->removeButton->setText(QString());

    m_chooserUi = new QMakeBuildDirChooser(m_prefsUi->groupBox, project());
    m_chooserUi->kcfg_buildDir->setEnabled(false);  // build directory MUST NOT be changed here
    connect(m_chooserUi->kcfg_qmakeBin, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    connect(m_chooserUi->kcfg_buildDir, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    connect(m_chooserUi->kcfg_installPrefix, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    l->addWidget( w );
    addConfig( QMakeBuilderSettings::self(), w );

    connect(m_prefsUi->buildDirCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(loadOtherConfig(QString)));
    connect(m_prefsUi->addButton, SIGNAL(pressed()), this, SLOT(addBuildConfig()));
    connect(m_prefsUi->removeButton, SIGNAL(pressed()), this, SLOT(removeBuildConfig()));
    connect(this, SIGNAL(changed(bool)), this, SLOT(validate()));

    // there is no Default values
    setButtons(buttons() & (~KCModule::Default));
}

QMakeBuilderPreferences::~QMakeBuilderPreferences()
{
    // not a QObject !
    delete m_chooserUi;
}

void QMakeBuilderPreferences::load()
{
    kDebug() << "loading data";
    KCModule::load();
    // refresh combobox
    KConfigGroup cg(project()->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    KUrl buildPath = cg.readEntry(QMakeConfig::BUILD_FOLDER, KUrl());

    // update build list (this will trigger loadOtherConfig if signals are still connected)
    disconnect(m_prefsUi->buildDirCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(loadOtherConfig(QString)));
    m_prefsUi->buildDirCombo->clear();
    m_prefsUi->buildDirCombo->insertItems(0, cg.groupList());
    m_prefsUi->buildDirCombo->setCurrentItem(buildPath.toLocalFile());
    kDebug() << "Loaded" << m_prefsUi->buildDirCombo->count() << (m_prefsUi->buildDirCombo->count() > 1);
    m_prefsUi->removeButton->setEnabled(m_prefsUi->buildDirCombo->count() > 1);
    connect(m_prefsUi->buildDirCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(loadOtherConfig(QString)));

    validate();
}


void QMakeBuilderPreferences::save()
{
    kDebug() << "Saving data";
    QString errormsg;

    if(m_chooserUi->isValid(&errormsg))
    {
        // data is valid: save
        KCModule::save();
        m_chooserUi->saveConfig();
    }
    else
    {
        // invalid data: message box
        KMessageBox::error(0, errormsg, "Data is invalid!");
        //FIXME dialog behaves like if save really happend (dialog closes if user click ok) even if changed signal is emitted
    }
}

void QMakeBuilderPreferences::validate()
{
    m_chooserUi->isValid();
}

void QMakeBuilderPreferences::loadOtherConfig(const QString& config)
{
    kDebug() << "Loding config "<<config;
    kDebug() << "Change state " << managedWidgetChangeState();
    // changes must be saved before switch
    if(managedWidgetChangeState())
    {
        int ret = KMessageBox::questionYesNoCancel(this,
                      i18n("Current changes will be lost. Would you want to save them?"));
        if(ret == KMessageBox::Yes)
        {
            save();
        }
        else if (ret == KMessageBox::Cancel)
        {
            return;
        }
    }
    m_chooserUi->loadConfig(config);
    save();  // since current config has changed, it must be saved immediateley
    load();  // cause the combobox to be recalculated
}


void QMakeBuilderPreferences::addBuildConfig()
{
    kDebug() << "Adding a new config.";
    // for more simpicity, just launch regular dialog
    QMakeBuildDirChooserDialog *dlg = new QMakeBuildDirChooserDialog(project());
    if(dlg->exec() == QDialog::Accepted) {
        loadOtherConfig(dlg->buildDir().toLocalFile());
        //TODO run qmake
    }
}

void QMakeBuilderPreferences::removeBuildConfig()
{
    kDebug() << "Removing config" << m_prefsUi->buildDirCombo->currentText();
    QString removed = m_prefsUi->buildDirCombo->currentText();
    KConfigGroup cg(project()->projectConfiguration(), QMakeConfig::CONFIG_GROUP);

    m_prefsUi->buildDirCombo->removeItem(m_prefsUi->buildDirCombo->currentIndex());
    cg.group(removed).deleteGroup(KConfigBase::Persistent);

    if(QDir(removed).exists())
    {
        int ret=KMessageBox::warningYesNo(this,
                i18n("The %1 directory is about to be removed in KDevelop's list.\n"
                    "Do you want KDevelop to remove it in the file system as well?", removed));
        if(ret==KMessageBox::Yes)
        {
            bool correct=KIO::NetAccess::del(KUrl(removed), this);
            if(!correct)
                KMessageBox::error(this, i18n("Could not remove: %1.", removed));
        }
    }
}
