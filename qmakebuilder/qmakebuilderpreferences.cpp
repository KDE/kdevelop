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
    connect(m_chooserUi->kcfg_qmakeBin, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_chooserUi->kcfg_buildDir, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_chooserUi->kcfg_installPrefix, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_chooserUi->kcfg_buildType, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()));
    connect(m_chooserUi->kcfg_extraArgs, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    l->addWidget( w );
    addConfig( QMakeBuilderSettings::self(), w );

    connect(m_prefsUi->buildDirCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(loadOtherConfig(QString)));
    connect(m_prefsUi->buildDirCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(changed()));
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
    // refresh combobox
    KConfigGroup cg(project()->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    const QString buildPath = cg.readEntry(QMakeConfig::BUILD_FOLDER, QString());

    // update build list (this will trigger loadOtherConfig if signals are still connected)
    disconnect(m_prefsUi->buildDirCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(loadOtherConfig(QString)));
    m_prefsUi->buildDirCombo->clear();
    m_prefsUi->buildDirCombo->insertItems(0, cg.groupList());
    if (m_prefsUi->buildDirCombo->contains(buildPath)) {
        m_prefsUi->buildDirCombo->setCurrentItem(buildPath);
        m_chooserUi->loadConfig(buildPath);
    }
    kDebug() << "Loaded" << cg.groupList() << buildPath;
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
        // data is valid: save, once in the build dir's data and also as current data
        m_chooserUi->saveConfig();
        KConfigGroup config(project()->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
        m_chooserUi->saveConfig(config);
        config.writeEntry(QMakeConfig::BUILD_FOLDER, m_chooserUi->buildDir());
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
    if (!verifyChanges()) {
        return;
    }
    m_chooserUi->loadConfig(config);
    save();  // since current config has changed, it must be saved immediateley
}

bool QMakeBuilderPreferences::verifyChanges()
{
    // changes must be saved before switch
    if (managedWidgetChangeState()) {
        int ret = KMessageBox::questionYesNoCancel(this, i18n("Current changes will be lost. Would you want to save them?"));
        if (ret == KMessageBox::Yes) {
            save();
        } else if (ret == KMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void QMakeBuilderPreferences::addBuildConfig()
{
    if (!verifyChanges()) {
        return;
    }
    kDebug() << "Adding a new config.";
    // for more simpicity, just launch regular dialog
    QMakeBuildDirChooserDialog *dlg = new QMakeBuildDirChooserDialog(project());
    if(dlg->exec() == QDialog::Accepted) {
        m_prefsUi->buildDirCombo->setCurrentItem(dlg->buildDir(), true);
        m_prefsUi->removeButton->setEnabled(m_prefsUi->buildDirCombo->count() > 1);
        //TODO run qmake
    }
}

void QMakeBuilderPreferences::removeBuildConfig()
{
    kDebug() << "Removing config" << m_prefsUi->buildDirCombo->currentText();
    QString removed = m_prefsUi->buildDirCombo->currentText();
    KConfigGroup cg(project()->projectConfiguration(), QMakeConfig::CONFIG_GROUP);

    m_prefsUi->buildDirCombo->removeItem(m_prefsUi->buildDirCombo->currentIndex());
    m_prefsUi->removeButton->setEnabled(m_prefsUi->buildDirCombo->count() > 1);
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
