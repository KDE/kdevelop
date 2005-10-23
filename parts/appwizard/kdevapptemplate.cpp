/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2005 by Sascha Cunz                                     *
 *   sascha@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QHash>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kmacroexpander.h>
#include <ktempfile.h>
#include <ktempdir.h>
#include <kfileitem.h>
#include <kio/chmodjob.h>
#include <kiconloader.h>
#include <karchive.h>
#include <ktar.h>
#include <kstandarddirs.h>

#include "kdevappfrontend.h"
#include "filetemplate.h"

#include "kdevapptemplate.h"
#include "appwizardfactory.h"
#include "misc.h"

inline void expandMacros( QString& str, const QHash<QString,QString>& hash )
{
    QString s;
    kdDebug(9010) << "expandMacros1" << endl;
    s = KMacroExpander::expandMacros( str, hash );
    kdDebug(9010) << "expandMacros2" << endl;
    str = s;
}

KDevAppItem::KDevAppItem( const QString &name, KDevItemGroup *parent )
        : KDevItemCollection( name, parent )
{}

KDevAppItem::~KDevAppItem()
{}


KDevAppGroup::KDevAppGroup(const QString &name, const QString path, KDevItemGroup *parent)
    : KDevAppItem(name, parent), m_path( path )
{
    m_icon = SmallIcon( "folder" );
}

KDevAppTemplate::KDevAppTemplate( KConfig& config, const QString& rootDir, KDevAppGroup* parent )
        : KDevAppItem("root", parent), m_basePath( rootDir )
{
    m_name = config.readEntry("Name");
    m_iconName = config.readEntry("Icon");
    m_icon = SmallIcon( "kdevelop" );
    m_comment = config.readEntry("Comment");
    m_fileTemplates = config.readEntry("FileTemplates");
    m_openFilesAfterGeneration = config.readListEntry("ShowFilesAfterGeneration");
    m_sourceArchive = config.readEntry("Archive");

    // Grab includes list
    QStringList groups = config.groupList();
    groups.remove("General");
    QString group;
    foreach( group, groups )
    {
        config.setGroup( group );
        if( config.readEntry("Type").lower() == "include" )
        {
            QString include( config.readEntry( "File" ) );
            kdDebug(9010) << "Adding: " << include << endl;
            QHash<QString,QString> hash;
            hash["kdevelop"] = rootDir;
            expandMacros( include, hash );
            if( !include.isEmpty() )
            {
                KConfig tmpCfg( include );
                tmpCfg.copyTo( "", &config);
                kdDebug(9010) << "Merging: " << tmpCfg.name() << endl;
            }
        }
    }

    groups = config.groupList();    // may be changed by the merging above
    foreach( group, groups )
    {
        config.setGroup( group );
        QString type = config.readEntry("Type").lower();
        if( type == "value" )  // Add value
        {
            QString name = config.readEntry( "Value" );
            QString label = config.readEntry( "Comment" );
            QString type = config.readEntry( "ValueType", "String" );
            QVariant::Type variantType = QVariant::nameToType( type.latin1());
            QVariant value = config.readPropertyEntry( "Default", variantType );
            value.cast( variantType );  // fix this in kdelibs...
            //if( !name.isEmpty() && !label.isEmpty() )
                //info->propValues->addProperty( new PropertyLib::Property( (int)variantType, name, label, value ) );
        }
        else if( type == "install" ) // copy dir
        {
            File file;
            file.source = config.readPathEntry("Source");
            file.dest = config.readPathEntry("Dest");
            file.process = config.readBoolEntry("Process",true);
            file.isXML = config.readBoolEntry("EscapeXML",false);
            file.option = config.readEntry("Option");
            addFile( file );
        }
        else if( type == "install archive" )
        {
            Archive arch;
            arch.source = config.readPathEntry("Source");
            arch.dest = config.readPathEntry("Dest");
            arch.process = config.readBoolEntry("Process",true);
            arch.option = config.readEntry("Option", "" );
            m_archList.append( arch );
        }
        else if( type == "mkdir" )
        {
            Dir dir;
            dir.dir = config.readPathEntry("Dir");
            dir.option = config.readEntry("Option", "" );
            dir.perms = config.readNumEntry("Perms", 0777 );
            m_dirList.append( dir );
        }
        else if( type == "finishcmd" )
        {
            m_finishCmd = config.readPathEntry("Command");
            m_finishCmdDir = config.readPathEntry("Directory");
        }
        else if( type == "ui")
        {
            m_customUI = config.readPathEntry("File");
        }
        else if( type == "message" )
        {
            m_message = config.readEntry( "Comment" );
        }
    }
}

void KDevAppTemplate::addDir( Dir& dir )
{
    m_dirList.append( dir );
}

void KDevAppTemplate::addFile( File file )
{
    kdDebug(9010) << "Adding file: " << file.dest << endl;
    m_fileList.append( file );
    foreach( File f, m_fileList )
        kdDebug(9010) << f.source << "/" << f.dest << endl;
}

void KDevAppTemplate::expandLists()
{
    kdDebug(9010) << "KDevAppTemplate::expandLists()" << endl;

    QList<File>::iterator fit( m_fileList.begin() );
    for( ; fit != m_fileList.end(); ++fit )
        fit->expand( m_subMap );

    QList<Archive>::iterator ait( m_archList.begin() );
    for( ; ait != m_archList.end(); ++ait )
        ait->expand( m_subMap );

    QList<Dir>::iterator dit( m_dirList.begin() );
    for( ; dit != m_dirList.end(); ++dit )
        dit->expand( m_subMap );
}

void KDevAppTemplate::File::expand( QHash<QString, QString> hash )
{
    //kdDebug(9010) << "File::expand1" << endl;
    expandMacros( source, hash );
    //kdDebug(9010) << "File::expand2" << endl;
    expandMacros( dest, hash );
}

void KDevAppTemplate::Archive::expand( QHash<QString, QString> hash )
{
    expandMacros( source, hash );
    expandMacros( dest, hash );
}

void KDevAppTemplate::Dir::expand( QHash<QString, QString> hash )
{
    expandMacros( dir, hash );
}

void KDevAppTemplate::setSubMapXML()
{
    m_subMapXML = FileTemplate::normalSubstMapToXML( m_subMap );
}

bool KDevAppTemplate::unpackTemplateArchive()
{
    QString archiveName = m_basePath + "/" + m_sourceArchive;

    // Unpack template archive to temp dir, and get the name
    KTempDir archDir;
    archDir.setAutoDelete(true);
    KTar templateArchive( archiveName, "application/x-gzip" );
    if( templateArchive.open( QIODevice::ReadOnly ) )
    {
    //    unpackArchive(templateArchive.directory(), archDir.name(), false);
    }
    else
    {
        kdDebug(9010) << "After KTar::open fail" << endl;
        KMessageBox::sorry(0/**@todo*/, i18n("The template %1 cannot be opened.").arg( archiveName ) );
        templateArchive.close();
        return false;
    }
    templateArchive.close();

    addToSubMap( "src", archDir.name() );

    return true;
}

bool KDevAppTemplate::execFinishCommand( AppWizardPart* part )
{
    if( m_finishCmd.isEmpty())
        return true;

    KDevAppFrontend *appFrontend = part->extension<KDevAppFrontend>("KDevelop/AppFrontend");
    if( !appFrontend )
        return false;

    QString finishCmdDir = KMacroExpander::expandMacros(m_finishCmdDir, m_subMap);
    QString finishCmd = KMacroExpander::expandMacros(m_finishCmd, m_subMap);
    appFrontend->startAppCommand(finishCmdDir, finishCmd, false);

    return true;
}

bool KDevAppTemplate::installProject( QWidget* parentWidget )
{
    // Create dirs
    foreach( Dir dir, m_dirList )
    {
        kdDebug( 9000 ) << "Process dir " << dir.dir  << endl;
        if( m_subMap[dir.option] != "false" )
        {
            if( !KIO::NetAccess::mkdir( dir.dir, parentWidget ) )
            {
                KMessageBox::sorry(parentWidget, i18n(
                                   "The directory %1 cannot be created.").
                                   arg( dir.dir ) );
                return false;
            }
        }
    }

    // Unpack archives
    foreach( Archive arch, m_archList )
        if( m_subMap[arch.option] != "false" )
        {
            kdDebug( 9010 ) << "unpacking archive " << arch.source << endl;
            KTar archive( arch.source, "application/x-gzip" );
            if( archive.open( QIODevice::ReadOnly ) )
            {
                arch.unpack( archive.directory() );
            }
            else
            {
                KMessageBox::sorry(parentWidget, i18n("The archive %1 cannot be opened.").arg(arch.source) );
                archive.close();
                return false;
            }
            archive.close();
        }

    // Copy files & Process
    foreach( File file, m_fileList )
    {
        kdDebug( 9010 ) << "Process file " << file.source << endl;
        if( m_subMap[file.option] != "false" )
        {
            if( !file.copy(this) )
            {
                KMessageBox::sorry(parentWidget, i18n("The file %1 cannot be created.").arg(file.dest) );
                return false;
            }
            file.setPermissions();
        }
    }
    return true;
}

bool KDevAppTemplate::File::copy( KDevAppTemplate* templ )
{
    kdDebug( 9010 ) << "Copy: " << source << " to " << dest << endl;

    if( !process )  // Do a simple copy operation
        return KIO::NetAccess::copy( source, dest, 0 );

    // Process the file and save it at the destFile location
    QFile inputFile( source);
    QFile outputFile( dest );

    const QHash<QString,QString> &subMap = isXML ? templ->subMapXML() : templ->subMap();
    if( inputFile.open( QIODevice::ReadOnly ) && outputFile.open(QIODevice::WriteOnly) )
    {
        QTextStream input( &inputFile );
        QTextStream output( &outputFile );
        while( !input.atEnd() )
            output << KMacroExpander::expandMacros(input.readLine(), subMap) << "\n";
        // Preserve file mode...
        struct stat fmode;
        ::fstat( inputFile.handle(), &fmode);
        ::fchmod( outputFile.handle(), fmode.st_mode );
    }
    else
    {
        inputFile.close();
        outputFile.close();
        return false;
    }
    return true;
}

void KDevAppTemplate::File::setPermissions() const
{
    kdDebug(9010) << "KDevAppTemplate::File::setPermissions()" << endl;
    kdDebug(9010) << "  dest: " << dest << endl;

    KIO::UDSEntry sourceentry;
    KURL sourceurl = KURL::fromPathOrURL(source);
    if( KIO::NetAccess::stat(sourceurl, sourceentry, 0) )
    {
        KFileItem sourceit(sourceentry, sourceurl);
        int sourcemode = sourceit.permissions();
        if( sourcemode & 00100 )
        {
            kdDebug(9010) << "source is executable" << endl;
            KIO::UDSEntry entry;
            KURL kurl = KURL::fromPathOrURL(dest);
            if( KIO::NetAccess::stat(kurl, entry, 0) )
            {
                KFileItem it(entry, kurl);
                int mode = it.permissions();
                kdDebug(9010) << "stat shows permissions: " << mode << endl;
                KIO::chmod(KURL::fromPathOrURL(dest), mode | 00100 );
            }
        }
    }
}

void KDevAppTemplate::Archive::unpack( const KArchiveDirectory *dir )
{
/*
    KIO::NetAccess::mkdir( dest , this );
    kdDebug(9010) << "Dir : " << dir->name() << " at " << dest << endl;
    QStringList entries = dir->entries();
    kdDebug(9010) << "Entries : " << entries.join(",") << endl;

    KTempDir tdir;

    QStringList::Iterator entry = entries.begin();
    for( ; entry != entries.end(); ++entry )
    {

        if( dir->entry( (*entry) )->isDirectory()  )
        {
            const KArchiveDirectory *file = (KArchiveDirectory *)dir->entry( (*entry) );
            unpackArchive( file , dest + "/" + file->name(), process);
        }
        else if( dir->entry( (*entry) )->isFile()  )
        {
            const KArchiveFile *file = (KArchiveFile *) dir->entry( (*entry) );
            if( !process )
            {
                file->copyTo( dest );
                setPermissions(file, dest + "/" + file->name());
            }
            else
            {
                file->copyTo(tdir.name());
                // assume that an archive does not contain XML files
                // ( where should we currently get that info from? )
                if ( !copyFile( QDir::cleanDirPath(tdir.name()+"/"+file->name()), dest + "/" + file->name(), false, process ) )
                {
                    KMessageBox::sorry(this, i18n("The file %1 cannot be created.").arg( dest) );
                    return;
                }
                setPermissions(file, dest + "/" + file->name());
            }
        }
    }
    tdir.unlink();
*/
}
/*
void KDevAppTemplate::Archive::setPermissions(const KArchiveFile *source, QString dest)
{
    kdDebug(9010) << "KDevAppTemplate::Archive::setPermissions(const KArchiveFile *source, QString dest)" << endl;
    kdDebug(9010) << "  dest: " << dest << endl;

    if( source->permissions() & 00100 )
    {
        kdDebug(9010) << "source is executable" << endl;
        KIO::UDSEntry entry;
        KURL kurl = KURL::fromPathOrURL(dest);
        if( KIO::NetAccess::stat(kurl, entry, 0) )
        {
            KFileItem it(entry, kurl);
            int mode = it.permissions();
            kdDebug(9010) << "stat shows permissions: " << mode << endl;
            KIO::chmod(KURL::fromPathOrURL(dest), mode | 00100 );
        }
    }
}
*/
KDevAppTemplateModel::KDevAppTemplateModel(QObject *parent)
        : KDevItemModel(parent), folderIcon( SmallIcon( "folder" ) )
{
    KConfig *config = kapp->config();
    config->setGroup("General Options");

    KStandardDirs *dirs = AppWizardFactory::instance()->dirs();
    QStringList templateNames = dirs->findAllResources("apptemplates", QString::null, false, true);

    kdDebug(9010) << "Templates: " << endl;
    foreach( QString templateName, templateNames )
    {
        QString templateFile = KGlobal::dirs()->findResource("apptemplates", templateName);
        kdDebug(9010) << templateName << " in " << templateFile << endl;
        KConfig templateConfig(templateFile);
        templateConfig.setGroup("General");

        QString category = templateConfig.readEntry("Category");
        if( category.endsWith('/') )
            category.remove(category.length()-1, 1);
        if( category.startsWith('/') )
            category.remove(1,1);

        QString basePath( AppWizardUtil::kdevRoot( templateFile ) );
        KDevAppGroup* group = getCategory( category );
        appendItem( new KDevAppTemplate( templateConfig, basePath, group ), group );
    }
}

KDevAppGroup* KDevAppTemplateModel::getCategory( const QString& path )
{
    KDevItemCollection *curCollection = root();
    QStringList list = QStringList::split("/",path);
    QString curPath;
    foreach( QString current, list )
    {
        curPath += current;
        bool found = false;
        int i = 0;
        while( i < curCollection->itemCount() && !found )
        {
            KDevAppGroup* curGroup = reinterpret_cast<KDevAppItem*>(curCollection->itemAt(i++))->groupItem();
            if( curGroup && path.lower().startsWith( curGroup->path().lower() ) )
            {
                curCollection = curGroup;
                found = true;
            }
        }
        if( !found )
        {
            KDevItemCollection* prevCollection = curCollection;
            curCollection = new KDevAppGroup(current, curPath, curCollection);
            appendItem( curCollection, prevCollection );
        }
        curPath += '/';
    }
    return reinterpret_cast<KDevAppItem*>(curCollection)->groupItem();
}

int KDevAppTemplateModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
