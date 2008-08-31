/***************************************************************************
 *   Copyright 2001 Bernd Gehrmann <bernd@kdevelop.org>                    *
 *   Copyright 2004-2005 Sascha Cunz <sascha@kdevelop.org>                 *
 *   Copyright 2005 Ian Reinhart Geiser <ian@geiseri.com>                  *
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "appwizardplugin.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextStream>

#include <ktar.h>
#include <kzip.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <ktempdir.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kmacroexpander.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kactioncollection.h>
#include <kmimetype.h>
#include <kio/copyjob.h>
#include <kio/netaccess.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <vcs/vcsmapping.h>
#include <vcs/vcslocation.h>
#include <vcs/vcsjob.h>
#include <vcs/interfaces/icentralizedversioncontrol.h>
#include <vcs/interfaces/idistributedversioncontrol.h>

#include "appwizarddialog.h"
#include "projectselectionpage.h"
#include "projectvcspage.h"
#include "projecttemplatesmodel.h"
#include "importproject.h"

using KDevelop::IBasicVersionControl;
using KDevelop::ICentralizedVersionControl;
using KDevelop::IDistributedVersionControl;
using KDevelop::IPlugin;
using KDevelop::VcsJob;
using KDevelop::VcsLocation;
using KDevelop::VcsMapping;

K_PLUGIN_FACTORY(AppWizardFactory,
    registerPlugin<AppWizardPlugin>();
    KComponentData compData = componentData();
    KStandardDirs *dirs = compData.dirs();
    dirs->addResourceType("apptemplates", "data", "kdevappwizard/templates/");
    dirs->addResourceType("apptemplate_descriptions","data", "kdevappwizard/template_descriptions/");
    dirs->addResourceType("appimports", "data", "kdevappwizard/imports/");
    dirs->addResourceType("appimportfiles", "data", "kdevappwizard/importfiles/");
    setComponentData(compData);
)
K_EXPORT_PLUGIN(AppWizardFactory("kdevappwizard"))

AppWizardPlugin::AppWizardPlugin(QObject *parent, const QVariantList &)
    :KDevelop::IPlugin(AppWizardFactory::componentData(), parent)
{
    setXMLFile("kdevappwizard.rc");

    QAction *action = actionCollection()->addAction("project_new");
    action->setIcon(KIcon("window-new"));
    action->setText(i18n("&New Project..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotNewProject()));
    action->setToolTip( i18n("Generate a new project from a template") );
    action->setWhatsThis( i18n("<b>New project</b><p>"
                               "This starts KDevelop's application wizard. "
                               "It helps you to generate a skeleton for your "
                               "application from a set of templates.</p>") );

    action = actionCollection()->addAction( "project_import" );
    action->setIcon(KIcon("document-import"));
    action->setText(i18n( "&Import Existing Project..." ));
    connect( action, SIGNAL( triggered( bool ) ), SLOT( slotImportProject() ) );
    action->setToolTip( i18n( "Import existing project" ) );

    m_templatesModel = new ProjectTemplatesModel(this);
}

AppWizardPlugin::~AppWizardPlugin()
{
}

void AppWizardPlugin::slotNewProject()
{
    m_templatesModel->refresh();
    AppWizardDialog dlg(core()->pluginController(), m_templatesModel);


    if (dlg.exec() == QDialog::Accepted)
    {
        QString project = createProject( dlg.appInfo() );
        if (!project.isEmpty())
            core()->projectController()->openProject(KUrl::fromPath(project));
    }
}

void AppWizardPlugin::slotImportProject()
{
    ImportProject import(this, QApplication::activeWindow());
    import.exec();
}

namespace
{

IDistributedVersionControl* toDVCS(IPlugin* plugin)
{
    Q_ASSERT(plugin);
    return plugin->extension<IDistributedVersionControl>();
}

ICentralizedVersionControl* toCVCS(IPlugin* plugin)
{
    Q_ASSERT(plugin);
    return plugin->extension<ICentralizedVersionControl>();
}

/*! Trouble while initializing version control. Show failure message to user. */
void vcsError(const QString &errorMsg, KTempDir &tmpdir, const KUrl &dest)
{
    KMessageBox::error(0, errorMsg);
    KIO::NetAccess::del(dest, 0);
    tmpdir.unlink();
}

/*! Setup distributed version control for a new project defined by @p info. Use @p scratchArea for temporary files  */
bool initializeDVCS(IDistributedVersionControl* dvcs, const ApplicationInfo& info, KTempDir& scratchArea)
{
    Q_ASSERT(dvcs);
    kDebug() << "DVCS system is used, just initializing DVCS";

    KUrl dest = info.location;
    //TODO: check if we want to handle KDevelop project files (like now) or only SRC dir
    VcsJob* job = dvcs->init(dest.toLocalFile());
    if (!job || !job->exec() || job->status() != VcsJob::JobSucceeded)
    {
        vcsError(i18n("Could not initialize DVCS repository"), scratchArea, dest);
        return false;
    }
    kDebug() << "Initializing DVCS repository:" << dest.toLocalFile();

    job = dvcs->add(KUrl::List(dest), IBasicVersionControl::Recursive);
    if (!job || !job->exec() || job->status() != VcsJob::JobSucceeded)
    {
        vcsError(i18n("Could not add files to the DVCS repository"), scratchArea, dest);
        return false;
    }
    job = dvcs->commit(QString("initial project import from KDevelop"), KUrl::List(dest),
                            IBasicVersionControl::Recursive);
    if (!job || !job->exec() || job->status() != VcsJob::JobSucceeded)
    {
        vcsError(i18n("Could not import project into") + dvcs->name(), scratchArea, dest);
        return false;
    }

    return true; // We'r good
}

/*! Setup version control for a new project defined by @p info. Use @p scratchArea for temporary files  */
bool initializeCVCS(ICentralizedVersionControl* cvcs, const ApplicationInfo& info, KTempDir& scratchArea)
{
    Q_ASSERT(cvcs);

    KUrl dest = info.location;
    VcsMapping import;
    VcsLocation srcloc = info.importInformation.sourceLocations().first();
    import.addMapping( VcsLocation( KUrl( scratchArea.name() ) ),
                       info.importInformation.destinationLocation( srcloc ),
                       info.importInformation.mappingFlag( srcloc ) );

    VcsLocation importLoc(KUrl( scratchArea.name()));
    kDebug() << "Importing" << srcloc.localUrl() << "to"
             << import.destinationLocation(importLoc).repositoryServer();
    kDebug() << "Using temp dir" << scratchArea.name()
             << import.sourceLocations().first().localUrl();
    VcsLocation checkoutLoc = info.checkoutInformation.sourceLocations().first();
    kDebug() << "Checking out" << checkoutLoc.repositoryServer()
             << "to" << info.checkoutInformation.destinationLocation(checkoutLoc).localUrl();

    VcsJob* job = cvcs->import( import, info.importCommitMessage );
    if(job && job->exec() && job->status() == VcsJob::JobSucceeded )
    {
        VcsJob* job = cvcs->checkout( info.checkoutInformation );
        if (!job || !job->exec() || job->status() != VcsJob::JobSucceeded )
        {
            vcsError(i18n("Could not checkout imported project"), scratchArea, dest);
            return false;
        }
    } else {
        vcsError(i18n("Could not import project"), scratchArea, dest);
        return false;
    }

    return true; // initialization phase complete
}

} // end anonymous namespace

QString AppWizardPlugin::createProject(const ApplicationInfo& info)
{
    QFileInfo templateInfo(info.appTemplate);
    if (!templateInfo.exists())
        return QString();

    QString templateName = templateInfo.baseName();
    kDebug() << "creating project for template:" << templateName << " with VCS:" << info.vcsPluginName;

    QString templateArchive = componentData().dirs()->findResource("apptemplates", templateName + ".zip");
    if( templateArchive.isEmpty() )
    {
        templateArchive = componentData().dirs()->findResource("apptemplates", templateName + ".tar.bz2");
    }

    kDebug() << "Using archive:" << templateArchive;

    if (templateArchive.isEmpty())
        return QString();


    KUrl dest = info.location;

    //prepare variable substitution hash
    m_variables.clear();
    m_variables["APPNAME"] = info.name;
    m_variables["APPNAMEUC"] = info.name.toUpper();
    m_variables["APPNAMELC"] = info.name.toLower();
    m_variables["PROJECTDIRNAME"] = dest.fileName();
    m_variables["VERSIONCONTROLPLUGIN"] = info.vcsPluginName;

    KArchive* arch = 0;
    if( templateArchive.endsWith(".zip") )
    {
        arch = new KZip(templateArchive);
    }
    else
    {
        arch = new KTar(templateArchive, "application/x-bzip");
    }
    if (arch->open(QIODevice::ReadOnly))
    {
        KTempDir tmpdir;
        QString unpackDir = tmpdir.name(); //the default value for all Centralized VCS
        IPlugin* plugin = core()->pluginController()->loadPlugin( info.vcsPluginName );
        if( info.vcsPluginName.isEmpty() || ( plugin && plugin->extension<KDevelop::IDistributedVersionControl>() ) )
        {
            if( !QFileInfo( dest.toLocalFile() ).exists() )
            {
                QDir::root().mkdir( dest.toLocalFile() );
            }
            unpackDir = dest.toLocalFile(); //in DVCS we unpack template directly to the project's directory
        }
        else
        {
            KUrl parentdir = dest;
            parentdir.cd( ".." );
            if( !QFileInfo( parentdir.toLocalFile() ).exists() )
            {
                QDir::root().mkdir( parentdir.toLocalFile() );
            }
        }

        if ( !unpackArchive( arch->directory(), unpackDir ) )
        {
            QString errorMsg = i18n("Could not create new project");
            vcsError(errorMsg, tmpdir, KUrl(unpackDir));
            return QString();
        }

        if( !info.vcsPluginName.isEmpty() )
        {
            if (!plugin)
            {
                // Red Alert, serious program corruption.
                // This should never happen, the vcs dialog presented a list of vcs
                // systems and now the chosen system doesn't exist anymore??
                tmpdir.unlink();
                return QString();
            }

            IDistributedVersionControl* dvcs = toDVCS(plugin);
            ICentralizedVersionControl* cvcs = toCVCS(plugin);
            bool success = false;
            if (dvcs)
            {
                success = initializeDVCS(dvcs, info, tmpdir);
            }
            else if (cvcs)
            {
                success = initializeCVCS(cvcs, info, tmpdir);
            }
            else
            {
                if (KMessageBox::Continue ==
                    KMessageBox::warningContinueCancel(0,
                    "Failed to initialize version control system, "
                    "plugin is neither VCS nor DVCS."))
                    success = true;
            }
            if (!success) return QString();
        }
        tmpdir.unlink();
    }else
    {
        kDebug() << "failed to open template archive";
        return QString();
    }

    kDebug() << "Returning" << QDir::cleanPath(dest.toLocalFile() + '/' + info.name.toLower() + ".kdev4");
    return QDir::cleanPath(dest.toLocalFile() + '/' + dest.fileName() + ".kdev4");
}

bool AppWizardPlugin::unpackArchive(const KArchiveDirectory *dir, const QString &dest)
{
    kDebug() << "unpacking dir:" << dir->name() << "to" << dest;
    QStringList entries = dir->entries();
    kDebug() << "entries:" << entries.join(",");

    //This extra tempdir is needed just for the files files have special names,
    //which may contain macros also files contain content with macros. So the
    //easiest way to extract the files from the archive and then rename them
    //and replace the macros is to use a tempdir and copy the file (and
    //replacing while copying). This also allows to easily remove all files, by
    //just unlinking the tempdir
    KTempDir tdir;

    bool ret = true;

    foreach (QString entry, entries)
    {
        if (entry.endsWith(".kdevtemplate"))
            continue;
        if (dir->entry(entry)->isDirectory())
        {
            const KArchiveDirectory *file = (KArchiveDirectory *)dir->entry(entry);
            QString newdest = dest+"/"+file->name();
            if( !QFileInfo( newdest ).exists() )
            {
                QDir::root().mkdir( newdest  );
            }
            ret |= unpackArchive(file, newdest);
        }
        else if (dir->entry(entry)->isFile())
        {
            const KArchiveFile *file = (KArchiveFile *)dir->entry(entry);
            file->copyTo(tdir.name());
            QString destName = dest + '/' + file->name();
            if (!copyFileAndExpandMacros(QDir::cleanPath(tdir.name()+'/'+file->name()),
                    KMacroExpander::expandMacros(destName, m_variables)))
            {
                KMessageBox::sorry(0, i18n("The file %1 cannot be created.", dest));
                return false;
            }
        }
    }
    tdir.unlink();
    return ret;
}

bool AppWizardPlugin::copyFileAndExpandMacros(const QString &source, const QString &dest)
{
    kDebug() << "copy:" << source << "to" << dest;
    if( KMimeType::isBinaryData(source) ) 
    {
        KIO::CopyJob* job = KIO::copy( KUrl(source), KUrl(dest), KIO::HideProgressInfo );
        if( !job->exec() )
        {
            return false;
        }
        return true;
    } else 
    {
        QFile inputFile(source);
        QFile outputFile(dest);
    
    
        if (inputFile.open(QFile::ReadOnly) && outputFile.open(QFile::WriteOnly))
        {
            QTextStream input(&inputFile);
            input.setCodec(QTextCodec::codecForName("UTF-8"));
            QTextStream output(&outputFile);
            output.setCodec(QTextCodec::codecForName("UTF-8"));
            while(!input.atEnd())
            {
                QString line = input.readLine();
                output << KMacroExpander::expandMacros(line, m_variables) << "\n";
            }
            // Preserve file mode...
            struct stat fmode;
            ::fstat(inputFile.handle(), &fmode);
            ::fchmod(outputFile.handle(), fmode.st_mode);
            return true;
        }
        else
        {
            inputFile.close();
            outputFile.close();
            return false;
        }
    }
}

#include "appwizardplugin.moc"
