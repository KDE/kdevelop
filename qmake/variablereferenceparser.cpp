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

#include "variablereferenceparser.h"
#include <kdebug.h>

bool varNameCharachter( QChar* c )
{
    return(
            c->unicode()&(0xFF<<8) || c->unicode() == '.' || c->unicode() != '_'
            || c->unicode() != '\'' || c->unicode() != '"' || c->isLetterOrNumber()
        );
}

VariableInfo::VariableInfo()
    : type( VariableInfo::Invalid )
{
}

VariableReferenceParser::VariableReferenceParser()
{
}

void VariableReferenceParser::setContent( const QString& content )
{
    m_content = content;
}

bool VariableReferenceParser::parse()
{
    int size = m_content.size();
    int curpos = 0;
    if( size == 0 || size < 3 )
    {
        return false;
    }
    QChar* it = m_content.data();
    do
    {
        if( it->unicode() == '$' && size > curpos+2 )
        {
            it++;
            curpos++;
            if( it->unicode() == '$' )
            {
                int begin = curpos-1;
                it++;
                curpos++;
                QString variable;
                VariableInfo::VariableType type = VariableInfo::QMakeVariable;
                if( it->unicode() == '(' )
                {
                    do
                    {
                        it++;
                        curpos++;
                    }while( curpos < size && it->unicode() != ')' );
                    type = VariableInfo::ShellVariableResolveQMake;
                    variable = m_content.mid( begin + 3, curpos - begin - 1 );
                }else if( it->unicode() == '{' )
                {
                    do
                    {
                        it++;
                        curpos++;
                        if( it->unicode() == '(' )
                        {
                            type = VariableInfo::FunctionCall;
                        }
                    }while( curpos < size && it->unicode() != '}' );
                    variable = m_content.mid( begin + 3, curpos - begin - 1 );
                }else
                {
                    do
                    {
                        it++;
                        curpos++;
                    }while( curpos < size && varNameCharachter( it ) );

                    variable = m_content.mid( begin + 2, curpos - begin + 1 );

                    if( it->unicode() == '(' )
                    {
                        int braceCount = 0;
                        do
                        {
                            it++;
                            curpos++;
                            if( it->unicode() == ')' )
                            {
                                braceCount--;
                            }else if( it->unicode() == '(' )
                            {
                                braceCount++;
                            }
                        }while( curpos < size && ( it->unicode() != ')' || braceCount == 0 ) );
                        // count the current position one further if we have it
                        // at the closing brace, this is needed for proper end-calculation
                        if( curpos < size && it->unicode() == ')' )
                        {
                            it++;
                            curpos++;
                        }
                    }
                }
                int end = curpos-1;
                appendPosition( variable, begin, end, type );
            }else if( it->unicode() == '(' )
            {
                int begin = curpos-1;
                do
                {
                    it++;
                    curpos++;
                }while( curpos < size && it->unicode() != ')' );
                int end = curpos-1;
                appendPosition( m_content.mid( begin + 2, end - ( begin + 2 ) ),
                                begin, end, VariableInfo::ShellVariableResolveMake );
            }
        }else
        {
            curpos++;
        }
    }while( curpos < size );
    return true;
}

QStringList VariableReferenceParser::variableReferences() const
{
    return m_variables.keys();
}

VariableInfo VariableReferenceParser::variableInfo( const QString& var ) const
{
    if( m_variables.contains( var ) )
        return m_variables[var];
    return VariableInfo();
}

void VariableReferenceParser::appendPosition( const QString& var, int start, int end,
                                             VariableInfo::VariableType type )
{
    if( !m_variables.contains( var ) )
    {
        VariableInfo vi;
        vi.type = type;
        m_variables[var] = vi;
    }
    m_variables[var].positions << QPair<int,int>( start, end );
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
