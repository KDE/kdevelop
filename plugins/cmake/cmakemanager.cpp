/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2007-2013 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakemanager.h"
#include "cmakeutils.h"
#include "cmakeprojectdata.h"
#include "duchain/cmakeparsejob.h"
#include "cmakeimportjsonjob.h"
#include "debug.h"
#include "cmakecodecompletionmodel.h"
#include "cmakenavigationwidget.h"
#include "icmakedocumentation.h"
#include "cmakemodelitems.h"
#include "testing/ctestutils.h"
#include "testing/ctestsuite.h"
#include "testing/ctestfindjob.h"
#include "cmakeserverimportjob.h"
#include "cmakeserver.h"
#include "cmakefileapi.h"
#include "cmakefileapiimportjob.h"

#ifndef CMAKEMANAGER_NO_SETTINGS
#include "settings/cmakepreferences.h"
#endif

#include <QApplication>
#include <QDir>
#include <QReadWriteLock>
#include <QThread>
#include <QFileSystemWatcher>
#include <QTimer>

#include <KPluginFactory>
#include <QUrl>
#include <QAction>
#include <KTextEditor/Document>
#include <KDirWatch>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <interfaces/idocumentation.h>
#include <util/executecompositejob.h>
#include <language/highlighting/codehighlighting.h>
#include <project/projectmodel.h>
#include <project/helper.h>
#include <project/interfaces/iprojectbuilder.h>
#include <language/codecompletion/codecompletion.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/use.h>
#include <language/duchain/duchain.h>
#include <makefileresolver/makefileresolver.h>
#include <sublime/message.h>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(CMakeSupportFactory, "kdevcmakemanager.json", registerPlugin<CMakeManager>(); )

CMakeManager::CMakeManager(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::AbstractFileManagerPlugin(QStringLiteral("kdevcmakemanager"), parent, metaData)
{
    if (CMake::findExecutable().isEmpty()) {
        setErrorDescription(i18n("Unable to find a CMake executable. Is one installed on the system?"));
        m_highlight = nullptr;
        return;
    }

    m_highlight = new KDevelop::CodeHighlighting(this);

    new CodeCompletion(this, new CMakeCodeCompletionModel(this), name());

    connect(ICore::self()->projectController(), &IProjectController::projectClosing, this, &CMakeManager::projectClosing);
    connect(ICore::self()->runtimeController(), &IRuntimeController::currentRuntimeChanged, this, &CMakeManager::reloadProjects);
    connect(this, &KDevelop::AbstractFileManagerPlugin::folderAdded, this, &CMakeManager::folderAdded);
}

CMakeManager::~CMakeManager()
{
    parseLock()->lockForWrite();
    // By locking the parse-mutexes, we make sure that parse jobs get a chance to finish in a good state
    parseLock()->unlock();
}

bool CMakeManager::hasBuildInfo(ProjectBaseItem* item) const
{
    return m_projects[item->project()].data.compilationData.files.contains(item->path());
}

Path CMakeManager::buildDirectory(KDevelop::ProjectBaseItem *item) const
{
    return Path(CMake::currentBuildDir(item->project()));
}

KDevelop::ProjectFolderItem* CMakeManager::import( KDevelop::IProject *project )
{
    CMake::checkForNeedingConfigure(project);

    return AbstractFileManagerPlugin::import(project);
}

class ChooseCMakeInterfaceJob : public ExecuteCompositeJob
{
    Q_OBJECT
public:
    ChooseCMakeInterfaceJob(IProject* project, CMakeManager* manager, bool forceConfigure)
        : ExecuteCompositeJob(manager, {})
        , project(project)
        , manager(manager)
        , forceConfigure(forceConfigure)
    {
    }

    void start() override {
        if (CMake::FileApi::supported(CMake::currentCMakeExecutable(project).toLocalFile())) {
            qCDebug(CMAKE) << "Using cmake-file-api for import of" << project->path();

            // try to import the data directly, if possible and not outdated
            if (forceConfigure) {
                reconfigureThenImport();
            } else {
                tryDirectImport();
            }
            ExecuteCompositeJob::start();
        } else {
            tryCMakeServer();
        }
    }

private:
    void tryCMakeServer()
    {
        qCDebug(CMAKE) << "try cmake server for import";
        server.reset(new CMakeServer(project));
        connect(server.data(), &CMakeServer::connected, this, &ChooseCMakeInterfaceJob::successfulConnection);
        connect(server.data(), &CMakeServer::finished, this, &ChooseCMakeInterfaceJob::failedConnection);
    }

    void successfulConnection() {
        auto job = new CMakeServerImportJob(project, server, this);
        connect(job, &CMakeServerImportJob::result, this, [this, job](){
            if (job->error() == 0) {
                manager->integrateData(job->projectData(), job->project(), server);
            }
        });
        addSubjob(job);
        ExecuteCompositeJob::start();
    }

    void failedConnection(int code) {
        Q_ASSERT(code > 0);
        Q_ASSERT(!server->isServerAvailable());

        qCDebug(CMAKE) << "CMake does not provide server mode, using compile_commands.json to import" << project->name();

        // parse the JSON file
        auto* job = new CMakeImportJsonJob(project, this);

        // create the JSON file if it doesn't exist
        auto commandsFile = CMake::commandsFile(project);
        if (!QFileInfo::exists(commandsFile.toLocalFile())) {
            qCDebug(CMAKE) << "couldn't find commands file:" << commandsFile << "- now trying to reconfigure";
            addSubjob(manager->builder()->configure(project));
        }

        connect(job, &CMakeImportJsonJob::result, this, [this, job]() {
            if (job->error() == 0) {
                manager->integrateData(job->projectData(), job->project());
            }
        });
        addSubjob(job);
        ExecuteCompositeJob::start();
    }

    void reconfigureThenImport()
    {
        addSubjob(manager->builder()->configure(project));
        auto* importJob = new CMake::FileApi::ImportJob(project, this);
        connect(importJob, &CMake::FileApi::ImportJob::dataAvailable, this, &ChooseCMakeInterfaceJob::fileImportDone);
        addSubjob(importJob);
    }

    void tryDirectImport()
    {
        auto* importJob = new CMake::FileApi::ImportJob(project, this);
        importJob->setInvalidateOutdatedData();
        importJob->setEmitInvalidData();
        connect(importJob, &CMake::FileApi::ImportJob::dataAvailable, this, [this](const CMakeProjectData& data) {
            if (!data.compilationData.isValid) {
                qCDebug(CMAKE) << "reconfiguring project" << project->name() << "because project data is outdated";
                reconfigureThenImport();
            } else {
                Q_ASSERT_X(!data.isOutdated, Q_FUNC_INFO,
                           "ImportJob::setInvalidateOutdatedData() failed to mark outdated data invalid.");
                qCDebug(CMAKE) << "skipping configure project" << project->name()
                               << "because project data is up to date";
                fileImportDone(data);
            }
        });
        addSubjob(importJob);
    }

    void fileImportDone(const CMakeProjectData& data)
    {
        Q_ASSERT(data.compilationData.isValid);
        manager->integrateData(data, project);
    }

    QSharedPointer<CMakeServer> server;
    IProject* const project;
    CMakeManager* const manager;
    const bool forceConfigure;
};

KJob* CMakeManager::createImportJob(ProjectFolderItem* item, bool forceConfigure)
{
    auto project = item->project();

    delete m_configureStatusMessages.value(project); // discard now-obsolete message from the previous configuration

    auto job = new ChooseCMakeInterfaceJob(project, this, forceConfigure);
    connect(job, &KJob::result, this, [this, job, project]() {
        if (job->error() != 0) {
            qCWarning(CMAKE) << "couldn't load project successfully" << project->name() << job->error()
                             << job->errorText();
            showConfigureErrorMessage(*project, job->errorString());
        }
    });

    const QList<KJob*> jobs = {
        job,
        KDevelop::AbstractFileManagerPlugin::createImportJob(item) // generate the file system listing
    };

    Q_ASSERT(!jobs.contains(nullptr));
    auto* composite = new ExecuteCompositeJob(this, jobs);
    // even if the cmake call failed, we want to load the project so that the project can be worked on
    composite->setAbortOnSubjobError(false);
    return composite;
}

KJob* CMakeManager::createImportJob(ProjectFolderItem* item)
{
    return createImportJob(item, false);
}

QList<KDevelop::ProjectTargetItem*> CMakeManager::targets() const
{
    QList<KDevelop::ProjectTargetItem*> ret;
    for (auto it = m_projects.begin(), end = m_projects.end(); it != end; ++it) {
        IProject* p = it.key();
        ret+=p->projectItem()->targetList();
    }
    return ret;
}

CMakeFile CMakeManager::fileInformation(KDevelop::ProjectBaseItem* item) const
{
    const auto projectData = m_projects.constFind(item->project());
    if (projectData == m_projects.cend()) {
        return {};
    }
    const auto& data = projectData->data.compilationData;

    const auto itemPath = item->path();
    if (!data.isValid || data.missingFiles.contains(itemPath)) {
        return {};
    }

    auto toCanonicalPath = [](const Path &path) -> Path {
        // if the path contains a symlink, then we will not find it in the lookup table
        // as that only only stores canonicalized paths. Thus, we fallback to
        // to the canonicalized path and see if that brings up any matches
        const auto localPath = path.toLocalFile();
        const auto canonicalPath = QFileInfo(localPath).canonicalFilePath();
        return (localPath == canonicalPath) ? path : Path(canonicalPath);
    };

    auto path = itemPath;
    if (!item->folder()) {
        // try to look for file meta data directly
        auto it = data.files.find(path);
        if (it == data.files.end()) {
            // fallback to canonical path lookup
            auto canonical = toCanonicalPath(path);
            if (canonical != path) {
                it = data.files.find(canonical);
            }
        }
        if (it != data.files.end()) {
            return *it;
        }
        // else look for a file in the parent folder
        path = path.parent();
    }

    while (true) {
        // try to look for a file in the current folder path
        auto it = data.fileForFolder.find(path);
        if (it == data.fileForFolder.end()) {
            // fallback to canonical path lookup
            auto canonical = toCanonicalPath(path);
            if (canonical != path) {
                it = data.fileForFolder.find(canonical);
            }
        }
        if (it != data.fileForFolder.end()) {
            return data.files[it.value()];
        }
        if (!path.hasParent()) {
            break;
        }
        path = path.parent();
    }

    qCDebug(CMAKE) << "no information found for" << itemPath;
    data.missingFiles.insert(itemPath);
    return {};
}

Path::List CMakeManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    return fileInformation(item).includes;
}

Path::List CMakeManager::frameworkDirectories(KDevelop::ProjectBaseItem *item) const
{
    return fileInformation(item).frameworkDirectories;
}

QHash<QString, QString> CMakeManager::defines(KDevelop::ProjectBaseItem *item ) const
{
    return fileInformation(item).defines;
}

QString CMakeManager::extraArguments(KDevelop::ProjectBaseItem *item) const
{
    return fileInformation(item).compileFlags;
}

KDevelop::IProjectBuilder * CMakeManager::builder() const
{
    IPlugin* i = core()->pluginController()->pluginForExtension( QStringLiteral("org.kdevelop.IProjectBuilder"), QStringLiteral("KDevCMakeBuilder"));
    Q_ASSERT(i);
    auto* _builder = i->extension<KDevelop::IProjectBuilder>();
    Q_ASSERT(_builder );
    return _builder ;
}

bool CMakeManager::reload(KDevelop::ProjectFolderItem* folder)
{
    qCDebug(CMAKE) << "reloading" << folder->path();

    IProject* project = folder->project();
    if (!project->isReady()) {
        qCDebug(CMAKE) << "the project is being reloaded, aborting reload!";
        return false;
    }

    KJob* job = createImportJob(folder, true);
    project->setReloadJob(job);
    ICore::self()->runController()->registerJob( job );
    if (folder == project->projectItem()) {
        connect(job, &KJob::finished, this, [project](KJob* job) {
            if (job->error())
                return;

            emit KDevelop::ICore::self()->projectController()->projectConfigurationChanged(project);
            KDevelop::ICore::self()->projectController()->reparseProject(project);
        });
    }

    return true;
}

static void populateTargets(ProjectFolderItem* folder, const QHash<KDevelop::Path, QVector<CMakeTarget>>& targets)
{
    auto isValidTarget = [](const CMakeTarget& target) -> bool {
        if (target.type != CMakeTarget::Custom)
            return true;

        // utility targets with empty sources are strange (e.g. _QCH) -> skip them
        if (target.sources.isEmpty())
            return false;

        auto match
            = [](const auto& needles, auto&& op) { return std::any_of(std::begin(needles), std::end(needles), op); };
        auto startsWith = [&](const auto& needle) { return target.name.startsWith(needle); };
        auto endsWith = [&](const auto& needle) { return target.name.endsWith(needle); };
        auto equals = [&](const auto& needle) { return target.name == needle; };

        const auto invalidPrefixes = { QLatin1String("install/") };
        const auto invalidSuffixes
            = { QLatin1String("_automoc"), QLatin1String("_autogen"), QLatin1String("_autogen_timestamp_deps") };
        const auto standardTargets
            = { QLatin1String("edit_cache"), QLatin1String("rebuild_cache"), QLatin1String("list_install_components"),
                QLatin1String("test"), // not really standard, but applicable for make and ninja
                QLatin1String("install") };
        return !match(invalidPrefixes, startsWith) && !match(invalidSuffixes, endsWith)
            && !match(standardTargets, equals);
    };

    auto isValidTargetSource = [](const Path& source) {
        const auto& segments = source.segments();
        const auto lastSegment = source.lastPathSegment();
        // skip non-existent cmake internal rule files
        if (lastSegment.endsWith(QLatin1String(".rule"))) {
            return false;
        }

        const auto secondToLastSegment = segments.value(segments.size() - 2);
        // ignore generated cmake-internal files
        if (secondToLastSegment == QLatin1String("CMakeFiles")) {
            return false;
        }

        // also skip *_autogen/timestamp files
        if (lastSegment == QLatin1String("timestamp") && secondToLastSegment.endsWith(QLatin1String("_autogen"))) {
            return false;
        }

        return true;
    };

    // start by deleting all targets, the type may have changed anyways
    const auto tl = folder->targetList();
    for (ProjectTargetItem* item : tl) {
        delete item;
    }

    QHash<QString, ProjectBaseItem*> folderItems;
    folderItems[{}] = folder;
    auto findOrCreateFolderItem = [&folderItems, folder](const QString& targetFolder)
    {
        auto& item = folderItems[targetFolder];
        if (!item) {
            item = new ProjectTargetItem(folder->project(), targetFolder, folder);
            // these are "virtual" folders, they keep the original path
            item->setPath(folder->path());
        }
        return item;
    };

    // target folder name (or empty) to list of targets
    for (const auto &target : targets[folder->path()]) {
        if (!isValidTarget(target)) {
            continue;
        }

        auto* targetFolder = findOrCreateFolderItem(target.folder);
        auto* targetItem = [&]() -> ProjectBaseItem* {
            switch(target.type) {
                case CMakeTarget::Executable:
                    return new CMakeTargetItem(targetFolder, target.name, target.artifacts.value(0));
                case CMakeTarget::Library:
                    return new ProjectLibraryTargetItem(folder->project(), target.name, targetFolder);
                case CMakeTarget::Custom:
                    return new ProjectTargetItem(folder->project(), target.name, targetFolder);
            }
            Q_UNREACHABLE();
        }();

        for (const auto& source : target.sources) {
            if (!isValidTargetSource(source)) {
                continue;
            }
            new ProjectFileItem(folder->project(), source, targetItem);
        }
    }
}

static void populateTargetsRecursively(ProjectFolderItem* folder, const QHash<Path, QList<CMakeTarget>>& targets)
{
    populateTargets(folder, targets);
    const auto children = folder->children();
    for (auto* const child : children) {
        if (auto* const folder = child->folder())
            populateTargetsRecursively(folder, targets);
    }
}

static void cleanupTestSuites(const QVector<CTestSuite*>& testSuites, const QVector<CTestFindJob*>& testSuiteJobs)
{
    for (auto* testSuiteJob : testSuiteJobs) {
        testSuiteJob->kill(KJob::Quietly);
    }
    for (auto* testSuite : testSuites) {
        ICore::self()->testController()->removeTestSuite(testSuite);
        delete testSuite;
    }
}

void CMakeManager::integrateData(const CMakeProjectData &data, KDevelop::IProject* project, const QSharedPointer<CMakeServer>& server)
{
    // TODO: show the warning message only after the entire import job finishes. When the message
    // is shown here (earlier), the user can follow the advice and manage to reload the project
    // before the current import finishes. Then KDevelop would print a kdevelop.plugins.cmake.debug
    // message "the project is being reloaded, aborting reload!" and ignore the reload request.
    if (data.isOutdated) {
        showConfigureOutdatedMessage(*project);
    }

    if (server) {
        connect(server.data(), &CMakeServer::response, project, [this, project](const QJsonObject& response) {
            if (response[QStringLiteral("type")] == QLatin1String("signal")) {
                if (response[QStringLiteral("name")] == QLatin1String("dirty")) {
                    m_projects[project].server->configure({});
                } else
                    qCDebug(CMAKE) << "unhandled signal response..." << project << response;
            } else if (response[QStringLiteral("type")] == QLatin1String("error")) {
                showConfigureErrorMessage(*project, response[QStringLiteral("errorMessage")].toString());
            } else if (response[QStringLiteral("type")] == QLatin1String("reply")) {
                const auto inReplyTo = response[QStringLiteral("inReplyTo")];
                if (inReplyTo == QLatin1String("configure")) {
                    m_projects[project].server->compute();
                } else if (inReplyTo == QLatin1String("compute")) {
                    m_projects[project].server->codemodel();
                } else if(inReplyTo == QLatin1String("codemodel")) {
                    auto &data = m_projects[project].data;
                    CMakeServerImportJob::processCodeModel(response, data);
                    populateTargets(project->projectItem(), data.targets);
                } else {
                    qCDebug(CMAKE) << "unhandled reply response..." << project << response;
                }
            } else {
                qCDebug(CMAKE) << "unhandled response..." << project << response;
            }
        });
    } else if (!m_projects.contains(project)) {
        auto* reloadTimer = new QTimer(project);
        reloadTimer->setSingleShot(true);
        reloadTimer->setInterval(1000);
        connect(reloadTimer, &QTimer::timeout, this, [project, this]() {
            reload(project->projectItem());
        });
        connect(projectWatcher(project), &KDirWatch::dirty, reloadTimer, [this, project, reloadTimer](const QString &strPath) {
            const auto it = m_projects.constFind(project);
            if (it == m_projects.cend() || !it->data.cmakeFiles.contains(Path{strPath})) {
                return;
            }
            qCDebug(CMAKE) << "eventually starting reload due to change of" << strPath;
            reloadTimer->start();
        });
    }

    auto& projectData = m_projects[project];
    cleanupTestSuites(projectData.testSuites, projectData.testSuiteJobs);

    QVector<CTestSuite*> testSuites;
    QVector<CTestFindJob*> testSuiteJobs;
    for (auto& suite : CTestUtils::createTestSuites(data.testSuites, data.targets, project)) {
        auto* testSuite = suite.release();
        testSuites.append(testSuite);
        auto* job = new CTestFindJob(testSuite);
        connect(job, &KJob::result, this, [this, job, project, testSuite]() {
            if (!job->error()) {
                ICore::self()->testController()->addTestSuite(testSuite);
            }
            m_projects[project].testSuiteJobs.removeOne(job);
        });
        ICore::self()->runController()->registerJob(job);
        testSuiteJobs.append(job);
    }

    projectData = { data, server, std::move(testSuites), std::move(testSuiteJobs) };
    populateTargetsRecursively(project->projectItem(), data.targets);
}

QList< KDevelop::ProjectTargetItem * > CMakeManager::targets(KDevelop::ProjectFolderItem * folder) const
{
    return folder->targetList();
}

QString CMakeManager::name() const
{
    return languageName().str();
}

IndexedString CMakeManager::languageName()
{
    static IndexedString name("CMake");
    return name;
}

KDevelop::ParseJob * CMakeManager::createParseJob(const IndexedString &url)
{
    return new CMakeParseJob(url, this);
}

KDevelop::ICodeHighlighting* CMakeManager::codeHighlighting() const
{
    return m_highlight;
}

bool CMakeManager::removeFilesFromTargets(const QList<ProjectFileItem*> &/*files*/)
{
    return false;
}

bool CMakeManager::addFilesToTarget(const QList< ProjectFileItem* > &/*_files*/, ProjectTargetItem* /*target*/)
{
    return false;
}

KTextEditor::Range CMakeManager::termRangeAtPosition(const KTextEditor::Document* textDocument,
                                                     const KTextEditor::Cursor& position) const
{
    const KTextEditor::Cursor step(0, 1);

    enum ParseState {
        NoChar,
        NonLeadingChar,
        AnyChar,
    };

    ParseState parseState = NoChar;
    KTextEditor::Cursor start = position;
    while (true) {
        const QChar c = textDocument->characterAt(start);
        if (c.isDigit()) {
            parseState = NonLeadingChar;
        } else if (c.isLetter() || c == QLatin1Char('_')) {
            parseState = AnyChar;
        } else {
            // also catches going out of document range, where c is invalid
            break;
        }
        start -= step;
    }

    if (parseState != AnyChar) {
        return KTextEditor::Range::invalid();
    }
    // undo step before last valid char
    start += step;

    KTextEditor::Cursor end = position + step;
    while (true) {
        const QChar c = textDocument->characterAt(end);
        if (!(c.isDigit() || c.isLetter() || c == QLatin1Char('_'))) {
            // also catches going out of document range, where c is invalid
            break;
        }
        end += step;
    }

    return KTextEditor::Range(start, end);
}

void CMakeManager::showConfigureOutdatedMessage(const KDevelop::IProject& project)
{
    const QString messageText = i18n(
        "Configured project '%1' with outdated CMake data."
        " As a result, KDevelop's code understanding may be wrong.\n"
        "\n"
        "To fix this issue, please right-click the project item in the projects tool view and click 'Reload'.",
        project.name());
    showConfigureStatusMessage(project, messageText, Sublime::Message::Warning);
}

void CMakeManager::showConfigureErrorMessage(const IProject& project, const QString& errorMessage)
{
    const QString messageText = i18n(
        "Failed to configure project '%1' (error message: %2)."
        " As a result, KDevelop's code understanding will likely be broken.\n"
        "\n"
        "To fix this issue, please ensure that the project's CMakeLists.txt files"
        " are correct, and KDevelop is configured to use the correct CMake version and settings."
        " Then right-click the project item in the projects tool view and click 'Reload'.",
        project.name(), errorMessage);
    showConfigureStatusMessage(project, messageText, Sublime::Message::Error);
}

void CMakeManager::showConfigureStatusMessage(const IProject& project, const QString& messageText,
                                              Sublime::Message::MessageType messageType)
{
    auto& message = m_configureStatusMessages[&project];
    Q_ASSERT_X(!message, Q_FUNC_INFO, "The previous message must have been discarded earlier.");
    message = new Sublime::Message(messageText, messageType);
    ICore::self()->uiController()->postMessage(message);
}

QPair<QWidget*, KTextEditor::Range> CMakeManager::specialLanguageObjectNavigationWidget(const QUrl& url, const KTextEditor::Cursor& position)
{
    KTextEditor::Range itemRange;
    CMakeNavigationWidget* doc = nullptr;

    KDevelop::TopDUContextPointer top= TopDUContextPointer(KDevelop::DUChain::self()->chainForDocument(url));
    if(top)
    {
        int useAt=top->findUseAt(top->transformToLocalRevision(position));
        if(useAt>=0)
        {
            Use u=top->uses()[useAt];
            doc = new CMakeNavigationWidget(top, u.usedDeclaration(top->topContext()));
            itemRange = u.m_range.castToSimpleRange();
        }
    }

    if (!doc) {
        ICMakeDocumentation* docu=CMake::cmakeDocumentation();
        if( docu )
        {
            const auto* document = ICore::self()->documentController()->documentForUrl(url);
            const auto* textDocument = document->textDocument();
            itemRange = termRangeAtPosition(textDocument, position);
            if (itemRange.isValid()) {
                const auto id = textDocument->text(itemRange);

                if (!id.isEmpty()) {
                    IDocumentation::Ptr desc=docu->description(id, url);
                    if (desc) {
                        doc=new CMakeNavigationWidget(top, desc);
                    }
                }
            }
        }
    }

    return {doc, itemRange};
}

QPair<QString, QString> CMakeManager::cacheValue(KDevelop::IProject* /*project*/, const QString& /*id*/) const
{ return QPair<QString, QString>(); }

void CMakeManager::projectClosing(IProject* p)
{
    const auto it = m_projects.constFind(p);
    if (it != m_projects.cend()) {
        cleanupTestSuites(it->testSuites, it->testSuiteJobs);
        m_projects.erase(it);
    }

    delete m_configureStatusMessages.take(p); // discard the message, because closing its project obsoletes it
}

void CMakeManager::folderAdded(KDevelop::ProjectFolderItem* folder)
{
    populateTargets(folder, m_projects.value(folder->project()).data.targets);
}

ProjectFolderItem* CMakeManager::createFolderItem(IProject* project, const Path& path, ProjectBaseItem* parent)
{
//     TODO: when we have data about targets, use folders with targets or similar
    if (QFile::exists(path.toLocalFile()+QLatin1String("/CMakeLists.txt")))
        return new KDevelop::ProjectBuildFolderItem( project, path, parent );
    else
        return KDevelop::AbstractFileManagerPlugin::createFolderItem(project, path, parent);
}

int CMakeManager::perProjectConfigPages() const
{
    return 1;
}

ConfigPage* CMakeManager::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
#ifdef CMAKEMANAGER_NO_SETTINGS
    Q_UNUSED(number);
    Q_UNUSED(options);
    Q_UNUSED(parent);
    return nullptr;
#else
    if (number == 0) {
        return new CMakePreferences(this, options, parent);
    }
    return nullptr;
#endif
}

void CMakeManager::reloadProjects()
{
    const auto& projects = ICore::self()->projectController()->projects();
    for (IProject* project : projects) {
        if (project->buildSystemManager() == this) {
            CMake::checkForNeedingConfigure(project);
            reload(project->projectItem());
        }
    }
}

CMakeTarget CMakeManager::targetInformation(KDevelop::ProjectTargetItem* item) const
{
    const auto targets = m_projects[item->project()].data.targets[item->parent()->path()];
    for (auto target: targets) {
        if (item->text() == target.name) {
            return target;
        }
    }
    return {};
}

KDevelop::Path CMakeManager::compiler(KDevelop::ProjectTargetItem* item) const
{
    const auto targetInfo = targetInformation(item);
    if (targetInfo.sources.isEmpty()) {
        qCDebug(CMAKE) << "could not find target" << item->text();
        return {};
    }

    const auto info = m_projects[item->project()].data.compilationData.files[targetInfo.sources.constFirst()];
    const auto lang = info.language;
    if (lang.isEmpty()) {
        qCDebug(CMAKE) << "no language for" << item << item->text() << info.defines << targetInfo.sources.constFirst();
        return {};
    }
    const QString var = QLatin1String("CMAKE_") + lang + QLatin1String("_COMPILER");
    const auto ret = CMake::readCacheValues(KDevelop::Path(buildDirectory(item), QStringLiteral("CMakeCache.txt")), {var});
    qCDebug(CMAKE) << "compiler for" << lang << var << ret;
    return KDevelop::Path(ret.value(var));
}

#include "cmakemanager.moc"
#include "moc_cmakemanager.cpp"
