/***************************************************************************
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NVP_H
#define NVP_H

#include <boost/serialization/extended_type_info_typeid.hpp>
#include <boost/serialization/nvp.hpp>

///NVP means Name-Value-Pair, and is used for serialization into Xml-Archives
#undef NVP
#define NVP(x) BOOST_SERIALIZATION_NVP(x)

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
