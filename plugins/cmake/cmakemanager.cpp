/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007-2013 Aleix Pol <aleixpol@kde.org>
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

#include "cmakemanager.h"
#include "cmakeedit.h"
#include "cmakeutils.h"
#include "cmakeprojectdata.h"
#include "duchain/cmakeparsejob.h"
#include "cmakeimportjsonjob.h"
#include "debug.h"
#include "settings/cmakepreferences.h"
#include "cmakecodecompletionmodel.h"
#include "cmakenavigationwidget.h"
#include "icmakedocumentation.h"
#include "cmakemodelitems.h"
#include "testing/ctestutils.h"
#include "cmakeserverimportjob.h"
#include "cmakeserver.h"
#include "cmakefileapi.h"
#include "cmakefileapiimportjob.h"

#include <QApplication>
#include <QDir>
#include <QReadWriteLock>
#include <QThread>
#include <QFileSystemWatcher>
#include <QTimer>

#include <KPluginFactory>
#include <QUrl>
#include <QAction>
#include <KMessageBox>
#include <KTextEditor/Document>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <interfaces/idocumentation.h>
#include <util/executecompositejob.h>
#include <language/highlighting/codehighlighting.h>
#include <project/projectmodel.h>
#include <project/helper.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/projectfiltermanager.h>
#include <language/codecompletion/codecompletion.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/use.h>
#include <language/duchain/duchain.h>
#include <makefileresolver/makefileresolver.h>
#include <sublime/message.h>

Q_DECLARE_METATYPE(KDevelop::IProject*)

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(CMakeSupportFactory, "kdevcmakemanager.json", registerPlugin<CMakeManager>(); )

const QString DIALOG_CAPTION = i18n("KDevelop - CMake Support");

CMakeManager::CMakeManager( QObject* parent, const QVariantList& )
    : KDevelop::AbstractFileManagerPlugin( QStringLiteral("kdevcmakemanager"), parent )
    , m_filter( new ProjectFilterManager( this ) )
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
    return m_projects[item->project()].compilationData.files.contains(item->path());
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
    ChooseCMakeInterfaceJob(IProject* project, CMakeManager* manager)
        : ExecuteCompositeJob(manager, {})
        , project(project)
        , manager(manager)
    {
    }

    void start() override {
        auto tryCMakeServer = [this]() {
            qCDebug(CMAKE) << "try cmake server for import";
            server.reset(new CMakeServer(project));
            connect(server.data(), &CMakeServer::connected, this, &ChooseCMakeInterfaceJob::successfulConnection);
            connect(server.data(), &CMakeServer::finished, this, &ChooseCMakeInterfaceJob::failedConnection);
        };
        if (CMake::FileApi::supported(CMake::currentCMakeExecutable(project).toLocalFile())) {
            qCDebug(CMAKE) << "Using cmake-file-api for import of" << project->path();
            addSubjob(manager->builder()->configure(project));
            auto* importJob = new CMake::FileApi::ImportJob(project, this);
            connect(importJob, &CMake::FileApi::ImportJob::dataAvailable, this, [this, tryCMakeServer](const CMakeProjectData& data) {
                if (!data.compilationData.isValid) {
                    tryCMakeServer();
                } else {
                    manager->integrateData(data, project);
                }
            });
            addSubjob(importJob);
            ExecuteCompositeJob::start();
        } else {
            tryCMakeServer();
        }
    }

private:
    void successfulConnection() {
        auto job = new CMakeServerImportJob(project, server, this);
        connect(job, &CMakeServerImportJob::result, this, [this, job](){
            if (job->error() == 0) {
                manager->integrateData(job->projectData(), job->project());
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

    QSharedPointer<CMakeServer> server;
    IProject* const project;
    CMakeManager* const manager;
};

KJob* CMakeManager::createImportJob(ProjectFolderItem* item)
{
    auto project = item->project();

    auto job = new ChooseCMakeInterfaceJob(project, this);
    connect(job, &KJob::result, this, [this, job, project](){
        if (job->error() != 0) {
            qCWarning(CMAKE) << "couldn't load project successfully" << project->name() << job->error() << job->errorText();
            m_projects.remove(project);
            showConfigureErrorMessage(project->name(), job->errorText());
        }
    });

    const QList<KJob*> jobs = {
        job,
        KDevelop::AbstractFileManagerPlugin::createImportJob(item) // generate the file system listing
    };

    Q_ASSERT(!jobs.contains(nullptr));
    auto* composite = new ExecuteCompositeJob(this, jobs);
//     even if the cmake call failed, we want to load the project so that the project can be worked on
    composite->setAbortOnError(false);
    return composite;
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
    const auto & data = m_projects[item->project()].compilationData;
    QHash<KDevelop::Path, CMakeFile>::const_iterator it = data.files.constFind(item->path());

    if (it == data.files.constEnd()) {
        // if the item path contains a symlink, then we will not find it in the lookup table
        // as that only only stores canonicalized paths. Thus, we fallback to
        // to the canonicalized path and see if that brings up any matches
        const auto canonicalized = Path(QFileInfo(item->path().toLocalFile()).canonicalFilePath());
        it = data.files.constFind(canonicalized);
    }

    if (it != data.files.constEnd()) {
        return *it;
    } else {
        // otherwise look for siblings and use the include paths of any we find
        const Path folder = item->folder() ? item->path() : item->path().parent();

        for( it = data.files.constBegin(); it != data.files.constEnd(); ++it) {
            if (folder.isDirectParentOf(it.key())) {
                return *it;
            }
        }
    }

    // last-resort fallback: bubble up the parent chain, and keep looking for include paths
    if (auto parent = item->parent()) {
        return fileInformation(parent);
    }

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
    if (!project->isReady())
        return false;

    KJob *job = createImportJob(folder);
    project->setReloadJob(job);
    ICore::self()->runController()->registerJob( job );
    if (folder == project->projectItem()) {
        connect(job, &KJob::finished, this, [project](KJob* job) {
            if (job->error())
                return;

            emit KDevelop::ICore::self()->projectController()->projectConfigurationChanged(project);
            KDevelop::ICore::self()->projectController()->reparseProject(project, true);
        });
    }

    return true;
}

static void populateTargets(ProjectFolderItem* folder, const QHash<KDevelop::Path, QVector<CMakeTarget>>& targets)
{
    static QSet<QString> standardTargets = {
        QStringLiteral("edit_cache"), QStringLiteral("rebuild_cache"),
        QStringLiteral("list_install_components"),
        QStringLiteral("test"), //not really standard, but applicable for make and ninja
        QStringLiteral("install")

    };
    QList<CMakeTarget> dirTargets = kFilter<QList<CMakeTarget>>(targets[folder->path()], [](const CMakeTarget& target) -> bool {
        return target.type != CMakeTarget::Custom ||
              (!target.name.endsWith(QLatin1String("_automoc"))
            && !target.name.endsWith(QLatin1String("_autogen"))
            && !standardTargets.contains(target.name)
            && !target.name.startsWith(QLatin1String("install/"))
              );
    });

    const auto tl = folder->targetList();
    for (ProjectTargetItem* item : tl) {
        const auto idx = kIndexOf(dirTargets, [item](const CMakeTarget& target) { return target.name == item->text(); });
        if (idx < 0) {
            delete item;
        } else {
            auto cmakeItem = dynamic_cast<CMakeTargetItem*>(item);
            if (cmakeItem)
                cmakeItem->setBuiltUrl(dirTargets[idx].artifacts.value(0));
            dirTargets.removeAt(idx);
        }
    }

    for (const auto& target : qAsConst(dirTargets)) {
        switch(target.type) {
            case CMakeTarget::Executable:
                new CMakeTargetItem(folder, target.name, target.artifacts.value(0));
                break;
            case CMakeTarget::Library:
                new ProjectLibraryTargetItem(folder->project(), target.name, folder);
                break;
            case CMakeTarget::Custom:
                new ProjectTargetItem(folder->project(), target.name, folder);
                break;
        }
    }

    const auto folderItems = folder->folderList();
    for (ProjectFolderItem* children : folderItems) {
        populateTargets(children, targets);
    }
}

void CMakeManager::integrateData(const CMakeProjectData &data, KDevelop::IProject* project)
{
    if (data.m_server) {
        connect(data.m_server.data(), &CMakeServer::response, project, [this, project](const QJsonObject& response) {
            serverResponse(project, response);
        });
    }
    m_projects[project] = data;

    populateTargets(project->projectItem(), data.targets);
    CTestUtils::createTestSuites(data.m_testSuites, data.targets, project);
}

void CMakeManager::serverResponse(KDevelop::IProject* project, const QJsonObject& response)
{
    if (response[QStringLiteral("type")] == QLatin1String("signal")) {
        if (response[QStringLiteral("name")] == QLatin1String("dirty")) {
            m_projects[project].m_server->configure({});
        } else
            qCDebug(CMAKE) << "unhandled signal response..." << project << response;
    } else if (response[QStringLiteral("type")] == QLatin1String("error")) {
        showConfigureErrorMessage(project->name(), response[QStringLiteral("errorMessage")].toString());
    } else if (response[QStringLiteral("type")] == QLatin1String("reply")) {
        const auto inReplyTo = response[QStringLiteral("inReplyTo")];
        if (inReplyTo == QLatin1String("configure")) {
            m_projects[project].m_server->compute();
        } else if (inReplyTo == QLatin1String("compute")) {
            m_projects[project].m_server->codemodel();
        } else if(inReplyTo == QLatin1String("codemodel")) {
            auto &data = m_projects[project];
            CMakeServerImportJob::processCodeModel(response, data);
            populateTargets(project->projectItem(), data.targets);
        } else {
            qCDebug(CMAKE) << "unhandled reply response..." << project << response;
        }
    } else {
        qCDebug(CMAKE) << "unhandled response..." << project << response;
    }
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

void CMakeManager::showConfigureErrorMessage(const QString& projectName, const QString& errorMessage) const
{
    if (!QApplication::activeWindow()) {
        // Do not show a message box if there is no active window in order not to block unit tests.
        return;
    }
    const QString messageText = i18n(
        "Failed to configure project '%1' (error message: %2)."
        " As a result, KDevelop's code understanding will likely be broken.\n"
        "\n"
        "To fix this issue, please ensure that the project's CMakeLists.txt files"
        " are correct, and KDevelop is configured to use the correct CMake version and settings."
        " Then right-click the project item in the projects tool view and click 'Reload'.",
        projectName, errorMessage);
    auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
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
    m_projects.remove(p);
}

ProjectFilterManager* CMakeManager::filterManager() const
{
    return m_filter;
}

void CMakeManager::folderAdded(KDevelop::ProjectFolderItem* folder)
{
    populateTargets(folder, m_projects[folder->project()].targets);
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
    if (number == 0) {
        return new CMakePreferences(this, options, parent);
    }
    return nullptr;
}

void CMakeManager::reloadProjects()
{
    const auto& projects = m_projects.keys();
    for (IProject* project : projects) {
        CMake::checkForNeedingConfigure(project);
        reload(project->projectItem());
    }
}

CMakeTarget CMakeManager::targetInformation(KDevelop::ProjectTargetItem* item) const
{
    const auto targets = m_projects[item->project()].targets[item->parent()->path()];
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

    const auto info = m_projects[item->project()].compilationData.files[targetInfo.sources.constFirst()];
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
