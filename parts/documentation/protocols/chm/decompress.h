/*  This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __decompress_h__
#define __decompress_h__

#ifdef __cplusplus__
extern "C" {
#endif

typedef unsigned char  UBYTE; /* 8 bits exactly    */
typedef unsigned short UWORD; /* 16 bits (or more) */
typedef unsigned int   ULONG; /* 32 bits (or more) */
typedef   signed int    LONG; /* 32 bits (or more) */

typedef struct lzx_bits
{
	ULONG bb;
	int bl;
	UBYTE *ip;
} lzx_bits;

int LZXinit( int window );
int LZXdecompress( UBYTE *inpos, int inlen, UBYTE *outpos, int outlen );

#ifdef __cplusplus__
}
#endif

#endif // __decompress_h__
