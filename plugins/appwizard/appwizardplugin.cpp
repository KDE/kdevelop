/*
    SPDX-FileCopyrightText: 2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2004-2005 Sascha Cunz <sascha@kdevelop.org>
    SPDX-FileCopyrightText: 2005 Ian Reinhart Geiser <ian@geiseri.com>
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "appwizardplugin.h"

#include <QAction>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QMimeType>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>
#include <qplatformdefs.h>

#include <KActionCollection>
#include <KConfigGroup>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KParts/MainWindow>
#include <KPluginFactory>
#include <KSharedConfig>
#include <KTar>
#include <KZip>
#include <KMacroExpander>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <util/scopeddialog.h>
#include <sublime/message.h>
#include <vcs/vcsjob.h>
#include <vcs/interfaces/icentralizedversioncontrol.h>
#include <vcs/interfaces/idistributedversioncontrol.h>

#include "appwizarddialog.h"
#include "projectselectionpage.h"
#include "projectvcspage.h"
#include "debug.h"

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(AppWizardFactory, "kdevappwizard.json", registerPlugin<AppWizardPlugin>();)

AppWizardPlugin::AppWizardPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevappwizard"), parent, metaData)
{
    setXMLFile(QStringLiteral("kdevappwizard.rc"));

    m_newFromTemplate = actionCollection()->addAction(QStringLiteral("project_new"));
    m_newFromTemplate->setIcon(QIcon::fromTheme(QStringLiteral("project-development-new-template")));
    m_newFromTemplate->setText(i18nc("@action", "New from Template..."));
    connect(m_newFromTemplate, &QAction::triggered, this, &AppWizardPlugin::slotNewProject);
    m_newFromTemplate->setToolTip( i18nc("@info:tooltip", "Generate a new project from a template") );
    m_newFromTemplate->setWhatsThis( i18nc("@info:whatsthis", "This starts KDevelop's application wizard. "
                                          "It helps you to generate a skeleton for your "
                                          "application from a set of templates.") );
}

AppWizardPlugin::~AppWizardPlugin()
{
}

void AppWizardPlugin::slotNewProject()
{
    ScopedDialog<AppWizardDialog> dlg(core()->pluginController(), *this);

    if (dlg->exec() == QDialog::Accepted)
    {
        QString project = createProject( dlg->appInfo() );
        if (!project.isEmpty())
        {
            core()->projectController()->openProject(QUrl::fromLocalFile(project));

            KConfig templateConfig(dlg->appInfo().appTemplate);
            KConfigGroup general(&templateConfig, QStringLiteral("General"));
            const QStringList fileArgs =
                general.readEntry("ShowFilesAfterGeneration").split(QLatin1Char(','), Qt::SkipEmptyParts);
            for (const auto& fileArg : fileArgs) {
                QString file = KMacroExpander::expandMacros(fileArg.trimmed(), m_variables);
                if (QDir::isRelativePath(file)) {
                    file = m_variables[QStringLiteral("PROJECTDIR")] + QLatin1Char('/') + file;
                }
                core()->documentController()->openDocument(QUrl::fromUserInput(file));
            }
        } else {
            const QString messageText = i18n("Could not create project from template.");
            auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
            ICore::self()->uiController()->postMessage(message);
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
        displayDetails = i18n("Please see the Version Control tool view.");
    }
    KMessageBox::detailedError(nullptr, errorMsg, displayDetails, i18nc("@title:window", "Version Control System Error"));
    KIO::del(dest, KIO::HideProgressInfo)->exec();
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

    qCDebug(PLUGIN_APPWIZARD) << "Checking for valid files in the DVCS repository:" << dest;
    job = dvcs->status({dest}, KDevelop::IBasicVersionControl::Recursive);
    if (!job || !job->exec() || job->status() != VcsJob::JobSucceeded)
    {
        vcsError(i18n("Could not check status of the DVCS repository"), scratchArea, dest);
        return false;
    }

    if (job->fetchResults().toList().isEmpty())
    {
        qCDebug(PLUGIN_APPWIZARD) << "No files to add, skipping commit in the DVCS repository:" << dest;
        return true;
    }

    job = dvcs->add({dest}, KDevelop::IBasicVersionControl::Recursive);
    if (!job || !job->exec() || job->status() != VcsJob::JobSucceeded)
    {
        vcsError(i18n("Could not add files to the DVCS repository"), scratchArea, dest);
        return false;
    }

    job = dvcs->commit(info.importCommitMessage, {dest},
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
    static const QRegularExpression re(QStringLiteral("[^\\w]"));
    return tmp.replace(re, QStringLiteral("_"));
}

} // end anonymous namespace

QString AppWizardPlugin::createProject(const ApplicationInfo& info)
{
    QFileInfo templateInfo(info.appTemplate);
    if (!templateInfo.exists()) {
        qCWarning(PLUGIN_APPWIZARD) << "Project app template does not exist:" << info.appTemplate;
        return QString();
    }

    QString templateName = templateInfo.baseName();
    qCDebug(PLUGIN_APPWIZARD) << "Searching archive for template name:" << templateName;

    QString templateArchive;
    const QStringList filters = {templateName + QStringLiteral(".*")};
    const QStringList matchesPaths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("kdevappwizard/templates/"), QStandardPaths::LocateDirectory);
    for (const QString& matchesPath : matchesPaths) {
        const QStringList files = QDir(matchesPath).entryList(filters);
        if(!files.isEmpty()) {
            templateArchive = matchesPath + files.first();
        }
    }

    if(templateArchive.isEmpty()) {
        qCWarning(PLUGIN_APPWIZARD) << "Template name does not exist in the template list";
        return QString();
    }

    qCDebug(PLUGIN_APPWIZARD) << "Using template archive:" << templateArchive;

    QUrl dest = info.location;

    //prepare variable substitution hash
    m_variables.clear();
    m_variables[QStringLiteral("APPNAME")] = info.name;
    m_variables[QStringLiteral("APPNAMEUC")] = generateIdentifier(info.name.toUpper());
    m_variables[QStringLiteral("APPNAMELC")] = info.name.toLower();
    m_variables[QStringLiteral("APPNAMEID")] = generateIdentifier(info.name);
    m_variables[QStringLiteral("PROJECTDIR")] = dest.toLocalFile();
    // backwards compatibility
    m_variables[QStringLiteral("dest")] = m_variables[QStringLiteral("PROJECTDIR")];
    m_variables[QStringLiteral("PROJECTDIRNAME")] = dest.fileName();
    m_variables[QStringLiteral("VERSIONCONTROLPLUGIN")] = info.vcsPluginName;

    KArchive* arch = nullptr;
    if( templateArchive.endsWith(QLatin1String(".zip")) )
    {
        arch = new KZip(templateArchive);
    }
    else
    {
        arch = new KTar(templateArchive, QStringLiteral("application/x-bzip"));
    }
    if (arch->open(QIODevice::ReadOnly))
    {
        QTemporaryDir tmpdir;
        QString unpackDir = tmpdir.path(); //the default value for all Centralized VCS
        IPlugin* plugin = core()->pluginController()->loadPlugin( info.vcsPluginName );
        if( info.vcsPluginName.isEmpty() || ( plugin && plugin->extension<KDevelop::IDistributedVersionControl>() ) )
        {
            if( !QFileInfo::exists( dest.toLocalFile() ) )
            {
                QDir::root().mkpath( dest.toLocalFile() );
            }
            unpackDir = dest.toLocalFile(); //in DVCS we unpack template directly to the project's directory
        }
        else
        {
            QUrl url = KIO::upUrl(dest);
            if(!QFileInfo::exists(url.toLocalFile())) {
                QDir::root().mkpath(url.toLocalFile());
            }
        }

        // estimate metadata files which should not be copied
        QStringList metaDataFileNames;

        // try by same name
        const KArchiveEntry *templateEntry =
            arch->directory()->entry(templateName + QLatin1String(".kdevtemplate"));

        // but could be different name, if e.g. downloaded, so make a guess
        if (!templateEntry || !templateEntry->isFile()) {
            const auto& entries = arch->directory()->entries();
            for (const auto& entryName : entries) {
                if (entryName.endsWith(QLatin1String(".kdevtemplate"))) {
                    templateEntry = arch->directory()->entry(entryName);
                    break;
                }
            }
        }

        if (templateEntry && templateEntry->isFile()) {
            metaDataFileNames << templateEntry->name();

            // check if a preview file is to be ignored
            const auto *templateFile = static_cast<const KArchiveFile*>(templateEntry);
            QTemporaryDir temporaryDir;
            templateFile->copyTo(temporaryDir.path());

            KConfig config(temporaryDir.path() + QLatin1Char('/') + templateEntry->name());
            KConfigGroup group(&config, QStringLiteral("General"));
            if (group.hasKey("Icon")) {
                const KArchiveEntry* iconEntry = arch->directory()->entry(group.readEntry("Icon"));
                if (iconEntry && iconEntry->isFile()) {
                    metaDataFileNames << iconEntry->name();
                }
            }
        }

        if (!unpackArchive(arch->directory(), unpackDir, metaDataFileNames)) {
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
                    KMessageBox::warningContinueCancel(nullptr,
                    QStringLiteral("Failed to initialize version control system, "
                    "plugin is neither VCS nor DVCS.")))
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

    QString projectFileName = QDir::cleanPath(dest.toLocalFile() + QLatin1Char('/') + info.name + QLatin1String(".kdev4"));

    // Loop through the new project directory and try to detect the first .kdev4 file.
    // If one is found this file will be used. So .kdev4 file can be stored in any subdirectory and the
    // project templates can be more complex.
    QDirIterator it(QDir::cleanPath( dest.toLocalFile()), QStringList() << QStringLiteral("*.kdev4"), QDir::NoFilter, QDirIterator::Subdirectories);
    if(it.hasNext() == true)
    {
        projectFileName = it.next();
    }

    qCDebug(PLUGIN_APPWIZARD) << "Returning" << projectFileName << QFileInfo::exists( projectFileName ) ;

    const QFileInfo projectFileInfo(projectFileName);
    if (!projectFileInfo.exists()) {
        qCDebug(PLUGIN_APPWIZARD) << "creating .kdev4 file";
        KSharedConfigPtr cfg = KSharedConfig::openConfig( projectFileName, KConfig::SimpleConfig );
        KConfigGroup project = cfg->group(QStringLiteral("Project"));
        project.writeEntry( "Name", info.name );
        QString manager = QStringLiteral("KDevGenericManager");

        QDir d( dest.toLocalFile() );
        const auto data = ICore::self()->pluginController()->queryExtensionPlugins(QStringLiteral("org.kdevelop.IProjectFileManager"));
        for (const KPluginMetaData& info : data) {
            const auto filter = info.value(QStringLiteral("X-KDevelop-ProjectFilesFilter"), QStringList());
            if (!filter.isEmpty()) {
                if (!d.entryList(filter).isEmpty()) {
                    manager = info.pluginId();
                    break;
                }
            }
        }
        project.writeEntry( "Manager", manager );
        project.sync();
        cfg->sync();
        KConfigGroup project2 = cfg->group(QStringLiteral("Project"));
        qCDebug(PLUGIN_APPWIZARD) << "kdev4 file contents:" << project2.readEntry("Name", "") << project2.readEntry("Manager", "" );
    }

    // create developer .kde4 file
    const QString developerProjectFileName = projectFileInfo.canonicalPath() + QLatin1String("/.kdev4/") + projectFileInfo.fileName();

    qCDebug(PLUGIN_APPWIZARD) << "creating developer .kdev4 file:" << developerProjectFileName;
    KSharedConfigPtr developerCfg = KSharedConfig::openConfig(developerProjectFileName, KConfig::SimpleConfig);
    KConfigGroup developerProjectGroup = developerCfg->group(QStringLiteral("Project"));
    developerProjectGroup.writeEntry("VersionControlSupport", info.vcsPluginName);
    developerProjectGroup.sync();

    developerCfg->sync();

    return projectFileName;
}

bool AppWizardPlugin::unpackArchive(const KArchiveDirectory* dir, const QString& dest, const QStringList& skipList)
{
    qCDebug(PLUGIN_APPWIZARD) << "unpacking dir:" << dir->name() << "to" << dest;
    const QStringList entries = dir->entries();
    qCDebug(PLUGIN_APPWIZARD) << "entries:" << entries.join(QLatin1Char(','));

    //This extra tempdir is needed just for the files that have special names,
    //which may contain macros also files contain content with macros. So the
    //easiest way to extract the files from the archive and then rename them
    //and replace the macros is to use a tempdir and copy the file (and
    //replacing while copying). This also allows one to easily remove all files,
    //by just unlinking the tempdir
    QTemporaryDir tdir;

    bool ret = true;

    for (const auto& entryName : entries) {
        if (skipList.contains(entryName)) {
            continue;
        }

        const auto entry = dir->entry(entryName);
        if (entry->isDirectory()) {
            const auto* subdir = static_cast<const KArchiveDirectory*>(entry);
            const QString newdest = dest + QLatin1Char('/') + KMacroExpander::expandMacros(subdir->name(), m_variables);
            if( !QFileInfo::exists( newdest ) )
            {
                QDir::root().mkdir( newdest  );
            }
            ret |= unpackArchive(subdir, newdest);
        }
        else if (entry->isFile()) {
            const auto* file = static_cast<const KArchiveFile*>(entry);
            file->copyTo(tdir.path());
            const QString destName = dest + QLatin1Char('/') + file->name();
            if (!copyFileAndExpandMacros(QDir::cleanPath(tdir.path() + QLatin1Char('/') + file->name()),
                    KMacroExpander::expandMacros(destName, m_variables)))
            {
                KMessageBox::error(nullptr, i18n("The file %1 cannot be created.", dest));
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
    if( !mime.inherits(QStringLiteral("text/plain")) )
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
            QTextStream output(&outputFile);
            while(!input.atEnd())
            {
                QString line = input.readLine();
                output << KMacroExpander::expandMacros(line, m_variables) << "\n";
            }
#ifndef Q_OS_WIN
            // Preserve file mode...
            QT_STATBUF statBuf;
            QT_FSTAT(inputFile.handle(), &statBuf);
            // Unix only, won't work in Windows, maybe KIO::chmod could be used
            ::fchmod(outputFile.handle(), statBuf.st_mode);
#endif
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
KDevelop::ContextMenuExtension AppWizardPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    Q_UNUSED(parent);
    KDevelop::ContextMenuExtension ext;
    if ( context->type() != KDevelop::Context::ProjectItemContext || !static_cast<KDevelop::ProjectItemContext*>(context)->items().isEmpty() ) {
        return ext;
    }
    ext.addAction(KDevelop::ContextMenuExtension::ProjectGroup, m_newFromTemplate);
    return ext;
}

QString AppWizardPlugin::modelTypePrefix() const
{
    return QStringLiteral("kdevappwizard");
}

QString AppWizardPlugin::knsConfigurationFile() const
{
    return QStringLiteral("kdevappwizard.knsrc");
}

QIcon AppWizardPlugin::icon() const
{
    return QIcon::fromTheme(QStringLiteral("project-development-new-template"));
}

QString AppWizardPlugin::name() const
{
    return i18n("Project Templates");
}

#include "appwizardplugin.moc"
#include "moc_appwizardplugin.cpp"
