/* KDevelop Run Settings
*
* Copyright 2006  Matt Rogers <mattr@kde.org>
* Copyright 2007-2008  Hamish Rodda <rodda@kde.org>
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

#include "runpreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>
#include <KConfigDialogManager>
#include <KMessageBox>

#include "core.h"
#include "ilanguagecontroller.h"

#include "environmentgrouplist.h"

#include "runconfig.h"

#include "ui_runsettings.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(RunPreferencesFactory, registerPlugin<RunPreferences>();)
K_EXPORT_PLUGIN(RunPreferencesFactory("kcm_kdev_runsettings"))

RunPreferences::RunPreferences( QWidget *parent, const QVariantList &args )
    : ProjectKCModule<RunSettings>( RunPreferencesFactory::componentData(), parent, args )
    , m_currentRunTarget(0)
    , m_deletingCurrentRunTarget(false)
{
    QVBoxLayout * l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    preferencesDialog = new Ui::RunSettings;
    preferencesDialog->setupUi( w );

    EnvironmentGroupList env( RunSettings::self()->config() );
    preferencesDialog->kcfg_environment->addItems( env.groups() );

    preferencesDialog->kcfg_workingDirectory->setMode(KFile::Directory);

    l->addWidget( w );

    m_manager = addConfig( RunSettings::self(), w );

    KConfigGroup group(RunSettings::self()->config(), "Run Options");
    m_runTargets = group.readEntry("Run Targets", QStringList() << "Project Default");

    foreach (const QString& target, m_runTargets) {
        if (target != "Project Default")
            preferencesDialog->runTarget->addItem(target);
    }

    load();

    connect(preferencesDialog->runTarget, SIGNAL(currentIndexChanged(int)), SLOT(runConfigSelected(int)));
    connect(preferencesDialog->runTarget, SIGNAL(editTextChanged(QString)), SLOT(runConfigRenamed(QString)));
    connect(preferencesDialog->buttonNewTarget, SIGNAL(clicked(bool)), SLOT(newRunConfig()));
    connect(preferencesDialog->buttonDeleteTarget, SIGNAL(clicked(bool)), SLOT(deleteRunConfig()));

#warning TODO: switch to stacked widget and mulitple RunSettings-es.
}

RunPreferences::~RunPreferences( )
{
    delete preferencesDialog;
}

void RunPreferences::save()
{
    KCModule::save();

    m_runTargets.clear();
    for (int i = 0; i < preferencesDialog->runTarget->count(); ++i)
        m_runTargets << preferencesDialog->runTarget->itemText(i);

    KConfigGroup group(RunSettings::self()->config(), "Run Options");
    group.writeEntry("Run Targets", m_runTargets);
}

void RunPreferences::load()
{
    KCModule::load();
}

void RunPreferences::newRunConfig()
{
    preferencesDialog->runTarget->addItem(QString());
    preferencesDialog->runTarget->setCurrentIndex(preferencesDialog->runTarget->count() - 1);

    // Check that it was not cancelled
    if (preferencesDialog->runTarget->currentIndex() != preferencesDialog->runTarget->count() - 1) {
        // Cancelled ... hrm
        preferencesDialog->runTarget->removeItem(preferencesDialog->runTarget->count() - 1);
        return;
    }

    preferencesDialog->runTarget->setFocus(Qt::MouseFocusReason);

    defaults();
}

void RunPreferences::runConfigSelected(int index)
{
    if (!m_deletingCurrentRunTarget && m_manager->hasChanged()) {
        switch (KMessageBox::questionYesNoCancel(this, i18n("You have unsaved changes in the current run target.  Would you like to save them?"), i18n("Unsaved changes"), KStandardGuiItem::save(), KStandardGuiItem::discard())) {
            case KMessageBox::Yes:
                save();
                break;

            case KMessageBox::Cancel:
                blockSignals(true);
                preferencesDialog->runTarget->setCurrentIndex(m_currentRunTarget);
                blockSignals(false);
                return;

            case KMessageBox::No:
                // Nothing to do
                break;
        }
    }

    if (preferencesDialog->runTarget->currentIndex() == 0)
        RunSettings::self()->setCurrentGroup("No Group");
    else
        RunSettings::self()->setCurrentGroup(preferencesDialog->runTarget->currentText());

    load();

    preferencesDialog->runTarget->setEditable(index != 0);
    preferencesDialog->buttonDeleteTarget->setEnabled(index != 0);

    m_currentRunTarget = index;
    m_currentRunTargetName = preferencesDialog->runTarget->currentText();

    configNameValid(m_currentRunTargetName);
}

void RunPreferences::deleteRunConfig()
{
    m_deletingCurrentRunTarget = true;

    // Delete any saved settings under this name
    KConfigGroup group(RunSettings::self()->config(), m_currentRunTargetName);
    group.deleteGroup();

    int currentIndex = preferencesDialog->runTarget->currentIndex();
    preferencesDialog->runTarget->setCurrentIndex(0);
    preferencesDialog->runTarget->removeItem(currentIndex);

    m_deletingCurrentRunTarget = false;
}

void RunPreferences::runConfigRenamed(const QString & newName)
{
    if (preferencesDialog->runTarget->currentIndex() == 0)
        return;

    RunSettings::self()->setCurrentGroup(newName);

    configNameValid(newName);

    // Delete any saved settings under this name
    KConfigGroup group(RunSettings::self()->config(), m_currentRunTargetName);
    group.deleteGroup();

    // Maybe we need a delay timer in here, but that could get complex
    RunSettings::self()->setCurrentGroup(preferencesDialog->runTarget->currentText());
    // Read config from file (doesn't change widget values)
    RunSettings::self()->readConfig();
    // Check if config on disk differs from widgets, it should... thus we're changed :)
    m_manager->updateSettings();

    m_currentRunTargetName = newName;
}

bool RunPreferences::configNameValid(const QString & name)
{
    bool nameValid = true;

    if (name.isEmpty()) {
        nameValid = false;

    } else {
        for (int i = 0; i < preferencesDialog->runTarget->count(); ++i) {
            if (i == preferencesDialog->runTarget->currentIndex())
                continue;

            if (preferencesDialog->runTarget->itemText(i) == name) {
                nameValid = false;
                break;
            }
        }
    }

    if (!nameValid)
        preferencesDialog->labelProjectNameIndicator->setPixmap(KIcon("dialog-warning").pixmap(QSize(16,16)));
    else
        preferencesDialog->labelProjectNameIndicator->clear();

    return nameValid;
}

#include "runpreferences.moc"
