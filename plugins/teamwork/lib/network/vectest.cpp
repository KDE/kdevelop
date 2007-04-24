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