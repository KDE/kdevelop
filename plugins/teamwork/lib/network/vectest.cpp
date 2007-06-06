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
#include "vector_stream.h"
#include <assert.h>


int main() {
	std::vector<char> testbuf;
	for( int a = 0; a < 100000; a++ )
		testbuf.push_back( a );

	std::vector<char> testbuf2 = testbuf;///lesetest
	std::vector<char> testbuf3;///schreibtest

	vector_device readDevice( testbuf2 );
	vector_device writeDevice( testbuf3 );

	for( int a = 0; a < 100; a++ ) {
		char buf[1000];
		readDevice.read( buf, 1000 );
		writeDevice.write( buf, 1000 );
	}

	for( int a = 0; a < 100000; a++ )
		assert( testbuf[a] == testbuf2[a] && testbuf[a] == testbuf3[a] );

	cout << "assert success\n";

	return 0;
}
