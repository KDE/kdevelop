/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2007 by Andreas Pakulat                                 *
 *   apaku@gmx.de                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "customprojectpart.h"

#include <qapplication.h>
#include <kapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qvaluestack.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qdom.h>

#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <keditlistbox.h>
#include <kdevgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kparts/part.h>
#include <kpopupmenu.h>
#include <kdeversion.h>
#include <kprocess.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "runoptionswidget.h"
#include "makeoptionswidget.h"
#include "custombuildoptionswidget.h"
#include "custommakeconfigwidget.h"
#include "customotherconfigwidget.h"
#include "custommanagerwidget.h"
#include "config.h"
#include "envvartools.h"
#include "urlutil.h"

#include "selectnewfilesdialog.h"

#include <kdevplugininfo.h>

typedef KDevGenericFactory<CustomProjectPart> CustomProjectFactory;
static const KDevPluginInfo data( "kdevcustomproject" );
K_EXPORT_COMPONENT_FACTORY( libkdevcustomproject, CustomProjectFactory( data ) )

CustomProjectPart::CustomProjectPart( QObject *parent, const char *name, const QStringList & )
        : KDevBuildTool( &data, parent, name ? name : "CustomProjectPart" )
        , m_lastCompilationFailed( false ), m_recursive( false ), m_first_recursive( false )
{
    setInstance( CustomProjectFactory::instance() );
    setXMLFile( "kdevcustomproject.rc" );

    m_executeAfterBuild = false;

    KAction *action;

    action = new KAction( i18n( "Re-Populate Project" ), 0, this, SLOT( populateProject() ), actionCollection(), "repopulate_project" );
    action->setToolTip( i18n( "Re-Populate Project" ) );
    action->setWhatsThis( i18n( "<b>Re-Populate Project</b><p>Re-Populate's the project, searches through the project directory and adds all files that match one of the wildcards set in the custom manager options to the project filelist." ) );

    action = new KAction( i18n( "&Build Project" ), "make_kdevelop", Key_F8,
                          this, SLOT( slotBuild() ),
                          actionCollection(), "build_build" );
    action->setToolTip( i18n( "Build project" ) );
    action->setWhatsThis( i18n( "<b>Build project</b><p>Runs <b>make</b> from the project directory.<br>"
                                "Environment variables and make arguments can be specified "
                                "in the project settings dialog, <b>Build Options</b> tab." ) );

    action = new KAction( i18n( "&Build Active Directory" ), "make_kdevelop", Key_F7,
                          this, SLOT( slotBuildActiveDir() ),
                          actionCollection(), "build_buildactivetarget" );
    action->setToolTip( i18n( "Build active directory" ) );
    action->setWhatsThis( i18n( "<b>Build active directory</b><p>Constructs a series of make commands to build the active directory. "
                                "Environment variables and make arguments can be specified "
                                "in the project settings dialog, <b>Make Options</b> tab." ) );

    action = new KAction( i18n( "Compile &File" ), "make_kdevelop",
                          this, SLOT( slotCompileFile() ),
                          actionCollection(), "build_compilefile" );
    action->setToolTip( i18n( "Compile file" ) );
    action->setWhatsThis( i18n( "<b>Compile file</b><p>Runs <b>make filename.o</b> command from the directory where 'filename' is the name of currently opened file.<br>"
                                "Environment variables and make arguments can be specified "
                                "in the project settings dialog, <b>Build Options</b> tab." ) );

    action = new KAction( i18n( "Install" ), 0,
                          this, SLOT( slotInstall() ),
                          actionCollection(), "build_install" );
    action->setToolTip( i18n( "Install" ) );
    action->setWhatsThis( i18n( "<b>Install</b><p>Runs <b>make install</b> command from the project directory.<br>"
                                "Environment variables and make arguments can be specified "
                                "in the project settings dialog, <b>Make Options</b> tab." ) );

    action = new KAction( i18n( "Install Active Directory" ), 0,
                          this, SLOT( slotInstallActiveDir() ),
                          actionCollection(), "build_installactivetarget" );
    action->setToolTip( i18n( "Install active directory" ) );
    action->setWhatsThis( i18n( "<b>Install active directory</b><p>Runs <b>make install</b> command from the active directory.<br>"
                                "Environment variables and make arguments can be specified "
                                "in the project settings dialog, <b>Make Options</b> tab." ) );

    action = new KAction( i18n( "Install (as root user)" ), 0,
                          this, SLOT( slotInstallWithKdesu() ),
                          actionCollection(), "build_install_kdesu" );
    action->setToolTip( i18n( "Install as root user" ) );
    action->setWhatsThis( i18n( "<b>Install</b><p>Runs <b>make install</b> command from the project directory with root privileges.<br>"
                                "It is executed via kdesu command.<br>"
                                "Environment variables and make arguments can be specified "
                                "in the project settings dialog, <b>Make Options</b> tab." ) );

    action = new KAction( i18n( "&Clean Project" ), 0,
                          this, SLOT( slotClean() ),
                          actionCollection(), "build_clean" );
    action->setToolTip( i18n( "Clean project" ) );
    action->setWhatsThis( i18n( "<b>Clean project</b><p>Runs <b>make clean</b> command from the project directory.<br>"
                                "Environment variables and make arguments can be specified "
                                "in the project settings dialog, <b>Build Options</b> tab." ) );

    action = new KAction( i18n( "Execute Program" ), "exec", 0,
                          this, SLOT( slotExecute() ),
                          actionCollection(), "build_execute" );
    action->setToolTip( i18n( "Execute program" ) );
    action->setWhatsThis( i18n( "<b>Execute program</b><p>Executes the main program specified in project settings, <b>Run Options</b> tab. "
                                "If it is not specified then the active target is used to determine the application to run." ) );

    KActionMenu *menu = new KActionMenu( i18n( "Build &Target" ),
                                         actionCollection(), "build_target" );
    m_targetMenu = menu->popupMenu();
    menu->setToolTip( i18n( "Build target" ) );
    menu->setWhatsThis( i18n( "<b>Build target</b><p>Runs <b>make targetname</b> from the project directory (targetname is the name of the target selected).<br>"
                              "Environment variables and make arguments can be specified "
                              "in the project settings dialog, <b>Build Options</b> tab." ) );

    m_targetObjectFilesMenu = new QPopupMenu();
    m_targetOtherFilesMenu = new QPopupMenu();

    m_makeEnvironmentsSelector = new KSelectAction( i18n( "Make &Environment" ), 0,
            actionCollection(), "build_make_environment" );
    m_makeEnvironmentsSelector->setToolTip( i18n( "Make environment" ) );
    m_makeEnvironmentsSelector->setWhatsThis( i18n( "<b>Make Environment</b><p> Choose the set of environment variables to be passed on to make.<br>"
            "Environment variables can be specified in the project "
            "settings dialog, <b>Build Options</b> tab." ) );

    connect( m_targetMenu, SIGNAL( aboutToShow() ),
             this, SLOT( updateTargetMenu() ) );
    connect( m_targetMenu, SIGNAL( activated( int ) ),
             this, SLOT( targetMenuActivated( int ) ) );
    connect( m_targetObjectFilesMenu, SIGNAL( activated( int ) ),
             this, SLOT( targetObjectFilesMenuActivated( int ) ) );
    connect( m_targetOtherFilesMenu, SIGNAL( activated( int ) ),
             this, SLOT( targetOtherFilesMenuActivated( int ) ) );
    connect( m_makeEnvironmentsSelector->popupMenu(), SIGNAL( aboutToShow() ),
             this, SLOT( updateMakeEnvironmentsMenu() ) );
    connect( m_makeEnvironmentsSelector->popupMenu(), SIGNAL( activated( int ) ),
             this, SLOT( makeEnvironmentsMenuActivated( int ) ) );
    connect( core(), SIGNAL( projectConfigWidget( KDialogBase* ) ),
             this, SLOT( projectConfigWidget( KDialogBase* ) ) );
    connect( core(), SIGNAL( contextMenu( QPopupMenu *, const Context * ) ),
             this, SLOT( contextMenu( QPopupMenu *, const Context * ) ) );

    connect( makeFrontend(), SIGNAL( commandFinished( const QString& ) ),
             this, SLOT( slotCommandFinished( const QString& ) ) );
    connect( makeFrontend(), SIGNAL( commandFailed( const QString& ) ),
             this, SLOT( slotCommandFailed( const QString& ) ) );
}


CustomProjectPart::~CustomProjectPart()
{}


void CustomProjectPart::projectConfigWidget( KDialogBase *dlg )
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage( i18n( "Custom Manager" ), i18n( "Custom Manager" ), BarIcon( "make", KIcon::SizeMedium ) );
    CustomManagerWidget *w0 = new CustomManagerWidget( this, vbox );
    connect( dlg, SIGNAL( okClicked() ), w0, SLOT( accept() ) );

    vbox = dlg->addVBoxPage( i18n( "Run Options" ), i18n( "Run Options" ), BarIcon( "make", KIcon::SizeMedium ) );
    RunOptionsWidget *w1 = new RunOptionsWidget( *projectDom(), "/kdevcustomproject", buildDirectory(), vbox );
    connect( dlg, SIGNAL( okClicked() ), w1, SLOT( accept() ) );
    vbox = dlg->addVBoxPage( i18n( "Build Options" ), i18n( "Build Options" ), BarIcon( "make", KIcon::SizeMedium ) );
    QTabWidget *buildtab = new QTabWidget( vbox );

    CustomBuildOptionsWidget *w2 = new CustomBuildOptionsWidget( *projectDom(), buildtab );
    connect( dlg, SIGNAL( okClicked() ), w2, SLOT( accept() ) );
    buildtab->addTab( w2, i18n( "&Build" ) );

    CustomOtherConfigWidget *w4 = new CustomOtherConfigWidget( this, "/kdevcustomproject", buildtab );
    connect( dlg, SIGNAL( okClicked() ), w4, SLOT( accept() ) );
    buildtab->addTab( w4, i18n( "&Other" ) );

    CustomMakeConfigWidget *w3 = new CustomMakeConfigWidget( this, "/kdevcustomproject", buildtab );
    buildtab->addTab( w3, i18n( "Ma&ke" ) );
    w2->setMakeOptionsWidget( buildtab, w3, w4 );
    connect( dlg, SIGNAL( okClicked() ), w3, SLOT( accept() ) );

}


void CustomProjectPart::contextMenu( QPopupMenu *popup, const Context *context )
{
    if ( !context->hasType( Context::FileContext ) )
        return;

    const FileContext *fcontext = static_cast<const FileContext*>( context );

    m_contextAddFiles.clear();
    m_contextRemoveFiles.clear();

    QString popupstr = fcontext->urls().first().fileName();

    if ( popupstr == QString::null )
        popupstr = ".";

    if ( fcontext->urls().size() == 1 && URLUtil::isDirectory( fcontext->urls().first() ) && !isInBlacklist( fcontext->urls().first().path() ) )
    {
        popup->insertSeparator();
        // remember the name of the directory
        m_contextDirName = fcontext->urls().first().path();
        m_contextDirName = m_contextDirName.mid( project()->projectDirectory().length() + 1 );
        int id = popup->insertItem( i18n( "Make Active Directory" ),
                                    this, SLOT( slotChooseActiveDirectory() ) );
        popup->setWhatsThis( id, i18n( "<b>Make active directory</b><p>"
                                       "Chooses this directory as the destination for new files created using wizards "
                                       "like the <i>New Class</i> wizard." ) );
    }

    kdDebug( 9025 ) << "context urls: " << fcontext->urls() << endl;
    if ( fcontext->urls().size() == 1 && ( isProjectFileType( fcontext->urls().first().path() ) || URLUtil::isDirectory( fcontext->urls().first() ) ) )
    {
        popup->insertSeparator();
        m_contextDirName = fcontext->urls().first().path();
        m_contextDirName = m_contextDirName.mid( project()->projectDirectory().length() + 1 );
        int id;
        if ( isInBlacklist( m_contextDirName ) )
        {
            id = popup->insertItem( i18n( "Remove from blacklist" ),
                                    this, SLOT( slotChangeBlacklist() ) );
            popup->setWhatsThis( id, i18n( "<b>Remove from blacklist</b><p>"
                                           "Removes the given file or directory from the "
                                           "blacklist if its already in it.<br>The blacklist contains files and"
                                           " directories that should be ignored even if they match a project filetype "
                                           "pattern" ) );
        }
        else
        {
            id = popup->insertItem( i18n( "Add to blacklist" ),
                                    this, SLOT( slotChangeBlacklist() ) );
            popup->setWhatsThis( id, i18n( "<b>Add to blacklist</b><p>"
                                           "Adds the given file or directory to the blacklist.<br>The blacklist contains files and"
                                           " directories that should be ignored even if they match a project filetype "
                                           "pattern" ) );
        }
    }

    const KURL::List urls = fcontext->urls();

    bool dirAddRecursive = false;
    bool dirDelRecursive = false;

    for ( KURL::List::ConstIterator it = urls.begin(); it != urls.end(); ++it )
    {
        kdDebug( 9025 ) << "Checking URL: " << *it << endl;
        QString canPath( URLUtil::canonicalPath(( *it ).path() ) );
        QString relPath = relativeToProject( canPath );
        kdDebug( 9025 ) << "relpath: " << relPath << "|canpath: " << canPath << endl;
        if ( isInBlacklist( relPath ) )
            continue;
        if ((( *it ).isLocalFile() && isProjectFileType(( *it ).fileName() ) ) )
        {
            if ( project()->isProjectFile( canPath ) )
                m_contextRemoveFiles << relPath;
            if ( !project()->isProjectFile( canPath ) )
                m_contextAddFiles << relPath;
        }
        if ( QFileInfo(( *it ).path() ).isDir() )
        {
            if ( containsProjectFiles( canPath ) || project()->isProjectFile( canPath ) )
            {
                if ( containsProjectFiles( canPath ) )
                    dirDelRecursive = true;
                m_contextRemoveFiles << relPath;
            }
            if ( containsNonProjectFiles( canPath ) || !project()->isProjectFile( canPath ) )
            {
                if ( containsNonProjectFiles( canPath ) )
                    dirAddRecursive = true;
                m_contextAddFiles << relPath;
            }
        }
    }

    if ( m_contextAddFiles.size() > 0 || m_contextRemoveFiles.size() > 0 )
        popup->insertSeparator();
    if ( m_contextAddFiles.size() > 0 )
    {
        int id = popup->insertItem( i18n( "Add Selected File/Dir(s) to Project" ),
                                    this, SLOT( slotAddToProject() ) );
        popup->setWhatsThis( id, i18n( "<b>Add to project</b><p>Adds selected file/dir(s) to the list of files in project. "
                                       "Note that the files should be manually added to corresponding makefile or build.xml." ) );
        if ( dirAddRecursive )
        {
            int id = popup->insertItem( i18n( "Add Selected Dir(s) to Project (recursive)" ),
                                        this, SLOT( slotAddToProjectRecursive() ) );
            popup->setWhatsThis( id, i18n( "<b>Add to project</b><p>Recursively adds selected dir(s) to the list of files in project. "
                                           "Note that the files should be manually added to corresponding makefile or build.xml." ) );
        }
    }

    if ( m_contextRemoveFiles.size() > 0 )
    {
        int id = popup->insertItem( i18n( "Remove Selected File/Dir(s) From Project" ),
                                    this, SLOT( slotRemoveFromProject() ) );
        popup->setWhatsThis( id, i18n( "<b>Remove from project</b><p>Removes selected file/dir(s) from the list of files in project. "
                                       "Note that the files should be manually excluded from corresponding makefile or build.xml." ) );

        if ( dirDelRecursive )
        {
            int id = popup->insertItem( i18n( "Remove Selected Dir(s) From Project (recursive)" ),
                                        this, SLOT( slotRemoveFromProjectRecursive() ) );
            popup->setWhatsThis( id, i18n( "<b>Remove from project</b><p>Recursively removes selected dir(s) from the list of files in project. "
                                           "Note that the files should be manually excluded from corresponding makefile or build.xml." ) );
        }
    }
}


void CustomProjectPart::slotAddToProject()
{
    m_recursive = false;
    m_first_recursive = true;
    addFiles( m_contextAddFiles );
}


void CustomProjectPart::slotRemoveFromProject()
{
    m_recursive = false;
    m_first_recursive = true;
    removeFiles( m_contextRemoveFiles );
}


void CustomProjectPart::slotAddToProjectRecursive()
{
    m_recursive = true;
    addFiles( m_contextAddFiles );
    m_recursive = false;
}


void CustomProjectPart::slotRemoveFromProjectRecursive()
{
    m_recursive = true;
    removeFiles( m_contextRemoveFiles );
    m_recursive = false;
}

void CustomProjectPart::slotChangeBlacklist()
{
    switchBlacklistEntry( m_contextDirName );
}

void CustomProjectPart::slotChooseActiveDirectory()
{
    QString olddir = activeDirectory();
    QDomDocument &dom = *projectDom();
    DomUtil::writeEntry( dom, "/kdevcustomproject/general/activedir", m_contextDirName );
    emit activeDirectoryChanged( olddir, activeDirectory() );
}


void CustomProjectPart::openProject( const QString &dirName, const QString &projectName )
{
    m_projectDirectory = dirName;
    m_projectName = projectName;

    QDomDocument &dom = *projectDom();
    // Set the default directory radio to "executable"
    if ( DomUtil::readEntry( dom, "/kdevcustomproject/run/directoryradio" ) == "" )
    {
        DomUtil::writeEntry( dom, "/kdevcustomproject/run/directoryradio", "executable" );
    }

    if ( filetypes().isEmpty() )
    {
        QStringList types;
        types << "*.java" << "*.h" << "*.H" << "*.hh" << "*.hxx" << "*.hpp" << "*.c" << "*.C"
        << "*.cc" << "*.cpp" << "*.c++" << "*.cxx" << "Makefile" << "CMakeLists.txt";
        DomUtil::writeListEntry( dom, "/kdevcustomproject/filetypes", "filetype", types );
    }

    /*this entry is currently only created by the cmake kdevelop3 project generator
     in order to support completely-out-of-source builds, where nothing, not
     even the kdevelop project files are created in the source directory, Alex <neundorf@kde.org>
     */
    m_filelistDir = DomUtil::readEntry( dom, "/kdevcustomproject/filelistdirectory" );
    if ( m_filelistDir.isEmpty() )
        m_filelistDir = dirName;

    if ( QFileInfo( m_filelistDir + "/" + projectName.lower() +
                    ".kdevelop.filelist" ).exists() )
    {
        QDir( m_filelistDir ).rename(
            projectName.lower() + ".kdevelop.filelist",
            projectName + ".kdevelop.filelist" );
    }

    QFile f( m_filelistDir + "/" + projectName + ".kdevelop.filelist" );
    if ( f.open( IO_ReadOnly ) )
    {
        QTextStream stream( &f );
        while ( !stream.atEnd() )
        {
            QString s = stream.readLine();
            // Skip comments.
            if ( s.isEmpty() || s.startsWith( "#" ) )
                continue;
            // Skip non-existent files.
            if ( ! QFileInfo( projectDirectory() + "/" + s ).exists() )
                continue;
            // Do not bother with files already in project or on blacklist.
            if ( isInProject( s ) || isInBlacklist( s ) )
                continue;
            addToProject( s );
        }
        QStringList newfiles;
        findNewFiles( dirName, newfiles );

        if ( newfiles.count() > 0 )
        {
            addNewFilesToProject( newfiles );
        }

    }
    else
    {
        int r = KMessageBox::questionYesNo( mainWindow()->main(),
                                            i18n( "This project does not contain any files yet.\n"
                                                  "Populate it with all C/C++/Java files below "
                                                  "the project directory?" ), QString::null, i18n( "Populate" ), i18n( "Do Not Populate" ) );
        if ( r == KMessageBox::Yes )
            populateProject();
    }


    // check if there is an old envvars entry (from old project file with single make environment)
    QString buildtool = DomUtil::readEntry( dom , "/kdevcustomproject/build/buildtool" );
    QDomElement el =
        DomUtil::elementByPath( dom , "/kdevcustomproject/" + buildtool + "/envvars" );
    if ( !el.isNull() )
    {
        QDomElement envs = DomUtil::createElementByPath( dom , "/kdevcustomproject/" + buildtool + "/environments" );
        DomUtil::makeEmpty( envs );
        el.setTagName( "default" );
        envs.appendChild( el );
    }
    KDevProject::openProject( dirName, projectName );
}


/**
 * @brief Recursively search given directory searching for files which may be part of this project.
 *
 * The files found not in a black list,
 * and not being already part of our project are gathered.
 *
 * @param dir directory to scan (and recurse) for potential project files.
 * @param[out] fileList the list of files found.
 */
void CustomProjectPart::findNewFiles( const QString& dir, QStringList& filelist ) const
{
    if ( dir.isEmpty() )
        return;
    QStringList fileentries = QDir( dir ).entryList( filetypes().join( ";" ) );
    QStringList dirs = QDir( dir ).entryList( QDir::Dirs );
    QStringList entries = fileentries + dirs;
    QString relpath = relativeToProject( dir );
    if ( !relpath.isEmpty() )
        relpath += "/";
    for ( QStringList::const_iterator it = entries.begin(); it != entries.end(); ++it )
    {
        // Only process genuine entries - files and directories.
        if (( *it == "." ) || ( *it == ".." ) )
            continue;
        // If the entry (be it a file or a directory) is already part of this project, proceed to next one.
        const QString relativeEntry( relpath + *it );
        if ( isInProject( relativeEntry ) )
            continue;
        // If the entry is blacklisted, proceed to next one.
        // Note that by using generic isInBlacklist(),
        // we are actually wasting resources,
        // because it also tests whether any parent directory of relativeEntry is blacklisted.
        // But by the order we are traversing and processing the directories,
        // we know it is not the case, ever.
        if ( isInBlacklist( relativeEntry ) )
            continue;
        // We have a new, non-blacklisted entry.
        // Recurse into it (a directory) or add it to the potential list of new project files.
        const QString absoluteEntry( dir + "/" + *it );
        if ( QFileInfo( absoluteEntry ).isFile() )
        {
            filelist << relativeEntry;
        }
        else if ( QFileInfo( absoluteEntry ).isDir() )
        {
            bool searchRecursive = true;
            QFileInfo fi( absoluteEntry );
            if( fi.isSymLink() )
            {
                QString realDir = fi.readLink(); 
                if( QFileInfo( realDir ).exists() )
                {
                    for( QStringList::const_iterator it = filelist.constBegin(); it != filelist.constEnd(); ++it )
                    {

                        if( QFileInfo(projectDirectory()+"/"+*it).absFilePath().startsWith( realDir ) )
                        {
                            searchRecursive = false;
                        }
                    }
                } else {
                    searchRecursive = false;
                }
            }
            if( searchRecursive )
            {
                findNewFiles( absoluteEntry, filelist );
            }
        }
    }
}


void CustomProjectPart::populateProject()
{

    KDialogBase* dlg = new KDialogBase( mainWindow()->main(), "typeselector", true,
                                        "Select filetypes of project", KDialogBase::Ok | KDialogBase::Cancel );
    QVBox* box = dlg->makeVBoxMainWidget();
    KEditListBox* lb = new KEditListBox( "Filetypes in the project", box, "selecttypes",
                                         false, KEditListBox::Add | KEditListBox::Remove );
    lb->setItems( filetypes() );
    if ( dlg->exec() == QDialog::Accepted )
    {
        setFiletypes( lb->items() );
    }

    QApplication::setOverrideCursor( Qt::waitCursor );
    removeFiles( allFiles() );
    updateBlacklist( QStringList() );

    QStringList newlist;

    findNewFiles( projectDirectory(), newlist );

    QApplication::restoreOverrideCursor();
    addNewFilesToProject( newlist );
}


void CustomProjectPart::closeProject()
{
    saveProject();
}

void CustomProjectPart::saveProject()
{
    QFile f( m_filelistDir + "/" + m_projectName + ".kdevelop.filelist" );
    if ( !f.open( IO_WriteOnly ) )
        return;

    QTextStream stream( &f );
    stream << "# KDevelop Custom Project File List" << endl;

    ProjectFilesSet::ConstIterator it;
    for ( it = m_sourceFilesSet.constBegin(); it != m_sourceFilesSet.constEnd(); ++it )
        stream << it.key() << endl;
    f.close();
}


QString CustomProjectPart::projectDirectory() const
{
    return m_projectDirectory;
}


QString CustomProjectPart::projectName() const
{
    return m_projectName;
}


/** Retuns a PairList with the run environment variables */
DomUtil::PairList CustomProjectPart::runEnvironmentVars() const
{
    return DomUtil::readPairListEntry( *projectDom(), "/kdevcustomproject/run/envvars", "envvar", "name", "value" );
}


/** Retuns the currently selected run directory
  * The returned string can be:
  *   if run/directoryradio == executable
  *        The directory where the executable is
  *   if run/directoryradio == build
  *        The directory where the executable is relative to build directory
  *   if run/directoryradio == custom
  *        The custom directory absolute path
  */
QString CustomProjectPart::runDirectory() const
{
    QString cwd = defaultRunDirectory( "kdevcustomproject" );
    if ( cwd.isEmpty() )
        cwd = buildDirectory();
    return cwd;
}


/** Retuns the currently selected main program
  * The returned string can be:
  *   if run/directoryradio == executable
  *        The executable name
  *   if run/directoryradio == build
  *        The path to executable relative to build directory
  *   if run/directoryradio == custom or relative == false
  *        The absolute path to executable
  */
QString CustomProjectPart::mainProgram() const
{
    QDomDocument * dom = projectDom();

    if ( !dom ) return QString();

    QString DomMainProgram = DomUtil::readEntry( *dom, "/kdevcustomproject/run/mainprogram" );

    if ( DomMainProgram.isEmpty() ) return QString();

    if ( DomMainProgram.startsWith( "/" ) ) // assume absolute path
    {
        return DomMainProgram;
    }
    else // assume project relative path
    {
        return projectDirectory() + "/" + DomMainProgram;
    }

    return QString();
}

/** Retuns a QString with the debug command line arguments */
QString CustomProjectPart::debugArguments() const
{
    return DomUtil::readEntry( *projectDom(), "/kdevcustomproject/run/globaldebugarguments" );
}


/** Retuns a QString with the run command line arguments */
QString CustomProjectPart::runArguments() const
{
    return DomUtil::readEntry( *projectDom(), "/kdevcustomproject/run/programargs" );
}

QString CustomProjectPart::activeDirectory() const
{
    QDomDocument &dom = *projectDom();
    return DomUtil::readEntry( dom, "/kdevcustomproject/general/activedir", "." );
}


QStringList CustomProjectPart::allFiles() const
{
    return m_sourceFilesSet.keys();
}


void CustomProjectPart::addFile( const QString &fileName )
{
    QStringList fileList;
    fileList.append( fileName );

    this->addFiles( fileList );
}

void CustomProjectPart::addFiles( const QStringList& fileList )
{
    QStringList::ConstIterator it;
    QStringList addedFiles;
    QStringList myfileList = fileList;
    kdDebug( 9025 ) << "Adding files: " << myfileList << endl;
    myfileList.remove( "." );
    myfileList.remove( "" );
    myfileList.remove( ".." );
    for ( it = myfileList.begin(); it != myfileList.end(); ++it )
    {
        if ( isInBlacklist( *it ) )
            continue;
        QString relpath;
        kdDebug( 9025 ) << "Checking path: " << *it << endl;
        if ( QDir::isRelativePath( *it ) )
        {
            kdDebug( 9025 ) << *it << " is relative" << endl;
            relpath = *it;
        }
        else
        {
            kdDebug( 9025 ) << *it << " is not relative" << endl;
            relpath = relativeToProject( *it );
        }

        if ( !QFileInfo( projectDirectory() + "/" + relpath ).exists() )
            continue;

        if ( QFileInfo( projectDirectory() + "/" + relpath ).isDir() && ( m_recursive || m_first_recursive ) )
        {
            kdDebug( 9025 ) << "is a dir and " << m_recursive << "|" << m_first_recursive << endl;
            m_first_recursive = false;
            QStringList fileentries = QDir( projectDirectory() + "/" + relpath ).entryList( filetypes().join( ";" ) );
            QStringList dirs = QDir( projectDirectory() + "/" + relpath ).entryList( QDir::Dirs );
            QStringList subentries = fileentries + dirs;
            for ( QStringList::iterator subit = subentries.begin(); subit != subentries.end(); ++subit )
            {
                if ( *subit != "." && *subit != ".." )
                    *subit = relpath + "/" + ( *subit );
                if (( *subit ).startsWith( "/" ) )
                    *subit = ( *subit ).mid( 1, ( *subit ).length() );
            }
            addFiles( subentries );
            addedFiles << relpath;
            addToProject( relpath );
            m_first_recursive = true;
        }
        else if ( isProjectFileType( QFileInfo( relpath ).fileName() ) && ( ! isInProject( relpath ) ) )
        {
            QStringList paths = QStringList::split( "/", relpath );
            paths.pop_back();
            QString path;
            for ( QStringList::const_iterator it = paths.begin(); it != paths.end(); ++it )
            {
                path += *it;
                if ( ! isInProject( path ) )
                {
                    addedFiles << path;
                    addToProject( path );
                }
                path += "/";
            }
            addedFiles << relpath;
            addToProject( relpath );
        }
        else
        {
            kdDebug( 9025 ) << "not adding " << relpath << endl;
        }
    }
    m_first_recursive = false;
    saveProject();

    kdDebug( 9025 ) << "Emitting addedFilesToProject" << addedFiles << endl;
    emit addedFilesToProject( addedFiles );
}

void CustomProjectPart::removeFile( const QString &fileName )
{
    QStringList fileList;
    fileList.append( fileName );

    this->removeFiles( fileList );
}

void CustomProjectPart::removeFiles( const QStringList& fileList )
{
    kdDebug( 9025 ) << "Emitting removedFilesFromProject" << fileList << endl;
    QStringList removedFiles;
    QStringList myfileList = fileList;
    QStringList::ConstIterator it;
    myfileList.remove( "." );
    myfileList.remove( ".." );
    myfileList.remove( "" );

    for ( it = myfileList.begin(); it != myfileList.end(); ++it )
    {
        QString relpath;
        if ( QDir::isRelativePath( *it ) )
            relpath = *it;
        else
            relpath = relativeToProject( *it );

        if ( QFileInfo( projectDirectory() + "/" + relpath ).isDir() && ( m_recursive || m_first_recursive ) )
        {
            m_first_recursive = false;
            QStringList fileentries = QDir( projectDirectory() + "/" + relpath ).entryList( filetypes().join( ";" ) );
            QStringList dirs = QDir( projectDirectory() + "/" + relpath ).entryList( QDir::Dirs );
            QStringList subentries = fileentries + dirs;
            for ( QStringList::iterator subit = subentries.begin(); subit != subentries.end(); ++subit )
                if ( *subit != "." && *subit != ".." )
                    *subit = relpath + "/" + ( *subit );
            removeFiles( subentries );
            if ( !containsProjectFiles( relpath ) )
            {
                removedFiles << relpath;
                removeFromProject( relpath );
            }
            m_first_recursive = true;
        }
        else if ( isInProject( relpath ) )
        {
            removedFiles << relpath;
            removeFromProject( relpath );
            QStringList paths = QStringList::split( "/", relpath );
            QString lastsubentry = paths[paths.size()-1];
            paths.pop_back();
            while ( paths.size() > 0 )
            {
                QString dir = paths.join( "/" );
                QStringList projectentries = projectFilesInDir( dir );
                if ( projectentries.size() == 0 )
                {
                    removedFiles << dir;
                    removeFromProject( dir );
                }
                else
                    break;
                lastsubentry = paths[paths.size()-1];
                paths.pop_back();
            }
        }
    }

    saveProject();
    emit removedFilesFromProject( removedFiles );
}

QString CustomProjectPart::buildDirectory() const
{
    QString dir = DomUtil::readEntry( *projectDom(), "/kdevcustomproject/build/builddir" );
    if ( dir.isEmpty() )
        return projectDirectory();
    if ( QFileInfo( dir ).isRelative() )
        return QDir::cleanDirPath( projectDirectory() + "/" + dir );
    return dir;
}


QString CustomProjectPart::makeEnvironment() const
{
    // Get the make environment variables pairs into the environstr string
    // in the form of: "ENV_VARIABLE=ENV_VALUE"
    // Note that we quote the variable value due to the possibility of
    // embedded spaces
    QString buildtool = DomUtil::readEntry( *projectDom(), "/kdevcustomproject/build/buildtool" );
    DomUtil::PairList envvars =
        DomUtil::readPairListEntry( *projectDom(), "/kdevcustomproject/" + buildtool + "/environments/" + currentMakeEnvironment(), "envvar", "name", "value" );

    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for ( it = envvars.begin(); it != envvars.end(); ++it )
    {
        environstr += ( *it ).first;
        environstr += "=";
        environstr += EnvVarTools::quote(( *it ).second );
        environstr += " ";
    }

    KConfigGroup grp( kapp->config(), "MakeOutputView" );
    if( grp.readBoolEntry( "ForceCLocale", true ) )
        environstr += "LC_MESSAGES=" + EnvVarTools::quote( "C" )+" "+" "+"LC_CTYPE="+EnvVarTools::quote("C")+" ";

    return environstr;
}


void CustomProjectPart::startMakeCommand( const QString &dir, const QString &target, bool withKdesu )
{
    if ( partController()->saveAllFiles() == false )
        return; //user cancelled

    QDomDocument &dom = *projectDom();
    QString buildtool = DomUtil::readEntry( dom, "/kdevcustomproject/build/buildtool" );

    QString cmdline;
    if ( buildtool == "ant" )
    {
        cmdline = "ant";
    }
    else if ( buildtool == "other" )
    {
        cmdline = DomUtil::readEntry( dom, "/kdevcustomproject/other/otherbin" );
        if ( cmdline.isEmpty() )
            cmdline = "echo";
        else if ( cmdline.find( "/" ) == -1 )
            cmdline = "./" + cmdline;
        cmdline += " " + DomUtil::readEntry( dom, "/kdevcustomproject/other/otheroptions" );
    }
    else
    {
        cmdline = DomUtil::readEntry( dom, "/kdevcustomproject/make/makebin" );
        if ( cmdline.isEmpty() )
            cmdline = MAKE_COMMAND;
        if ( !DomUtil::readBoolEntry( dom, "/kdevcustomproject/make/abortonerror" ) )
            cmdline += " -k";
        int jobs = DomUtil::readIntEntry( dom, "/kdevcustomproject/make/numberofjobs" );
        if ( jobs != 0 )
        {
            cmdline += " -j";
            cmdline += QString::number( jobs );
        }
        if ( DomUtil::readBoolEntry( dom, "/kdevcustomproject/make/dontact" ) )
            cmdline += " -n";
        cmdline += " " + DomUtil::readEntry( dom, "/kdevcustomproject/make/makeoptions" );
    }

    cmdline += " ";
    if ( !target.isEmpty() )
        cmdline += KProcess::quote( target );

    QString dircmd = "cd ";
    dircmd += KProcess::quote( dir );
    dircmd += " && ";

    int prio = DomUtil::readIntEntry( dom, "/kdevcustomproject/" + buildtool + "/prio" );
    QString nice;
    if ( prio != 0 )
    {
        nice = QString( "nice -n%1 " ).arg( prio );
    }

    cmdline.prepend( nice );
    cmdline.prepend( makeEnvironment() );

    if ( withKdesu )
        cmdline = "kdesu -t -c '" + cmdline + "'";

    m_buildCommand = dircmd + cmdline;


    makeFrontend()->queueCommand( dir, dircmd + cmdline );
}


void CustomProjectPart::slotBuild()
{
    m_lastCompilationFailed = false;
    QString buildtool = DomUtil::readEntry( *projectDom(), "/kdevcustomproject/build/buildtool" );
    startMakeCommand( buildDirectory(), DomUtil::readEntry( *projectDom(),
                      "/kdevcustomproject/" + buildtool + "/defaulttarget" ) );
}

void CustomProjectPart::slotBuildActiveDir()
{
    m_lastCompilationFailed = false;
    QString buildtool = DomUtil::readEntry( *projectDom(), "/kdevcustomproject/build/buildtool" );
    startMakeCommand( buildDirectory() + "/" + activeDirectory(), DomUtil::readEntry( *projectDom(),
                      "/kdevcustomproject/" + buildtool + "/defaulttarget" ) );
}


void CustomProjectPart::slotCompileFile()
{
    KParts::ReadWritePart *part = dynamic_cast<KParts::ReadWritePart*>( partController()->activePart() );
    if ( !part || !part->url().isLocalFile() )
        return;

    QString fileName = part->url().path();
    QFileInfo fi( fileName );
    QString sourceDir = fi.dirPath();
    QString baseName = fi.baseName( true );
    kdDebug( 9025 ) << "Compiling " << fileName
    << "in dir " << sourceDir
    << " with baseName " << baseName << endl;

    // What would be nice: In case of non-recursive build system, climb up from
    // the source dir until a Makefile is found

    QString buildDir = sourceDir;
    QString target = baseName + ".o";

    QString buildtool = DomUtil::readEntry( *projectDom(), "/kdevcustomproject/build/buildtool" );

    //if there is no Makefile in the directory of the source file
    //try to build it from the main build dir
    //this works e.g. for non-recursive cmake Makefiles, Alex
    if ( buildtool == "make" && ( QFile::exists( sourceDir + "/Makefile" ) == false )
            && ( QFile::exists( sourceDir + "/makefile" ) == false ) )
    {
        buildDir = buildDirectory();
    }

    startMakeCommand( buildDir, target );
}

void CustomProjectPart::slotInstallActiveDir()
{
    startMakeCommand( buildDirectory() + "/" + activeDirectory(), QString::fromLatin1( "install" ) );
}

void CustomProjectPart::slotInstall()
{
    startMakeCommand( buildDirectory(), QString::fromLatin1( "install" ) );
}


void CustomProjectPart::slotInstallWithKdesu()
{
    // First issue "make" to build the entire project with the current user
    // This way we make sure all files are up to date before we do the "make install"
    slotBuild();

    // After that issue "make install" with the root user
    startMakeCommand( buildDirectory(), QString::fromLatin1( "install" ), true );
}

void CustomProjectPart::slotClean()
{
    startMakeCommand( buildDirectory(), QString::fromLatin1( "clean" ) );
}


void CustomProjectPart::slotExecute()
{
    partController()->saveAllFiles();

    bool _auto = false;
    if ( DomUtil::readBoolEntry( *projectDom(), "/kdevcustomproject/run/autocompile", true ) && ( isDirty() || !QFileInfo( mainProgram() ).exists() ) )
    {
        m_executeAfterBuild = true;
        slotBuild();
        _auto = true;
    }

    if ( DomUtil::readBoolEntry( *projectDom(), "/kdevcustomproject/run/autoinstall", false ) && ( isDirty() || !QFileInfo( mainProgram() ).exists() ) )
    {
        m_executeAfterBuild = true;
        // Use kdesu??
        if ( DomUtil::readBoolEntry( *projectDom(), "/kdevcustomproject/run/autokdesu", false ) )
            //slotInstallWithKdesu assumes that it hasn't just been build...
            _auto ? slotInstallWithKdesu() : startMakeCommand( buildDirectory(), QString::fromLatin1( "install" ), true );
        else
            slotInstall();
        _auto = true;
    }

    if ( _auto )
        return;

    // Get the run environment variables pairs into the environstr string
    // in the form of: "ENV_VARIABLE=ENV_VALUE"
    // Note that we quote the variable value due to the possibility of
    // embedded spaces
    DomUtil::PairList envvars = runEnvironmentVars();
    QString environstr;
    DomUtil::PairList::ConstIterator it;
    for ( it = envvars.begin(); it != envvars.end(); ++it )
    {
        environstr += ( *it ).first;
        environstr += "=";
        environstr += EnvVarTools::quote(( *it ).second );
        environstr += " ";
    }

    if ( mainProgram().isEmpty() )
        // Do not execute non executable targets
        return;

    QString program = environstr;
    program += mainProgram();
    program += " " + runArguments();

    bool inTerminal = DomUtil::readBoolEntry( *projectDom(), "/kdevcustomproject/run/terminal" );

    kdDebug( 9025 ) << "runDirectory: <" << runDirectory() << ">" << endl;
    kdDebug( 9025 ) << "environstr  : <" << environstr << ">" << endl;
    kdDebug( 9025 ) << "mainProgram : <" << mainProgram() << ">" << endl;
    kdDebug( 9025 ) << "runArguments: <" << runArguments() << ">" << endl;

    appFrontend()->startAppCommand( runDirectory(), program, inTerminal );
}

void CustomProjectPart::updateTargetMenu()
{
    m_targets.clear();
    m_targetsObjectFiles.clear();
    m_targetsOtherFiles.clear();
    m_targetMenu->clear();
    m_targetObjectFilesMenu->clear();
    m_targetOtherFilesMenu->clear();

    QDomDocument &dom = *projectDom();
    bool ant = DomUtil::readEntry( dom, "/kdevcustomproject/build/buildtool" ) == "ant";

    if ( ant )
    {
        QFile f( buildDirectory() + "/build.xml" );
        if ( !f.open( IO_ReadOnly ) )
        {
            kdDebug( 9025 ) << "No build file" << endl;
            return;
        }
        QDomDocument dom;
        if ( !dom.setContent( &f ) )
        {
            kdDebug( 9025 ) << "Build script not valid xml" << endl;
            f.close();
            return;
        }
        f.close();

        QDomNode node = dom.documentElement().firstChild();
        while ( !node.isNull() )
        {
            if ( node.toElement().tagName() == "target" )
                m_targets.append( node.toElement().attribute( "name" ) );
            node = node.nextSibling();
        }
    }
    else
    {
        kdDebug( 9025 ) << "Trying to load a makefile... " << endl;

        m_makefileVars.clear();
        m_parsedMakefiles.clear();
        m_makefilesToParse.clear();
        m_makefilesToParse.push( "Makefile" );
        m_makefilesToParse.push( "makefile" );
        putEnvVarsInVarMap();
        while ( !m_makefilesToParse.isEmpty() )
            parseMakefile( m_makefilesToParse.pop() );

        //free the memory again
        m_makefileVars.clear();
        m_parsedMakefiles.clear();

        m_targets.sort();
        m_targetsObjectFiles.sort();
        m_targetsOtherFiles.sort();

    }

    m_targetMenu->insertItem( i18n( "Object Files" ), m_targetObjectFilesMenu );
    m_targetMenu->insertItem( i18n( "Other Files" ), m_targetOtherFilesMenu );

    int id = 0;
    QStringList::ConstIterator it;
    for ( it = m_targets.begin(); it != m_targets.end(); ++it )
        m_targetMenu->insertItem( *it, id++ );

    id = 0;
    for ( it = m_targetsObjectFiles.begin(); it != m_targetsObjectFiles.end(); ++it )
        m_targetObjectFilesMenu->insertItem( *it, id++ );

    id = 0;
    for ( it = m_targetsOtherFiles.begin(); it != m_targetsOtherFiles.end(); ++it )
        m_targetOtherFilesMenu->insertItem( *it, id++ );
}

void CustomProjectPart::putEnvVarsInVarMap()
{
    DomUtil::PairList envvars =
        DomUtil::readPairListEntry( *projectDom(), "/kdevcustomproject/make/environments/" + currentMakeEnvironment(), "envvar", "name", "value" );

    for ( DomUtil::PairList::ConstIterator it = envvars.begin(); it != envvars.end(); ++it )
        m_makefileVars[( *it ).first] = ( *it ).second;  //is qouting here required as in makeEnvironment() ??
}

void CustomProjectPart::parseMakefile( const QString& filename )
{
    if ( m_parsedMakefiles.contains( filename ) )
        return;

    m_parsedMakefiles.insert( filename, 1 );

    QString absFilename = filename;
    if ( !filename.startsWith( "/" ) )
        absFilename = buildDirectory() + "/" + filename;

    QFile f( absFilename );
    if ( !f.open( IO_ReadOnly ) )
    {
        kdDebug( 9025 ) << "could not open " << absFilename << endl;
        return;
    }
    QRegExp targetRe( "^ *([^\\t$.#]\\S+) *:.*$" );
    targetRe.setMinimal( true );

    QRegExp variablesRe( "\\$\\(\\s*([^\\)\\s]+)\\s*\\)" );
    QRegExp assignmentRe( "^\\s*(\\S+)\\s*[:\\?]?=\\s*(\\S+)\\s*(#.*)?$" );

    QRegExp includedMakefilesRe( "^include\\s+(\\S+)" );
    QString str = "";
    while ( !f.atEnd() )
    {
        f.readLine( str, 200 );

        // Replace any variables in the current line
        int offset = -1;
        while (( offset = variablesRe.search( str, offset + 1 ) ) != -1 )
        {
            QString variableName = variablesRe.cap( 1 ).simplifyWhiteSpace();
            if ( m_makefileVars.contains( variableName ) )
            {
                str.replace( variablesRe.cap( 0 ), m_makefileVars[variableName] );
            }
        }

        // Read all continuation lines
        // kdDebug(9025) << "Trying: " << str.simplifyWhiteSpace() << endl;
        //while (str.right(1) == "\\" && !stream.atEnd()) {
        //    str.remove(str.length()-1, 1);
        //    str += stream.readLine();
        //}
        // Find any variables
        if ( assignmentRe.search( str ) != -1 )
        {
            m_makefileVars[assignmentRe.cap( 1 ).simplifyWhiteSpace()] = assignmentRe.cap( 2 ).simplifyWhiteSpace();
        }
        else if ( includedMakefilesRe.search( str ) != -1 )
        {
            QString includedMakefile = includedMakefilesRe.cap( 1 ).simplifyWhiteSpace();
            m_makefilesToParse.push( includedMakefile );
        }
        else if ( targetRe.search( str ) != -1 )
        {
            QString tmpTarget = targetRe.cap( 1 ).simplifyWhiteSpace();
            if ( tmpTarget.endsWith( ".o" ) )
            {
                if ( m_targetsObjectFiles.find( tmpTarget ) == m_targetsObjectFiles.end() )
                    m_targetsObjectFiles += tmpTarget;
            }
            else if ( tmpTarget.contains( '.' ) )
            {
                if ( m_targetsOtherFiles.find( tmpTarget ) == m_targetsOtherFiles.end() )
                    m_targetsOtherFiles += tmpTarget;
            }
            else
            {
                if ( m_targets.find( tmpTarget ) == m_targets.end() )
                    m_targets += tmpTarget;
            }
        }
    }
    f.close();
}

void CustomProjectPart::targetMenuActivated( int id )
{
    QString target = m_targets[id];
    startMakeCommand( buildDirectory(), target );
}

void CustomProjectPart::targetObjectFilesMenuActivated( int id )
{
    QString target = m_targetsObjectFiles[id];
    startMakeCommand( buildDirectory(), target );
}

void CustomProjectPart::targetOtherFilesMenuActivated( int id )
{
    QString target = m_targetsOtherFiles[id];
    startMakeCommand( buildDirectory(), target );
}

void CustomProjectPart::updateMakeEnvironmentsMenu()
{
    QDomDocument &dom = *projectDom();
    bool makeUsed = ( DomUtil::readEntry( dom, "/kdevcustomproject/build/buildtool" ) == "make" );
    if ( makeUsed )
    {
        QStringList l = allMakeEnvironments();
        m_makeEnvironmentsSelector->setItems( l );
        m_makeEnvironmentsSelector->setCurrentItem( l.findIndex( currentMakeEnvironment() ) );
    }
    else
    {
        m_makeEnvironmentsSelector->clear();
    }
    /*
    m_makeEnvironmentsMenu->clear();
    QDomDocument &dom = *projectDom();

        QStringList environments = allMakeEnvironments();
        QStringList::ConstIterator it;
        int id = 0;
        for (it = environments.begin(); it != environments.end(); ++it)
            m_makeEnvironmentsMenu->insertItem(*it, id++);
    }
    */
}

void CustomProjectPart::makeEnvironmentsMenuActivated( int id )
{
    QDomDocument &dom = *projectDom();
    QString environment = allMakeEnvironments()[id];
    DomUtil::writeEntry( dom, "/kdevcustomproject/make/selectedenvironment", environment );
}

void CustomProjectPart::slotCommandFinished( const QString& command )
{
    kdDebug( 9025 ) << "CustomProjectPart::slotProcessFinished()" << endl;

    if ( m_buildCommand != command )
        return;

    m_buildCommand = QString::null;

    m_timestamp.clear();
    QStringList fileList = allFiles();
    QStringList::Iterator it = fileList.begin();
    while ( it != fileList.end() )
    {
        QString fileName = *it;
        ++it;

        m_timestamp[ fileName ] = QFileInfo( projectDirectory(), fileName ).lastModified();
    }

    emit projectCompiled();

    if ( m_executeAfterBuild )
    {
        slotExecute();
        m_executeAfterBuild = false;
    }
}

void CustomProjectPart::slotCommandFailed( const QString& /*command*/ )
{
    m_lastCompilationFailed = true;
    m_executeAfterBuild = false;
}

bool CustomProjectPart::isDirty()
{
    if ( m_lastCompilationFailed ) return true;

    QStringList fileList = allFiles();
    QStringList::Iterator it = fileList.begin();
    while ( it != fileList.end() )
    {
        QString fileName = *it;
        ++it;

        QMap<QString, QDateTime>::Iterator it = m_timestamp.find( fileName );
        QDateTime t = QFileInfo( projectDirectory(), fileName ).lastModified();
        if ( it == m_timestamp.end() || *it != t )
        {
            return true;
        }
    }

    return false;
}


QStringList CustomProjectPart::allMakeEnvironments() const
{
    QDomDocument &dom = *projectDom();

    QStringList allConfigs;

    QDomNode node =
        DomUtil::elementByPath( dom , "/kdevcustomproject/make/environments" );
    // extract the names of the different make environments
    QDomElement childEl = node.firstChild().toElement();
    while ( !childEl.isNull() )
    {
        QString config = childEl.tagName();
        allConfigs.append( config );
        childEl = childEl.nextSibling().toElement();
    }
    if ( allConfigs.isEmpty() )
        allConfigs.append( "default" );

    return allConfigs;
}


QString CustomProjectPart::currentMakeEnvironment() const
{
    QStringList allEnvs = allMakeEnvironments();
    QDomDocument &dom = *projectDom();
    QString environment = DomUtil::readEntry( dom, "/kdevcustomproject/make/selectedenvironment" );
    if ( environment.isEmpty() || !allEnvs.contains( environment ) )
        environment  = allEnvs[0];
    return environment;
}

/*!
    \fn CustomProjectPart::distFiles() const
 */
QStringList CustomProjectPart::distFiles() const
{
    QStringList sourceList = allFiles();
    // Scan current source directory for any .pro files.
    QString projectDir = projectDirectory();
    QDir dir( projectDir );
    QStringList files = dir.entryList( "*README*" );
    return sourceList + files;
}

bool CustomProjectPart::containsNonProjectFiles( const QString& dir )
{
    if ( isInBlacklist( dir ) )
        return false;
    QStringList fileentries = QDir( dir ).entryList( filetypes().join( ";" ) );
    QStringList dirs = QDir( dir ).entryList( QDir::Dirs );
    QStringList subentries = fileentries + dirs;
    subentries.remove( "." );
    subentries.remove( ".." );
    for ( QStringList::const_iterator it = subentries.begin(); it != subentries.end(); ++it )
    {
        if ( isInBlacklist( *it ) )
            continue;
        if ( QFileInfo( dir + "/" + *it ).isDir() && !isInBlacklist( *it ) )
        {
            if ( containsNonProjectFiles( dir + "/" + *it ) )
            {
                return true;
            }
        }
        else if ( !project()->isProjectFile( URLUtil::canonicalPath( dir + "/" + *it ) )
                  && !isInBlacklist( *it ) )
        {
            return true;
        }
    }
    return false;
}

bool CustomProjectPart::containsProjectFiles( const QString& dir )
{
    if ( isInBlacklist( dir ) )
        return false;

    QStringList fileentries = QDir( dir ).entryList( filetypes().join( ";" ) );
    QStringList dirs = QDir( dir ).entryList( QDir::Dirs );
    QStringList subentries = fileentries + dirs;
    subentries.remove( "." );
    subentries.remove( ".." );
    for ( QStringList::const_iterator it = subentries.begin(); it != subentries.end(); ++it )
    {
        if ( isInBlacklist( *it ) )
            continue;

        if ( QFileInfo( dir + "/" + *it ).isDir() && !isInBlacklist( *it ) )
        {
            if ( containsProjectFiles( dir + "/" + *it ) )
            {
                return true;
            }
        }
        else if ( project()->isProjectFile( URLUtil::canonicalPath( dir + "/" + *it ) ) && !isInBlacklist( *it ) )
        {
            return true;
        }
    }
    return false;
}

QStringList CustomProjectPart::projectFilesInDir( const QString& dir )
{
    QStringList result;
    QStringList fileentries = QDir( projectDirectory() + "/" + dir ).entryList( filetypes().join( ";" ) );
    QStringList dirs = QDir( projectDirectory() + "/" + dir ).entryList( QDir::Dirs );
    QStringList subentries = fileentries + dirs;
    subentries.remove( "." );
    subentries.remove( ".." );
    for ( QStringList::const_iterator it = subentries.begin(); it != subentries.end(); ++it )
    {
        if ( isInProject( dir + "/" + *it ) )
        {
            result << ( *it );
        }
    }
    return result;
}

QStringList CustomProjectPart::filetypes( ) const
{
    return DomUtil::readListEntry( *projectDom(), "/kdevcustomproject/filetypes", "filetype" );
}

bool CustomProjectPart::isProjectFileType( const QString& filename ) const
{
    QStringList types = filetypes();
    QRegExp re( "", true, true );
    for ( QStringList::const_iterator it = types.begin(); it != types.end(); ++it )
    {
        re.setPattern( *it );
        int pos = re.search( filename );
        uint len = re.matchedLength();
        if ((( *it ).find( "*" ) != -1 || ( *it ).find( "?" ) != -1 ) && pos + len == filename.length() )
            return true;
        else if ( filename.find( "/" ) != -1 && filename.find( *it ) != -1 )
            return true;
        else if ( filename.find( "/" ) == -1 && filename == *it )
            return true;
    }
    return false;
}

void CustomProjectPart::switchBlacklistEntry( const QString& path )
{
    QStringList blacklist = this->blacklist();
    kdDebug( 9025 ) << "Switching path " << path << endl;
    if ( !isInBlacklist( path ) )
    {
        blacklist << path;
        m_recursive = true;
        removeFile( path );
        m_recursive = false;
    }
    else
    {
        blacklist.remove( path );
    }
    updateBlacklist( blacklist );
}

QString CustomProjectPart::relativeToProject( const QString& abspath ) const
{
    QString path = abspath.mid( projectDirectory().length() + 1 );
    kdDebug( 9025 ) << "abspath: " << "|project dir: " << projectDirectory() << "|path: " << path << endl;
    if ( path.endsWith( "/" ) )
        path = path.mid( 0, path.length() - 1 );
    if ( path.startsWith( "/" ) )
        path = path.mid( 1, path.length() );
    return path;
}

bool CustomProjectPart::isInBlacklist( const QString& path ) const
{
    QString relpath = path;
    QStringList blacklist = this->blacklist();
    if ( !QFileInfo( relpath ).isRelative() )
        relpath = relativeToProject( path );
    if ( blacklist.find( relpath ) != blacklist.end() )
        return true;
    QStringList paths = QStringList::split( "/", relpath );
    QString parentpath;
    for ( QStringList::const_iterator it = paths.begin(); it != paths.end(); ++it )
    {
        parentpath += *it;
        if ( blacklist.find( parentpath ) != blacklist.end() )
            return true;
        parentpath = parentpath + "/";
    }
    return false;
}

void CustomProjectPart::updateBlacklist( const QStringList& l )
{
    DomUtil::writeListEntry( *projectDom(), "kdevcustomproject/blacklist", "path", l );
}

QStringList CustomProjectPart::blacklist() const
{
    return DomUtil::readListEntry( *projectDom(), "kdevcustomproject/blacklist", "path" );
}

void CustomProjectPart::addNewFilesToProject( const QStringList& filelist )
{
    QStringList addfiles;
    for ( QStringList::const_iterator it = filelist.begin(); it != filelist.end(); ++it )
    {
        if (( ! isInProject( *it ) ) && ( isProjectFileType( *it ) || QFileInfo( projectDirectory() + "/" + *it ).isDir() ) && !isInBlacklist( *it ) )
        {
            addfiles << *it;
        }
    }

    if ( addfiles.isEmpty() )
        return;

    SelectNewFilesDialog *dlg = new SelectNewFilesDialog( addfiles, mainWindow()->main() );
    if ( dlg->exec() == KDialog::Accepted )
    {
        m_first_recursive = false;
        m_recursive = false;
        QStringList blacklist = this->blacklist();
        QStringList excludelist = dlg->excludedPaths();
        QStringList removeFromExcludes;
        for ( QStringList::const_iterator it = excludelist.begin(); it != excludelist.end(); ++it )
        {
            if ( QFileInfo( projectDirectory() + "/" + *it ).isDir() )
            {
                for ( ProjectFilesSet::ConstIterator it2 = m_sourceFilesSet.constBegin(); it2 != m_sourceFilesSet.constEnd(); ++it2 )
                {
                    if ( it2.key().find( *it ) != -1 )
                    {
                        removeFromExcludes << *it;
                    }
                }
            }
        }
        for ( QStringList::const_iterator it = removeFromExcludes.begin(); it != removeFromExcludes.end(); ++it )
        {
            excludelist.remove( *it );
        }
        blacklist += excludelist;
        updateBlacklist( blacklist );
        addFiles( dlg->includedPaths() );
    }
}

void CustomProjectPart::setFiletypes( const QStringList& l )
{
    DomUtil::writeListEntry( *projectDom(), "kdevcustomproject/filetypes", "filetype", l );
}


/**
 * @brief Is a given file (or a directory) part of this project?
 *
 * @param fileName
 */
bool CustomProjectPart::isInProject( const QString& fileName ) const
{
    return m_sourceFilesSet.contains( fileName );
}


/**
 * @brief Add a file (or a directory) to this project.
 *
 * @param fileName
 *
 * @see removeFromProject()
 */
void CustomProjectPart::addToProject( const QString& fileName )
{
    m_sourceFilesSet.insert( fileName, false );
}


/**
 * @brief Remove a file (or a directory) from this project.
 *
 * @param fileName
 */
void CustomProjectPart::removeFromProject( const QString& fileName )
{
    m_sourceFilesSet.remove( fileName );
}


#include "customprojectpart.moc"

