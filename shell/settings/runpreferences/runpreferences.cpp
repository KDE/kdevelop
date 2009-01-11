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
#include <kaboutdata.h>
#include <KMessageBox>

#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iproject.h>
#include <util/environmentgrouplist.h>
#include <shell/core.h>
#include <QDialogButtonBox>
#include <QStackedLayout>
#include <QComboBox>
#include <QCompleter>
#include <KUrl>
#include <ksettings/dispatcher.h>

#include "runconfig.h"
#include "asktargetname.h"
#include "projectitemlineedit.h"
#include "ui_runsettings.h"
#include "ui_runconfig.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(RunPreferencesFactory, registerPlugin<RunPreferences>();)
K_EXPORT_PLUGIN(RunPreferencesFactory(KAboutData("kcm_kdev_runsettings", "kdevplatform", ki18n("Run Settings"), "0.1")))

TargetProperties::TargetProperties(const QVariantList& args, const QString& targetName, QWidget* parent)
    : QWidget(parent), args0(args[0].toString()), groupPrefix(targetName)
{
    preferencesDialog = new Ui::RunSettings;
    preferencesDialog->setupUi(this);
    
    m_settings=new RunSettings(targetName, args.at(0).toString() );
    m_settings->setDeveloperTempFile( args.at(0).toString() );
    m_settings->setProjectTempFile( args.at(1).toString() );
    m_settings->setProjectFileUrl( args.at(2).toString() );
    m_settings->setDeveloperFileUrl( args.at(3).toString() );

    EnvironmentGroupList env( m_settings->config() );
    preferencesDialog->kcfg_environment->addItems( env.groups() );
    preferencesDialog->kcfg_workingDirectory->setMode(KFile::Directory);
    preferencesDialog->addCompilationProjectItem->setIcon(KIcon("list-add"));
    preferencesDialog->addCompilationProjectItem->setText(QString());
    preferencesDialog->removeCompilationProjectItem->setIcon(KIcon("list-remove"));
    preferencesDialog->removeCompilationProjectItem->setText(QString());
    preferencesDialog->upItem->setIcon(KIcon("go-up"));
    preferencesDialog->upItem->setText(QString());
    preferencesDialog->downItem->setIcon(KIcon("go-down"));
    preferencesDialog->downItem->setText(QString());
    preferencesDialog->executableWidget->setVisible(false);
    
    QAbstractItemModel* model=ICore::self()->projectController()->projectModel();
    preferencesDialog->compilationProjectItem->setCompleter(new ProjectItemCompleter(model, this));
    preferencesDialog->kcfg_runItem->setCompleter(new ProjectItemCompleter(model, this));
    connect(preferencesDialog->compilationProjectItem, SIGNAL(correctnessChanged(bool)),
            preferencesDialog->addCompilationProjectItem, SLOT(setEnabled(bool)));
    
    connect(preferencesDialog->addCompilationProjectItem, SIGNAL(clicked()), this, SLOT(slotAddCompileTarget()));
    connect(preferencesDialog->removeCompilationProjectItem, SIGNAL(clicked()), this, SLOT(removeCompileTarget()));
    connect(preferencesDialog->upItem, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(preferencesDialog->downItem, SIGNAL(clicked()), this, SLOT(downClicked()));
}

TargetProperties::~TargetProperties()
{
    delete preferencesDialog;
}

void TargetProperties::upClicked()
{
    int curr=preferencesDialog->compileItems->currentRow();
    QListWidgetItem* it=preferencesDialog->compileItems->takeItem(curr);
    preferencesDialog->compileItems->insertItem(curr+1, it);
    emit changed(true);
}

void TargetProperties::downClicked()
{
    int curr=preferencesDialog->compileItems->currentRow();
    QListWidgetItem* it=preferencesDialog->compileItems->takeItem(curr);
    preferencesDialog->compileItems->insertItem(curr-1, it);
    emit changed(true);
}

void TargetProperties::save() const
{
    QStringList itemsFound;
    for(int i=0; i<preferencesDialog->compileItems->count(); i++)
    {
        QListWidgetItem* it=preferencesDialog->compileItems->item(i);
        itemsFound += it->text();
    }
    
    KSharedConfig::Ptr config(KSharedConfig::openConfig(args0, KConfig::SimpleConfig));
    KConfigGroup group(config, groupPrefix+QLatin1String( "-Run Options" ));
    if(itemsFound.isEmpty())
        group.deleteEntry("Compile Items");
    else
        group.writeEntry("Compile Items", itemsFound);
    group.sync();
}

void TargetProperties::load()
{
    KSharedConfig::Ptr config(KSharedConfig::openConfig(args0, KConfig::SimpleConfig));
    KConfigGroup group(config, groupPrefix+QLatin1String( "-Run Options" ));
    QStringList ci=group.readEntry("Compile Items", QStringList());
    
    foreach(const QString& item, ci)
    {
        addCompileTarget(item);
    }
}

void TargetProperties::slotAddCompileTarget()
{
    addCompileTarget(preferencesDialog->compilationProjectItem->text());
}

void TargetProperties::addCompileTarget(const QString& name)
{
    QList<QListWidgetItem*> its=preferencesDialog->compileItems->findItems(name, Qt::MatchExactly);
    QListWidgetItem* it;
    
    if(its.isEmpty()) {
        it=new QListWidgetItem(name);
        preferencesDialog->compileItems->addItem(it);
        preferencesDialog->removeCompilationProjectItem->setEnabled(true);
        
        emit changed(true);
    } else
        it=its.first();
    it->setSelected(true);
    preferencesDialog->upItem->setEnabled(true);
    preferencesDialog->downItem->setEnabled(true);
}

void TargetProperties::removeCompileTarget()
{
    kDebug() << "removiiiiiiiing" << preferencesDialog->compileItems->currentRow();
    int curr=preferencesDialog->compileItems->currentRow();
    if(curr>=0)
    {
        delete preferencesDialog->compileItems->takeItem(curr);
        emit changed(true);
    }
    
    if(preferencesDialog->compileItems->count()==0)
    {
        preferencesDialog->removeCompilationProjectItem->setEnabled(false);
        preferencesDialog->upItem->setEnabled(false);
        preferencesDialog->downItem->setEnabled(false);
    }
}

RunPreferences::RunPreferences( QWidget *parent, const QVariantList &args )
    : KCModule( RunPreferencesFactory::componentData(), parent, args )
    , m_args(args)
    , m_config(KSharedConfig::openConfig(m_args[0].toString(), KConfig::SimpleConfig))
{
    m_configUi = new Ui::RunConfig;
    m_configUi->setupUi(this);
    
    m_configUi->buttonDeleteTarget->setIcon(KIcon("list-remove"));
    m_configUi->buttonNewTarget->setIcon(KIcon("list-add"));
    
    stacked= new QStackedLayout(m_configUi->targetSpecific);
    
    connect(m_configUi->targetCombo, SIGNAL(activated(int)), stacked, SLOT(setCurrentIndex(int)));
    connect(m_configUi->buttonNewTarget, SIGNAL(clicked(bool)), SLOT(newRunConfig()));
    connect(m_configUi->buttonDeleteTarget, SIGNAL(clicked(bool)), SLOT(deleteRunConfig()));
    
    KSettings::Dispatcher::registerComponent(RunPreferencesFactory::componentData(),
                                             ICore::self()->runController(), SLOT(slotConfigurationChanged()));
//     emit changed();
}

RunPreferences::~RunPreferences( ) { delete m_configUi; }

void RunPreferences::save()
{
    KCModule::save();

    QStringList runTargets;
    for (int i = 0; i < m_configUi->targetCombo->count(); ++i)
        runTargets << m_configUi->targetCombo->itemText(i);

    KConfigGroup group(m_config, "Run Options");
    group.writeEntry("Run Targets", runTargets);
    
    foreach(const QString& groupName, commitDeleteGroups)
    {
        KConfigGroup delGroup(m_config, groupName);
        delGroup.deleteGroup();
    }
    commitDeleteGroups.clear();
    
    foreach(TargetProperties* p, m_targetWidgets)
    {
        p->save();
    }
}

void RunPreferences::load()
{
    KConfigGroup group(m_config, "Run Options");
    QStringList runTargets = group.readEntry("Run Targets", QStringList());

    foreach(const QString& target, runTargets)
    {
        addTarget(target);
    }
    commitDeleteGroups.clear();
    
    foreach(TargetProperties* p, m_targetWidgets)
    {
        p->load();
    }
}

void RunPreferences::newRunConfig()
{
    AskTargetName ask(this);
    int i = ask.exec();
    if(i==QDialog::Accepted && m_configUi->targetCombo->findText(ask.name())<0)
    {
        addTarget(ask.name());
    }
}

void RunPreferences::deleteRunConfig()
{
    int currentIndex = m_configUi->targetCombo->currentIndex();
    if(currentIndex>=0)
        removeTarget(currentIndex);
}

void RunPreferences::addTarget(const QString& name)
{
    Q_ASSERT(!name.isEmpty());
    kDebug() << "adding target" << name;
    TargetProperties* target= new TargetProperties(m_args, name, this);
    stacked->addWidget(target);
    m_configUi->targetCombo->addItem(name);
    m_targetWidgets.append(target);
    target->m_manager = addConfig( target->m_settings, target);
    m_configUi->targetCombo->setFocus(Qt::MouseFocusReason);
    m_configUi->buttonDeleteTarget->setEnabled(true);
    if(!target->preferencesDialog->kcfg_executable->url().isEmpty())
    {
        target->preferencesDialog->execRadio->setChecked(true);
    }
    
    stacked->setCurrentIndex(m_configUi->targetCombo->count()-1);
    m_configUi->targetCombo->setCurrentIndex(m_configUi->targetCombo->count()-1);
    commitDeleteGroups.remove(name);
    Q_ASSERT(m_targetWidgets.count()==m_configUi->targetCombo->count() && m_targetWidgets.count()==stacked->count());
    Q_ASSERT(m_configUi->targetCombo->currentIndex()==stacked->currentIndex());
    
    connect(target, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
    
    emit changed(true);
}

void RunPreferences::removeTarget(int index)
{
    Q_ASSERT(index>=0 && index<m_configUi->targetCombo->count());
    
    kDebug() << "removing target" << index << m_configUi->targetCombo->currentText();
    TargetProperties* t=m_targetWidgets.takeAt(index);
    stacked->setCurrentIndex(0);
    stacked->takeAt(index);
    m_configUi->targetCombo->setCurrentIndex(0);
    m_configUi->targetCombo->removeItem(index);
    
    commitDeleteGroups += t->m_settings->currentGroup();
    
    if(m_configUi->targetCombo->count()==0)
        m_configUi->buttonDeleteTarget->setEnabled(false);
//     delete t;
    Q_ASSERT(m_targetWidgets.count()==m_configUi->targetCombo->count() && m_targetWidgets.count()==stacked->count());
    Q_ASSERT(m_configUi->targetCombo->currentIndex()==stacked->currentIndex());
    
    emit changed(true);
}

#include "runpreferences.moc"
