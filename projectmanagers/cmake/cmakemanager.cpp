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

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KAboutData>
#include <KDialog>
#include <kparts/mainwindow.h>
#include <KUrl>
#include <KAction>
#include <KMessageBox>
#include <kio/job.h>
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
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/duchain.h>
#include <language/duchain/dumpchain.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/codecompletion/codecompletion.h>

#include "applychangeswidget.h"
#include "cmakedocumentation.h"
#include "cmakemodelitems.h"
#include "cmakenavigationwidget.h"
#include "cmakehighlighting.h"
#include "cmakecachereader.h"
#include "cmakeastvisitor.h"
#include "cmakeprojectvisitor.h"
#include "cmakeexport.h"
#include "cmakecodecompletionmodel.h"
#include "icmakebuilder.h"
#include "cmakeutils.h"
#include "cmaketypes.h"

#ifdef CMAKEDEBUGVISITOR
#include "cmakedebugvisitor.h"
#endif

#include "ui_cmakepossibleroots.h"
#include <language/editor/editorintegrator.h>
#include <language/duchain/smartconverter.h>
#include <language/duchain/use.h>
#include <ktexteditor/smartinterface.h>
#include <interfaces/idocumentation.h>
#include "cmakeprojectdata.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(CMakeSupportFactory, registerPlugin<CMakeManager>(); )
K_EXPORT_PLUGIN(CMakeSupportFactory(KAboutData("kdevcmakemanager","kdevcmake", ki18n("CMake Manager"), "0.1", ki18n("Support for managing CMake projects"), KAboutData::License_GPL)))

namespace {

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

inline QString replaceInstallDir(QString in, QString installDir)
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
        newList.append(KUrl(s));
    }
    return newList;
}

}

CMakeManager::CMakeManager( QObject* parent, const QVariantList& )
    : KDevelop::IPlugin( CMakeSupportFactory::componentData(), parent ), m_builder(0)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ILanguageSupport )
    IPlugin* i = core()->pluginController()->pluginForExtension( "org.kdevelop.ICMakeBuilder" );
    Q_ASSERT(i);
    m_builder = i->extension<ICMakeBuilder>();
    Q_ASSERT(m_builder);

    new CodeCompletion(this, new CMakeCodeCompletionModel(), name());

    m_highlight = new CMakeHighlighting(this);
    QString cmakeCmd = KStandardDirs::findExe("cmake");
    m_doc = new CMakeDocumentation(cmakeCmd, this);
}

CMakeManager::~CMakeManager()
{
    delete m_doc;
}

KUrl CMakeManager::buildDirectory(KDevelop::ProjectBaseItem *item) const
{
    KUrl path = CMake::currentBuildDir(item->project());
    if(path.isEmpty())
    {
        return KUrl();
    }

    CMakeFolderItem *fi=dynamic_cast<CMakeFolderItem*>(item);
    for(; !fi && item; )
    {
        item=static_cast<ProjectBaseItem*>(item->parent());
        fi=dynamic_cast<CMakeFolderItem*>(item);
    }
    if(!fi) {
        return path;
    }

    KUrl projectPath = m_realRoot[item->project()];
    QString relative=KUrl::relativeUrl( projectPath, fi->url() );
    path.addPath(relative);
    path.cleanPath();

    kDebug() << "Build folder: " << path;
    return path;
}

KDevelop::ReferencedTopDUContext CMakeManager::initializeProject(KDevelop::IProject* project, const KUrl& baseUrl)
{
    QString cmakeCmd=KStandardDirs::findExe("cmake");
    
    QString versionOutput=CMake::executeProcess(cmakeCmd, QStringList("--version"));
    QRegExp rx("([0-9]+).([0-9]+)-patch ([0-9]+)");
    rx.indexIn(versionOutput);
    QStringList version=rx.capturedTexts();
    version.takeFirst();
    
    VariableMap m_varsDef;
    QStringList modulePathDef=guessCMakeModulesDirectories(cmakeCmd, version);
    m_modulePathPerProject[project]=modulePathDef;
    kDebug(9042) << "found module path is" << modulePathDef;
    m_varsDef.insert("CMAKE_BINARY_DIR", QStringList("#[bin_dir]"));
    m_varsDef.insert("CMAKE_INSTALL_PREFIX", QStringList("#[install_dir]"));
    m_varsDef.insert("CMAKE_COMMAND", QStringList(cmakeCmd));
    m_varsDef.insert("CMAKE_MAJOR_VERSION", QStringList(version[0]));
    m_varsDef.insert("CMAKE_MINOR_VERSION", QStringList(version[1]));
    m_varsDef.insert("CMAKE_PATCH_VERSION", QStringList(version[2]));
    
    QStringList cmakeInitScripts;
#ifdef Q_OS_WIN
    cmakeInitScripts << "CMakeMinGWFindMake.cmake";
    cmakeInitScripts << "CMakeMSYSFindMake.cmake";
    cmakeInitScripts << "CMakeNMakeFindMake.cmake";
    cmakeInitScripts << "CMakeVS8FindMake.cmake";
#else
    cmakeInitScripts << "CMakeUnixFindMake.cmake";
#endif
    cmakeInitScripts << "CMakeDetermineSystem.cmake";
    cmakeInitScripts << "CMakeSystemSpecificInformation.cmake";
    cmakeInitScripts << "CMakeDetermineCCompiler.cmake";
    cmakeInitScripts << "CMakeDetermineCXXCompiler.cmake";

    m_varsDef.insert("CMAKE_MODULE_PATH", modulePathDef);
    m_varsDef.insert("CMAKE_ROOT", QStringList(guessCMakeRoot(cmakeCmd, version)));

    //Defines the behaviour that can't be identified on initialization scripts
#ifdef Q_OS_WIN32
    m_varsDef.insert("WIN32", QStringList("1"));
    m_varsDef.insert("CMAKE_HOST_WIN32", QStringList("1"));
#else
    m_varsDef.insert("UNIX", QStringList("1"));
    m_varsDef.insert("CMAKE_HOST_UNIX", QStringList("1"));
#endif
#ifdef Q_OS_MAC
    m_varsDef.insert("APPLE", QStringList("1"));
    m_varsDef.insert("CMAKE_HOST_APPLE", QStringList("1"));
#endif

    m_macrosPerProject[project].clear();
    m_varsPerProject[project]=m_varsDef;
    m_varsPerProject[project].insert("CMAKE_SOURCE_DIR", QStringList(baseUrl.toLocalFile(KUrl::RemoveTrailingSlash)));

    KSharedConfig::Ptr cfg = project->projectConfiguration();
    KConfigGroup group(cfg.data(), "CMake");
    if(group.hasKey("CMakeDir"))
    {
        QStringList l;
        foreach(const QString &path, group.readEntry("CMakeDir", QStringList()) )
        {
            if( QFileInfo(path).exists() )
            {
                m_modulePathPerProject[project] << path;
                l << path;
            }
        }
        if( !l.isEmpty() )
            group.writeEntry("CMakeDir", l);
        else
            group.writeEntry("CMakeDir", modulePathDef);
    }
    else
        group.writeEntry("CMakeDir", modulePathDef);

    
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
            buildstrapContext=new TopDUContext(IndexedString(buildStrapUrl), SimpleRange(0,0, 0,0));
            DUChain::self()->addDocumentChain(buildstrapContext);
        }
        
        Q_ASSERT(buildstrapContext);
    }
    ReferencedTopDUContext ref=buildstrapContext;
    foreach(const QString& script, cmakeInitScripts)
    {
        ref = includeScript(CMakeProjectVisitor::findFile(script, m_modulePathPerProject[project], QStringList()),
                              project, ref);
    }
    
    return ref;
}

KDevelop::ProjectFolderItem* CMakeManager::import( KDevelop::IProject *project )
{
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
        KSharedConfig::Ptr cfg = project->projectConfiguration();
        KConfigGroup group(cfg.data(), "CMake");
        m_subprojectRoot[project] = folderUrl;

        if(group.hasKey("ProjectRootRelative"))
        {
            QString relative=group.readEntry("ProjectRootRelative");
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
                QString relative=KUrl::relativeUrl(folderUrl, choice);
                group.writeEntry("ProjectRootRelative", relative);
                folderUrl=choice;
            }
            else
            {
                group.writeEntry("ProjectRootRelative", "./");
            }
        }

        m_realRoot[project] = folderUrl;
        m_watchers[project] = new KDirWatch(project);
        m_rootItem = new CMakeFolderItem(project, folderUrl.url(), 0 );
        m_rootItem->setProjectRoot(true);

        KUrl cachefile=buildDirectory(m_rootItem);
        if( cachefile.isEmpty() ) {
            CMake::checkForNeedingConfigure(m_rootItem);
        }
        cachefile.addPath("CMakeCache.txt");
        m_projectCache[project]=readCache(cachefile);

        m_watchers[project]->disconnect( SIGNAL(dirty(QString) ), this, SLOT(dirtyFile(QString)));
        connect(m_watchers[project], SIGNAL(dirty(QString)), this, SLOT(dirtyFile(QString)));
        Q_ASSERT(m_rootItem->rowCount()==0);
    }
    return m_rootItem;
}


KDevelop::ReferencedTopDUContext CMakeManager::includeScript(const QString& file,
                                                        KDevelop::IProject * project, ReferencedTopDUContext parent)
{
    kDebug(9042) << "Running cmake script: " << file;
    CMakeFileContent f = CMakeListsParser::readCMakeFile(file);
    if(f.isEmpty())
    {
        kDebug() << "There is no such file: " << file;
        return 0;
    }

    VariableMap *vm=&m_varsPerProject[project];
    MacroMap *mm=&m_macrosPerProject[project];
    vm->insert("CMAKE_CURRENT_BINARY_DIR", QStringList(vm->value("CMAKE_BINARY_DIR")[0]));
    vm->insert("CMAKE_CURRENT_LIST_FILE", QStringList(file));
    vm->insert("CMAKE_CURRENT_SOURCE_DIR", QStringList(project->folder().toLocalFile(KUrl::RemoveTrailingSlash)));

    CMakeProjectVisitor v(file, parent);
    v.setCacheValues( &m_projectCache[project] );
    v.setVariableMap(vm);
    v.setMacroMap(mm);
    v.setModulePath(m_modulePathPerProject[project]);
    v.walk(f, 0, true);

    vm->remove("CMAKE_CURRENT_LIST_FILE");
    vm->remove("CMAKE_CURRENT_SOURCE_DIR");
    vm->remove("CMAKE_CURRENT_BINARY_DIR");

    m_watchers[project]->addFile(file);
    return v.context();
}

QSet<QString> removeMatches(const QString& exp, const QStringList& orig)
{
    QSet<QString> ret;
    QRegExp rx(exp);
    foreach(const QString& str, orig)
    {
        if(rx.indexIn(str)<0)
            ret.insert(str);
    }
    return ret;
}

QList<KDevelop::ProjectFolderItem*> CMakeManager::parse( KDevelop::ProjectFolderItem* item )
{
    QList<KDevelop::ProjectFolderItem*> folderList;
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( item );

    QStringList entriesL = QDir( item->url().toLocalFile() ).entryList( QDir::AllEntries | QDir::NoDotAndDotDot );
    QSet<QString> entries = removeMatches("\\w*~$|\\w*\\.bak$", entriesL);
    if(folder && folder->type()==KDevelop::ProjectBaseItem::BuildFolder)
    {
        Q_ASSERT(folder->rowCount()==0);

        kDebug(9042) << "parse:" << folder->url();
        KUrl cmakeListsPath(folder->url());
        cmakeListsPath.addPath("CMakeLists.txt");

        CMakeFileContent f = CMakeListsParser::readCMakeFile(cmakeListsPath.toLocalFile());
        
        if(f.isEmpty())
        {
            kDebug() << "There is no" << cmakeListsPath;
        }
        else
        {
            
            KDevelop::ReferencedTopDUContext curr;
            if(item->parent()==0)
                curr=initializeProject(item->project(), m_realRoot[item->project()]);
            else
                curr=dynamic_cast<CMakeFolderItem*>(folder->parent())->topDUContext();
            
            kDebug(9042) << "Adding cmake: " << cmakeListsPath << " to the model";

            m_watchers[item->project()]->addFile(cmakeListsPath.toLocalFile());
            QString binDir=KUrl::relativePath(m_realRoot[folder->project()].toLocalFile(), folder->url().toLocalFile());
            if(binDir.startsWith("./"))
                binDir=binDir.remove(0, 2);
            
            CMakeProjectData data;
            
            //Im not sure if we want to save taht, it might be a lot of data,
            //but can be useful when regenerating.
            data.vm=m_varsPerProject[item->project()];
            data.mm=m_macrosPerProject[item->project()];
            QString currentBinDir=data.vm.value("CMAKE_BINARY_DIR")[0]+binDir;

            data.vm.insert("CMAKE_CURRENT_BINARY_DIR", QStringList(currentBinDir));
            data.vm.insert("CMAKE_CURRENT_LIST_FILE", QStringList(cmakeListsPath.toLocalFile(KUrl::RemoveTrailingSlash)));
            data.vm.insert("CMAKE_CURRENT_SOURCE_DIR", QStringList(folder->url().toLocalFile(KUrl::RemoveTrailingSlash)));

            kDebug(9042) << "currentBinDir" << KUrl(data.vm.value("CMAKE_BINARY_DIR")[0]) << data.vm.value("CMAKE_CURRENT_BINARY_DIR");

        #ifdef CMAKEDEBUGVISITOR
            CMakeAstDebugVisitor dv;
            dv.walk(cmakeListsPath.toLocalFile(), f, 0);
        #endif
            
            {
                CMakeProjectVisitor v(folder->url().toLocalFile(KUrl::RemoveTrailingSlash), curr);
                v.setCacheValues(&m_projectCache[item->project()]);
                v.setVariableMap(&data.vm);
                v.setMacroMap(&data.mm);
                v.setModulePath(m_modulePathPerProject[item->project()]);
                v.setDefinitions(folder->definitions());
                v.walk(f, 0);
                
                folder->setTopDUContext(v.context());
                data.projectName=v.projectName();
                data.subdirectories=v.subdirectories();
                data.definitions=v.definitions();
                data.includeDirectories=v.includeDirectories();
                data.targets=v.targets();
                data.folderDeclarations=v.folderDeclarations();
                data.properties=v.properties();
                
                QList<Target>::iterator it=data.targets.begin(), itEnd=data.targets.end();
                for(; it!=itEnd; ++it)
                {
                    it->files=v.resolveDependencies(it->files);
                }
            }
            data.vm.remove("CMAKE_CURRENT_LIST_FILE");
            data.vm.remove("CMAKE_CURRENT_SOURCE_DIR");
            data.vm.remove("CMAKE_CURRENT_BINARY_DIR");
            
            m_varsPerProject[item->project()]=data.vm;
            m_macrosPerProject[item->project()]=data.mm;

            /*{
            kDebug() << "dumpiiiiiing" << folder->url();
            KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
            KDevelop::DumpChain dump;
            dump.dump( v.context(), false);
            }*/

            if(folder->text()=="/" && !data.projectName.isEmpty())
            {
                folder->setText(data.projectName);
            }

            KUrl subroot=m_subprojectRoot[item->project()];

            foreach (const QString& subf, data.subdirectories)
            {
                if(subf.isEmpty()) //This would not be necessary if we didn't parse the wrong lines
                    continue;
                
                KUrl path(subf);
                if(path.isRelative())
                {
                    KUrl pp=KUrl(folder->url());
                    pp.addPath(subf);
                    path=pp;
                }
                path.adjustPath(KUrl::AddTrailingSlash);

                kDebug(9042) << "Found subdir " << path << "which should be into" << subroot;
                if(subroot.isParentOf(path) || path.isParentOf(subroot))
                {
                    entries.remove(subf);

                    CMakeFolderItem* a = new CMakeFolderItem( folder->project(), subf, folder );
                    kDebug() << "folder: " << a << a->index();
                    a->setUrl(path);
                    a->setDefinitions(data.definitions);
                    folderList.append( a );

                    DescriptorAttatched* datt=static_cast<DescriptorAttatched*>(a);
                    datt->setDescriptor(data.folderDeclarations[subf]);
                }
            }

    //         if(folderList.isEmpty() && path.isParentOf(item->url()))
    //             kDebug() << "poor guess";

            QStringList directories;
            directories += folder->url().toLocalFile(KUrl::RemoveTrailingSlash);
            directories += currentBinDir;

            foreach(const QString& s, data.includeDirectories)
            {
                QString dir(s);
                if(KUrl( s ).isRelative() && !s.startsWith("#["))
                {
                    KUrl path=folder->url();
                    path.addPath(s);
                    dir=path.toLocalFile();
                }

                KUrl simp(dir); //We use this to simplify dir
                simp.cleanPath();
                dir=simp.toLocalFile();

                if(!directories.contains(dir))
                    directories.append(dir);
            }
            folder->setIncludeDirectories(directories);
            folder->setDefinitions(data.definitions);

            foreach ( const Target& t, data.targets)
            {
                QStringList files=t.files;
                QString outputName=t.name;
                if(data.properties[TARGET].contains(t.name) && data.properties[TARGET][t.name].contains("OUTPUT_NAME"))
                    outputName=data.properties[TARGET][t.name]["OUTPUT_NAME"].first();

                KDevelop::ProjectTargetItem* targetItem;
                switch(t.type)
                {
                    case Target::Library:
                        targetItem = new CMakeLibraryTargetItem( item->project(), t.name, folder, t.declaration, outputName );
                        break;
                    case Target::Executable:
                        targetItem = new CMakeExecutableTargetItem( item->project(), t.name, folder, t.declaration, outputName );
                        break;
                    case Target::Custom:
                        targetItem = new CMakeCustomTargetItem( item->project(), t.name, folder, t.declaration, outputName );
                        break;
                }
                DescriptorAttatched* datt=dynamic_cast<DescriptorAttatched*>(targetItem);
                datt->setDescriptor(t.desc);

                foreach( const QString & sFile, t.files)
                {
                    if(sFile.isEmpty())
                        continue;

                    KUrl sourceFile(sFile);
                    if(sourceFile.isRelative()) {
                        sourceFile = folder->url();
                        sourceFile.adjustPath( KUrl::RemoveTrailingSlash );
                        sourceFile.addPath( sFile );
                    }
                    
                    bool removed=entries.remove( sourceFile.fileName() );
                    if(removed && folder->url()==sourceFile.upUrl()) {
                        //Even if a file is inside 2 targets we do not want to have it
                        //2 times on the folder.
                        new KDevelop::ProjectFileItem( item->project(), sourceFile, folder );
                    }

                    new KDevelop::ProjectFileItem( item->project(), sourceFile, targetItem );
                    item->project()->addToFileSet( KDevelop::IndexedString( sourceFile ) );
                    kDebug(9042) << "..........Adding:" << sourceFile;
                }
            }

        }
    }

    foreach( const QString& entry, entries )
    {
        if( item->hasFileOrFolder( entry ) )
            continue;

        KUrl fileurl = item->url();
        fileurl.addPath( entry );

        KUrl cache=fileurl;
        cache.addPath("CMakeCache.txt");
        if( QFileInfo( fileurl.toLocalFile() ).isDir())
        {
            fileurl.adjustPath(KUrl::AddTrailingSlash);
            if(!QFile::exists(cache.toLocalFile()))
                folderList.append(new KDevelop::ProjectFolderItem( item->project(), fileurl, item ));
        }
        else
        {
            addFile(fileurl, item);
        }
    }

    return folderList;
}

bool CMakeManager::reload(KDevelop::ProjectBaseItem* item)
{
    CMakeFolderItem* folderItem = dynamic_cast<CMakeFolderItem*>(item);
    if (folderItem) {
        if (item == item->project()->projectItem()) {
            item->project()->reloadModel();
        } else {
            QStandardItem *parent = folderItem->parent();
            CMakeFolderItem* fi = new CMakeFolderItem( folderItem->project(), folderItem->url().toLocalFile(), parent);
            parent->removeRow(folderItem->row());
            reimport(fi);
        }
    }
    return true;
}

QList<KDevelop::ProjectTargetItem*> CMakeManager::targets() const
{
    QList<KDevelop::ProjectTargetItem*> ret;
    foreach(IProject* p, m_realRoot.keys())
    {
        ret+=p->projectItem()->targetList();
    }
    return ret;
}


KUrl::List CMakeManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    CMakeFolderItem* folder=0;
//     kDebug(9042) << "Querying inc dirs for " << item;
    while(!folder && item)
    {
        folder = dynamic_cast<CMakeFolderItem*>( item );
        item = static_cast<KDevelop::ProjectBaseItem*>(item->parent());
//         kDebug(9042) << "Looking for a folder: " << folder << item;
    }

    if(!folder)
        return KUrl::List();

//     kDebug(9042) << "Include directories! -- before" << folder->includeDirectories();
    KUrl::List l = resolveSystemDirs(folder->project(), folder->includeDirectories());
//     kDebug(9042) << "Include directories!" << l;
    return l;
}

QHash< QString, QString > CMakeManager::defines(KDevelop::ProjectBaseItem *item ) const
{
    CMakeFolderItem* folder=0;
    kDebug(9042) << "Querying defines dirs for " << item;
    while(!folder)
    {
        folder = dynamic_cast<CMakeFolderItem*>( item );
        item = static_cast<KDevelop::ProjectBaseItem*>(item->parent());
        kDebug(9042) << "Looking for a folder: " << folder << item;
    }

    if(!folder)
        return QHash<QString, QString>();

    return folder->definitions();
}

KDevelop::IProjectBuilder * CMakeManager::builder(KDevelop::ProjectFolderItem *) const
{
    Q_ASSERT(m_builder);
    return m_builder;
}

QString CMakeManager::guessCMakeShare(const QString& cmakeBin)
{
    KUrl bin(cmakeBin);
    bin=bin.upUrl();
    bin=bin.upUrl();
    return bin.toLocalFile(KUrl::RemoveTrailingSlash);
}

QString CMakeManager::guessCMakeRoot(const QString & cmakeBin, const QStringList& version)
{
    QString ret;
    KUrl bin(guessCMakeShare(cmakeBin));

    QString versionNumber = version[0]+'.'+version[1];

    bin.cd(QString("share/cmake-%1").arg(versionNumber));

    ret=bin.toLocalFile(KUrl::RemoveTrailingSlash);
    QDir d(ret);
    if(!d.exists(ret))
    {
        KUrl std(bin);
        std = std.upUrl();
        std.cd("cmake/");
        ret=std.toLocalFile(KUrl::RemoveTrailingSlash);
        bin = std;
    }

    kDebug(9042) << "guessing: " << bin.toLocalFile(KUrl::RemoveTrailingSlash);
    return ret;
}

QStringList CMakeManager::guessCMakeModulesDirectories(const QString& cmakeBin, const QStringList& version)
{
    return QStringList(guessCMakeRoot(cmakeBin, version)+"/Modules");
}

/*void CMakeProjectManager::parseOnly(KDevelop::IProject* project, const KUrl &url)
{
    kDebug(9042) << "Looking for" << url << " to regenerate";

    KUrl cmakeListsPath(url);
    cmakeListsPath.addPath("CMakeLists.txt");

    VariableMap *vm=&m_varsPerProject[project];
    MacroMap *mm=&m_macrosPerProject[project];

    CMakeFileContent f = CMakeListsParser::readCMakeFile(cmakeListsPath.toLocalFile());
    if(f.isEmpty())
    {
        kDebug() << "There is no" << cmakeListsPath;
        return;
    }

    QString currentBinDir=KUrl::relativeUrl(m_realRoot[project], url);
    vm->insert("CMAKE_CURRENT_BINARY_DIR", QStringList(vm->value("CMAKE_BINARY_DIR")[0]+currentBinDir));
    vm->insert("CMAKE_CURRENT_LIST_FILE", QStringList(cmakeListsPath.toLocalFile(KUrl::RemoveTrailingSlash)));
    vm->insert("CMAKE_CURRENT_SOURCE_DIR", QStringList(url.toLocalFile(KUrl::RemoveTrailingSlash)));
    CMakeProjectVisitor v(url.toLocalFile(), missingtopcontext);
    v.setCacheValues(m_projectCache[project]);
    v.setVariableMap(vm);
    v.setMacroMap(mm);
    v.setModulePath(m_modulePathPerProject[project]);
    v.walk(f, 0);
    vm->remove("CMAKE_CURRENT_LIST_FILE");
    vm->remove("CMAKE_CURRENT_SOURCE_DIR");
    vm->remove("CMAKE_CURRENT_BINARY_DIR");
}*/

//Copied from ImportJob
void CMakeManager::reimport(CMakeFolderItem* fi)
{
    QQueue< QList<KDevelop::ProjectFolderItem*> > workQueue;
    QList<KDevelop::ProjectFolderItem*> initial;
    initial.append( fi );
    workQueue.enqueue( initial );

    while( workQueue.count() > 0 )
    {
        QList<KDevelop::ProjectFolderItem*> front = workQueue.dequeue();
        Q_FOREACH( KDevelop::ProjectFolderItem* _item, front )
        {
            QList<KDevelop::ProjectFolderItem*> workingList = parse( _item );
            if( workingList.count() > 0 )
                workQueue.enqueue( workingList );
        }
    }
}

void CMakeManager::dirtyFile(const QString & dirty)
{
    KUrl dirtyFile(dirty);
    KUrl dir(dirtyFile.upUrl());
    IProject* p=ICore::self()->projectController()->findProjectForUrl(dirtyFile);

    if(p && dirtyFile.fileName() == "CMakeLists.txt")
    {
        QList<ProjectFileItem*> files=p->filesForUrl(dirtyFile);
        kDebug(9032) << dirtyFile << "is dirty" << files.count();

        // Debug output as apparently the Q_ASSERT after this is sometimes hitting
        // See https://bugs.kde.org/show_bug.cgi?id=187335
        if( files.count() > 1 ) {
            foreach(ProjectFileItem* item, files) {
                kDebug() << "item:" << item << item->url() << item->text() << item->parent()->type() << item->parent()->text();
            }
        }

        Q_ASSERT(files.count()==1);
        CMakeFolderItem *it=static_cast<CMakeFolderItem*>(files.first()->parent());

        KDevelop::IProject* proj=it->project();
        KUrl projectBaseUrl=m_realRoot[proj];
        projectBaseUrl.adjustPath(KUrl::AddTrailingSlash);

        kDebug(9032) << "reload:" << dir << projectBaseUrl << (dir!=projectBaseUrl);
        if(dir!=projectBaseUrl)
        {
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
            QStandardItem *parent=it->parent();
            parent->removeRow(it->row());
            CMakeFolderItem* fi=new CMakeFolderItem( proj, dir.toLocalFile(), parent);
            reload(fi);
        }
        else
        {
    //         qDebug() << "reloading";
            reload(proj->projectItem());
        }
    }
    else
    {
        foreach(KDevelop::IProject* project, m_watchers.uniqueKeys())
        {
            if(m_watchers[project]->contains(dirty))
                reload(project->projectItem());
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

KDevelop::ParseJob * CMakeManager::createParseJob(const KUrl &)
{
    return 0;
}

KDevelop::ILanguage * CMakeManager::language()
{
    return core()->languageController()->language(name());
}

const KDevelop::ICodeHighlighting* CMakeManager::codeHighlighting() const
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
        KAction* action = new KAction( i18n( "Jump to target definition" ), this );
        connect( action, SIGNAL( triggered() ), this, SLOT( jumpToDeclaration() ) );
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
            c = decl->range().start.textCursor();
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
            if(c.flag().isEmpty())
                ret[c.name()]=CacheEntry(c.value(), currentComment.join("\n"));
//             kDebug(9042) << "Cache line" << line << c.name();
        }
        else if(line.startsWith("//"))
            currentComment += line.right(line.count()-2);
    }
    return ret;
}

KDevelop::ProjectFolderItem* CMakeManager::addFolder( const KUrl& folder, KDevelop::ProjectFolderItem* parent)
{
    Q_ASSERT(QFile::exists(folder.toLocalFile()));
    KUrl lists=parent->url();
    lists.addPath("CMakeLists.txt");
    QString relative=KUrl::relativeUrl(parent->url(), folder);

    kDebug() << "Adding folder " << parent->url() << " to " << folder << " as " << relative;

    Q_ASSERT(!relative.contains("/"));
//     CMakeFileContent f = CMakeListsParser::readCMakeFile(file);

    ApplyChangesWidget e(i18n("Create a folder called '%1'.", relative), lists);

    e.document()->insertLine(e.document()->lines(), QString("add_subdirectory(%1)").arg(relative));

    if(e.exec())
    {
        KUrl newCMakeLists(folder);
        newCMakeLists.addPath("CMakeLists.txt");

        QFile f(newCMakeLists.toLocalFile());
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            KMessageBox::error(0, i18n("KDevelop - CMake Support"),
                                  i18n("Could not create the directory's CMakeLists.txt file."));
            return 0;
        }
        QTextStream out(&f);
        out << "\n";

        bool saved=e.document()->documentSave();
        if(!saved)
            KMessageBox::error(0, i18n("KDevelop - CMake Support"),
                                  i18n("Could not save the change."));
    }
    return 0;
}

bool CMakeManager::removeFolder( KDevelop::ProjectFolderItem* it)
{
    KUrl lists=it->url().upUrl();
    lists.addPath("CMakeLists.txt");
    if(it->type()!=KDevelop::ProjectBaseItem::BuildFolder)
    {
        it->parent()->removeRow(it->row());
        return true;
    }

    ApplyChangesWidget e(i18n("Remove a folder called '%1'.", it->text()), lists);
    CMakeFolderItem* cmit=static_cast<CMakeFolderItem*>(it);
    KTextEditor::Range r=cmit->descriptor().range().textRange();
    kDebug(9042) << "For " << lists << " remove " << r;
    e.document()->removeText(r);

    if(e.exec())
    {
        bool saved=e.document()->documentSave();
        if(!saved)
            KMessageBox::error(0, i18n("KDevelop - CMake Support"),
                                  i18n("Could not save the change."));
    }
    return 0;
}

bool followUses(KTextEditor::Document* doc, SimpleRange r, const QString& name, const KUrl& lists, bool add)
{
    bool ret=false;
    QString txt=doc->text(r.textRange());
    if(!add && txt.contains(name))
    {
        txt.remove(name);
        doc->replaceText(r.textRange(), txt);
        ret=true;
    }
    else
    {
        KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
        KDevelop::ReferencedTopDUContext topctx=DUChain::self()->chainForDocument(lists);
        QList<Declaration*> decls;
        for(int i=0; i<topctx->usesCount(); i++)
        {
            Use u = topctx->uses()[i];

            if(!r.contains(u.m_range))
                continue; //We just want the uses in the range, not the whole file

            Declaration* d=u.usedDeclaration(topctx);

            if(d->context()->topContext()->url().toUrl()==lists)
                decls += d;
        }

        if(add && decls.isEmpty())
        {
            doc->insertText(r.textRange().start(), name);
            ret=true;
        }
        else foreach(Declaration* d, decls)
        {
            r.start=d->range().end;

            for(int l=r.start.line; ;l++)
            {
                QString line=doc->line(l);
                int c;
                if((c=line.indexOf(')'))>=0) {
                    r.end=SimpleCursor(l,c);
                    break;
                } else if(line.isEmpty()) {
                    r=SimpleRange();
                    break;
                }
            }

            if(!r.isEmpty())
            {
                ret = ret || followUses(doc, r, name, lists, add);
            }
        }
    }
    return ret;
}

bool CMakeManager::removeFile( KDevelop::ProjectFileItem* it)
{
    bool ret=true;
    if(!static_cast<ProjectBaseItem*>(it->parent())->target())
    {
        it->project()->removeFromFileSet(KDevelop::IndexedString( it->url()));
        it->parent()->removeRow(it->row());
    }
    else
    {
        ProjectTargetItem* target=static_cast<ProjectTargetItem*>(it->parent());
        ret = removeFileFromTarget(it, target);
    }
    return ret;
}

bool CMakeManager::removeFileFromTarget( KDevelop::ProjectFileItem* it, KDevelop::ProjectTargetItem* target)
{
    if(it->parent()!=target)
        return false; //It is not a cmake-managed file

    CMakeFolderItem* folder=static_cast<CMakeFolderItem*>(target->parent());

    DescriptorAttatched* desc=dynamic_cast<DescriptorAttatched*>(target);
    SimpleRange r=desc->descriptor().range();
    r.start=SimpleCursor(desc->descriptor().arguments.first().range().end);

    KUrl lists=folder->url();
    lists.addPath("CMakeLists.txt");

    ApplyChangesWidget e(i18n("Remove a file called '%1'.", it->text()), lists);

    bool ret=followUses(e.document(), r, ' '+it->text(), lists, false);
    if(ret)
    {
        if(e.exec())
        {
            bool saved=e.document()->documentSave();
            if(!saved)
                KMessageBox::error(0, i18n("KDevelop - CMake Support"),
                                    i18n("Cannot save the change."));
            else
                it->project()->removeFromFileSet(IndexedString(it->url()));
        }
    }
    else
    {
        KMessageBox::error(0, i18n("KDevelop - CMake Support"),
                              i18n("Cannot remove the file."));
    }
    return ret;
}

//This is being called from ::parse() so we shouldn't make it block the ui
KDevelop::ProjectFileItem* CMakeManager::addFile( const KUrl& url, KDevelop::ProjectFolderItem* parent)
{
    ProjectFileItem* it = new KDevelop::ProjectFileItem( parent->project(), url, parent );
    parent->project()->addToFileSet( KDevelop::IndexedString( url ) );
    return it;
}

bool CMakeManager::addFileToTarget( KDevelop::ProjectFileItem* it, KDevelop::ProjectTargetItem* target)
{
    QSet<QString> headerExt=QSet<QString>() << ".h" << ".hpp" << ".hxx";
    foreach(const QString& ext, headerExt)
    {
        if(it->url().fileName().endsWith(ext))
            return false;
        else if(it->url().fileName().endsWith(ext.toUpper()))
            return false;
    }
    
    if(it->parent()==target)
        return true; //It already is in the target

    CMakeFolderItem* folder=static_cast<CMakeFolderItem*>(target->parent());

    DescriptorAttatched* desc=dynamic_cast<DescriptorAttatched*>(target);
    SimpleRange r=desc->descriptor().range();
    r.start=SimpleCursor(desc->descriptor().arguments.first().range().end);

    KUrl lists=folder->url();
    lists.addPath("CMakeLists.txt");

    ApplyChangesWidget e(i18n("Add a file called '%1' to target '%2'.", it->text(), target->text()), lists);

    bool ret=followUses(e.document(), r, ' '+it->text(), lists, true);

    if(ret && e.exec())
        ret=e.document()->documentSave();
    if(!ret)
            KMessageBox::error(0, i18n("KDevelop - CMake Support"),
                                  i18n("Cannot save the change."));
    return ret;
}

QWidget* CMakeManager::specialLanguageObjectNavigationWidget(const KUrl& url, const KDevelop::SimpleCursor& position)
{
    KDevelop::TopDUContextPointer top= TopDUContextPointer(KDevelop::DUChain::self()->chainForDocument(url));
    Declaration *decl=0;
    QString htmlDoc;
    if(top)
    {
        int useAt=top->findUseAt(position);
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
        KSharedPtr<IDocumentation> desc=m_doc->description(id, url);
        if(!desc.isNull())
        {
            doc=new CMakeNavigationWidget(top, desc);
        }
    }
    
    return doc;
}

QPair<QString, QString> CMakeManager::cacheValue(KDevelop::IProject* project, const QString& id) const
{
    QPair<QString, QString> ret;
    if(project==0 && !m_projectCache.keys().isEmpty())
    {
        project=m_projectCache.keys().first();
    }
    
    kDebug() << "cache value " << id << project << (m_projectCache.contains(project) && m_projectCache[project].contains(id));
    if(m_projectCache.contains(project) && m_projectCache[project].contains(id))
    {
        const CacheEntry& e=m_projectCache[project].value(id);
        ret.first=e.value;
        ret.second=e.doc;
    }
    return ret;
}

KSharedPtr< KDevelop::IDocumentation > CMakeManager::documentationForDeclaration(KDevelop::Declaration* declaration)
{
    return m_doc->documentationForDeclaration(declaration);
}

#include "cmakemanager.moc"
