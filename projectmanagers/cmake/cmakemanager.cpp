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
#include <KCompositeJob>

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

#ifdef CMAKEDEBUGVISITOR
#include "cmakedebugvisitor.h"
#endif

#include "ui_cmakepossibleroots.h"
#include <language/duchain/use.h>
#include <interfaces/idocumentation.h>
#include "cmakeprojectdata.h"
#include "cmakecommitchangesjob.h"
#include "cmakeimportjob.h"

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

    KUrl cmakeInfoFile(project->projectFileUrl());
    cmakeInfoFile = cmakeInfoFile.upUrl();
    cmakeInfoFile.addPath("CMakeLists.txt");

    KUrl folderUrl=project->folder();
    kDebug(9042) << "file is" << cmakeInfoFile.toLocalFile();

    if ( !cmakeInfoFile.isLocalFile() )
    {
        kWarning() << "error. not a local file. CMake support doesn't handle remote projects";
        return 0;
    }
    
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

    CMakeFolderItem* rootItem = new CMakeFolderItem(project, project->folder(), QString(), 0 );
    KUrl cachefile=buildDirectory(rootItem);
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
    Q_ASSERT(rootItem->rowCount()==0);
    
    return rootItem;
}

QList<ProjectFolderItem*> CMakeManager::parse(ProjectFolderItem*)
{ return QList< ProjectFolderItem* >(); }


KJob* CMakeManager::createImportJob(ProjectFolderItem* dom)
{
    KJob* job = new CMakeImportJob(dom, this);
    connect(job, SIGNAL(finished(KJob*)), SLOT(importFinished(KJob*)));
    return job;
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
    IProject* project = item->project();
//     kDebug(9042) << "Querying inc dirs for " << item;
    while(item)
    {
        if(CompilationDataAttached* includer = dynamic_cast<CompilationDataAttached*>( item )) {
            QStringList dirs = includer->includeDirectories(item);
            //Here there's the possibility that it might not be a target. We should make sure that's not the case
            return CMake::resolveSystemDirs(project, processGeneratorExpression(dirs, project, dynamic_cast<ProjectTargetItem*>(item)));
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
    m_busyProjects += fi->project();
    
    KJob *job=createImportJob(fi);
    connect(job, SIGNAL(result(KJob*)), SLOT(importFinished(KJob*)));
    ICore::self()->runController()->registerJob( job );
}

void CMakeManager::importFinished(KJob* j)
{
    CMakeImportJob* job = qobject_cast<CMakeImportJob*>(j);
    Q_ASSERT(job);
    m_busyProjects.remove(job->project());
    *m_projectsData[job->project()] = job->projectData();
}

bool CMakeManager::isReloading(IProject* p)
{
    return !p->isReady() || m_busyProjects.contains(p);
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
                qDeleteAll(p->itemsForUrl(path));
            }
        }
    } else if(p)
        qDeleteAll(p->itemsForUrl(path));
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
            reload(p->projectItem());
        }
    } else if(dirty.endsWith(".cmake"))
    {
        foreach(KDevelop::IProject* project, m_watchers.uniqueKeys())
        {
            if(m_watchers[project]->files().contains(dirty))
                reload(project->projectItem());
        }
    }
    else if(p && QFileInfo(dirty).isDir())
    {
        QList<ProjectFolderItem*> folders=p->foldersForUrl(dirty);
        Q_ASSERT(folders.isEmpty() || folders.size()==1);
        
        if(!folders.isEmpty()) {
            CMakeCommitChangesJob* job = new CMakeCommitChangesJob(dirtyFile, this, p);
            job->start();
        }
    }
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
        
        QList<ProjectBaseItem*> renamedItems = project->itemsForUrl(*it2);
        bool dir = QFileInfo(it2->toLocalFile()).isDir();
        foreach(ProjectBaseItem* item, renamedItems) {
            if(dir)
                emit folderRenamed(*it1, item->folder());
            else
                emit fileRenamed(*it1, item->file());
        }
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
    IProject* p = 0;
    QList<QUrl> urls;
    foreach(ProjectBaseItem* item, items)
    {
        Q_ASSERT(item->folder() || item->file());
        Q_ASSERT(!item->file() || !item->file()->parent()->target());

        urls += item->url();
        if(!p)
            p = item->project();
    }

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

    bool ret = true;
    //Then delete the files/folders
    foreach(const QUrl& file, urls)
    {
        ret &= KDevelop::removeUrl(p, file, QDir(file.toLocalFile()).exists());
    }

    return ret;
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
    if(!ret) {
        item->setUrl(oldUrl);
    }
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
    CMakeProjectData* data = m_projectsData[project];
    if(data && data->cache.contains(id))
    {
        const CacheEntry& e=data->cache.value(id);
        ret.first=e.value;
        ret.second=e.doc;
    }
    return ret;
}

void CMakeManager::projectClosing(IProject* p)
{
    delete m_projectsData.take(p); 
    delete m_watchers.take(p);
}

QStringList CMakeManager::processGeneratorExpression(const QStringList& expr, IProject* project, ProjectTargetItem* target) const
{
    QStringList ret;
    const CMakeProjectData* data = m_projectsData[project];
    GenerationExpressionSolver exec(data->properties);
    if(target)
        exec.setTargetName(target->text());

    exec.defineVariable("INSTALL_PREFIX", data->vm.value("CMAKE_INSTALL_PREFIX").join(QString()));
    for(QStringList::const_iterator it = expr.constBegin(), itEnd = expr.constEnd(); it!=itEnd; ++it) {
        QStringList val = exec.run(*it).split(';');
        ret += val;
    }
    return ret;
}

void CMakeManager::addPending(const KUrl& url, CMakeFolderItem* folder)
{
    m_pending.insert(url, folder);
}

CMakeFolderItem* CMakeManager::takePending(const KUrl& url)
{
    return m_pending.take(url);
}

void CMakeManager::addWatcher(IProject* p, const QString& path)
{
    m_watchers[p]->addPath(path);
}

CMakeProjectData CMakeManager::projectData(IProject* project)
{
    Q_ASSERT(QThread::currentThread() == project->thread());
    CMakeProjectData* data = m_projectsData[project];
    if(!data) {
        data = new CMakeProjectData;
        m_projectsData[project] = data;
    }
    return *data;
}
