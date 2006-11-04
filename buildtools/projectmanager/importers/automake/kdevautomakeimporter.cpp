/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2002 by Victor R�er <victor_roeder@gmx.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include "kdevautomakeimporter.h"
#include "automakeprojectmodel.h"

#include <kdevproject.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qdict.h>

K_EXPORT_COMPONENT_FACTORY(libkdevautomakeimporter, KGenericFactory<KDevAutomakeImporter>("kdevautomakeimporter"))

KDevAutomakeImporter::KDevAutomakeImporter(QObject *parent, const char *name, const QStringList &)
    : KDevProjectImporter(parent, name)
{
    m_project = ::qt_cast<KDevProject*>(parent);
    Q_ASSERT(m_project);
}

KDevAutomakeImporter::~KDevAutomakeImporter()
{
}

QString KDevAutomakeImporter::canonicalize(const QString &str)
{
    QString res;

    for (uint i = 0; i < str.length(); ++i)
        res += (str[i].isLetterOrNumber() || str[i] == '@') ? str[i] : QChar('_');

    return res;
}

void KDevAutomakeImporter::parseMakefile(const QString &fileName, ProjectItemDom dom)
{
    kdDebug(9000) << "Parsing makefile " << fileName << endl;
    QMap<QString,QString> list;
    QMap<QString,QString>* variables = &list;

    // cut & past from AutoProjectTool, misc.cpp
    QFile f(fileName);
    if (!f.open(IO_ReadOnly))
    {
        return ;
    }
    QTextStream stream(&f);

    QRegExp re("^(#kdevelop:[ \t]*)?([A-Za-z][@A-Za-z0-9_]*)[ \t]*([:\\+]?=)[ \t]*(.*)$");

    QString last;
    bool multiLine = false;
    while (!stream.atEnd())
    {
        QString s = stream.readLine().stripWhiteSpace();
        if (re.exactMatch(s))
        {
            QString lhs = re.cap(2);
            QString rhs = re.cap(4);
            if (rhs[ rhs.length() - 1 ] == '\\')
            {
                multiLine = true;
                last = lhs;
                rhs[rhs.length() - 1] = ' ';
            }

            // The need for stripWhiteSpace seems to be a Qt bug.
            // make our list nice and neat.
            QStringList bits = QStringList::split(" ", rhs);
            rhs = bits.join(" ");
            if (re.cap(3) == "+=")
            {
                ((*variables)[lhs] += ' ') += rhs;
            }
            else
            {
                variables->insert(lhs, rhs);
            }
        }
        else if (multiLine)
        {
            if (s[s.length()-1] == '\\')
            {
                s[s.length()-1] = ' ';
            }
            else
            {
                multiLine = false;
            }
            QStringList bits = QStringList::split(" ", s);
            ((*variables)[last] += ' ') += bits.join(" ");
        }
    }
    f.close();

    for (QMap<QString, QString>::iterator iter = list.begin();iter != list.end();iter++)
    {
        QStringList items = QStringList::split(" ", iter.data());
        QMap<QString, QString> unique;
        for (uint i = 0;i < items.size();i++)
        {
            unique.insert(items[i], "");
        }
        QString line;
        for (QMap<QString, QString>::iterator it = unique.begin();it != unique.end();it++)
        {
            line += it.key() + ' ';
        }
        if ( line.length()>1){
            line.setLength(line.length() - 1);
        }
        dom->setAttribute(iter.key(), line);
    }

}

void KDevAutomakeImporter::saveMakefile(const QString &fileName, ProjectItemDom dom)
{
    modifyMakefile(fileName, dom->attributes());
}

/**
 * This is a cut & past from AutoProjectTool, misc.cpp
 * Add entries to a variable. Will just add the variables to the existing line, removing duplicates
 * Will preserve += constructs and make sure that the variable only has one copy of the value across
 * all += constructs
 * @param fileName
 * @param variables key=value string of entries to add
 * @param add true= add these key,value pairs, false = remove. You can have empty values for an add - the whole line is
 * removed. For adding, we will not add an empty line.
 */
void KDevAutomakeImporter::addRemoveMakefileam(const QString &fileName, QMap<QString, QString> variables,  bool add)
{
    // input file reading
    QFile fin(fileName);
    if (!fin.open(IO_ReadOnly))
    {
        return ;
    }
    QTextStream ins(&fin);

    // output file writing.
    QFile fout(fileName + "#");
    if (!fout.open(IO_WriteOnly))
    {
        fin.close();
        return ;
    }
    QTextStream outs(&fout);

    // variables
    QRegExp re("^(#kdevelop:[ \t]*)?([A-Za-z][@A-Za-z0-9_]*)[ \t]*([:\\+]?=)[ \t]*(.*)$");

    // build key=map of values to add
    // map can be empty.we never add an empty key, but do remove empty keys from the file..
    QDict< QMap<QString, bool> > interest;
    for (QMap<QString, QString>::Iterator it0 = variables.begin(); it0 != variables.end(); ++it0)
    {
        QMap<QString, bool>* set = new QMap<QString, bool>();
        if (!it0.data().stripWhiteSpace().isEmpty())
        {
            QStringList variableList = QStringList::split(' ', it0.data());

            for (uint i = 0; i < variableList.count(); i++)
            {
                set->insert(variableList[i], true);
            }
        }
        interest.insert(it0.key(), set);
    }

    bool multiLine = false;
    QString lastLhs;
    QStringList lastRhs;
    QMap<QString, QString> seenLhs;
    while (!fin.atEnd())
    {
        QString s = ins.readLine();
        if (re.exactMatch(s))
        {
            QString lhs = re.cap(2);
            QMap<QString, bool>* ourRhs = interest.find(lhs);

            if (!ourRhs)
            {
                // not interested in this line at all
                // write it out as is..
                outs << s << endl;
            }
            else
            {
                // we are interested in this line..
                QString rhs = re.cap(4).stripWhiteSpace();
                if (rhs[ rhs.length() - 1 ] == '\\')
                {
                    // save it for when we have the whole line..
                    multiLine = true;
                    lastLhs = lhs;
                    rhs.setLength(rhs.length() - 1);
                    lastRhs += QStringList::split(" ", rhs);
                }
                else
                {
                    // deal with it now.

                    QStringList bits = QStringList::split(" ", rhs);
                    if (add)
                    {
                        // we are adding our interested values to this line and writing it

                        // add this line to we we want to add to remove duplicates.
                        for (uint index = 0; index < bits.size(); index++)
                        {
                            QMap<QString, bool>::iterator findEntry = ourRhs->find(bits[index]);
                            if (findEntry == ourRhs->end())
                            {
                                // we haven't seen it, so add it, so we don't add it again later..
                                ourRhs->insert(bits[index], true);
                            }
                            // else we have this value in our 'to add list' , it is either already been
                            // added, so we don't want to add it again, or it hasn't been added, in which
                            // case we will do so soon. so we can ignore this now..
                        }
                        // now write the line out if it is not going to be empty.
                        QString newLine(lhs);
                        if (seenLhs.find(lhs) == seenLhs.end())
                        {
                            newLine += " = ";
                            seenLhs[lhs] = "";
                        }
                        else
                        {
                            newLine += " += ";
                        }

                        int len = newLine.length();
                        bool added = false;
                        QValueList<QString> keys = ourRhs->keys();
                        for (uint count = 0; count < keys.size(); count++)
                        {
                            // if out entry is true, add it..
                            if ((*ourRhs)[keys[count]])
                            {
                                added = true;
                                len += keys[count].length() + 1;
                                if (len > 80)
                                {
                                    newLine += "\\\n\t";
                                    len = 8;
                                }
                                newLine += keys[count];
                                newLine += ' ';
                                // set our value so we don't add it again.
                                (*ourRhs)[keys[count]] = false;
                            }
                        }
                        // only print it out if there was a value to add..
                        if (added)
                        {
                            newLine.setLength(newLine.length() - 1);
                            outs << newLine << endl;
                        }
                    }
                    else
                    {
                        // we are removing our interested values from this line

                        // special case - no values, remove the line..
                        if (!ourRhs->empty())
                        {
                            // check if any of these values are down to remove.
                            QString newLine(lhs);
                            if (seenLhs.find(lhs) == seenLhs.end())
                            {
                                newLine += " = ";
                                seenLhs[lhs] = "";
                            }
                            else
                            {
                                newLine += " += ";
                            }

                            int len = newLine.length();
                            bool added = false;
                            for (QStringList::Iterator posIter = bits.begin(); posIter != bits.end();posIter++)
                            {
                                QMap<QString, bool>::iterator findEntry = ourRhs->find(*posIter);
                                if (findEntry == ourRhs->end())
                                {
                                    // we do not want to remove it..
                                    added = true;
                                    len += (*posIter).length() + 1;
                                    if (len > 80)
                                    {
                                        newLine += "\\\n\t";
                                        len = 8;
                                    }
                                    newLine += (*posIter);
                                    newLine += ' ';
                                }
                                // else we have this value in our 'to remove list', so don't add it.
                            }
                            // only print it out if there was a value on it..
                            if (added)
                            {
                                newLine.setLength(newLine.length() - 1);
                                outs << newLine << endl;
                            }
                        }
                    }//if (add)
                }//if ( rhs[ rhs.length() - 1 ] == '\\'  )
            }//if ( found == interest.end())
        }
        else if (multiLine)
        {
            s = s.stripWhiteSpace();
            // we are only here if were interested in this line..
            if (s[s.length()-1] == '\\')
            {
                s.setLength(s.length() - 1);
                // still more multi line we wait for..
            }
            else
            {
                // end of the multi line..
                multiLine = false;
            }
            lastRhs += QStringList::split(" ", s);

            if (!multiLine)
            {
                // now we have to deal with this multiLine value..
                // ourRhs will always be a value, as we only get multiLine if we're interested in it..
                QMap<QString, bool>* ourRhs = interest.find(lastLhs);

                if (add)
                {
                    // we are adding our interested values to this line and writing it

                    // add this line to we we want to add to remove duplicates.
                    for (uint index = 0; index < lastRhs.size(); index++)
                    {
                        QMap<QString, bool>::iterator findEntry = ourRhs->find(lastRhs[index]);
                        if (findEntry == ourRhs->end())
                        {
                            // we haven't seen it, so add it, so we don't add it again later..
                            ourRhs->insert(lastRhs[index], true);
                        }
                        // else we have this value in our 'to add list' , it is either already been
                        // added, so we don't want to add it again, or it hasn't been added, in which
                        // case we will do so soon. so we can ignore this now..
                    }
                    // now write the line out if it is not going to be empty.
                    QString newLine(lastLhs);
                    if (seenLhs.find(lastLhs) == seenLhs.end())
                    {
                        newLine += " = ";
                        seenLhs[lastLhs] = "";
                    }
                    else
                    {
                        newLine += " += ";
                    }

                    int len = newLine.length();
                    bool added = false;
                    QValueList<QString> keys = ourRhs->keys();
                    for (uint count = 0; count < keys.size(); count++)
                    {
                        // if out entry is true, add it..
                        if ((*ourRhs)[keys[count]])
                        {
                            added = true;
                            len += keys[count].length() + 1;
                            if (len > 80)
                            {
                                newLine += "\\\n\t";
                                len = 8;
                            }
                            newLine += keys[count];
                            newLine += ' ';
                            // set our value so we don't add it again.
                            (*ourRhs)[keys[count]] = false;
                        }
                    }
                    // only print it out if there was a value to add..
                    if (added)
                    {
                        newLine.setLength(newLine.length() - 1);
                        outs << newLine << endl;
                    }
                }
                else
                {
                    // we are removing our interested values from this line

                    // special case - no values, remove the line..
                    if (!ourRhs->empty())
                    {
                        // check if any of these values are down to remove.
                        QString newLine(lastLhs);
                        if (seenLhs.find(lastLhs) == seenLhs.end())
                        {
                            newLine += " = ";
                            seenLhs[lastLhs] = "";
                        }
                        else
                        {
                            newLine += " += ";
                        }
                        int len = newLine.length();
                        bool added = false;
                        for (QStringList::Iterator posIter = lastRhs.begin(); posIter != lastRhs.end();posIter++)
                        {
                            QMap<QString, bool>::iterator findEntry = ourRhs->find(*posIter);
                            if (findEntry == ourRhs->end())
                            {
                                // we do not want to remove it..
                                added = true;
                                len += (*posIter).length() + 1;
                                if (len > 80)
                                {
                                    newLine += "\\\n\t";
                                    len = 8;
                                }
                                newLine += (*posIter);
                                newLine += ' ';
                            }
                            // else we have this value in our 'to remove list', so don't add it.
                        }
                        // only print it out if there was a value on it..
                        if (added)
                        {
                            newLine.setLength(newLine.length() - 1);
                            outs << newLine << endl;
                        }
                    }
                }

                lastLhs.setLength(0);
                lastRhs.clear();
            }
        }
        else
        {
            // can write this line out..
            // not a match, not a multi line,
            outs << s << endl;
        }
    }

    if (add)
    {
        QDictIterator<QMap<QString, bool> > it(interest);
        for (; it.current(); ++it)
        {
            QString lhs = it.currentKey();
            QMap<QString, bool>* ourRhs = it.current();

            QString newLine(lhs);
            if (seenLhs.find(lastLhs) == seenLhs.end())
            {
                newLine += " = ";
                seenLhs[lastLhs] = "";
            }
            else
            {
                newLine += " += ";
            }
            int len = newLine.length();
            bool added = false;
            QValueList<QString> keys = ourRhs->keys();
            for (uint count = 0; count < keys.size(); count++)
            {
                if ((*ourRhs)[keys[count]])
                {
                    added = true;
                    len += keys[count].length() + 1;
                    if (len > 80)
                    {
                        newLine += "\\\n\t";
                        len = 8;
                    }
                    newLine += keys[count];
                    newLine += ' ';
                    // set our value so we don't add it again.
                    (*ourRhs)[keys[count]] = false;
                }
            }
            // only print it out if there was a value to add..
            if (added)
            {
                newLine.setLength(newLine.length() - 1);
                outs << newLine << endl;
            }
        }
    }
    interest.setAutoDelete(true);
    interest.clear();

    fin.close();
    fout.close();

    QDir().rename(fileName + "#", fileName);
}



void KDevAutomakeImporter::modifyMakefile(const QString &fileName, const Environment &env)
{
    Environment variables = env;
    QMap<QString, QString> update;
    for(Environment::iterator it = variables.begin(); it!= variables.end(); it++){
        update.insert(it.key(), it.data().toString());
        kdDebug(9000) << "modify: " << it.data().toString() << endl;
    }
    addRemoveMakefileam(fileName, update, true);
}

void KDevAutomakeImporter::removeFromMakefile(const QString &fileName, const Environment &env)
{
    Environment variables = env;
    QMap<QString, QString> update;
    for(Environment::iterator it = variables.begin(); it!= variables.end(); it++){
        kdDebug(9000) << "remove: " << it.data().toString() << endl;
        update.insert(it.key(), it.data().toString());
    }
    addRemoveMakefileam(fileName, update, false);
}

QString KDevAutomakeImporter::findMakefile(ProjectFolderDom dom) const
{
    return dom->name() + "/Makefile.am";
}

QStringList KDevAutomakeImporter::findMakefiles(ProjectFolderDom dom) const
{
    return const_cast<KDevAutomakeImporter*>(this)->findMakefiles(dom);
}

QStringList KDevAutomakeImporter::findMakefiles(ProjectFolderDom dom)
{
    QStringList files;

    if (AutomakeFolderDom automakeFolder = AutomakeFolderModel::from(dom))
        files += automakeFolder->name() + "/Makefile.am";

    ProjectFolderList folder_list = dom->folderList();
    for (ProjectFolderList::Iterator it = folder_list.begin(); it != folder_list.end(); ++it)
        files += findMakefiles(*it);

    return files;
}

ProjectItemDom KDevAutomakeImporter::import(ProjectModel *model, const QString &fileName)
{
    QFileInfo fileInfo(fileName);

    ProjectItemDom item;

    if (fileInfo.isDir()) {
        AutomakeFolderDom folder = model->create<AutomakeFolderModel>();
        folder->setName(fileName);
        item = folder->toItem();
    } else if (fileInfo.isFile()) {
        AutomakeFileDom file = model->create<AutomakeFileModel>();
        file->setName(fileName);
        item = file->toItem();
    }

    return item;
}

// ---------- Automake parser
namespace AutoProjectPrivate
{

bool isHeader( const QString& fileName )
{
    return QStringList::split( ";", "h;H;hh;hxx;hpp;tcc;h++" ).contains( QFileInfo(fileName).extension(false) );
}

static QString cleanWhitespace( const QString &str )
{
    QString res;

    QStringList l = QStringList::split( QRegExp( "[ \t]" ), str );
    QStringList::ConstIterator it;
    for ( it = l.begin(); it != l.end(); ++it )
    {
        res += *it;
        res += " ";
    }

    return res.left( res.length() - 1 );
}

static void removeDir( const QString& dirName )
{
    QDir d( dirName );
    const QFileInfoList* fileList = d.entryInfoList();
    if( !fileList )
    return;

    QFileInfoListIterator it( *fileList );
    while( it.current() ){
        const QFileInfo* fileInfo = it.current();
        ++it;

        if( fileInfo->fileName() == "." || fileInfo->fileName() == ".." )
            continue;

        if( fileInfo->isDir() && !fileInfo->isSymLink() )
            removeDir( fileInfo->absFilePath() );

        d.remove( fileInfo->fileName(), false );
    }

    d.rmdir( d.absPath(), true );
}

}

ProjectFolderList KDevAutomakeImporter::parse(ProjectFolderDom item)
{
    Q_ASSERT(item);

    ProjectFolderList subproject_list;

    headers.clear();

    // ### AutomakeFolderDom folder = AutomakeFolderModel::from(item->toFolder());

    parseMakefile(item->name() + "/Makefile.am", item->toItem());

    QMap<QString, QVariant> env = item->attributes();
    for ( QMap<QString, QVariant>::ConstIterator it = env.begin(); it != env.end(); ++it )
    {
        QString lhs = it.key();
        QString rhs = it.data().toString();
        if ( lhs == "KDE_DOCS" )
            parseKDEDOCS( item->toItem(), lhs, rhs );
        else if ( lhs.right( 5 ) == "_ICON" )
            parseKDEICON( item->toItem(), lhs, rhs );
        else if ( lhs.find( '_' ) > 0 )
            parsePrimary( item->toItem(), lhs, rhs );
        else if ( lhs.right( 3 ) == "dir" )
            parsePrefix( item->toItem(), lhs, rhs );
        else if ( lhs == "SUBDIRS" )
            subproject_list += parseSUBDIRS( item->toItem(), lhs, rhs );
    }

    /// @todo only if in a c++ project
    AutomakeTargetDom noinst_HEADERS_item = findNoinstHeaders(item);

    QDir dir(item->name());
    QStringList headersList = QStringList::split( QRegExp("[ \t]"),
        item->attribute("noinst_HEADERS").toString());

    headersList += dir.entryList( "*.h;*.H;*.hh;*.hxx;*.hpp;*.tcc", QDir::Files );
    headersList.sort();

    QStringList::Iterator fileIt = headersList.begin();
    while( fileIt != headersList.end() ){
        QString fname = *fileIt;
        ++fileIt;

        if (noinst_HEADERS_item && AutoProjectPrivate::isHeader(fname) && !headers.contains(fname)) {
            AutomakeFileDom fitem = item->projectModel()->create<AutomakeFileModel>();
            fitem->setName(noinst_HEADERS_item->path + "/" + fname);
            noinst_HEADERS_item->addFile(fitem->toFile());
        }
    }

    return subproject_list;
}

void KDevAutomakeImporter::parseKDEDOCS(ProjectItemDom item, const QString &lhs, const QString &rhs)
{
    Q_UNUSED(lhs);
    Q_UNUSED(rhs);

    // Handle the line KDE_ICON =
    // (actually, no parsing is involved here)

    QString prefix = "kde_docs";
    QString primary = "KDEDOCS";

    AutomakeTargetDom titem = item->projectModel()->create<AutomakeTargetModel>();
    titem->path = item->name();
    setup(titem, "", prefix, primary);
    item->toFolder()->addTarget(titem->toTarget());

    QDir d( item->name() );
    QStringList l = d.entryList( QDir::Files );

    QRegExp re( "Makefile.*|\\..*|.*~|index.cache.bz2" );

    QStringList::ConstIterator it;
    for ( it = l.begin(); it != l.end(); ++it )
    {
        if ( !re.exactMatch( *it ) )
        {
            QString fname = *it;
            AutomakeFileDom fitem = item->projectModel()->create<AutomakeFileModel>();
            fitem->setName(titem->path + "/" + fname);
            titem->addFile(fitem->toFile());
        }
    }
}

void KDevAutomakeImporter::parseKDEICON(ProjectItemDom item, const QString &lhs, const QString &rhs)
{
    // Parse a line foo_ICON = bla bla

    int pos = lhs.find( "_ICON" );
    QString prefix = lhs.left( pos );
    if ( prefix == "KDE" )
        prefix = "kde_icon";

    QString primary = "KDEICON";

    AutomakeTargetDom titem = item->projectModel()->create<AutomakeTargetModel>();
    titem->path = item->name();
    setup(titem, "", prefix, primary);
    item->toFolder()->addTarget(titem->toTarget());

    QDir d( item->name() );
    QStringList l = d.entryList( QDir::Files );

    QString regexp;

    if ( rhs == "AUTO" )
    {
        regexp = ".*\\.(png|mng|xpm)";
    }
    else
    {
        QStringList appNames = QStringList::split( QRegExp( "[ \t\n]" ), rhs );
        regexp = ".*(-" + appNames.join( "|-" ) + ")\\.(png|mng|xpm)";
    }

    QRegExp re( regexp );

    QStringList::ConstIterator it;
    for ( it = l.begin(); it != l.end(); ++it )
    {
        if ( re.exactMatch( *it ) )
        {
            AutomakeFileDom fitem = item->projectModel()->create<AutomakeFileModel>();
            fitem->setName(titem->path + "/" + *it);
            titem->addFile(fitem->toFile());
        }
    }
}

void KDevAutomakeImporter::parsePrimary(ProjectItemDom item, const QString &lhs, const QString &rhs)
{
    // Parse line foo_bar = bla bla

    int pos = lhs.findRev( '_' );
    QString prefix = lhs.left( pos );
    QString primary = lhs.right( lhs.length() - pos - 1 );


#if 0

    QStrList prefixes;
    prefixes.append( "bin" );
    prefixes.append( "pkglib" );
    prefixes.append( "pkgdata" );
    prefixes.append( "noinst" );
    prefixes.append( "check" );
    prefixes.append( "sbin" );
    QStrList primaries;
    primaries.append( "PROGRAMS" );
    primaries.append( "LIBRARIES" );
    primaries.append( "LTLIBRARIES" );
    primaries.append( "SCRIPTS" );
    primaries.append( "HEADERS" );
    primaries.append( "DATA" );
#endif

    // Not all combinations prefix/primary are possible, so this
    // could also be checked... not trivial because the list of
    // possible prefixes can be extended dynamically (see below)
    if ( primary == "PROGRAMS" || primary == "LIBRARIES" || primary == "LTLIBRARIES" )
    {
        QStringList l = QStringList::split( QRegExp( "[ \t\n]" ), rhs );
        QStringList::Iterator it1;
        for ( it1 = l.begin(); it1 != l.end(); ++it1 )
        {
            AutomakeTargetDom titem = item->projectModel()->create<AutomakeTargetModel>();
            titem->path = item->name();
            setup(titem, *it1, prefix, primary);
            item->toFolder()->addTarget(titem->toTarget());

            QString canonname = canonicalize( *it1 );
            titem->ldflags = AutoProjectPrivate::cleanWhitespace( item->attribute(canonname + "_LDFLAGS").toString() );
            titem->ldadd = AutoProjectPrivate::cleanWhitespace( item->attribute(canonname + "_LDADD").toString());
            titem->libadd = AutoProjectPrivate::cleanWhitespace( item->attribute(canonname + "_LIBADD").toString() );
            titem->dependencies = AutoProjectPrivate::cleanWhitespace( item->attribute(canonname + "_DEPENDENCIES").toString() );

            QString sources = item->attribute( canonname + "_SOURCES").toString();
            QStringList sourceList = QStringList::split( QRegExp( "[ \t\n]" ), sources );
            QMap<QString, bool> dict;
            QStringList::Iterator it = sourceList.begin();
            while( it != sourceList.end() ){
                dict.insert( *it, true );
                ++it;
            }

            QMap<QString, bool>::Iterator dictIt = dict.begin();
            while( dictIt != dict.end() ){
                QString fname = dictIt.key();
                ++dictIt;

                AutomakeFileDom fitem = item->projectModel()->create<AutomakeFileModel>();
                fitem->setName(titem->path + "/" + fname);
                titem->addFile(fitem->toFile());

                if( AutoProjectPrivate::isHeader(fname) ) {
                    headers += fname;
                }
            }
        }
    }
    else if ( primary == "SCRIPTS" || primary == "HEADERS" || primary == "DATA" )
    {
        // See if we have already such a group
                ProjectTargetList target_list = item->toFolder()->targetList();
        for ( uint i = 0; i < target_list.count(); ++i )
        {
            AutomakeTargetDom titem = AutomakeTargetModel::from(*target_list.at(i));
            if ( primary == titem->primary && prefix == titem->prefix )
            {
                item->toFolder()->removeTarget(titem->toTarget());
                break;
            }
        }

        AutomakeTargetDom titem = item->projectModel()->create<AutomakeTargetModel>();
        titem->path = item->name();
        setup(titem, "", prefix, primary);
        item->toFolder()->addTarget(titem->toTarget());

        QStringList l = QStringList::split( QRegExp( "[ \t]" ), rhs );
        QStringList::Iterator it3;
        for ( it3 = l.begin(); it3 != l.end(); ++it3 )
        {
            QString fname = *it3;

            AutomakeFileDom fitem = item->projectModel()->create<AutomakeFileModel>();
            fitem->setName(titem->path + "/" + fname);
            titem->addFile(fitem->toFile());

            if( AutoProjectPrivate::isHeader(fname) ) {
                headers += fname;
            }

        }
    }
    else if ( primary == "JAVA" )
    {
        QStringList l = QStringList::split( QRegExp( "[ \t\n]" ), rhs );
        QStringList::Iterator it1;

        AutomakeTargetDom titem = item->projectModel()->create<AutomakeTargetModel>();
        titem->path = item->name();
        setup(titem, "", prefix, primary);
        item->toFolder()->addTarget(titem->toTarget());

        for ( it1 = l.begin(); it1 != l.end(); ++it1 )
        {
            AutomakeFileDom fitem = item->projectModel()->create<AutomakeFileModel>();
            fitem->setName(titem->path + "/" + *it1);
            titem->addFile(fitem->toFile());
        }
    }
}

void KDevAutomakeImporter::parsePrefix(ProjectItemDom item, const QString &lhs, const QString &rhs)
{
    // Parse a line foodir = bla bla
    QString name = lhs.left( lhs.length() - 3 );
    QString dir = rhs;

    AutomakeFolderDom folder = AutomakeFolderModel::from(item->toFolder());
    folder->prefixes.insert( name, dir );
}

ProjectFolderList KDevAutomakeImporter::parseSUBDIRS(ProjectItemDom item, const QString &lhs, const QString &rhs)
{
    Q_UNUSED(lhs);

    // Parse a line SUBDIRS = bla bla
    QString subdirs = rhs;

    // Take care of KDE hacks:
    // TOPSUBDIRS is an alias for all directories
    // listed in the subdirs file
    if ( subdirs.find( "$(TOPSUBDIRS)" ) != -1 )
    {
        QStringList dirs;
        QFile subdirsfile( item->name() + "/subdirs" );
        if ( subdirsfile.open( IO_ReadOnly ) )
        {
            QTextStream subdirsstream( &subdirsfile );
            while ( !subdirsstream.atEnd() )
                dirs.append( subdirsstream.readLine() );
            subdirsfile.close();
        }
        subdirs.replace( QRegExp( "\\$\\(TOPSUBDIRS\\)" ), dirs.join( " " ) );
    }

    // AUTODIRS is an alias for all subdirectories
    if ( subdirs.find( "$(AUTODIRS)" ) != -1 )
    {
        QDir d( item->name() );
        QStringList dirs = d.entryList( QDir::Dirs );
        dirs.remove( "." );
        dirs.remove( ".." );
        dirs.remove( "CVS" );
        subdirs.replace( QRegExp( "\\$\\(AUTODIRS\\)" ), dirs.join( " " ) );
    }

    // If there are any variables in the subdirs line then search
    // the Makefile(.am?) for its definition. Unfortunately, it may be
    // defined outside this file in which case those dirs won't be added.
    QRegExp varre( "\\$\\(\\s*(.*)\\s*\\)" );
    varre.setMinimal( true );
    while ( varre.search( subdirs ) != -1 )
    {
        QString varname = varre.cap( 1 );
        QString varvalue;

        // Search the whole Makefile(.am?)
        // Note that if the variable isn't found it just disappears
        // (Perhaps we should add it back in this case?)
        if (item->hasAttribute(varname))
        {
            varvalue = item->attribute(varname).toString();
        }
        else
        {
            kdDebug(9000) << "Not found Makefile var " << varname << endl;
        }
        subdirs.replace( QRegExp( "\\$\\(\\s*" + varname + "\\s*\\)" ), varvalue );
    }

    QStringList l = QStringList::split( QRegExp( "[ \t]" ), subdirs );
    l.sort();
    QStringList::Iterator it;

    ProjectFolderList subproject_list;
    for ( it = l.begin(); it != l.end(); ++it )
    {
        if ( *it == "." )
            continue;

        AutomakeFolderDom newitem = item->projectModel()->create<AutomakeFolderModel>();
        newitem->setName(item->name() + "/" + *it);
        item->toFolder()->addFolder(newitem->toFolder());
        subproject_list.append(newitem->toFolder());
    }

    return subproject_list;
}

AutomakeTargetDom KDevAutomakeImporter::findNoinstHeaders(ProjectFolderDom item)
{
    Q_ASSERT(item);

    AutomakeTargetDom noinst_HEADERS_item;

    ProjectTargetList target_list = item->targetList();
    for (ProjectTargetList::Iterator it = target_list.begin(); it != target_list.end(); ++it) {
        AutomakeTargetDom titem = AutomakeTargetModel::from(*it);
        if (!titem)
            continue;

        if( titem->prefix == "noinst" && titem->primary == "HEADERS" ) {
            noinst_HEADERS_item = titem;
            break;
        }
    }

    if (!noinst_HEADERS_item){
        noinst_HEADERS_item = item->projectModel()->create<AutomakeTargetModel>();
        noinst_HEADERS_item->path = item->name();
        setup(noinst_HEADERS_item, "", "noinst", "HEADERS");

        item->addTarget(noinst_HEADERS_item->toTarget());
    }

    return noinst_HEADERS_item;
}

void KDevAutomakeImporter::setup( AutomakeTargetDom dom, const QString & name, const QString & prefix, const QString & primary )
{
    bool group = !( primary == "PROGRAMS" || primary == "LIBRARIES"
                    || primary == "LTLIBRARIES" || primary == "JAVA" );
    bool docgroup = ( primary == "KDEDOCS" );
    bool icongroup = ( primary == "KDEICON" );

    QString text;
    if ( docgroup )
        text = i18n( "Documentation data" );
    else if ( icongroup )
        text = i18n( "Icon data in %1" ).arg( prefix );
    else if ( group )
        text = i18n( "%1 in %2" ).arg( nicePrimary( primary ) ).arg( prefix );
    else
        text = i18n( "%1 (%2 in %3)" ).arg( name ).arg( nicePrimary( primary ) ).arg( prefix );

    dom->setName(text);
    dom->prefix = prefix;
    dom->primary = primary;
}

QString KDevAutomakeImporter::nicePrimary( const QString & primary )
{
    if ( primary == "PROGRAMS" )
        return i18n( "Program" );
    else if ( primary == "LIBRARIES" )
        return i18n( "Library" );
    else if ( primary == "LTLIBRARIES" )
        return i18n( "Libtool Library" );
    else if ( primary == "SCRIPTS" )
        return i18n( "Script" );
    else if ( primary == "HEADERS" )
        return i18n( "Header" );
    else if ( primary == "DATA" )
        return i18n( "Data" );
    else if ( primary == "JAVA" )
        return i18n( "Java" );

    return QString::null;
}

KDevProject *KDevAutomakeImporter::project() const
{
    return m_project;
}

#include "kdevautomakeimporter.moc"
