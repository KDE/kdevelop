/***************************************************************************
*   Copyright (C) 2003 by Jonas B. Jacobi                                 *
*   j.jacobi@gmx.de                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#include "doxydoc.h"

#include <list>

#include <qstring.h>
#include <qstringlist.h>
#include <qdom.h>
#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>

void DoxyDoc::formatType( QString& str )
{
	str.replace( QRegExp( " " ), "" );
}

DoxyDoc::DoxyDoc( const QStringList& dir )
{
	for ( uint i = 0; i < dir.count(); ++i )
		m_dirs.push_back( QDir( *( dir.at( i ) ) ) );
}

//crappy implementation, change later
QString DoxyDoc::functionDescription( const QString& tmpscope, const QString& name,
                                      const QString& tmptype, const QString& tmparguments )
{
	QString scope = tmpscope;
	bool foundfile = false;
	//produce doxygen conform filenames
	QString filename = "/class" + scope.replace( QRegExp( "_" ), "__" ).replace( QRegExp( "::" ), "_1_1" ) + ".xml";
	
	//search for file in all directories
	for ( std::list<QDir>::const_iterator ci = m_dirs.begin(); !foundfile && ci != m_dirs.end(); ++ci )
	{
		if ( QFile::exists( ci->path() + filename ) )
		{
			if ( m_file.name() != ci->path() + filename )
			{
				m_file.close();
				m_file.setName( ci->path() + filename );
				if ( !m_file.open( IO_ReadOnly ) )
				{
					m_file.setName( "" );
					return "";
				}
				QDomDocument m_doc;
				m_doc.setContent( m_file.readAll() );
				m_file.close();
				m_list = m_doc.elementsByTagName( "memberdef" );
				foundfile = true;
				
			}
			else //file is already opened
				foundfile = true;
		}
	}
	if ( !foundfile )
		return QString::null;
	
	QString type = tmptype;
	formatType( type );
	
	for ( uint i = 0; i < m_list.count(); ++i )
	{
		QDomElement elem = m_list.item( i ).toElement();
		if ( elem.elementsByTagName( "name" ).item( 0 ).toElement().text() == name && 
		     elem.elementsByTagName( "type" ).item( 0 ).toElement().text() == tmptype )
		{
			QDomNodeList paramnodes = elem.elementsByTagName( "param" );
			QString nodearguments = "", arguments = tmparguments;
			for ( unsigned int j = 0; j < paramnodes.count(); ++j )
				nodearguments += paramnodes.item( j ).childNodes().item( 0 ).toElement().text() + ",";
			if ( nodearguments != "" )
			{
				nodearguments = nodearguments.left( nodearguments.length() - 1 );
				formatType( nodearguments );
			}
			formatType( arguments );
			if ( arguments == nodearguments )
			{
				QString brief = "";
				QDomNode briefnode = elem.elementsByTagName( "briefdescription" ).item( 0 );
				if ( briefnode.hasChildNodes() )
					brief = briefnode.firstChild().toElement().text();
				
				QString detailstr = "", paramstr = "";
				QDomNode detail = elem.elementsByTagName( "detaileddescription" ).item( 0 );
				if ( detail.hasChildNodes() )
					detail = detail.firstChild();
				
				QDomNode descnode = detail.firstChild();
				while ( !descnode.isNull() )
				{
					if ( descnode.nodeName() == "parameterlist" )
					{
						int tmpcount = descnode.childNodes().count();
						for ( int k = 0; k < tmpcount; ++k )
						{
							//add parametername
							paramstr += "<li><i>" + descnode.childNodes().item( k++ ).toElement().text() + "</i>\t";
							//add parameterdescription
							paramstr += descnode.childNodes().item( k ).toElement().text() + "</li>";
						}
					}
					else
						if ( descnode.nodeName() == "simplesect" )
						{}
					else
					{
						if ( descnode.isText() )
							detailstr += descnode.toText().data();
						else
							detailstr += descnode.toElement().text();
					}
					descnode = descnode.nextSibling();
				}
				
				
				QString description = "";
				if ( brief != "" )
					description += brief + "<p>";
				if ( detailstr != "" )
					description += detailstr + "<p>";
				if ( paramstr != "" )
					description += "<b>Parameterlist:</b><p>" + paramstr;
				
				if ( description == "" )
					return QString::null;
				else
					return description;
			}
		}
		
	}
	
	return QString::null;
}

//kate: indent-mode csands; tab-width 4; space-indent off;
