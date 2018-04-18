/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef VARIABLEREFERENCEPARSER_H
#define VARIABLEREFERENCEPARSER_H

#include <QList>
#include <QMap>
#include <QString>

class QStringList;

class VariableInfo
{
public:
    enum VariableType
    {
        QMakeVariable,
        QtConfigVariable,
        ShellVariableResolveQMake,
        ShellVariableResolveMake,
        FunctionCall,
        Invalid
    };

    struct Position {
        Position(int start_, int end_)
            : start(start_), end(end_)
        {
        }
        int start;
        int end;
    };

    VariableInfo();
    QList< Position > positions;
    VariableType type = VariableInfo::Invalid;
};

class VariableReferenceParser
{
    public:
        VariableReferenceParser();
        void setContent( const QString& );
        bool parse();
        QStringList variableReferences() const;
        VariableInfo variableInfo( const QString& ) const;
    private:
        void appendPosition( const QString&, int, int, VariableInfo::VariableType );
        QString m_content;
        QMap< QString, VariableInfo > m_variables;
};

#endif

