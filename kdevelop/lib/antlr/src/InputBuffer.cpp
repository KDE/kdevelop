/* ANTLR Translator Generator
 * Project led by Terence Parr at http://www.jGuru.com
 * Software rights: http://www.antlr.org/RIGHTS.html
 *
 */

#include "antlr/config.hpp"
#include "antlr/InputBuffer.hpp"

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
namespace antlr {
#endif

/** Ensure that the character buffer is sufficiently full */
void InputBuffer::fill(int amount)
{
	syncConsume();
	// Fill the buffer sufficiently to hold needed characters
	while (queue.entries() < amount + markerOffset)
	{
		// Append the next character
		queue.append(getChar());
	}
}

/** get the current lookahead characters as a string
 * @warning it may treat 0 and EOF values wrong
 */
ANTLR_USE_NAMESPACE(std)string InputBuffer::getLAChars( void ) const
{
	ANTLR_USE_NAMESPACE(std)string ret;

	for(int i = markerOffset; i < queue.entries(); i++)
		ret += queue.elementAt(i);

	return ret;
}

/** get the current marked characters as a string
 * @warning it may treat 0 and EOF values wrong
 */
ANTLR_USE_NAMESPACE(std)string InputBuffer::getMarkedChars( void ) const
{
	ANTLR_USE_NAMESPACE(std)string ret;

	for(int i = 0; i < markerOffset; i++)
		ret += queue.elementAt(i);

	return ret;
}

/** Return an integer marker that can be used to rewind the buffer to
 * its current state.
 */
int InputBuffer::mark()
{
	syncConsume();
	nMarkers++;
	return markerOffset;
}

/** Rewind the character buffer to a marker.
 * @param mark Marker returned previously from mark()
 */
void InputBuffer::rewind(int mark)
{
	syncConsume();
	markerOffset = mark;
	nMarkers--;
}

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
}
#endif
