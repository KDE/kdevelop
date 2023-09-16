/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef VARIABLEREFERENCEPARSER_H
#define VARIABLEREFERENCEPARSER_H

#include <QList>
#include <QMap>
#include <QString>

#include <QStringList>

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

