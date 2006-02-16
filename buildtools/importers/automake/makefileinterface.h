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

class QFileInfo;
class QDir;

namespace AutoTools { class ProjectAST; }

template <typename T> class QList;
template <typename T1, typename T2> class QMap;
template <typename T1, typename T2> class QHash;


typedef QHash<QFileInfo, AutoTools::ProjectAST*> ASTHash;
typedef QMap<QFileInfo, AutoTools::ProjectAST*> ASTMap;
typedef QList<AutoTools::ProjectAST*> ASTList;

uint qHash( const QFileInfo& key );

class MakefileInterface : public QObject
{
    Q_OBJECT
public:
    enum ParserRecursion{ Recursive, NonRecursive };
    MakefileInterface( QObject* parent );
    ~MakefileInterface();

    bool parse( const QDir& dir, ParserRecursion recursive = Recursive );

    void setProjectRoot( const QDir& dir );
    QStringList topSubDirs() const;
    QStringList subdirsFor( const QDir& dir ) const;

private:
    bool isVariable( const QString& item ) const;

    QString resolveVariable( const QString& variable, AutoTools::ProjectAST* ast );

private:
    class Private;
    Private* d;

};

#endif
// kate: space-indent on; indent-width 4; auto-insert-doxygen on; replace-tabs on;
