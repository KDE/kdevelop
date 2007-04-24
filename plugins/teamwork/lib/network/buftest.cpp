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
