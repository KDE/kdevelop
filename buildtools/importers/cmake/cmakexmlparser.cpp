/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#include "cmakexmlparser.h"

ProjectInfo CMakeXmlParser::parseProject( const QDomDocument& doc )
{
    QDomElement e = doc.documentElement();
    ProjectInfo pi;
    if ( e.tagName() == "project" )
    {
        pi.name = e.attribute("name");
        QDomNode n = e.firstChild();
        while ( !n.isNull() )
        {
            QDomElement fe = n.toElement();
            if ( !fe.isNull() && fe.tagName() == "folder" )
            {
                pi.folders.append( CMakeXmlParser::parseFolder( fe ) );
            }
            n = n.nextSibling();
        }
    }
    return pi;
}

FolderInfo CMakeXmlParser::parseFolder( const QDomElement& docElem )
{
    FolderInfo mainInfo;
    if ( docElem.tagName() == "folder" )
    {
        mainInfo.name = docElem.attribute( "name" );
        QDomNode n = docElem.firstChild();
        while( !n.isNull() )
        {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if( !e.isNull() )
            {
                if ( e.tagName() == "definitions" )
                {
                    QDomNode dn = e.firstChild();
                    while ( !dn.isNull() )
                    {
                        QDomElement de = dn.toElement(); // try to convert the node to an element.
                        if ( !de.isNull() )
                        {
                            if ( de.tagName() == "define" )
                                mainInfo.defines.append( de.text() );
                        }
                        dn = dn.nextSibling();
                    }
                }

                if ( e.tagName() == "includes" )
                {
                    QDomNode in = e.firstChild();
                    while ( !in.isNull() )
                    {
                        QDomElement ie = in.toElement(); // try to convert the node to an element.
                        if ( !ie.isNull() )
                        {
                            if ( ie.tagName() == "include" )
                                mainInfo.includes.append( ie.text() );
                        }
                        in = in.nextSibling();
                    }
                }

                if ( e.tagName() == "folder" )
                {
                    FolderInfo fi;
                    fi.name = e.attribute("name");
                    mainInfo.subFolders.append(fi);
                }
            }
            n = n.nextSibling();
        }
    }
    return mainInfo;
}

TargetInfo CMakeXmlParser::parseTarget( const QDomElement& docElem )
{
    TargetInfo ti;
    if ( docElem.tagName() == "target" )
    {
        ti.name = docElem.attribute( "name" );
        ti.type = docElem.attribute( "type" );
        QDomNode n = docElem.firstChild();
        while ( !n.isNull() )
        {
            QDomElement e = n.toElement();
            if ( !e.isNull() )
            {
                if ( e.tagName() == "sources" )
                {
                    QDomNode sn = e.firstChild();
                    while ( !sn.isNull() )
                    {
                        QDomElement se = sn.toElement();
                        if ( !se.isNull() )
                        {
                            if ( se.tagName() == "source" )
                                ti.sources.append( se.text() );
                        }
                        sn = sn.nextSibling();
                    }
                }
            }
            n = n.nextSibling();
        }
    }

    return ti;
}
