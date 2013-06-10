/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007-2009 Aleix Pol <aleixpol@kde.org>
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

#include <QList>
#include <QVector>
#include <QDomDocument>
#include <QDir>
#include <QQueue>
#include <QThread>
#include <QFileSystemWatcher>
#include <QTimer>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KAboutData>
#include <KDialog>
#include <kparts/mainwindow.h>
#include <KUrl>
#include <KAction>
#include <KMessageBox>
#include <ktexteditor/document.h>
#include <KStandardDirs>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <project/projectmodel.h>
#include <project/importprojectjob.h>
#include <project/helper.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/duchain.h>
#include <language/duchain/dumpchain.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/codecompletion/codecompletion.h>

#include "cmakemodelitems.h"
#include "cmakenavigationwidget.h"
#include "cmakecachereader.h"
#include "cmakeastvisitor.h"
#include "cmakeprojectvisitor.h"
#include "cmakeexport.h"
#include "cmakecodecompletionmodel.h"
#include "cmakeutils.h"
#include "cmaketypes.h"
#include "parser/cmakeparserutils.h"
#include <generationexpressionsolver.h>
#include "icmakedocumentation.h"
#include "testing/ctestutils.h"

#ifdef CMAKEDEBUGVISITOR
#include "cmakedebugvisitor.h"
#endif

#include "ui_cmakepossibleroots.h"
#include <language/duchain/use.h>
#include <interfaces/idocumentation.h>
#include "cmakeprojectdata.h"

#include <language/highlighting/codehighlighting.h>
#include <interfaces/iruncontroller.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/vcsjob.h>
#include <project/interfaces/iprojectbuilder.h>
#include <util/environmentgrouplist.h>

Q_DECLARE_METATYPE(KDevelop::IProject*);

using namespace KDevelop;

K_PLUGIN_FACTORY(CMakeSupportFactory, registerPlugin<CMakeManager>(); )
K_EXPORT_PLUGIN(CMakeSupportFactory(KAboutData("kdevcmakemanager","kdevcmake", ki18n("CMake Manager"), "0.1", ki18n("Support for managing CMake projects"), KAboutData::License_GPL)))

namespace {

const QString DIALOG_CAPTION = i18n("KDevelop - CMake Support");

QString fetchBuildDir(KDevelop::IProject* project)
{
    Q_ASSERT(project);
    return CMake::currentBuildDir(project).toLocalFile(KUrl::AddTrailingSlash);
}

QString fetchInstallDir(KDevelop::IProject* project)
{
    Q_ASSERT(project);
    return CMake::currentInstallDir(project).toLocalFile(KUrl::AddTrailingSlash);
}

inline QString replaceBuildDir(QString in, QString buildDir)
{
    return in.replace("#[bin_dir]", buildDir);
}

inline  QString replaceInstallDir(QString in, QString installDir)
{
    return in.replace("#[install_dir]", installDir);
}

KUrl::List resolveSystemDirs(KDevelop::IProject* project, const QStringList& dirs)
{
    QString buildDir = fetchBuildDir(project);
    QString installDir = fetchInstallDir(project);

    KUrl::List newList;
    foreach(const QString& _s, dirs)
    {
        QString s=_s;
        if(s.startsWith(QString::fromUtf8("#[bin_dir]")))
        {
            s= replaceBuildDir(s, buildDir);
        }
        else if(s.startsWith(QString::fromUtf8("#[install_dir]")))
        {
            s= replaceInstallDir(s, installDir);
        }
        KUrl d(s);
        d.cleanPath();
        d.adjustPath(KUrl::AddTrailingSlash);
//         kDebug(9042) << "resolved" << _s << "to" << d;

        if (!newList.contains(d))
        {
            newList.append(d);
        }
    }
    return newList;
}

void eatLeadingWhitespace(KTextEditor::Document* doc, KTextEditor::Range& eater, const KTextEditor::Range& bounds)
{
    QString text = doc->text(KTextEditor::Range(bounds.start(), eater.start()));
    int newStartLine = eater.start().line(), pos = text.length() - 2; //pos = index before eater.start
    while (pos > 0)
    {
        if (text[pos] == '\n')
            --newStartLine;
        else if (!text[pos].isSpace())
        {
            ++pos;
            break;
        }
        --pos;
    }
    int lastNewLinePos = text.lastIndexOf('\n', pos - 1);
    int newStartCol = lastNewLinePos == -1 ? eater.start().column() + pos :
                                             pos - lastNewLinePos - 1;
    eater.start().setLine(newStartLine);
    eater.start().setColumn(newStartCol);
}

KTextEditor::Range rangeForText(KTextEditor::Document* doc, const KTextEditor::Range& r, const QString& name)
{
    QString txt=doc->text(r);
    QRegExp match("([\\s]|^)(\\./)?"+QRegExp::escape(name));
    int namepos = match.indexIn(txt);
    int length = match.cap(0).size();
    
    if(namepos == -1)
        return KTextEditor::Range::invalid();
    //QRegExp doesn't support lookbehind asserts, and \b isn't good enough
    //so either match "^" or match "\s" and then +1 here
    if (txt[namepos].isSpace()) {
        ++namepos;
        --length;
    }
    
    KTextEditor::Cursor c(r.start());
    c.setLine(c.line() + txt.left(namepos).count('\n'));
    int lastNewLinePos = txt.lastIndexOf('\n', namepos);
    if (lastNewLinePos < 0)
        c.setColumn(r.start().column() + namepos);
    else
        c.setColumn(namepos - lastNewLinePos - 1);
    
    return KTextEditor::Range(c, KTextEditor::Cursor(c.line(), c.column()+length));
}

bool followUses(KTextEditor::Document* doc, RangeInRevision r, const QString& name, const KUrl& lists, bool add, const QString& replace)
{
    bool ret=false;
    KTextEditor::Range rx;
    if(!add)
        rx=rangeForText(doc, r.castToSimpleRange().textRange(), name);
    
    if(!add && rx.isValid())
    {
        if(replace.isEmpty())
        {
            eatLeadingWhitespace(doc, rx, r.castToSimpleRange().textRange());
            doc->removeText(rx);
        }
        else
            doc->replaceText(rx, replace);
        
        ret=true;
    }
    else
    {
        const IndexedString idxLists(lists);
        KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
        KDevelop::ReferencedTopDUContext topctx=DUChain::self()->chainForDocument(idxLists);
        QList<Declaration*> decls;
        for(int i=0; i<topctx->usesCount(); i++)
        {
            Use u = topctx->uses()[i];

            if(!r.contains(u.m_range))
                continue; //We just want the uses in the range, not the whole file

            Declaration* d=u.usedDeclaration(topctx);

            if(d && d->topContext()->url()==idxLists)
                decls += d;
        }

        if(add && decls.isEmpty())
        {
            doc->insertText(r.castToSimpleRange().textRange().start(), ' '+name);
            ret=true;
        }
        else foreach(Declaration* d, decls)
        {
            r.start=d->range().end;

            for(int lineNum = r.start.line; lineNum <= r.end.line; lineNum++)
            {
                int endParenIndex = doc->line(lineNum).indexOf(')');
                if(endParenIndex >= 0) {
                    r.end = CursorInRevision(lineNum, endParenIndex);
                    break;
                }
            }

            if(!r.isEmpty())
            {
                ret = ret || followUses(doc, r, name, lists, add, replace);
            }
        }
    }
    return ret;
}

QString dotlessRelativeUrl(const KUrl& baseUrl, const KUrl& url)
{
    QString dotlessRelative = KUrl::relativeUrl(baseUrl, url);
    if (dotlessRelative.startsWith("./"))
        dotlessRelative.remove(0, 2);
    return dotlessRelative;
}

QString relativeToLists(const QString& listsPath, const KUrl& url)
{
    KUrl listsFolder = KUrl(KUrl(listsPath).directory(KUrl::AppendTrailingSlash));
    QString relative = dotlessRelativeUrl(listsFolder, url);
    return relative;
}

KUrl afterMoveUrl(const KUrl& origUrl, const KUrl& movedOrigUrl, const KUrl& movedNewUrl)
{
    QString difference = dotlessRelativeUrl(movedOrigUrl, origUrl);
    KUrl newUrl(movedNewUrl);
    newUrl.addPath(difference);
    return newUrl;
}

QString itemListspath(const ProjectBaseItem* item)
{
    const DescriptorAttatched *desc = 0;
    if (item->parent()->target())
        desc = dynamic_cast<const DescriptorAttatched*>(item->parent());
    else if (item->type() == ProjectBaseItem::BuildFolder)
        desc = dynamic_cast<const DescriptorAttatched*>(item);

    if (!desc)
        return QString();
    return desc->descriptor().filePath;
}

bool itemAffected(const ProjectBaseItem *item, const KUrl &changeUrl)
{
    QString listsPath = itemListspath(item);
    if (listsPath.isEmpty())
        return false;

    KUrl listsFolder(KUrl(listsPath).directory(KUrl::AppendTrailingSlash));
    //Who thought it was a good idea to have KUrl::isParentOf return true if the urls are equal?
    return listsFolder.QUrl::isParentOf(changeUrl);
}

QList<ProjectBaseItem*> cmakeListedItemsAffectedByUrlChange(const IProject *proj, const KUrl &url, KUrl rootUrl = KUrl())
{
    if (rootUrl.isEmpty())
        rootUrl = url;

    QList<ProjectBaseItem*> dirtyItems;

    QList<ProjectBaseItem*> sameUrlItems = proj->itemsForUrl(url);
    foreach(ProjectBaseItem *sameUrlItem, sameUrlItems)
    {
        if (itemAffected(sameUrlItem, rootUrl))
            dirtyItems.append(sameUrlItem);

        foreach(ProjectBaseItem* childItem, sameUrlItem->children())
            dirtyItems.append(cmakeListedItemsAffectedByUrlChange(childItem->project(), childItem->url(), rootUrl));
    }
    return dirtyItems;
}

QList<ProjectBaseItem*> cmakeListedItemsAffectedByItemsChanged(const QList<ProjectBaseItem*> &items)
{
    QList<ProjectBaseItem*> dirtyItems;
    foreach(ProjectBaseItem *item, items)
        dirtyItems.append(cmakeListedItemsAffectedByUrlChange(item->project(), item->url()));
    return dirtyItems;
}

bool changesWidgetRenameFolder(const CMakeFolderItem *folder, const KUrl &newUrl, ApplyChangesWidget *widget)
{
    QString lists = folder->descriptor().filePath;
    widget->addDocuments(IndexedString(lists));
    QString relative(relativeToLists(lists, newUrl));
    KTextEditor::Range range = folder->descriptor().argRange().castToSimpleRange().textRange();
    return widget->document()->replaceText(range, relative);
}

bool changesWidgetRemoveCMakeFolder(const CMakeFolderItem *folder, ApplyChangesWidget *widget)
{
    widget->addDocuments(IndexedString(folder->descriptor().filePath));
    KTextEditor::Range range = folder->descriptor().range().castToSimpleRange().textRange();
    return widget->document()->removeText(range);
}

bool changesWidgetAddFolder(const KUrl &folderUrl, const CMakeFolderItem *toFolder, ApplyChangesWidget *widget)
{
    QString lists(toFolder->url().path(KUrl::AddTrailingSlash).append("CMakeLists.txt"));
    QString relative(relativeToLists(lists, folderUrl));
    if (relative.endsWith('/'))
        relative.chop(1);
    QString insert = QString("add_subdirectory(%1)").arg(relative);
    widget->addDocuments(IndexedString(lists));
    return widget->document()->insertLine(widget->document()->lines(), insert);
}

bool changesWidgetMoveTargetFile(const ProjectBaseItem *file, const KUrl &newUrl, ApplyChangesWidget *widget)
{
    const DescriptorAttatched *desc = dynamic_cast<const DescriptorAttatched*>(file->parent());
    if (!desc || desc->descriptor().arguments.isEmpty()) {
        return false;
    }
    RangeInRevision targetRange(desc->descriptor().arguments.first().range().end, desc->descriptor().argRange().end);
    QString listsPath = desc->descriptor().filePath;
    QString newRelative = relativeToLists(listsPath, newUrl);
    QString oldRelative = relativeToLists(listsPath, file->url());
    widget->addDocuments(IndexedString(listsPath));
    return followUses(widget->document(), targetRange, oldRelative, listsPath, false, newRelative);
}

bool changesWidgetAddFileToTarget(const ProjectFileItem *item, const ProjectTargetItem *target, ApplyChangesWidget *widget)
{
    const DescriptorAttatched *desc = dynamic_cast<const DescriptorAttatched*>(target);
    if (!desc || desc->descriptor().arguments.isEmpty()) {
        return false;
    }
    RangeInRevision targetRange(desc->descriptor().arguments.first().range().end, desc->descriptor().range().end);
    QString lists = desc->descriptor().filePath;
    QString relative = relativeToLists(lists, item->url());
    widget->addDocuments(IndexedString(lists));
    return followUses(widget->document(), targetRange, relative, lists, true, QString());
}

bool changesWidgetRemoveFileFromTarget(const ProjectBaseItem *item, ApplyChangesWidget *widget)
{
    const DescriptorAttatched *desc = dynamic_cast<const DescriptorAttatched*>(item->parent());
    if (!desc || desc->descriptor().arguments.isEmpty()) {
        return false;
    }
    RangeInRevision targetRange(desc->descriptor().arguments.first().range().end, desc->descriptor().range().end);
    QString lists = desc->descriptor().filePath;
    QString relative = relativeToLists(lists, item->url());
    widget->addDocuments(IndexedString(lists));
    return followUses(widget->document(), targetRange, relative, lists, false, QString());
}

bool changesWidgetRemoveItems(const QList<ProjectBaseItem*> &items, ApplyChangesWidget *widget)
{
    foreach(ProjectBaseItem *item, items)
    {
        CMakeFolderItem *folder = dynamic_cast<CMakeFolderItem*>(item);
        if (folder && !changesWidgetRemoveCMakeFolder(folder, widget))
            return false;
        else if (item->parent()->target() && !changesWidgetRemoveFileFromTarget(item, widget))
            return false;
    }
    return true;
}

bool changesWidgetRemoveFilesFromTargets(const QList<ProjectFileItem*> &files, ApplyChangesWidget *widget)
{
    foreach(ProjectBaseItem *file, files)
    {
        Q_ASSERT(file->parent()->target());
        if (!changesWidgetRemoveFileFromTarget(file, widget))
            return false;
    }
    return true;
}

bool changesWidgetAddFilesToTarget(const QList<ProjectFileItem*> &files, const ProjectTargetItem* target, ApplyChangesWidget *widget)
{
    foreach(ProjectFileItem *file, files)
    {
        if (!changesWidgetAddFileToTarget(file, target, widget))
            return false;
    }
    return true;
}

CMakeFolderItem* nearestCMakeFolder(ProjectBaseItem* item)
{
    while(!dynamic_cast<CMakeFolderItem*>(item) && item)
        item = item->parent();

    return dynamic_cast<CMakeFolderItem*>(item);
}

template<class T>
QList<ProjectBaseItem*> castToBase(const QList<T*>& ptrs)
{
    QList<ProjectBaseItem*> ret;
    foreach(ProjectBaseItem* ptr, ptrs)
        ret += ptr;
    
    return ret;
}

}

CMakeManager::CMakeManager( QObject* parent, const QVariantList& )
    : KDevelop::IPlugin( CMakeSupportFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ILanguageSupport )
    KDEV_USE_EXTENSION_INTERFACE( ICMakeManager)

    if (hasError()) {
        return;
    }

    m_highlight = new KDevelop::CodeHighlighting(this);

    new CodeCompletion(this, new CMakeCodeCompletionModel(this), name());
    
    connect(ICore::self()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)), SLOT(projectClosing(KDevelop::IProject*)));

    m_fileSystemChangeTimer = new QTimer(this);
    m_fileSystemChangeTimer->setSingleShot(true);
    m_fileSystemChangeTimer->setInterval(100);
    connect(m_fileSystemChangeTimer,SIGNAL(timeout()),SLOT(filesystemBuffererTimeout()));
}

bool CMakeManager::hasError() const
{
    return KStandardDirs::findExe("cmake").isEmpty();
}

QString CMakeManager::errorDescription() const
{
    return hasError() ? i18n("cmake is not installed") : QString();
}

CMakeManager::~CMakeManager()
{}

KUrl CMakeManager::buildDirectory(KDevelop::ProjectBaseItem *item) const
{
    CMakeFolderItem *fi=dynamic_cast<CMakeFolderItem*>(item);
    KUrl ret;
    ProjectBaseItem* parent = fi ? fi->formerParent() : item->parent();
    if (parent)
        ret=buildDirectory(parent);
    else
        ret=CMake::currentBuildDir(item->project());
    
    if(fi)
        ret.addPath(fi->buildDir());
    return ret;
}

KDevelop::ReferencedTopDUContext CMakeManager::initializeProject(CMakeFolderItem* rootFolder)
{
    KDevelop::IProject* project = rootFolder->project();
    QMutexLocker locker(&m_reparsingMutex);
    KUrl baseUrl=CMake::projectRoot(project);
    
    QPair<VariableMap,QStringList> initials = CMakeParserUtils::initialVariables();
    CMakeProjectData* data = &m_projectsData[project];
    
    data->clear();
    data->modulePath=initials.first["CMAKE_MODULE_PATH"];
    data->vm=initials.first;
    data->vm.insert("CMAKE_SOURCE_DIR", QStringList(baseUrl.toLocalFile(KUrl::RemoveTrailingSlash)));
    
    KUrl cachefile=buildDirectory(project->projectItem());
    cachefile.addPath("CMakeCache.txt");
    data->cache=readCache(cachefile);

    KDevelop::ReferencedTopDUContext buildstrapContext;
    {
        KUrl buildStrapUrl = baseUrl;
        buildStrapUrl.addPath("buildstrap");
        DUChainWriteLocker lock(DUChain::lock());
        
        buildstrapContext = DUChain::self()->chainForDocument(buildStrapUrl);
        
        if(buildstrapContext) {
            buildstrapContext->clearLocalDeclarations();
            buildstrapContext->clearImportedParentContexts();
            buildstrapContext->deleteChildContextsRecursively();
        }else{
            IndexedString idxpath(buildStrapUrl);
            buildstrapContext=new TopDUContext(idxpath, RangeInRevision(0,0, 0,0),
                                               new ParsingEnvironmentFile(idxpath));
            DUChain::self()->addDocumentChain(buildstrapContext);
        }
        
        Q_ASSERT(buildstrapContext);
    }
    ReferencedTopDUContext ref=buildstrapContext;
    foreach(const QString& script, initials.second)
    {
        ref = includeScript(CMakeProjectVisitor::findFile(script, m_projectsData[project].modulePath, QStringList()), project, baseUrl.toLocalFile(), ref);
    }
    
    //Initialize parent parts of the project that don't belong to the tree (because it's a partial import)
    if(baseUrl.isParentOf(project->folder()) && baseUrl!=project->folder())
    {
        QList<KUrl> toimport;
        toimport += baseUrl;
        QStringList includes;
        while(!toimport.isEmpty()) {
            KUrl script = toimport.takeFirst(), currentDir=script;
            script.addPath("CMakeLists.txt");
            
            QString dir = currentDir.toLocalFile();
            ref = includeScript(script.toLocalFile(), project, dir, ref);
            Q_ASSERT(ref);
            includes << data->properties[DirectoryProperty][dir]["INCLUDE_DIRECTORIES"];
            rootFolder->defineVariables(data->properties[DirectoryProperty][dir]["COMPILE_DEFINITIONS"]);
            
            foreach(const Subdirectory& s, data->subdirectories) {
                KUrl candidate = currentDir;
                candidate.addPath(s.name);
                
                if(candidate.isParentOf(project->folder()))
                    toimport += candidate;
            }
        }
        rootFolder->setIncludeDirectories(includes);
        rootFolder->setBuildDir(KUrl::relativeUrl(baseUrl, project->folder()));
    }
    return ref;
}

KDevelop::ProjectFolderItem* CMakeManager::import( KDevelop::IProject *project )
{
    kDebug(9042) << "== migrating cmake settings";
    CMake::attemptMigrate(project);
    kDebug(9042) << "== completed cmake migration";

    kDebug(9042) << "== updating cmake settings from model";
    int buildDirCount = CMake::buildDirCount(project);
    for( int i = 0; i < buildDirCount; ++i )
        CMake::updateConfig( project, i );
    kDebug(9042) << "== completed updating cmake settings";

    CMakeFolderItem* m_rootItem=0;
    KUrl cmakeInfoFile(project->projectFileUrl());
    cmakeInfoFile = cmakeInfoFile.upUrl();
    cmakeInfoFile.addPath("CMakeLists.txt");

    KUrl folderUrl=project->folder();
    kDebug(9042) << "file is" << cmakeInfoFile.toLocalFile();

    if ( !cmakeInfoFile.isLocalFile() )
    {
        kWarning() << "error. not a local file. CMake support doesn't handle remote projects";
    }
    else
    {
        if(CMake::hasProjectRootRelative(project))
        {
            QString relative=CMake::projectRootRelative(project);
            folderUrl.cd(relative);
        }
        else
        {
            KDialog chooseRoot;
            QWidget *e=new QWidget(&chooseRoot);
            Ui::CMakePossibleRoots ui;
            ui.setupUi(e);
            chooseRoot.setMainWidget(e);
            for(KUrl aux=folderUrl; QFile::exists(aux.toLocalFile()+"/CMakeLists.txt"); aux=aux.upUrl())
                ui.candidates->addItem(aux.toLocalFile());

            if(ui.candidates->count()>1)
            {
                connect(ui.candidates, SIGNAL(itemActivated(QListWidgetItem*)), &chooseRoot,SLOT(accept()));
                ui.candidates->setMinimumSize(384,192);
                int a=chooseRoot.exec();
                if(!a || !ui.candidates->currentItem())
                {
                    return 0;
                }
                KUrl choice=KUrl(ui.candidates->currentItem()->text());
                CMake::setProjectRootRelative(project, KUrl::relativeUrl(folderUrl, choice));
                folderUrl=choice;
            }
            else
            {
                CMake::setProjectRootRelative(project, "./");
            }
        }

        m_rootItem = new CMakeFolderItem(project, project->folder(), QString(), 0 );

        KUrl cachefile=buildDirectory(m_rootItem);
        if( cachefile.isEmpty() ) {
            CMake::checkForNeedingConfigure(project);
        }
        cachefile.addPath("CMakeCache.txt");
        
        QFileSystemWatcher* w = new QFileSystemWatcher(project);
        w->setObjectName(project->name()+"_ProjectWatcher");
        w->addPath(cachefile.toLocalFile());
        connect(w, SIGNAL(fileChanged(QString)), SLOT(dirtyFile(QString)));
        connect(w, SIGNAL(directoryChanged(QString)), SLOT(directoryChanged(QString)));
        m_watchers[project] = w;
        Q_ASSERT(m_rootItem->rowCount()==0);
    }
    return m_rootItem;
}


KDevelop::ReferencedTopDUContext CMakeManager::includeScript(const QString& file,
                                                        KDevelop::IProject * project, const QString& dir, ReferencedTopDUContext parent)
{
    m_watchers[project]->addPath(file);
    QString profile = CMake::currentEnvironment(project);
    const KDevelop::EnvironmentGroupList env( KGlobal::config() );
    return CMakeParserUtils::includeScript( file, parent, &m_projectsData[project], dir, env.variables(profile));
}



QSet<QString> filterFiles(const QStringList& orig)
{
    QSet<QString> ret;
    foreach(const QString& str, orig)
    {
        ///@todo This filter should be configurable, and filtering should be done on a manager-independent level
        if (str.endsWith(QLatin1Char('~')) || str.endsWith(QLatin1String(".bak")))
            continue;

        ret.insert(str);
    }
    return ret;
}

QStringList resolvePaths(const KUrl& baseUrl, const QStringList& pathsToResolve) {
    QStringList resolvedPaths;
    foreach(const QString& pathToResolve, pathsToResolve)
    {
        QString dir(pathToResolve);
        if(!pathToResolve.startsWith("#["))
        {
            if(KUrl( pathToResolve ).isRelative())
            {
                KUrl path(baseUrl);
                path.addPath(pathToResolve);
                dir=path.toLocalFile();
            }

            KUrl simp(dir); //We use this to simplify dir
            simp.cleanPath();
            dir=simp.toLocalFile();
        }
        resolvedPaths.append(dir);
    }
    return resolvedPaths;
}

QList<KDevelop::ProjectFolderItem*> CMakeManager::parse( KDevelop::ProjectFolderItem* item )
{
    Q_ASSERT(isReloading(item->project()));
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( item );

    {
        QMutexLocker locker(&m_dirWatchersMutex);
        m_watchers[item->project()]->addPath(item->url().toLocalFile());
    }
    
    KUrl cmakeListsPath(item->url());
    cmakeListsPath.addPath("CMakeLists.txt");
    
    if(folder && QFile::exists(cmakeListsPath.toLocalFile()))
    {
        kDebug(9042) << "parse:" << folder->url();
        
        KDevelop::ReferencedTopDUContext curr;
        if(item==item->project()->projectItem())
            curr=initializeProject(folder);
        else
            curr=folder->formerParent()->topDUContext();
        
        kDebug(9042) << "Adding cmake: " << cmakeListsPath << " to the model";

        QString binDir=KUrl::relativePath(folder->project()->projectItem()->url().toLocalFile(), folder->url().toLocalFile());
        if(binDir.startsWith("./"))
            binDir=binDir.remove(0, 2);
        
        CMakeProjectData& data=m_projectsData[item->project()];

//         kDebug(9042) << "currentBinDir" << KUrl(data.vm.value("CMAKE_BINARY_DIR")[0]) << data.vm.value("CMAKE_CURRENT_BINARY_DIR");

    #ifdef CMAKEDEBUGVISITOR
        CMakeAstDebugVisitor dv;
        dv.walk(cmakeListsPath.toLocalFile(), f, 0);
    #endif

        data.vm.pushScope();
        ReferencedTopDUContext ctx = includeScript(cmakeListsPath.toLocalFile(), folder->project(),
                                                   item->url().toLocalFile(KUrl::KUrl::RemoveTrailingSlash), curr);
        folder->setTopDUContext(ctx);
       /*{
        kDebug() << "dumpiiiiiing" << folder->url();
        KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
        KDevelop::dumpDUContext(v.context(), false);
        }*/

        QList<KDevelop::ProjectFolderItem*> folderList;
        QStringList alreadyAdded;
        deleteAllLater(castToBase(folder->cleanupBuildFolders(data.subdirectories)));
        foreach (const Subdirectory& subf, data.subdirectories)
        {
            if(subf.name.isEmpty() || alreadyAdded.contains(subf.name)) //empty case would not be necessary if we didn't process the wrong lines
                continue;
            
            KUrl path(subf.name);
            if(path.isRelative())
            {
                path=folder->url();
                path.addPath(subf.name);
            }
            path.adjustPath(KUrl::AddTrailingSlash);
            
            if(QDir(path.toLocalFile()).exists())
            {
                alreadyAdded.append(subf.name);
                CMakeFolderItem* parent=folder;
                if(path.upUrl()!=folder->url())
                    parent=0;

                CMakeFolderItem* a = 0;
                ProjectFolderItem* ff = folder->folderNamed(subf.name);
                if(ff)
                {
                    if(ff->type()!=ProjectBaseItem::BuildFolder)
                        deleteItemLater(ff);
                    else {
                        a = static_cast<CMakeFolderItem*>(ff);
                        m_cleanupItems.removeAll(ff);
                    }
                    
                }
                if(!a)
                    a = new CMakeFolderItem( folder->project(), path, subf.build_dir, parent );
                else
                    a->setUrl(path);
                
//                 kDebug() << "folder: " << a << a->index();
                folderList.append( a );
                
                if(!parent) {
                    m_pending[path]=a;
                    a->setFormerParent(folder);
                }

                DescriptorAttatched* datt=static_cast<DescriptorAttatched*>(a);
                datt->setDescriptor(subf.desc);
            }
        }

//         if(folderList.isEmpty() && path.isParentOf(item->url()))
//             kDebug() << "poor guess";

        QStringList directories;
        QString dir = folder->url().toLocalFile(KUrl::RemoveTrailingSlash);
        if(data.vm.value("CMAKE_INCLUDE_CURRENT_DIR").join(QString())=="ON") {
            directories += dir;
            directories += buildDirectory(folder).toLocalFile();
        }
        directories += resolvePaths(folder->url(), data.properties[DirectoryProperty][dir]["INCLUDE_DIRECTORIES"]);
        directories.removeAll(QString());
        folder->setIncludeDirectories(directories);
//             kDebug(9042) << "setting include directories: " << folder->url() << directories << "result: " << includeDirectories(folder);
        folder->defineVariables(data.properties[DirectoryProperty][dir]["COMPILE_DEFINITIONS"]);

        deleteAllLater(castToBase(folder->cleanupTargets(data.targets)));
        foreach ( const Target& t, data.targets)
        {
            const QMap<QString, QStringList> targetProps = data.properties[TargetProperty][t.name];
            if(targetProps["FOLDER"]==QStringList("CTestDashboardTargets"))
                continue; //filter some annoying targets
            
            QString outputName = targetProps.value("OUTPUT_NAME", QStringList(t.name)).first();
            
            QString path;
            switch(t.type)
            {
                case Target::Library:
                    if(targetProps.contains("LIBRARY_OUTPUT_DIRECTORY"))
                        path=targetProps["LIBRARY_OUTPUT_DIRECTORY"].first();
                    else
                        path=data.vm.value("LIBRARY_OUTPUT_PATH").join(QString());
                    break;
                case Target::Executable:
                    if(targetProps.contains("RUNTIME_OUTPUT_DIRECTORY"))
                        path=targetProps["RUNTIME_OUTPUT_DIRECTORY"].first();
                    else
                        path=data.vm.value("EXECUTABLE_OUTPUT_PATH").join(QString());
                    break;
                case Target::Custom:
                    break;
            }
            
            KUrl resolvedPath;
            if(!path.isEmpty())
                resolvedPath=resolveSystemDirs(folder->project(), QStringList(path)).first();
            
            KDevelop::ProjectTargetItem* targetItem = folder->targetNamed(t.type, t.name);
            if (!targetItem) {
                switch(t.type)
                {
                    case Target::Library:
                        targetItem = new CMakeLibraryTargetItem( item->project(), t.name,
                                                                folder, t.declaration, outputName, resolvedPath);
                        break;
                    case Target::Executable:
                        targetItem = new CMakeExecutableTargetItem( item->project(), t.name,
                                                                    folder, t.declaration, outputName, resolvedPath);
                        break;
                    case Target::Custom:
                        targetItem = new CMakeCustomTargetItem( item->project(), t.name,
                                                                folder, t.declaration, outputName );
                        break;
                }
            } else
                m_cleanupItems.removeAll(targetItem);
            
            DescriptorAttatched* descAtt=dynamic_cast<DescriptorAttatched*>(targetItem);
            if(descAtt)
                descAtt->setDescriptor(t.desc);

            QStringList targetDefines = targetProps["COMPILE_DEFINITIONS"];
            QStringList targetIncludes = targetProps["INCLUDE_DIRECTORIES"];

            foreach(const QString& dep, t.libraries) {
                const QMap<QString, QStringList>& depData = data.properties.value(TargetProperty).value(dep);
                if(!depData.isEmpty()) {
                    targetIncludes += depData["INTERFACE_INCLUDE_DIRECTORIES"];
                    targetDefines += depData["INTERFACE_COMPILE_DEFINITIONS"];
                } else {
                    kDebug() << "error: couldn't find dependency " << dep << data.properties.value(TargetProperty).keys();
                }
            }
            
            CompilationDataAttached* incAtt = dynamic_cast<CompilationDataAttached*>(targetItem);
            if(incAtt) {
                targetIncludes = resolvePaths(folder->url(), targetIncludes);
                incAtt->setIncludeDirectories(targetIncludes);
                incAtt->defineVariables(targetDefines);
            }
            
            KUrl::List tfiles;
            foreach( const QString & sFile, t.files)
            {
                if(sFile.startsWith("#[") || sFile.isEmpty() || sFile.endsWith('/'))
                    continue;

                KUrl sourceFile(sFile);

                // important: we want the behavior of KUrl::isRelative(), *not* KUrl::isRelativeUrl()
                if(sourceFile.isRelative()) {
                    sourceFile = folder->url();
                    sourceFile.addPath( sFile );
                    if(!QFile::exists(sourceFile.toLocalFile())) {
                        sourceFile = buildDirectory(folder);
                        sourceFile.addPath( sFile );
                    }
                }
                sourceFile.cleanPath();

                tfiles += sourceFile;
                kDebug(9042) << "..........Adding:" << sourceFile << sFile << folder->url();
            }
            
            setTargetFiles(targetItem, tfiles);
        }
        qRegisterMetaType<QList<Test> >("QList<Test>");
        qRegisterMetaType<KDevelop::ProjectFolderItem* >("KDevelop::ProjectFolderItem*");
        QMetaObject::invokeMethod(this, "createTestSuites", Qt::QueuedConnection,
                                  Q_ARG(QList<Test>, data.testSuites),
                                  Q_ARG(KDevelop::ProjectFolderItem*, item));
        
        foreach(KDevelop::ProjectFolderItem* item, folderList) {
            parse(item);
        }
        data.vm.popScope();
    } else if( folder ) {
        // Only do cmake-stuff if its a cmake folder
        deleteAllLater(castToBase(folder->cleanupBuildFolders(QList<Subdirectory>())));
        deleteAllLater(castToBase(folder->cleanupTargets(QList<CMakeTarget>())));
    } else {
        // Log non-cmake folders for debugging purposes
        kDebug(9042) << "Folder Item which is not a CMake folder parsed:" << item->url() << item->type();
    }
    // Use item here since folder may be 0.
    reloadFiles(item);

    return QList<KDevelop::ProjectFolderItem*>();
}

ProjectFileItem* containsFile(const KUrl& file, const QList<ProjectFileItem*>& tfiles)
{
    foreach(ProjectFileItem* f, tfiles) {
        if(f->url()==file)
            return f;
    }
    return 0;
}

void CMakeManager::setTargetFiles(ProjectTargetItem* target, const KUrl::List& files)
{
    QList<ProjectFileItem*> tfiles = target->fileList();
    foreach(ProjectFileItem* file, tfiles) {
        if(!files.contains(file->url()))
            deleteItemLater(file);
    }
    
    tfiles = target->fileList(); //We need to recreate the list without the removed items
    foreach(const KUrl& file, files) {
        ProjectFileItem* f = containsFile(file, tfiles);
        if(f)
            m_cleanupItems.removeAll(f);
        else
            new KDevelop::ProjectFileItem( target->project(), file, target );
    }
}

QList<KDevelop::ProjectTargetItem*> CMakeManager::targets() const
{
    QList<KDevelop::ProjectTargetItem*> ret;
    foreach(IProject* p, m_watchers.keys())
    {
        ret+=p->projectItem()->targetList();
    }
    return ret;
}


KUrl::List CMakeManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    CompilationDataAttached* includer=0;
//     kDebug(9042) << "Querying inc dirs for " << item;
    while(item)
    {
        includer = dynamic_cast<CompilationDataAttached*>( item );
        if(includer) {
            QStringList dirs = includer->includeDirectories(item);
            //Here there's the possibility that it might not be a target. We should make sure that's not the case
            return resolveSystemDirs(item->project(), processGeneratorExpression(dirs, item->project(), dynamic_cast<ProjectTargetItem*>(item)));
        }
        item = item->parent();
//         kDebug(9042) << "Looking for an includer: " << item;
    }
    // No includer found, so no include-directories to be returned;
    return KUrl::List();
}

QHash< QString, QString > CMakeManager::defines(KDevelop::ProjectBaseItem *item ) const
{
    CompilationDataAttached* att=0;
    ProjectBaseItem* it=item;
//     kDebug(9042) << "Querying defines for " << item << dynamic_cast<ProjectTargetItem*>(item);
    while(!att && item)
    {
        att = dynamic_cast<CompilationDataAttached*>( item );
        it = item;
        item = item->parent();
//         kDebug(9042) << "Looking for a folder: " << folder << item;
    }
    if( !att ) {
        // Not a CMake folder, so no defines to be returned;
        return QHash<QString,QString>();
    }

    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>(it);
    CMakeDefinitions defs = att->definitions(folder ? folder->formerParent() : dynamic_cast<CMakeFolderItem*>(item));
    //qDebug() << "lalala" << defs << it->url();
    return defs;
}

KDevelop::IProjectBuilder * CMakeManager::builder() const
{
    IPlugin* i = core()->pluginController()->pluginForExtension( "org.kdevelop.IProjectBuilder", "KDevCMakeBuilder");
    Q_ASSERT(i);
    KDevelop::IProjectBuilder* _builder = i->extension<KDevelop::IProjectBuilder>();
    Q_ASSERT(_builder );
    return _builder ;
}

bool CMakeManager::reload(KDevelop::ProjectFolderItem* folder)
{
    if(isReloading(folder->project()))
        return false;
    
    CMakeFolderItem* item=dynamic_cast<CMakeFolderItem*>(folder);
    if ( !item ) {
        ProjectBaseItem* it = folder;
        while(!item && it->parent()) {
            it = it->parent();
            item = dynamic_cast<CMakeFolderItem*>(it);
        }
        if(!item)
            return false;
    }

    reimport(item);
    return true;
}

void CMakeManager::reimport(CMakeFolderItem* fi)
{
    Q_ASSERT(fi && fi->project());
    Q_ASSERT(!isReloading(fi->project()));
    KJob *job=createImportJob(fi);
    job->setProperty("project", qVariantFromValue(fi->project()));
    
    QMutexLocker locker(&m_busyProjectsMutex);
    Q_ASSERT(!m_busyProjects.contains(fi->project()));
    m_busyProjects += fi->project();
    locker.unlock();
    
    connect( job, SIGNAL(result(KJob*)), this, SLOT(reimportDone(KJob*)) );
    ICore::self()->runController()->registerJob( job );
}

void CMakeManager::reimportDone(KJob* job)
{
    IProject* p = job->property("project").value<KDevelop::IProject*>();
    
    cleanupToDelete(p);
    
    QMutexLocker locker(&m_busyProjectsMutex);
    Q_ASSERT(m_busyProjects.contains(p));
    m_busyProjects.remove(p);
}

bool CMakeManager::isReloading(IProject* p)
{
    if(!p->isReady())
        return true;
    
    QMutexLocker locker(&m_busyProjectsMutex);
    
    return m_busyProjects.contains(p);
}


void CMakeManager::cleanupToDelete(IProject* p)
{
    Q_ASSERT(isReloading(p));

    for(QSet<QString>::iterator it=m_toDelete.begin(); it!=m_toDelete.end(); ) {
        QList<ProjectBaseItem*> items = p->itemsForUrl(*it);
        if(!items.isEmpty()) {
            deleteAllLater(items);
            it=m_toDelete.erase(it);
        } else 
            ++it;
    }

    for(QHash<KUrl, KUrl>::iterator it=m_renamed.begin(); it != m_renamed.end(); ) {
        QList<ProjectBaseItem*> items=p->itemsForUrl(it.key());
        if(!items.isEmpty()) {
            foreach(ProjectBaseItem* item, items) {
                if(item->file())
                    emit fileRenamed(it.value(), item->file());
                else
                    emit folderRenamed(it.value(), item->folder());
            }
            it = m_renamed.erase(it);
        } else {
            ++it;
        }
    }
}

void CMakeManager::deletedWatched(const QString& path)
{
    KUrl url(path);
    IProject* p=ICore::self()->projectController()->findProjectForUrl(url);
    
    if(p && !isReloading(p)) {
        if(p->folder().equals(url, KUrl::CompareWithoutTrailingSlash)) {
            ICore::self()->projectController()->closeProject(p);
        } else {
            if(url.fileName()=="CMakeLists.txt") {
                QList<ProjectFolderItem*> folders = p->foldersForUrl(url.upUrl());
                foreach(ProjectFolderItem* folder, folders) 
                    reload(folder);
                
            } else {
                QMutexLocker locker(&m_busyProjectsMutex);
                m_busyProjects += p;
                locker.unlock();
                
                m_toDelete += path;
                cleanupToDelete(p);
                
                locker.relock();
                m_busyProjects -= p;
            }
        }
    } else if(p)
        m_toDelete += path;
}

void CMakeManager::directoryChanged(const QString& dir)
{
    m_fileSystemChangedBuffer << dir;
    m_fileSystemChangeTimer->start();
}

void CMakeManager::filesystemBuffererTimeout()
{
    Q_FOREACH(const QString& file, m_fileSystemChangedBuffer) {
        realDirectoryChanged(file);
    }
    m_fileSystemChangedBuffer.clear();
}

QString addTrailingSlash(const QString& path)
{
    return (path.isEmpty() || path.endsWith('/')) ? path : path+'/';
}

void CMakeManager::realDirectoryChanged(const QString& dir)
{
    IProject* p=ICore::self()->projectController()->findProjectForUrl(KUrl(dir));
    if(!p || isReloading(p))
        return;
    
    if(!QFile::exists(dir))
        deletedWatched(addTrailingSlash(dir));
    else
        dirtyFile(dir);
}

void CMakeManager::dirtyFile(const QString & dirty)
{
    const KUrl dirtyFile(dirty);
    IProject* p=ICore::self()->projectController()->findProjectForUrl(dirtyFile);

    kDebug() << "dirty FileSystem: " << dirty << (p ? isReloading(p) : 0);
    if(p && isReloading(p))
        return;
    
    if(p && dirtyFile.fileName() == "CMakeLists.txt")
    {
        QMutexLocker locker(&m_reparsingMutex);
        
        QList<ProjectFileItem*> files=p->filesForUrl(dirtyFile);
        kDebug(9032) << dirtyFile << "is dirty" << files.count();

        Q_ASSERT(files.count()==1);
        CMakeFolderItem *folderItem=static_cast<CMakeFolderItem*>(files.first()->parent());
        
#if 0
            KUrl relative=KUrl::relativeUrl(projectBaseUrl, dir);
            initializeProject(proj, dir);
            KUrl current=projectBaseUrl;
            QStringList subs=relative.toLocalFile().split("/");
            subs.append(QString());
            for(; !subs.isEmpty(); current.cd(subs.takeFirst()))
            {
                parseOnly(proj, current);
            }
#endif

        
        reload(folderItem);
    }
    else if(dirtyFile.fileName() == "CMakeCache.txt") {
        KUrl builddirUrl;
        IProject* p=0;
        //we first have to check from which project is this builddir
        foreach(KDevelop::IProject* pp, m_watchers.uniqueKeys()) {
            KUrl url = pp->buildSystemManager()->buildDirectory(pp->projectItem());
            if(dirtyFile.upUrl().equals(url, KUrl::CompareWithoutTrailingSlash)) {
                builddirUrl=url;
                p=pp;
            }
        }
        
        if(p && !isReloading(p)) {
            p->reloadModel();
        }
    } else if(dirty.endsWith(".cmake"))
    {
        foreach(KDevelop::IProject* project, m_watchers.uniqueKeys())
        {
            if(m_watchers[project]->files().contains(dirty))
                project->reloadModel();
        }
    }
    else if(p && QFileInfo(dirty).isDir())
    {
        QList<ProjectFolderItem*> folders=p->foldersForUrl(dirty);
        Q_ASSERT(folders.isEmpty() || folders.size()==1);
        
        if(!folders.isEmpty()) {
            QMutexLocker locker(&m_busyProjectsMutex);
            m_busyProjects += p;
            locker.unlock();
            
            reloadFiles(folders.first());
            cleanupToDelete(p);
            
            locker.relock();
            m_busyProjects -= p;
            locker.unlock();
        }
    }
}

void CMakeManager::reloadFiles(ProjectFolderItem* item)
{
    Q_ASSERT(isReloading(item->project()));
    
    QDir d(item->url().toLocalFile());
    if(!d.exists()) {
        kDebug() << "Trying to return a directory that doesn't exist:" << item->url();
        return;
    }
    
    QStringList entriesL = d.entryList( QDir::AllEntries | QDir::NoDotAndDotDot);
    QSet<QString> entries = filterFiles(entriesL);
    
    KUrl folderurl = item->url();
    folderurl.cleanPath();

    kDebug() << "Reloading Directory!" << folderurl;
    
    //We look for removed elements
    for(int i=0; i<item->rowCount(); i++)
    {
        ProjectBaseItem* it=item->child(i);
        if(it->type()==ProjectBaseItem::Target || it->type()==ProjectBaseItem::ExecutableTarget || it->type()==ProjectBaseItem::LibraryTarget)
            continue;
        
        QString current=it->text();
        KUrl fileurl = folderurl;
        fileurl.addPath(current);
        
        if(!entries.contains(current))
            m_toDelete.insert(fileurl.toLocalFile());
        else if(!it->url().equals(fileurl, KUrl::CompareWithoutTrailingSlash))
            it->setUrl(fileurl);
    }
    
    //We look for new elements
    QList<ProjectBaseItem*> newItems;
    foreach( const QString& entry, entries )
    {
        KUrl fileurl = folderurl;
        fileurl.addPath( entry );

        if(m_cleanupItems.contains(item) || item->hasFileOrFolder( entry ))
            continue;

        m_toDelete.remove(fileurl.toLocalFile());
        if( QFileInfo( fileurl.toLocalFile() ).isDir() )
        {
            fileurl.adjustPath(KUrl::AddTrailingSlash);
            ProjectFolderItem* pendingfolder = m_pending.take(fileurl);
            
            if(pendingfolder) {
                newItems += pendingfolder;
            } else if(isCorrectFolder(fileurl, item->project())) {
                fileurl.adjustPath(KUrl::AddTrailingSlash);
                ProjectFolderItem* it = new ProjectFolderItem( item->project(), fileurl, 0 );
                reloadFiles(it);
                {
                    QMutexLocker locker(&m_dirWatchersMutex);
                    m_watchers[item->project()]->addPath(fileurl.toLocalFile());
                }
                newItems += it;
            }
        }
        else
        {
            newItems += new KDevelop::ProjectFileItem( item->project(), fileurl, 0 );
        }
    }
    foreach(ProjectBaseItem* it, newItems)
        item->appendRow(it);
}

bool CMakeManager::isCorrectFolder(const KUrl& url, IProject* p) const
{
    KUrl cache(url,"CMakeCache.txt"), missing(url, ".kdev_ignore");
    
    bool ret = !QFile::exists(cache.toLocalFile()) && !QFile::exists(missing.toLocalFile());
    ret &= !CMake::allBuildDirs(p).contains(url.toLocalFile(KUrl::RemoveTrailingSlash));
    
    return ret;
}

QList< KDevelop::ProjectTargetItem * > CMakeManager::targets(KDevelop::ProjectFolderItem * folder) const
{
    return folder->targetList();
}

QString CMakeManager::name() const
{
    return "CMake";
}

KDevelop::ParseJob * CMakeManager::createParseJob(const IndexedString &/*url*/)
{
    return 0;
}

KDevelop::ILanguage * CMakeManager::language()
{
    return core()->languageController()->language(name());
}

KDevelop::ICodeHighlighting* CMakeManager::codeHighlighting() const
{
    return m_highlight;
}

ContextMenuExtension CMakeManager::contextMenuExtension( KDevelop::Context* context )
{
    if( context->type() != KDevelop::Context::ProjectItemContext )
        return IPlugin::contextMenuExtension( context );

    KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
    QList<KDevelop::ProjectBaseItem*> items = ctx->items();

    if( items.isEmpty() )
        return IPlugin::contextMenuExtension( context );

    m_clickedItems = items;
    ContextMenuExtension menuExt;
    if(items.count()==1 && dynamic_cast<DUChainAttatched*>(items.first()))
    {
        KAction* action = new KAction( i18n( "Jump to Target Definition" ), this );
        connect( action, SIGNAL(triggered()), this, SLOT(jumpToDeclaration()) );
        menuExt.addAction( ContextMenuExtension::ProjectGroup, action );
    }

    return menuExt;
}

void CMakeManager::jumpToDeclaration()
{
    DUChainAttatched* du=dynamic_cast<DUChainAttatched*>(m_clickedItems.first());
    if(du)
    {
        KTextEditor::Cursor c;
        KUrl url;
        {
            KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
            Declaration* decl = du->declaration().data();
            if(!decl)
                return;
            c = decl->rangeInCurrentRevision().start.textCursor();
            url = decl->url().toUrl();
        }

        ICore::self()->documentController()->openDocument(url, c);
    }
}

CacheValues CMakeManager::readCache(const KUrl &path) const
{
    QFile file(path.toLocalFile());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        kDebug() << "error. Could not find the file" << path;
        return CacheValues();
    }

    CacheValues ret;
    QTextStream in(&file);
    kDebug(9042) << "Reading cache:" << path;
    QStringList currentComment;
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if(!line.isEmpty() && line[0].isLetter()) //it is a variable
        {
            CacheLine c;
            c.readLine(line);
            if(c.flag().isEmpty()) {
                ret[c.name()]=CacheEntry(c.value(), currentComment.join("\n"));
                currentComment.clear();
            }
//             kDebug(9042) << "Cache line" << line << c.name();
        }
        else if(line.startsWith("//"))
            currentComment += line.right(line.count()-2);
    }
    return ret;
}

bool CMakeManager::moveFilesAndFolders(const QList< ProjectBaseItem* > &items, ProjectFolderItem* toFolder)
{
    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(DIALOG_CAPTION);
    changesWidget.setInformation(i18n("Move files and folders within CMakeLists as follows:"));

    bool cmakeSuccessful = true;
    CMakeFolderItem *nearestCMakeFolderItem = nearestCMakeFolder(toFolder);
    IProject* project=toFolder->project();
    
    KUrl::List movedUrls;
    KUrl::List oldUrls;
    foreach(ProjectBaseItem *movedItem, items)
    {
        QList<ProjectBaseItem*> dirtyItems = cmakeListedItemsAffectedByUrlChange(project, movedItem->url());
        KUrl movedItemNewUrl = toFolder->url();
        movedItemNewUrl.addPath(movedItem->baseName());
        if (movedItem->folder())
            movedItemNewUrl.adjustPath(KUrl::AddTrailingSlash);
        foreach(ProjectBaseItem* dirtyItem, dirtyItems)
        {
            KUrl dirtyItemNewUrl = afterMoveUrl(dirtyItem->url(), movedItem->url(), movedItemNewUrl);
            if (CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>(dirtyItem))
            {
                cmakeSuccessful &= changesWidgetRemoveCMakeFolder(folder, &changesWidget);
                cmakeSuccessful &= changesWidgetAddFolder(dirtyItemNewUrl, nearestCMakeFolderItem, &changesWidget);
            }
            else if (dirtyItem->parent()->target())
            {
                cmakeSuccessful &= changesWidgetMoveTargetFile(dirtyItem, dirtyItemNewUrl, &changesWidget);
            }
        }
        
        oldUrls += movedItem->url();
        movedUrls += movedItemNewUrl;
    }

    if (changesWidget.hasDocuments() && cmakeSuccessful)
        cmakeSuccessful &= changesWidget.exec() && changesWidget.applyAllChanges();

    if (!cmakeSuccessful)
    {
        if (KMessageBox::questionYesNo( QApplication::activeWindow(),
                                        i18n("Changes to CMakeLists failed, abort move?"),
                                        DIALOG_CAPTION ) == KMessageBox::Yes)
            return false;
    }

    KUrl::List::const_iterator it1=oldUrls.constBegin(), it1End=oldUrls.constEnd();
    KUrl::List::const_iterator it2=movedUrls.constBegin();
    Q_ASSERT(oldUrls.size()==movedUrls.size());
    for(; it1!=it1End; ++it1, ++it2)
    {
        if (!KDevelop::renameUrl(project, *it1, *it2))
            return false;
        
        m_renamed[*it2] = *it1;
    }

    return true;
}

bool CMakeManager::copyFilesAndFolders(const KUrl::List &items, KDevelop::ProjectFolderItem* toFolder)
{
    IProject* project = toFolder->project();
    foreach(const KUrl& url, items) {
        if (!KDevelop::copyUrl(project, url, toFolder->url()))
            return false;
    }

    return true;
}

bool CMakeManager::removeFilesAndFolders(const QList<KDevelop::ProjectBaseItem*> &items)
{
    //First do CMakeLists changes
    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(DIALOG_CAPTION);
    changesWidget.setInformation(i18n("Remove files and folders from CMakeLists as follows:"));

    bool cmakeSuccessful = changesWidgetRemoveItems(cmakeListedItemsAffectedByItemsChanged(items), &changesWidget);

    if (changesWidget.hasDocuments() && cmakeSuccessful)
        cmakeSuccessful &= changesWidget.exec() && changesWidget.applyAllChanges();

    if (!cmakeSuccessful)
    {
        if (KMessageBox::questionYesNo( QApplication::activeWindow(),
                                        i18n("Changes to CMakeLists failed, abort deletion?"),
                                        DIALOG_CAPTION ) == KMessageBox::Yes)
            return false;
    }

    //Then delete the files/folders
    foreach(ProjectBaseItem* item, items)
    {
        Q_ASSERT(item->folder() || item->file());
        Q_ASSERT(!item->file() || !item->file()->parent()->target());

        if (!KDevelop::removeUrl(item->project(), item->url(), (bool)item->folder()))
            return false;
    }

    return true;
}

bool CMakeManager::removeFilesFromTargets(const QList<ProjectFileItem*> &files)
{
    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(DIALOG_CAPTION);
    changesWidget.setInformation(i18n("Modify project targets as follows:"));

    if (files.size() &&
        changesWidgetRemoveFilesFromTargets(files, &changesWidget) &&
        changesWidget.exec() &&
        changesWidget.applyAllChanges()) {
        return true;
    }
    return false;
}

ProjectFolderItem* CMakeManager::addFolder(const KUrl& folder, ProjectFolderItem* parent)
{
    CMakeFolderItem *cmakeParent = nearestCMakeFolder(parent);
    if(!cmakeParent)
        return 0;

    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(DIALOG_CAPTION);
    changesWidget.setInformation(i18n("Create folder '%1':",
                                      folder.fileName(KUrl::IgnoreTrailingSlash)));

    changesWidgetAddFolder(folder, cmakeParent, &changesWidget);

    if(changesWidget.exec() && changesWidget.applyAllChanges())
    {
        if(KDevelop::createFolder(folder)) { //If saved we create the folder then the CMakeLists.txt file
            KUrl newCMakeLists(folder);
            newCMakeLists.addPath("CMakeLists.txt");
            KDevelop::createFile( newCMakeLists );
        } else
            KMessageBox::error(0, i18n("Could not save the change."),
                                  DIALOG_CAPTION);
    }

    return 0;
}

KDevelop::ProjectFileItem* CMakeManager::addFile( const KUrl& url, KDevelop::ProjectFolderItem* parent)
{
    KDevelop::ProjectFileItem* created = 0;
    if ( KDevelop::createFile(url) ) {
        QList< ProjectFileItem* > files = parent->project()->filesForUrl(url);
        if(!files.isEmpty())
            created = files.first();
        else
            created = new KDevelop::ProjectFileItem( parent->project(), url, parent );
    }
    return created;
}

bool CMakeManager::addFilesToTarget(const QList< ProjectFileItem* > &_files, ProjectTargetItem* target)
{
    QSet<QString> headerExt = QSet<QString>() << ".h" << ".hpp" << ".hxx";
    QList< ProjectFileItem* > files = _files;
    for (int i = files.count() - 1; i >= 0; --i)
    {
        QString fileName = files[i]->fileName();
        QString fileExt = fileName.mid(fileName.lastIndexOf('.'));
        QList< ProjectBaseItem* > sameUrlItems = files[i]->project()->itemsForUrl(files[i]->url());
        foreach(ProjectBaseItem* item, sameUrlItems)
        {
            if (item->parent() == target)
            {
                files.removeAt(i);
                break;
            }
        }
        if (headerExt.contains(fileExt))
            files.removeAt(i);
    }

    if(!files.length())
        return true;

    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(DIALOG_CAPTION);
    changesWidget.setInformation(i18n("Modify target '%1' as follows:", target->baseName()));

    bool success = changesWidgetAddFilesToTarget(files, target, &changesWidget) &&
                   changesWidget.exec() &&
                   changesWidget.applyAllChanges();

    if(!success)
        KMessageBox::error(0, i18n("CMakeLists changes failed."), DIALOG_CAPTION);

    return success;
}

bool CMakeManager::renameFileOrFolder(ProjectBaseItem *item, const KUrl &newUrl)
{
    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(DIALOG_CAPTION);
    changesWidget.setInformation(i18n("Rename '%1' to '%2':", item->text(),
                                      newUrl.fileName(KUrl::IgnoreTrailingSlash)));
    
    bool cmakeSuccessful = true, changedCMakeLists=false;
    IProject* project=item->project();
    KUrl oldUrl=item->url();
    if (item->file())
    {
        QList<ProjectBaseItem*> targetFiles = cmakeListedItemsAffectedByUrlChange(project, oldUrl);
        foreach(ProjectBaseItem* targetFile, targetFiles)
            cmakeSuccessful &= changesWidgetMoveTargetFile(targetFile, newUrl, &changesWidget);
    }
    else if (CMakeFolderItem *folder = dynamic_cast<CMakeFolderItem*>(item))
        cmakeSuccessful &= changesWidgetRenameFolder(folder, newUrl, &changesWidget);
    
    item->setUrl(newUrl);
    if (changesWidget.hasDocuments() && cmakeSuccessful) {
        changedCMakeLists = changesWidget.exec() && changesWidget.applyAllChanges();
        cmakeSuccessful &= changedCMakeLists;
    }
    
    if (!cmakeSuccessful)
    {
        if (KMessageBox::questionYesNo( QApplication::activeWindow(),
                                        i18n("Changes to CMakeLists failed, abort rename?"),
                                        DIALOG_CAPTION ) == KMessageBox::Yes)
            return false;
    }

    bool ret = KDevelop::renameUrl(project, oldUrl, newUrl);
    if(ret) {
        m_renamed[newUrl] = oldUrl;
    } else
        item->setUrl(oldUrl);
    
    return ret;
}

bool CMakeManager::renameFile(ProjectFileItem *item, const KUrl &newUrl)
{
    return renameFileOrFolder(item, newUrl);
}

bool CMakeManager::renameFolder(ProjectFolderItem* item, const KUrl &newUrl)
{
    return renameFileOrFolder(item, newUrl);
}

QWidget* CMakeManager::specialLanguageObjectNavigationWidget(const KUrl& url, const KDevelop::SimpleCursor& position)
{
    KDevelop::TopDUContextPointer top= TopDUContextPointer(KDevelop::DUChain::self()->chainForDocument(url));
    Declaration *decl=0;
    if(top)
    {
        int useAt=top->findUseAt(top->transformToLocalRevision(position));
        if(useAt>=0)
        {
            Use u=top->uses()[useAt];
            decl=u.usedDeclaration(top->topContext());
        }
    }

    CMakeNavigationWidget* doc=0;
    if(decl)
    {
        doc=new CMakeNavigationWidget(top, decl);
    }
    else
    {
        const IDocument* d=ICore::self()->documentController()->documentForUrl(url);
        const KTextEditor::Document* e=d->textDocument();
        KTextEditor::Cursor start=position.textCursor(), end=position.textCursor(), step(0,1);
        for(QChar i=e->character(start); i.isLetter() || i=='_'; i=e->character(start-=step))
        {}
        start+=step;
        
        for(QChar i=e->character(end); i.isLetter() || i=='_'; i=e->character(end+=step))
        {}
        
        QString id=e->text(KTextEditor::Range(start, end));
        ICMakeDocumentation* docu=CMake::cmakeDocumentation();
        if( docu )
        {
            KSharedPtr<IDocumentation> desc=docu->description(id, url);
            if(!desc.isNull())
            {
                doc=new CMakeNavigationWidget(top, desc);
            }
        }
    }
    
    return doc;
}

QPair<QString, QString> CMakeManager::cacheValue(KDevelop::IProject* project, const QString& id) const
{
    QPair<QString, QString> ret;
    if(project==0 && !m_projectsData.keys().isEmpty())
    {
        project=m_projectsData.keys().first();
    }
    
//     kDebug() << "cache value " << id << project << (m_projectsData.contains(project) && m_projectsData[project].cache.contains(id));
    if(m_projectsData.contains(project) && m_projectsData[project].cache.contains(id))
    {
        const CacheEntry& e=m_projectsData[project].cache.value(id);
        ret.first=e.value;
        ret.second=e.doc;
    }
    return ret;
}

void CMakeManager::projectClosing(IProject* p)
{
    m_projectsData.remove(p); 
    
    QMutexLocker locker(&m_dirWatchersMutex);
    delete m_watchers.take(p);
}

void CMakeManager::addDeleteItem(ProjectBaseItem* item)
{
    if(item->parent() && item->model() && item->model()->thread()!=QThread::currentThread()) {
        m_cleanupItems += item;
    } else 
        delete item;
}

void CMakeManager::deleteItemLater(ProjectBaseItem* item)
{
    addDeleteItem(item);
    
    if(!m_cleanupItems.isEmpty()) {
        QMetaObject::invokeMethod(this, "cleanupItems", Qt::BlockingQueuedConnection);
    }
}

void CMakeManager::deleteAllLater(const QList< ProjectBaseItem* >& items)
{
    foreach(KDevelop::ProjectBaseItem* item, items) {
        addDeleteItem(item);
    }
    
    if(!m_cleanupItems.isEmpty()) {
        QMetaObject::invokeMethod(this, "cleanupItems", Qt::BlockingQueuedConnection);
    }
}

void CMakeManager::cleanupItems()
{
    qDeleteAll(m_cleanupItems);
    m_cleanupItems.clear();
}

void CMakeManager::createTestSuites(const QList< Test >& testSuites, ProjectFolderItem* folder)
{
    kDebug();
    CTestUtils::createTestSuites(testSuites, folder);
}

QStringList CMakeManager::processGeneratorExpression(const QStringList& expr, IProject* project, ProjectTargetItem* target) const
{
    QStringList ret;
    GenerationExpressionSolver exec(m_projectsData[project].properties);
    if(target)
        exec.setTargetName(target->text());

    for(QStringList::const_iterator it = expr.constBegin(), itEnd = expr.constEnd(); it!=itEnd; ++it) {
        QStringList val = exec.run(*it).split(';');
        qDebug() << "wop wop" << *it << val;
        ret += val;
    }
    return ret;
}


#include "cmakemanager.moc"
