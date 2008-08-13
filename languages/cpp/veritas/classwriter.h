/*
* KDevelop xUnit testing support
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#ifndef VERITAS_CPP_CLASSWRITER_INCLUDED
#define VERITAS_CPP_CLASSWRITER_INCLUDED

class QIODevice;
#include "veritascppexport.h"

namespace Veritas
{
class ClassSkeleton;

/*! @unittest Veritas::Test::ClassWriterTest */
class VERITASCPP_EXPORT ClassSerializer
{
public:
    void write(const ClassSkeleton& source, QIODevice* target);

};

}

#endif // VERITAS_CPP_CLASSWRITER_INCLUDED
