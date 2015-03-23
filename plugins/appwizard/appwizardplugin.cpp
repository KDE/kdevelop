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
#include <QDirIterator>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QDebug>
#include <QMimeType>
#include <QMimeDatabase>

#include <ktar.h>
#include <kzip.h>
#include <KLocalizedString>
#include <QAction>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kmessagebox.h>
#include <kmacroexpander.h>
#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kpluginloader.h>
#include <kactioncollection.h>
#include <kparts/mainwindow.h>
#include <kio/copyjob.h>
#include <kio/netaccess.h>
#include <kde_file.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <vcs/vcslocation.h>
#include <vcs/vcsjob.h>
#include <vcs/interfaces/icentralizedversioncontrol.h>
#include <vcs/interfaces/idistributedversioncontrol.h>

#include "appwizarddialog.h"
#include "projectselectionpage.h"
#include "projectvcspage.h"
#include "projecttemplatesmodel.h"
#include "debug.h"

using namespace KDevelop;

Q_LOGGING_CATEGORY(PLUGIN_APPWIZARD, "kdevplatform.plugins.appwizard")
K_PLUGIN_FACTORY_WITH_JSON(AppWizardFactory, "kdevappwizard.json", registerPlugin<AppWizardPlugin>();)

AppWizardPlugin::AppWizardPlugin(QObject *parent, const QVariantList &)
    : KDevelop::IPlugin("kdevappwizard", parent)
    , m_templatesModel(0)
{
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::ITemplateProvider);
    setXMLFile("kdevappwizard.rc");

    m_newFromTemplate = actionCollection()->addAction("project_new");
    m_newFromTemplate->setIcon(QIcon::fromTheme("project-development-new-template"));
    m_newFromTemplate->setText(i18n("New From Template..."));
    connect(m_newFromTemplate, &QAction::triggered, this, &AppWizardPlugin::slotNewProject);
    m_newFromTemplate->setToolTip( i18n("Generate a new project from a template") );
    m_newFromTemplate->setWhatsThis( i18n("This starts KDevelop's application wizard. "
                                          "It helps you to generate a skeleton for your "
                                          "application from a set of templates.") );
}

AppWizardPlugin::~AppWizardPlugin()
{
}

void AppWizardPlugin::slotNewProject()
{
    model()->refresh();

    AppWizardDialog dlg(core()->pluginController(), m_templatesModel);

    if (dlg.exec() == QDialog::Accepted)
    {
        QString project = createProject( dlg.appInfo() );
        if (!project.isEmpty())
        {
            core()->projectController()->openProject(QUrl::fromLocalFile(project));

            KConfig templateConfig(dlg.appInfo().appTemplate);
            KConfigGroup general(&templateConfig, "General");
            QString file = general.readEntry("ShowFilesAfterGeneration");
            if (!file.isEmpty())
            {
                file = KMacroExpander::expandMacros(file, m_variables);
                core()->documentController()->openDocument(QUrl::fromUserInput(file));
            }
        } else {
            KMessageBox::error( ICore::self()->uiController()->activeMainWindow(), i18n("Could not create project from template\n"), i18n("Failed to create project") );
        }
    }
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
void vcsError(const QString &errorMsg, QTemporaryDir &tmpdir, const QUrl &dest, const QString &details = QString())
{
    QString displayDetails = details;
    if (displayDetails.isEmpty())
    {
        displayDetails = i18n("Please see the Version Control toolview");
    }
    KMessageBox::detailedError(0, errorMsg, displayDetails, i18n("Version Control System Error"));
    KIO::del(dest)->exec();
    tmpdir.remove();
}

/*! Setup distributed version control for a new project defined by @p info. Use @p scratchArea for temporary files  */
bool initializeDVCS(IDistributedVersionControl* dvcs, const ApplicationInfo& info, QTemporaryDir& scratchArea)
{
    Q_ASSERT(dvcs);
    qCDebug(PLUGIN_APPWIZARD) << "DVCS system is used, just initializing DVCS";

    const QUrl& dest = info.location;
    //TODO: check if we want to handle KDevelop project files (like now) or only SRC dir
    VcsJob* job = dvcs->init(dest);
    if (!job || !job->exec() || job->status() != VcsJob::JobSucceeded)
    {
        vcsError(i18n("Could not initialize DVCS repository"), scratchArea, dest);
        return false;
    }
    qCDebug(PLUGIN_APPWIZARD) << "Initializing DVCS repository:" << dest;

    job = dvcs->add({dest}, KDevelop::IBasicVersionControl::Recursive);
    if (!job || !job->exec() || job->status() != VcsJob::JobSucceeded)
    {
        vcsError(i18n("Could not add files to the DVCS repository"), scratchArea, dest);
        return false;
    }
    job = dvcs->commit(QStringLiteral("initial project import from KDevelop"), {dest},
                            KDevelop::IBasicVersionControl::Recursive);
    if (!job || !job->exec() || job->status() != VcsJob::JobSucceeded)
    {
        vcsError(i18n("Could not import project into %1.", dvcs->name()), scratchArea, dest, job ? job->errorString() : QString());
        return false;
    }

    return true; // We're good
}

/*! Setup version control for a new project defined by @p info. Use @p scratchArea for temporary files  */
bool initializeCVCS(ICentralizedVersionControl* cvcs, const ApplicationInfo& info, QTemporaryDir& scratchArea)
{
    Q_ASSERT(cvcs);

    qCDebug(PLUGIN_APPWIZARD) << "Importing" << info.sourceLocation << "to"
             << info.repository.repositoryServer();
    VcsJob* job = cvcs->import( info.importCommitMessage, QUrl::fromLocalFile(scratchArea.path()), info.repository);
    if (!job || !job->exec() || job->status() != VcsJob::JobSucceeded )
    {
        vcsError(i18n("Could not import project"), scratchArea, QUrl::fromUserInput(info.repository.repositoryServer()));
        return false;
    }

    qCDebug(PLUGIN_APPWIZARD) << "Checking out";
    job = cvcs->createWorkingCopy( info.repository, info.location, IBasicVersionControl::Recursive);
    if (!job || !job->exec() || job->status() != VcsJob::JobSucceeded )
    {
        vcsError(i18n("Could not checkout imported project"), scratchArea, QUrl::fromUserInput(info.repository.repositoryServer()));
        return false;
    }

    return true; // initialization phase complete
}

QString generateIdentifier( const QString& appname )
{
    QString tmp = appname;
    QRegExp re("[^a-zA-Z0-9_]");
    return tmp.replace(re, "_");
}

} // end anonymous namespace

QString AppWizardPlugin::createProject(const ApplicationInfo& info)
{
    QFileInfo templateInfo(info.appTemplate);
    if (!templateInfo.exists()) {
        qWarning() << "Project app template does not exist:" << info.appTemplate;
        return QString();
    }

    QString templateName = templateInfo.baseName();
    QString templateArchive;
    const QStringList filters = {templateName + QStringLiteral(".*")};
    const QStringList matchesPaths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("kdevappwizard/templates/"), QStandardPaths::LocateDirectory);
    foreach(const QString& matchesPath, matchesPaths) {
        const QStringList files = QDir(matchesPath).entryList(filters);
        if(!files.isEmpty()) {
            templateArchive = matchesPath + files.first();
        }
    }

    if(templateArchive.isEmpty()) {
        qWarning() << "Template name does not exist in the template list";
        return QString();
    }

    QUrl dest = info.location;

    //prepare variable substitution hash
    m_variables.clear();
    m_variables["APPNAME"] = info.name;
    m_variables["APPNAMEUC"] = info.name.toUpper();
    m_variables["APPNAMELC"] = info.name.toLower();
    m_variables["APPNAMEID"] = generateIdentifier(info.name);
    m_variables["PROJECTDIR"] = dest.toLocalFile();
    // backwards compatibility
    m_variables["dest"] = m_variables["PROJECTDIR"];
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
        QTemporaryDir tmpdir;
        QString unpackDir = tmpdir.path(); //the default value for all Centralized VCS
        IPlugin* plugin = core()->pluginController()->loadPlugin( info.vcsPluginName );
        if( info.vcsPluginName.isEmpty() || ( plugin && plugin->extension<KDevelop::IDistributedVersionControl>() ) )
        {
            if( !QFileInfo( dest.toLocalFile() ).exists() )
            {
                QDir::root().mkpath( dest.toLocalFile() );
            }
            unpackDir = dest.toLocalFile(); //in DVCS we unpack template directly to the project's directory
        }
        else
        {
            QUrl url = KIO::upUrl(dest);
            if(!QFileInfo(url.toLocalFile()).exists()) {
                QDir::root().mkpath(url.toLocalFile());
            }
        }

        if ( !unpackArchive( arch->directory(), unpackDir ) )
        {
            QString errorMsg = i18n("Could not create new project");
            vcsError(errorMsg, tmpdir, QUrl::fromLocalFile(unpackDir));
            return QString();
        }

        if( !info.vcsPluginName.isEmpty() )
        {
            if (!plugin)
            {
                // Red Alert, serious program corruption.
                // This should never happen, the vcs dialog presented a list of vcs
                // systems and now the chosen system doesn't exist anymore??
                tmpdir.remove();
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
        tmpdir.remove();
    }else
    {
        qCDebug(PLUGIN_APPWIZARD) << "failed to open template archive";
        return QString();
    }

    QString projectFileName = QDir::cleanPath( dest.toLocalFile() + '/' + info.name + ".kdev4" );

    // Loop through the new project directory and try to detect the first .kdev4 file.
    // If one is found this file will be used. So .kdev4 file can be stored in any subdirectory and the
    // project templates can be more complex.
    QDirIterator it(QDir::cleanPath( dest.toLocalFile()), QStringList() << "*.kdev4", QDir::NoFilter, QDirIterator::Subdirectories);
    if(it.hasNext() == true)
    {
        projectFileName = it.next();
    }

    qCDebug(PLUGIN_APPWIZARD) << "Returning" << projectFileName << QFileInfo( projectFileName ).exists() ;

    if( ! QFileInfo( projectFileName ).exists() )
    {
        qCDebug(PLUGIN_APPWIZARD) << "creating .kdev4 file";
        KSharedConfigPtr cfg = KSharedConfig::openConfig( projectFileName, KConfig::SimpleConfig );
        KConfigGroup project = cfg->group( "Project" );
        project.writeEntry( "Name", info.name );
        QString manager = "KDevGenericManager";

        QDir d( dest.toLocalFile() );
        foreach(const KPluginInfo& info, ICore::self()->pluginController()->queryExtensionPlugins( "org.kdevelop.IProjectFileManager" ) )
        {
            QVariant filter = info.property("X-KDevelop-ProjectFilesFilter");
            if( filter.isValid() )
            {
                if( !d.entryList( filter.toStringList() ).isEmpty() )
                {
                    manager = info.pluginName();
                    break;
                }
            }
        }
        project.writeEntry( "Manager", manager );
        project.sync();
        cfg->sync();
        KConfigGroup project2 = cfg->group( "Project" );
        qCDebug(PLUGIN_APPWIZARD) << "kdev4 file contents:" << project2.readEntry("Name", "") << project2.readEntry("Manager", "" );
    }

    return projectFileName;
}

bool AppWizardPlugin::unpackArchive(const KArchiveDirectory *dir, const QString &dest)
{
    qCDebug(PLUGIN_APPWIZARD) << "unpacking dir:" << dir->name() << "to" << dest;
    const QStringList entries = dir->entries();
    qCDebug(PLUGIN_APPWIZARD) << "entries:" << entries.join(",");

    //This extra tempdir is needed just for the files files have special names,
    //which may contain macros also files contain content with macros. So the
    //easiest way to extract the files from the archive and then rename them
    //and replace the macros is to use a tempdir and copy the file (and
    //replacing while copying). This also allows to easily remove all files, by
    //just unlinking the tempdir
    QTemporaryDir tdir;

    bool ret = true;

    foreach (const QString& entry, entries)
    {
        if (entry.endsWith(".kdevtemplate"))
            continue;
        if (dir->entry(entry)->isDirectory())
        {
            const KArchiveDirectory *file = (KArchiveDirectory *)dir->entry(entry);
            QString newdest = dest + '/' + KMacroExpander::expandMacros(file->name(), m_variables);
            if( !QFileInfo( newdest ).exists() )
            {
                QDir::root().mkdir( newdest  );
            }
            ret |= unpackArchive(file, newdest);
        }
        else if (dir->entry(entry)->isFile())
        {
            const KArchiveFile *file = (KArchiveFile *)dir->entry(entry);
            file->copyTo(tdir.path());
            QString destName = dest + '/' + file->name();
            if (!copyFileAndExpandMacros(QDir::cleanPath(tdir.path()+'/'+file->name()),
                    KMacroExpander::expandMacros(destName, m_variables)))
            {
                KMessageBox::sorry(0, i18n("The file %1 cannot be created.", dest));
                return false;
            }
        }
    }
    tdir.remove();
    return ret;
}

bool AppWizardPlugin::copyFileAndExpandMacros(const QString &source, const QString &dest)
{
    qCDebug(PLUGIN_APPWIZARD) << "copy:" << source << "to" << dest;
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(source);
    if( !mime.inherits("text/plain") )
    {
        KIO::CopyJob* job = KIO::copy( QUrl::fromUserInput(source), QUrl::fromUserInput(dest), KIO::HideProgressInfo );
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
            QT_STATBUF statBuf;
            QT_FSTAT(inputFile.handle(), &statBuf);
            // Unix only, won't work in Windows, maybe KIO::chmod could be used
            ::fchmod(outputFile.handle(), statBuf.st_mode);
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
KDevelop::ContextMenuExtension AppWizardPlugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::ContextMenuExtension ext;
    if ( context->type() != KDevelop::Context::ProjectItemContext || !static_cast<KDevelop::ProjectItemContext*>(context)->items().isEmpty() ) {
        return ext;
    }
    ext.addAction(KDevelop::ContextMenuExtension::ProjectGroup, m_newFromTemplate);
    return ext;
}

ProjectTemplatesModel* AppWizardPlugin::model()
{
    if(!m_templatesModel)
        m_templatesModel = new ProjectTemplatesModel(this);
    return m_templatesModel;
}

QAbstractItemModel* AppWizardPlugin::templatesModel()
{
    return model();
}

QString AppWizardPlugin::knsConfigurationFile() const
{
    return "kdevappwizard.knsrc";
}

QStringList AppWizardPlugin::supportedMimeTypes() const
{
    QStringList types;
    types << "application/x-desktop";
    types << "application/x-bzip-compressed-tar";
    types << "application/zip";
    return types;
}

QIcon AppWizardPlugin::icon() const
{
    return QIcon::fromTheme("project-development-new-template");
}

QString AppWizardPlugin::name() const
{
    return i18n("Project Templates");
}

void AppWizardPlugin::loadTemplate(const QString& fileName)
{
    model()->loadTemplateFile(fileName);
}

void AppWizardPlugin::reload()
{
    model()->refresh();
}


#include "appwizardplugin.moc"
