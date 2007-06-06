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
#include <sstream>
#include "serializationutils.h"

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

struct IArchiveContainer::Private {
	XmlIArchive arch;
	Private( istream& str ) : arch( str ) {
	}
};

struct OArchiveContainer::Private {
	XmlOArchive arch;
	Private( ostream& str ) : arch( str ) {
	}
};
IArchiveContainer::IArchiveContainer( istream& str ) : priv( new Private( str ) ), arch( priv->arch )  {
}

IArchiveContainer::~IArchiveContainer() {
	delete priv;
}

OArchiveContainer::OArchiveContainer( ostream& str ) : priv( new Private( str ) ), arch( priv->arch ) {
}

OArchiveContainer::~OArchiveContainer() {
	delete priv;
}
