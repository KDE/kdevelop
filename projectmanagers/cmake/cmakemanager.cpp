/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007-2008 Aleix Pol <aleixpol@gmail.com>
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
#include <KProcess>
#include <KAction>
#include <KMessageBox>
#include <kio/job.h>
#include <ktexteditor/document.h>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <interfaces/irun.h>
#include <interfaces/iruncontroller.h>
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
#include "cmakeconfig.h"
#include "cmakemodelitems.h"
#include "cmakehighlighting.h"
#include "cmakecachereader.h"
#include "cmakeastvisitor.h"
#include "cmakeprojectvisitor.h"
#include "cmakeexport.h"
#include "cmakecodecompletionmodel.h"
#include "icmakebuilder.h"
#include "cmakeutils.h"

#ifdef CMAKEDEBUGVISITOR
#include "cmakedebugvisitor.h"
#endif

#include "ui_cmakepossibleroots.h"
#include <language/editor/editorintegrator.h>
#include <language/duchain/smartconverter.h>
#include <language/duchain/use.h>
#include <ktexteditor/smartinterface.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(CMakeSupportFactory, registerPlugin<CMakeProjectManager>(); )
K_EXPORT_PLUGIN(CMakeSupportFactory(KAboutData("kdevcmakemanager","kdevcmake", ki18n("CMake Manager"), "0.1", ki18n("Support for managing CMake projects"), KAboutData::License_GPL)))

namespace {

QString executeProcess(const QString& execName, const QStringList& args=QStringList())
{
    kDebug(9042) << "Executing:" << execName << "::" << args /*<< "into" << *m_vars*/;

    KProcess p;
    p.setOutputChannelMode(KProcess::MergedChannels);
    p.setProgram(execName, args);
    p.start();

    if(!p.waitForFinished())
    {
        kDebug() << "failed to execute:" << execName;
    }

    QByteArray b = p.readAllStandardOutput();
    QString t;
    t.prepend(b.trimmed());
    kDebug(9042) << "executed" << execName << "<" << t;

    return t;
}

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

CMakeProjectManager::CMakeProjectManager( QObject* parent, const QVariantList& )
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
    {
        DUChainWriteLocker lock(DUChain::lock());
        m_buildstrapContext=new TopDUContext(IndexedString("buildstrap"), SimpleRange(0,0, 0,0));

        DUChain::self()->addDocumentChain(m_buildstrapContext);
        Q_ASSERT(DUChain::self()->chainForDocument(KUrl("buildstrap")));
    }

    QStringList envVars;
    QString executable;
#ifdef Q_OS_WIN
    envVars=CMakeProjectVisitor::envVarDirectories("Path");
    executable="cmake.exe";
#else
    envVars=CMakeProjectVisitor::envVarDirectories("PATH");
    executable="cmake";
#endif
    QString cmakeCmd = CMakeProjectVisitor::findFile(executable, envVars);
    m_modulePathDef=guessCMakeModulesDirectories(cmakeCmd);
    m_varsDef.insert("CMAKE_BINARY_DIR", QStringList("#[bin_dir]"));
    m_varsDef.insert("CMAKE_INSTALL_PREFIX", QStringList("#[install_dir]"));
    m_varsDef.insert("CMAKE_COMMAND", QStringList(cmakeCmd));
#ifdef Q_OS_WIN
    cmakeInitScripts << "CMakeMinGWFindMake.cmake";
    cmakeInitScripts << "CMakeMSYSFindMake.cmake";
    cmakeInitScripts << "CMakeNMakeFindMake.cmake";
    cmakeInitScripts << "CMakeVS8FindMake.cmake";
#else
    cmakeInitScripts << "CMakeUnixFindMake.cmake";
#endif
    cmakeInitScripts << "CMakeDetermineSystem.cmake";
    cmakeInitScripts << "CMakeDetermineCCompiler.cmake";
    cmakeInitScripts << "CMakeDetermineCXXCompiler.cmake";
    cmakeInitScripts << "CMakeSystemSpecificInformation.cmake";


    m_varsDef.insert("CMAKE_MODULE_PATH", m_modulePathDef);
    m_varsDef.insert("CMAKE_ROOT", QStringList(guessCMakeRoot(cmakeCmd)));

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

    kDebug(9042) << "modPath" << m_varsDef.value("CMAKE_MODULE_PATH") << m_modulePathDef;
}

CMakeProjectManager::~CMakeProjectManager()
{}

KUrl CMakeProjectManager::buildDirectory(const KDevelop::ProjectBaseItem *item) const
{
    KUrl path = CMake::currentBuildDir(item->project());
    if(path.isEmpty())
    {
        return KUrl();
    }

    KUrl projectPath = m_realRoot[item->project()];

    const ProjectFolderItem *fi=dynamic_cast<const ProjectFolderItem*>(item);
    for(; !fi && item; )
    {
        item=dynamic_cast<const ProjectBaseItem*>(item->parent());
        fi=dynamic_cast<const ProjectFolderItem*>(item);
    }
    if(!fi) {
        return path;
    }

    QString relative=KUrl::relativeUrl( projectPath, fi->url() );
    path.addPath(relative);

    kDebug() << "Build folder: " << path;
    return path;
}

void CMakeProjectManager::initializeProject(KDevelop::IProject* project, const KUrl& baseUrl)
{
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
            group.writeEntry("CMakeDir", m_modulePathDef);
    }
    else
        group.writeEntry("CMakeDir", m_modulePathDef);

    ReferencedTopDUContext ref=m_buildstrapContext;
    foreach(const QString& script, cmakeInitScripts)
    {
        ref = includeScript(CMakeProjectVisitor::findFile(script, m_modulePathPerProject[project], QStringList()),
                              project, ref);
    }
    m_buildstrapContext=ref;
}

KDevelop::ProjectFolderItem* CMakeProjectManager::import( KDevelop::IProject *project )
{
    CMakeFolderItem* m_rootItem=0;
    KUrl cmakeInfoFile(project->projectFileUrl());
    cmakeInfoFile = cmakeInfoFile.upUrl();
    cmakeInfoFile.addPath("CMakeLists.txt");

    KUrl folderUrl=project->folder();
    kDebug(9042) << "found module path is" << m_modulePathDef;
    kDebug(9042) << "file is" << cmakeInfoFile.path();

    if ( !cmakeInfoFile.isLocalFile() )
    {
//         KMessageBox::error( ICore::self()->uiControllerInternal()->defaultMainWindow(),
//                                 i18n("Not a local file. CMake support doesn't handle remote projects") );
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
        m_modulePathPerProject[project]=m_modulePathDef;
        m_rootItem = new CMakeFolderItem(project, folderUrl.url(), 0 );
        m_rootItem->setProjectRoot(true);

        KUrl cachefile=buildDirectory(m_rootItem);
        if( cachefile.isEmpty() ) {
            CMake::checkForNeedingConfigure(m_rootItem);
        }
        cachefile.addPath("CMakeCache.txt");
        m_projectCache[project]=readCache(cachefile);
        initializeProject(project, folderUrl);

        m_watchers[project]->disconnect( SIGNAL(dirty(QString) ), this, SLOT(dirtyFile(QString)));
        connect(m_watchers[project], SIGNAL(dirty(QString)), this, SLOT(dirtyFile(QString)));
        Q_ASSERT(m_rootItem->rowCount()==0);
    }
    return m_rootItem;
}


KDevelop::ReferencedTopDUContext CMakeProjectManager::includeScript(const QString& file,
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

    CMakeProjectVisitor v(file, parent);
    v.setCacheValues( &m_projectCache[project] );
    v.setVariableMap(vm);
    v.setMacroMap(mm);
    v.setModulePath(m_modulePathPerProject[project]);
    v.walk(f, 0);

    vm->remove("CMAKE_CURRENT_LIST_FILE");
    vm->remove("CMAKE_CURRENT_SOURCE_DIR");
    vm->remove("CMAKE_CURRENT_BINARY_DIR");

    m_watchers[project]->addFile(file);
    return v.context();
}

QStringList removeMatches(const QString& exp, const QStringList& orig)
{
    QStringList ret;
    QRegExp rx(exp);
    foreach(const QString& str, orig)
    {
        if(rx.indexIn(str)<0)
            ret.append(str);
    }
    return ret;
}

QList<KDevelop::ProjectFolderItem*> CMakeProjectManager::parse( KDevelop::ProjectFolderItem* item )
{
    QList<KDevelop::ProjectFolderItem*> folderList;
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( item );

    QStringList entries = QDir( item->url().toLocalFile() ).entryList( QDir::AllEntries | QDir::NoDotAndDotDot );
    entries = removeMatches("\\w*~$|\\w*\\.bak$", entries);
    if(folder && folder->type()==KDevelop::ProjectBaseItem::BuildFolder)
    {
        Q_ASSERT(folder->rowCount()==0);

        kDebug(9042) << "parse:" << folder->url();
        KUrl cmakeListsPath(folder->url());
        cmakeListsPath.addPath("CMakeLists.txt");

        VariableMap *vm=&m_varsPerProject[item->project()];
        MacroMap *mm=&m_macrosPerProject[item->project()];
        CMakeFileContent f = CMakeListsParser::readCMakeFile(cmakeListsPath.toLocalFile());
        m_watchers[item->project()]->addFile(cmakeListsPath.toLocalFile());

        if(f.isEmpty())
        {
            kDebug() << "There is no" << cmakeListsPath;
        }
        else
        {
            kDebug(9042) << "Adding cmake: " << cmakeListsPath << " to the model";

            m_watchers[item->project()]->addFile(cmakeListsPath.toLocalFile());
            QString binDir=KUrl::relativePath(m_realRoot[folder->project()].toLocalFile(), folder->url().toLocalFile());
            if(binDir.startsWith("./"))
                binDir=binDir.remove(0, 2);
            QString currentBinDir=vm->value("CMAKE_BINARY_DIR")[0]+binDir;

            vm->insert("CMAKE_CURRENT_BINARY_DIR", QStringList(currentBinDir));
            vm->insert("CMAKE_CURRENT_LIST_FILE", QStringList(cmakeListsPath.toLocalFile(KUrl::RemoveTrailingSlash)));
            vm->insert("CMAKE_CURRENT_SOURCE_DIR", QStringList(folder->url().toLocalFile(KUrl::RemoveTrailingSlash)));

            kDebug(9042) << "currentBinDir" << KUrl(vm->value("CMAKE_BINARY_DIR")[0]) << vm->value("CMAKE_CURRENT_BINARY_DIR");

        #ifdef CMAKEDEBUGVISITOR
            CMakeAstDebugVisitor dv;
            dv.walk(cmakeListsPath.toLocalFile(), f, 0);
        #endif

            ReferencedTopDUContext curr;
            if(folder->parent())
            {
                curr=dynamic_cast<CMakeFolderItem*>(folder->parent())->topDUContext();
            }
            else
                curr=m_buildstrapContext;
            CMakeProjectVisitor v(folder->url().toLocalFile(KUrl::RemoveTrailingSlash), curr);
            v.setCacheValues(&m_projectCache[item->project()]);
            v.setVariableMap(vm);
            v.setMacroMap(mm);
            v.setModulePath(m_modulePathPerProject[item->project()]);
            v.setDefinitions(folder->definitions());
            v.walk(f, 0);
            folder->setTopDUContext(v.context());
            vm->remove("CMAKE_CURRENT_LIST_FILE");
            vm->remove("CMAKE_CURRENT_SOURCE_DIR");
            vm->remove("CMAKE_CURRENT_BINARY_DIR");

            EditorIntegrator editor;
            editor.setCurrentUrl(IndexedString(v.context()->url().toUrl()));
            LockedSmartInterface smart(editor.smart());
            if(smart)
            {
                smart->clearRevision();
                
                DUChainWriteLocker lock(DUChain::lock());
                SmartConverter converter(&editor);
                converter.convertDUChain(v.context());
                CMakeHighlighting highlight;
                highlight.highlightDUChain(v.context());
                smart->useRevision(smart->currentRevision());
            }

            /*{
            kDebug() << "dumpiiiiiing" << folder->url();
            KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
            KDevelop::DumpChain dump;
            dump.dump( v.context(), false);
            }*/

            if(folder->text()=="/" && !v.projectName().isEmpty())
            {
                folder->setText(v.projectName());
            }

            KUrl subroot=m_subprojectRoot[item->project()];

            foreach (const QString& subf, v.subdirectories())
            {
                if(subf.isEmpty()) //This would not be necessary if we didn't parse the wrong lines
                    continue;
                KUrl path(folder->url());
                path.addPath(subf);
                path.adjustPath(KUrl::AddTrailingSlash);

                kDebug(9042) << "Found subdir " << path << "which should be into" << subroot;
                if(subroot.isParentOf(path) || path.isParentOf(subroot))
                {
                    if(entries.contains(subf))
                    {
                        entries.removeAll(subf);
                    }

                    CMakeFolderItem* a = new CMakeFolderItem( folder->project(), subf, folder );
                    kDebug() << "folder: " << a << a->index();
                    a->setUrl(path);
                    a->setDefinitions(v.definitions());
                    folderList.append( a );

                    DescriptorAttatched* datt=static_cast<DescriptorAttatched*>(a);
                    datt->setDescriptor(v.folderDeclarationDescriptor(subf));
                }
            }

    //         if(folderList.isEmpty() && path.isParentOf(item->url()))
    //             kDebug() << "poor guess";

            QStringList directories;
            directories += folder->url().toLocalFile(KUrl::RemoveTrailingSlash);
            directories += currentBinDir;

            foreach(const QString& s, v.includeDirectories())
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
            folder->setDefinitions(v.definitions());

            foreach ( const QString& t, v.targets())
            {
                QStringList dependencies=v.targetDependencies(t);
                if(true || !dependencies.isEmpty()) //Just to remove verbosity
                {

                    QString outputName=t;
                    if(v.targetHasProperty(t, "OUTPUT_NAME"))
                        outputName=v.targetProperty(t, "OUTPUT_NAME");

                    KDevelop::ProjectTargetItem* targetItem;
                    switch(v.targetType(t))
                    {
                        case CMakeProjectVisitor::Library:
                            targetItem = new CMakeLibraryTargetItem( item->project(), t, folder, v.declarationsPerTarget()[t], outputName );
                            break;
                        case CMakeProjectVisitor::Executable:
                            targetItem = new CMakeExecutableTargetItem( item->project(), t, folder, v.declarationsPerTarget()[t], outputName );
                            break;
                        case CMakeProjectVisitor::Custom:
                            targetItem = new CMakeCustomTargetItem( item->project(), t, folder, v.declarationsPerTarget()[t], outputName );
                            break;
                    }
                    DescriptorAttatched* datt=dynamic_cast<DescriptorAttatched*>(targetItem);
                    datt->setDescriptor(v.targetDeclarationDescriptor(outputName));

                    foreach( const QString & sFile, dependencies )
                    {
                        if(sFile.isEmpty())
                            continue;

                        KUrl sourceFile(sFile);
                        if(sourceFile.isRelative()) {
                            sourceFile = folder->url();
                            sourceFile.adjustPath( KUrl::RemoveTrailingSlash );
                            sourceFile.addPath( sFile );
                        }

                        if( entries.contains( sourceFile.fileName() ) )
                        {
                            entries.removeAll( sourceFile.fileName() );
                        }

                        new KDevelop::ProjectFileItem( item->project(), sourceFile, targetItem );
                        item->project()->addToFileSet( KDevelop::IndexedString( sourceFile ) );
                        kDebug(9042) << "..........Adding:" << sourceFile;
                    }
                    m_targets.append(targetItem);
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

bool CMakeProjectManager::reload(KDevelop::ProjectBaseItem* item)
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

QList<KDevelop::ProjectTargetItem*> CMakeProjectManager::targets() const
{
    return m_targets;
}


KUrl::List CMakeProjectManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
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

QHash< QString, QString > CMakeProjectManager::defines(KDevelop::ProjectBaseItem *item ) const
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

KDevelop::IProjectBuilder * CMakeProjectManager::builder(KDevelop::ProjectFolderItem *) const
{
    Q_ASSERT(m_builder);
    return m_builder;
}

QString CMakeProjectManager::guessCMakeShare(const QString& cmakeBin)
{
    KUrl bin(cmakeBin);
    bin=bin.upUrl();
    bin=bin.upUrl();
    return bin.toLocalFile(KUrl::RemoveTrailingSlash);
}

QString CMakeProjectManager::guessCMakeRoot(const QString & cmakeBin)
{
    QString ret;
    KUrl bin(guessCMakeShare(cmakeBin));

    QString version=executeProcess(cmakeBin, QStringList("--version"));
    QRegExp rx("[a-z* ]*([0-9.]*)-[0-9]*");
    rx.indexIn(version);
    QString versionNumber = rx.capturedTexts()[1];

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

QStringList CMakeProjectManager::guessCMakeModulesDirectories(const QString& cmakeBin)
{
    return QStringList(guessCMakeRoot(cmakeBin)+"/Modules");
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
void CMakeProjectManager::reimport(CMakeFolderItem* fi)
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

void CMakeProjectManager::dirtyFile(const QString & dirty)
{
    KUrl dirtyFile(dirty);
    KUrl dir(dirtyFile.upUrl());
    IProject* p=ICore::self()->projectController()->findProjectForUrl(dirtyFile);

    if(p && dirtyFile.fileName() == "CMakeLists.txt")
    {
        QList<ProjectFileItem*> files=p->filesForUrl(dirtyFile);
        kDebug(9032) << dirtyFile << "is dirty" << files;

        Q_ASSERT(p->fileCount()>0);
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

QList< KDevelop::ProjectTargetItem * > CMakeProjectManager::targets(KDevelop::ProjectFolderItem * folder) const
{
    return folder->targetList();
}

QString CMakeProjectManager::name() const
{
    return "CMake";
}

KDevelop::ParseJob * CMakeProjectManager::createParseJob(const KUrl &)
{
    return 0;
}

KDevelop::ILanguage * CMakeProjectManager::language()
{
    return core()->languageController()->language(name());
}

const KDevelop::ICodeHighlighting* CMakeProjectManager::codeHighlighting() const
{
    return m_highlight;
}

ContextMenuExtension CMakeProjectManager::contextMenuExtension( KDevelop::Context* context )
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

void CMakeProjectManager::jumpToDeclaration()
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

CacheValues CMakeProjectManager::readCache(const KUrl &path)
{
    QFile file(path.toLocalFile());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        kDebug() << "error. Could not find the file";
        return CacheValues();
    }

    CacheValues ret;
    QTextStream in(&file);
    kDebug(9042) << "Reading cache:" << path;
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if(!line.isEmpty() && line[0].isLetter()) //it is a variable
        {
            CacheLine c;
            c.readLine(line);
            if(c.flag().isEmpty())
                ret[c.name()]=c.value();
//             kDebug(9042) << "Cache line" << line << c.name();
        }
    }
    return ret;
}

KDevelop::ProjectFolderItem* CMakeProjectManager::addFolder( const KUrl& folder, KDevelop::ProjectFolderItem* parent)
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

bool CMakeProjectManager::removeFolder( KDevelop::ProjectFolderItem* it)
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
    bool ret=true;
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
                ret = ret && followUses(doc, r, name, lists, add);
            }
        }
    }
    return ret;
}

bool CMakeProjectManager::removeFile( KDevelop::ProjectFileItem* it)
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

bool CMakeProjectManager::removeFileFromTarget( KDevelop::ProjectFileItem* it, KDevelop::ProjectTargetItem* target)
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
KDevelop::ProjectFileItem* CMakeProjectManager::addFile( const KUrl& url, KDevelop::ProjectFolderItem* parent)
{
    ProjectFileItem* it = new KDevelop::ProjectFileItem( parent->project(), url, parent );
    parent->project()->addToFileSet( KDevelop::IndexedString( url ) );
    return it;
}

bool CMakeProjectManager::addFileToTarget( KDevelop::ProjectFileItem* it, KDevelop::ProjectTargetItem* target)
{
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

    if(e.exec())
    {
        bool saved=e.document()->documentSave();
        if(!saved)
            KMessageBox::error(0, i18n("KDevelop - CMake Support"),
                                  i18n("Cannot save the change."));
    }
    return ret;
}

#include "cmakemanager.moc"
