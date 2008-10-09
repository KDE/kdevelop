/* KDevelop Run Settings
*
* Copyright 2006  Matt Rogers <mattr@kde.org>
* Copyright 2007-2008  Hamish Rodda <rodda@kde.org>
* Copyright 2008  Aleix Pol <aleixpol@gmail.com>
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
#include <QQueue>

#include <kgenericfactory.h>
#include <KConfigDialogManager>
#include <KMessageBox>

#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/ilanguagecontroller.h>
#include <util/environmentgrouplist.h>
#include <shell/core.h>
#include <QDialogButtonBox>
#include <QStackedLayout>
#include <QComboBox>

#include "runconfig.h"
#include "asktargetname.h"
#include "ui_runsettings.h"
#include "ui_runconfig.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(RunPreferencesFactory, registerPlugin<RunPreferences>();)
K_EXPORT_PLUGIN(RunPreferencesFactory("kcm_kdev_runsettings"))

class TargetProperties : public QWidget
{
    public:
        TargetProperties(const QVariantList& args, const QString& targetName, RunPreferences* parent=0)
            : QWidget(parent), m_targetName(targetName)
        {
            preferencesDialog.setupUi(this);
            
            m_settings=new RunSettings(targetName, args.at(0).toString() );
            m_settings->setDeveloperTempFile( args.at(0).toString() );
            m_settings->setProjectTempFile( args.at(1).toString() );
            m_settings->setProjectFileUrl( args.at(2).toString() );
            m_settings->setDeveloperFileUrl( args.at(3).toString() );

            EnvironmentGroupList env( m_settings->config() );
            preferencesDialog.kcfg_environment->addItems( env.groups() );
            preferencesDialog.kcfg_workingDirectory->setMode(KFile::Directory);
        }
        
        QString m_targetName;
        Ui::RunSettings preferencesDialog;
        RunSettings* m_settings;
        KConfigDialogManager* m_manager;
};

RunPreferences::RunPreferences( QWidget *parent, const QVariantList &args )
    : KCModule( RunPreferencesFactory::componentData(), parent, args )
    , m_currentRunTarget(0)
    , m_deletingCurrentRunTarget(false)
    , m_args(args)
    , m_config(KSharedConfig::openConfig(m_args[0].toString(), KConfig::SimpleConfig))
{
    m_configUi = new Ui::RunConfig;
    m_configUi->setupUi(this);
    
    m_configUi->buttonDeleteTarget->setIcon(KIcon("list-remove"));
    m_configUi->buttonNewTarget->setIcon(KIcon("list-add"));
    
    stacked= new QStackedLayout(m_configUi->targetSpecific);
    
    m_projectFile=args.at(2).toString();

    connect(m_configUi->targetCombo, SIGNAL(activated(int)), stacked, SLOT(setCurrentIndex(int)));
    connect(m_configUi->buttonNewTarget, SIGNAL(clicked(bool)), SLOT(newRunConfig()));
    connect(m_configUi->buttonDeleteTarget, SIGNAL(clicked(bool)), SLOT(deleteRunConfig()));
}

RunPreferences::~RunPreferences( ) { delete m_configUi; }

void RunPreferences::save()
{
    KCModule::save();

    m_runTargets.clear();
    for (int i = 0; i < m_configUi->targetCombo->count(); ++i)
        m_runTargets << m_configUi->targetCombo->itemText(i);

    KConfigGroup group(m_config, "Run Options");
    group.writeEntry("Run Targets", m_runTargets);
}

void RunPreferences::load()
{
    KConfigGroup group(m_config, "Run Options");
    m_runTargets = group.readEntry("Run Targets", QStringList());

    foreach(const QString& target, m_runTargets)
    {
        addTarget(target);
    }
}

void RunPreferences::newRunConfig()
{
    AskTargetName ask(this);
    int i = ask.exec();
    if(i==QDialog::Accepted && m_configUi->targetCombo->findText(ask.name())>=0)
    {
        addTarget(ask.name());
    }
}

void RunPreferences::deleteRunConfig()
{
    m_deletingCurrentRunTarget = true;

    int currentIndex = m_configUi->targetCombo->currentIndex();
    removeTarget(currentIndex);
    m_deletingCurrentRunTarget = false;
}

void RunPreferences::addTarget(const QString& name)
{
    Q_ASSERT(!name.isEmpty());
    qDebug() << "addingggggg" << name;
    TargetProperties* target= new TargetProperties(m_args, name, this);
    stacked->addWidget(target);
    m_configUi->targetCombo->addItem(name);
    m_targetWidgets.append(target);
    target->m_manager = addConfig( target->m_settings, target);
    m_configUi->targetCombo->setFocus(Qt::MouseFocusReason);
}

void RunPreferences::removeTarget(int index)
{
    Q_ASSERT(index>=0 && index<m_configUi->targetCombo->count());
    
    TargetProperties* t=m_targetWidgets.takeAt(index);
    stacked->takeAt(index);
    m_configUi->targetCombo->setCurrentIndex(0);
    m_configUi->targetCombo->removeItem(index);
    
    KConfigGroup group(t->m_settings->config(), t->m_settings->currentGroup());
    group.deleteGroup();
}

#include "runpreferences.moc"
