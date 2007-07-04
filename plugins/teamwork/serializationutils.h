/***************************************************************************
  copyright            : (C) 2006 by David Nolden
  email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SERIALIZATION_UTILS_H
#define SERIALIZATION_UTILS_H

///Unfortunately it seems that boost doesn't like the things that Qt syntactically does to the xml-syntax.
///It fails to load logically same documents when they come from QDom, so we embed the data as simple text into the xml-document.

#define XML_USE_TEXT_ARCHIVE

#ifdef XML_USE_TEXT_ARCHIVE
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
typedef boost::archive::polymorphic_text_iarchive XmlIArchive;
typedef boost::archive::polymorphic_text_oarchive XmlOArchive;
#else
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
typedef boost::archive::polymorphic_xml_iarchive XmlIArchive;
typedef boost::archive::polymorphic_xml_oarchive XmlOArchive;
#endif

#include <boost/serialization/extended_type_info_typeid.hpp>
#include <boost/serialization/base_object.hpp>
//#include <boost/serialization/extended_type_info.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/level.hpp>
#include "nvp.h"

#include <QString>
#include <QDomDocument>
#include <QDomNode>
#include <QTextStream>

///All these functions throw QString's as error-descriptions on error

template <class Type>
void xmlTextDeserialize( QString& buffer, Type& object ) throw( QString ) {
  QByteArray bufLocal = buffer.toUtf8().data();
  std::istringstream str;
  str.str( bufLocal.data() );
  try {
    XmlIArchive arch( str );
    arch & object;
  } catch ( std::exception & exc ) {
    throw QString( "exception occurred while deserialization: " ) + exc.what();
  }
}

template <class Type>
void xmlTextSerialize( QString& buffer, Type& object ) throw( QString ) {
  buffer.clear();

  std::ostringstream str;

  try {
    XmlOArchive arch( str );

    arch & object;
  } catch ( std::exception & exc ) {
    throw QString( "exception occurred while serialization: " ) + exc.what();
  }

  buffer = str.str().c_str();
}

template <class Type>
void xmlDeserialize( QDomNode& node, Type& object ) throw( QString ) {
  QDomNode header = node.namedItem( "header" );
  ///QDomNode type = node.namedItem( "doctype" );
  QDomNode serializationNode = node.namedItem( "boost_serialization" );
  if ( header.isNull() )
    throw QString( "document does not contain header" );
  /*  if( type.isNull() )
      throw QString( "document does not contain type" );*/
  if ( serializationNode.isNull() )
    throw QString( "document does not contain serialization-node" );
  if ( !header.isElement() && !header.isText() )
    throw QString( "header is no element" );
  /*if( type.isElement() && !type.isText() )
  throw QString( "type is no element" );*/
  /*  if( serializationNode.isElement() )
      throw QString( "serialization-node is no element" );*/

  QDomNode headerText = header;
  if ( !header.isText() )
    headerText = header.firstChild();
  /*QDomNode typeText = type;
  if( !type.isText() )
  typeText = header.firstChild();*/

  if ( headerText.isNull()  /*|| typeText.isNull()*/ || !headerText.isText()  /*|| !typeText.isText()*/ )
    throw QString( "header- or type-elements have wrong type" );

  QString text;
  {
    QTextStream stream( &text );
    serializationNode.save( stream, 1 );
  }

  //text = headerText.data() +"\n" + typeText.data() + "\n" + text;
  if ( text.isEmpty() )
    throw QString( "text is empty" );
  text = "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\n <!DOCTYPE boost_serialization>\n" + text;
  cout << "deserializing: " << endl << text.toUtf8().data();
  xmlTextDeserialize( text, object );
}

template <class Type>
void xmlSerialize( QDomNode& container, Type& object, const QString& nodeName = "" ) throw( QString ) {
  QString buf;
  xmlTextSerialize( buf, object );
  if ( buf.isEmpty() )
    throw QString( "serialized buffer is empty" );

  QDomDocument doc( nodeName );
  doc.setContent( buf );

  QDomNode header = doc.firstChild();
  QDomDocumentType type = doc.doctype();
  QString headerText, typeText;

  cout << "full serialized data: \n" << buf.toUtf8().data() << endl;

  {
    QTextStream stream( &headerText );
    header.save( stream, 1 );
  }
  {
    QTextStream stream( &typeText );
    type.save( stream, 1 );
  }

  QDomDocument domDoc = container.ownerDocument();
  QDomElement storedHeaderNode = domDoc.createElement( "header" );
  QDomElement storedTypeNode = domDoc.createElement( "doctype" );
  storedHeaderNode.appendChild( domDoc.createTextNode( headerText + "\n" + typeText ) );
  storedTypeNode.appendChild( domDoc.createTextNode( typeText ) );

  container.appendChild( storedHeaderNode );
  //  container.appendChild( storedTypeNode );
  container.appendChild( doc.firstChildElement().cloneNode() ); ///While serialization, there is one "boost_serialization"-child created. Only that should be exported.
}

/**This deletes the previous element of the given name, and replaces it with a new one containing the data.
  *the given object-type must already be a name-value-pair(see NVP(...) )
  * Errors are thrown as QString
*/

template <class Type>
void xmlSerializeToElementItem( QDomElement* el, const QString& item, Type& object ) throw( QString ) {
  QDomDocument domDoc = el->ownerDocument();
  {
    QDomNode containerNode = el->namedItem( item );

    if ( !containerNode.isNull() )
      el->removeChild( containerNode );
  }

  QDomElement containerNode = domDoc.createElement( item );
  el->appendChild( containerNode );

#ifdef XML_USE_TEXT_ARCHIVE

  QString buffer;
  xmlTextSerialize( buffer, object );
  if ( buffer.isEmpty() )
    throw QString( "deserialized buffer is empty" );
  containerNode.appendChild( domDoc.createTextNode( buffer ) );
#else

  xmlSerialize( containerNode, object, item );

#endif
}

/**is invoked the same way as xmlSerializeToElementItem(...)
 * Errors are thrown as QString
 */

template <class Type>
void xmlDeserializeFromElementItem( const QDomElement* el, const QString& item, Type& object ) throw( QString ) {
  QDomNode containerNode = el->namedItem( item );
  if ( containerNode.isNull() )
    throw "container-node of name \"" + item + "\" is missing";

#ifdef XML_USE_TEXT_ARCHIVE

  QDomNode node = containerNode.firstChild();
  if ( node.isNull() )
    throw "the container-node of name \"" + item + "\" does not contain the correct data";

  QString data;

  QDomText textNode = node.toText();
  if ( textNode.isNull() )
    throw "the container-node of name \"" + item + "\" does not contain the correct data";

  data = textNode.data();
  if ( data.isEmpty() )
    throw QString( "stored data is empty" );
  xmlTextDeserialize( data, object );
#else

  xmlDeserialize( containerNode, object );

#endif

}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
