/* KDevelop Automake Support
 *
 * Copyright (C)  2005  Matt Rogers <mattr@kde.org>
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
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef MAKEFILEINTERFACE_H
#define MAKEFILEINTERFACE_H

#include <QObject>
#include <QRegExp>
#include <QDir>
#include <kurl.h>

class QFileInfo;

namespace AutoMake
{
    class ProjectAST;
    enum TargetType { Program, Library, LibtoolLibrary, Lisp, Python, Java,
                      Scripts, Data, Headers, ManPages, Texinfo, Unknown };
    enum InstallLocation { Bin, Libexec, Lib, None, Check, Info, Man, Custom };

    const QRegExp targetPrimaries = QRegExp( "_(PROGRAMS|LIBRARIES|LISP|PYTHON|JAVA|SCRIPTS|DATA|HEADERS|MANS|TEXINFOS|LTLIBRARIES)" );

    TargetType convertToType( const QString& );
    InstallLocation convertToLocation( const QString& );

}

template <typename T> class QList;
template <typename T1, typename T2> class QMap;
template <typename T1, typename T2> class QHash;

typedef QHash<QFileInfo, AutoMake::ProjectAST*> AutoMakeASTHash;
typedef QMap<QFileInfo, AutoMake::ProjectAST*> AutoMakeASTMap;
typedef QList<AutoMake::ProjectAST*> AutoMakeASTList;

struct TargetInfo
{
    AutoMake::TargetType type;
    AutoMake::InstallLocation location;
    QString name;
    KUrl url;
    QString display;
};

class MakefileInterface : public QObject
{
    Q_OBJECT
public:
    enum ParserRecursion { Recursive, NonRecursive };
    enum TargetType { Program, Library, LibtoolLibrary, Lisp, Python, Java,
                      Scripts, Data, Headers, ManPages, Texinfo };
    enum InstallLocations { Bin, Libexec, Lib, None, Check, Info, Man, Extra };

    MakefileInterface( QObject* parent );
    ~MakefileInterface();

    static QString canonicalize( const QString& target );
    static bool isVariable( const QString& item );

    QString resolveVariable( const QString& variable, AutoMake::ProjectAST* ast ) const;

    bool parse( const KUrl& dir, ParserRecursion recursive = Recursive );

    QString projectRoot() const;
    void setProjectRoot( const KUrl& dir );
    QStringList topSubDirs() const;
    QStringList subdirsFor( const KUrl& dir ) const;

    QList<TargetInfo> targetsForFolder( const KUrl& dir ) const;
    QList<QFileInfo> filesForTarget( const TargetInfo& ) const;

private:
    QStringList subdirsFor( AutoMake::ProjectAST* ) const;
    AutoMake::ProjectAST* astForFolder( const KUrl& dir ) const;
    QStringList valuesForId( const QString&, AutoMake::ProjectAST* ) const;

private:
    class Private;
    Private* d;

};

#endif

