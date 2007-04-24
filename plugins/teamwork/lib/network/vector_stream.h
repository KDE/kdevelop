#include <boost/iostreams/categories.hpp>   // seekable_device_tag
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/stream.hpp>
#include <streambuf>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <memory.h>
#include <stdio.h>

#ifndef VECTOR_STREAM_H
#define VECTOR_STREAM_H

using namespace boost::iostreams;
using namespace std;

class vector_device {
	public:
		typedef char                 char_type;
		typedef bidirectional_device_tag category;
		
		vector_device( std::vector<char>& vec ) : v_(vec), readPosition_(0) {
		}

		std::streamsize read(char* s, std::streamsize n)
		{
			if( n + readPosition_ > v_.size() )
				n = v_.size() - readPosition_;

			memcpy( s, &(v_[readPosition_]), n );
			readPosition_ += n;
			
			return n;
		}

		std::streamsize write(const char* s, std::streamsize n)
		{
			uint oldSize = v_.size();
			v_.resize( oldSize + n );
			memcpy( &(v_[oldSize]), s, n );
			return n;
		}

	private:
		std::vector<char>& v_;
		uint readPosition_;
};

class vector_read_device {
	public:
		typedef char                 char_type;
		typedef bidirectional_device_tag category;
		
		vector_read_device( const std::vector<char>& vec ) : v_(vec), readPosition_(0) {
		}

		std::streamsize read(char* s, std::streamsize n)
		{
			if( n + readPosition_ > v_.size() )
				n = v_.size() - readPosition_;

			memcpy( s, &(v_[readPosition_]), n );
			readPosition_ += n;
			
			return n;
		}

		std::streamsize write(const char* s, std::streamsize n)
		{
			return 0;
		}

	private:
		const std::vector<char>& v_;
		uint readPosition_;
};

typedef boost::iostreams::stream<vector_device> vector_stream;
typedef boost::iostreams::stream<vector_read_device> vector_read_stream;

#endif
