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

void DoxyDoc::formatType( QString& str ) {
	str.replace( QRegExp(" "),"" );
};

DoxyDoc::DoxyDoc( const QStringList& dir ) {
	for (uint i = 0; i < dir.count(); ++i)
		m_dirs.push_back(QDir(*(dir.at(i))));
}

//crappy implementation, change later
QString DoxyDoc::functionDescription( const QString& tmpscope, const QString& name, const QString& tmptype, const QString& tmparguments) {
	QString scope = tmpscope;
	bool foundfile = false;
	//produce doxygen conform filenames
	QString filename = "/class" + scope.replace(QRegExp("_"), "__").replace( QRegExp("::"), "_1_1" ) + ".xml";
	//search for file in all directories
	for (std::list<QDir>::const_iterator ci = m_dirs.begin(); !foundfile && ci != m_dirs.end(); ++ci){
		if (QFile::exists( ci->path() + filename)){
			if ( m_file.name() != ci->path() + filename ) {
				m_file.close();
				m_file.setName( ci->path() + filename );
				if ( !m_file.open( IO_ReadOnly ) ) {
					m_file.setName( "" );
					return "";
				}
				QDomDocument m_doc;
				m_doc.setContent( m_file.readAll() );
				m_file.close();
				m_list = m_doc.elementsByTagName( "memberdef" );
				foundfile = true;

			} else //file is already opened
				foundfile = true;
		}
	}
	if (!foundfile)
		return QString::null;

	QString type = tmptype;
	formatType( type );

	for ( uint i = 0; i < m_list.count(); ++i ) {
		if ( m_list.item( i ).childNodes().item( 3 ).toElement().text() == name ) {
			QDomNodeList nodes = m_list.item( i ).childNodes();
			QString nodetype = nodes.item( 0 ).toElement().text();
			formatType( nodetype );
			if ( nodetype )
				formatType( nodetype );
			else
				nodetype = "";
			if ( nodetype == type ) {
				QString nodearguments = "";
				QString arguments = tmparguments;
				if (!arguments)
					arguments = "";

				formatType( arguments );

				uint j = 4;
				for (; j < nodes.count() && nodes.item( j ).nodeName() == "param"; ++j ) {
					nodearguments += nodes.item( j ).childNodes().item( 0 ).toElement().text() + ",";
				}

				if ( nodearguments != "") {
					nodearguments = nodearguments.left( nodearguments.length() - 1 );
					formatType( nodearguments );
				}

				if ( arguments == nodearguments ) {
					if (nodes.item(j).nodeName() == "exceptions")
						++j;
				QString brief = nodes.item( j++ ).toElement().text();
				QString detailstr = "";
				QString paramstr = "";
				QDomNode detail = nodes.item( j );
				QDomNode descnode;
				for (uint index = 0; index < detail.childNodes().count(); ++index){
					descnode = detail.childNodes().item(index);
					if (descnode.hasChildNodes() && descnode.childNodes().item(0).nodeName() == "parameterlist"){
						int tmpcount = descnode.childNodes().count();
						for (int k = 0; k < tmpcount; ++k){
							//add parametername
							paramstr += "<li><i>" + descnode.childNodes().item(0).childNodes().item(k++).toElement().text() + "</i>\t";
							//add parameterdescription
							paramstr += descnode.childNodes().item(0).childNodes().item(k).toElement().text() + "</li>";
						}
					} else {
						detailstr += descnode.toElement().text();
					}
				}


				QString description = "";
				if (brief != "")
					description += brief + "<p>";
				if (detailstr != "")
					description += detailstr + "<p>";
				if (paramstr != "")
					description += "<b>Parameterlist:</b><p>" + paramstr;

				if (description == "")
					return QString::null;
				else
					return description;
				}
			}
		}
	}

	return QString::null;
};


