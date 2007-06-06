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
#include <boost/iostreams/stream.hpp>
#include "mem_iostream.h"

int main() {
cout << "testing" <<  "\n";
std::vector<char> v;
	stream<vector_device> io(v);
	char tv[4] = {0, 0, 0, 0};
	io.write( (char*)tv, 4 );
	io.flush();
	cout << v.size() << endl;

  return 1;
}
