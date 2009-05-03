/* This file is part of KDevelop
   Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
   Copyright 2009 Andreas Pakulat <apaku@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "valgrindconfig.h"

#include <kicon.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kparts/mainwindow.h>

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <util/executecompositejob.h>

#include <execute/executepluginconstants.h>

#include "valgrindjob.h"
#include "ui_valgrindconfig.h"

//TODO: Share the dependency code somehow via ExecutePluginConstants
//TODO: Extract separate launch modes for memcheck vs. callgrind vs. hellgrind etc
KJob* ValgrindLauncher::start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg)
{
    Q_ASSERT(cfg);
    if( !cfg )
        return 0;
    if( launchMode == "profile" )
    {
        QStringList deps = cfg->config().readEntry( ExecutePlugin::dependencyEntry, QStringList() );
        QString depAction = cfg->config().readEntry( ExecutePlugin::dependencyActionEntry, "Nothing" );
        if( depAction != "Nothing" && !deps.isEmpty() ) 
        {
            QList<KJob*> l;
            KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
            foreach( const QString& dep, deps )
            {
                KDevelop::ProjectBaseItem* item = model->item( model->pathToIndex( dep.split('/') ) );
                if( item )
                {
                    KDevelop::ProjectBaseItem* folder = item;
                    while( folder && !folder->folder() )
                    {
                        folder = dynamic_cast<KDevelop::ProjectBaseItem*>( folder->parent() );
                    }
                    if( folder && item->project()->buildSystemManager()->builder( folder->folder() ) )
                    {
                        KDevelop::IProjectBuilder* builder = item->project()->buildSystemManager()->builder( folder->folder() );
                        if( depAction == "Build" )
                        {
                            l << builder->build( item );
                        } else if( depAction == "Install" )
                        {
                            l << builder->install( item );
                        } else if( depAction == "SudoInstall" )
                        {
                            KMessageBox::information( KDevelop::ICore::self()->uiController()->activeMainWindow(), 
                                                      i18n("Installing via sudo is not yet implemented"), 
                                                      i18n("Not implemented") );
                        }
                        
                    }
                }
            }
            l << new ValgrindJob( cfg, KDevelop::ICore::self()->runController() );
            return new KDevelop::ExecuteCompositeJob( KDevelop::ICore::self()->runController(), l );
        }else
        {
            return new ValgrindJob( cfg, KDevelop::ICore::self()->runController() );
        }
    }
    kWarning() << "Unknown launch mode " << launchMode << "for config:" << cfg->name();
    return 0;
}

ValgrindLauncher::ValgrindLauncher()
{
    factories << new ValgrindConfigPageFactory();
}


QStringList ValgrindLauncher::supportedModes() const
{
    return QStringList() << "profile";
}

QList< KDevelop::LaunchConfigurationPageFactory* > ValgrindLauncher::configPages() const
{
    return factories;
}

QString ValgrindLauncher::description() const
{
    return i18n( "Profile application with valgrind" );
}

QString ValgrindLauncher::id()
{
    return "valgrind";
}

QString ValgrindLauncher::name() const
{
    return i18n("Valgrind");
}


ValgrindConfigPage::ValgrindConfigPage(QWidget* parent) 
    : LaunchConfigurationPage(parent)
{
    ui = new Ui::ValgrindConfig();
    ui->setupUi( this );
    
    connect( ui->valgrindExecutable, SIGNAL(textChanged(QString)), SIGNAL(changed()) );
    connect( ui->valgrindExecutable, SIGNAL(urlSelected(KUrl)), SIGNAL(changed()) );
    connect( ui->valgrindParameters, SIGNAL(textEdited(KUrl)), SIGNAL(changed()) );
    connect( ui->freeBlockList, SIGNAL(valueChanged(int)), SIGNAL(changed()) );
    connect( ui->happensBefore, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );
    connect( ui->leakResolution, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );
    connect( ui->leakSummary, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );
    connect( ui->limitErrors, SIGNAL(toggled(bool)), SIGNAL(changed()) );
    connect( ui->maxStackFrame, SIGNAL(valueChanged(int)), SIGNAL(changed()) );
    connect( ui->numCallers, SIGNAL(valueChanged(int)), SIGNAL(changed()) );
    connect( ui->separateThreads, SIGNAL(toggled(bool)), SIGNAL(changed()) );
    connect( ui->showReachable, SIGNAL(toggled(bool)), SIGNAL(changed()) );
    connect( ui->simulateCache, SIGNAL(toggled(bool)), SIGNAL(changed()) );
    connect( ui->simulateHWPref, SIGNAL(toggled(bool)), SIGNAL(changed()) );
}

KIcon ValgrindConfigPage::icon() const
{
    return KIcon("fork");
}

void ValgrindConfigPage::loadFromConfiguration(const KConfigGroup& cfg)
{
    ui->valgrindExecutable->setUrl( cfg.readEntry( "Valgrind Executable", KUrl( "/usr/bin/valgrind" ) ) );
    ui->valgrindParameters->setText( cfg.readEntry( "Valgrind Arguments", "" ) );
    ui->numCallers->setValue( cfg.readEntry( "Framestack Depth", 12 ) );
    ui->maxStackFrame->setValue( cfg.readEntry( "Maximum Framestack Size", 2000000 ) );
    ui->limitErrors->setChecked( cfg.readEntry( "Limit Errors", false ) );
    ui->leakSummary->setCurrentIndex( cfg.readEntry( "Leak Summary", 1 ) );
    ui->leakResolution->setCurrentIndex( cfg.readEntry( "Leak Resolution Matching", 0 ) );
    ui->showReachable->setChecked( cfg.readEntry("Show Reachable Blocks", false ) );
    ui->freeBlockList->setValue( cfg.readEntry( "Free Block List Size", 5000000 ) );
    ui->separateThreads->setChecked( cfg.readEntry( "Separate Thread Reporting", false ) );
    ui->simulateCache->setChecked( cfg.readEntry( "Full Cache Simulation", false ) );
    ui->simulateHWPref->setChecked( cfg.readEntry( "Simulate Hardware Prefetcher", false ) );
    ui->happensBefore->setCurrentIndex( cfg.readEntry("Extra Synchronization Events", 0 ) );
}

void ValgrindConfigPage::saveToConfiguration( KConfigGroup cfg ) const
{
    
    cfg.writeEntry( "Valgrind Executable", ui->valgrindExecutable->url() );
    cfg.writeEntry( "Valgrind Arguments", ui->valgrindParameters->text() );
    cfg.writeEntry( "Framestack Depth", ui->numCallers->value() );
    cfg.writeEntry( "Maximum Framestack Size", ui->maxStackFrame->value() );
    cfg.writeEntry( "Limit Errors", ui->limitErrors->isChecked() );
    cfg.writeEntry( "Leak Summary", ui->leakSummary->currentIndex() );
    cfg.writeEntry( "Leak Resolution Matching", ui->leakResolution->currentIndex() );
    cfg.writeEntry( "Show Reachable Blocks", ui->showReachable->isChecked() );
    cfg.writeEntry( "Free Block List Size", ui->freeBlockList->value() );
    cfg.writeEntry( "Separate Thread Reporting", ui->separateThreads->isChecked() );
    cfg.writeEntry( "Full Cache Simulation", ui->simulateCache->isChecked() );
    cfg.writeEntry( "Simulate Hardware Prefetcher", ui->simulateHWPref->isChecked() );
    cfg.writeEntry( "Extra Synchronization Events", ui->happensBefore->currentIndex() );
}

QString ValgrindConfigPage::title() const
{
    return i18n("Valgrind");
}

KDevelop::LaunchConfigurationPage* ValgrindConfigPageFactory::createWidget(QWidget* parent)
{
    return new ValgrindConfigPage( parent );
}

ValgrindConfigPageFactory::ValgrindConfigPageFactory()
{

}


#include "valgrindconfig.moc"
