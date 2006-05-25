
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

#ifndef __COMPLETIONDEBUG_H__
#define __COMPLETIONDEBUG_H__

#include <qstringlist.h>
#include <kdebug.h>

namespace CompletionDebug {

/**A helper-class for debugging, which can be used to prevent too deep recursion
   and to format the output nicely with a prefix for each function that is entered */
template <class StreamType>
class KDDebugState {
private:
 StreamType m_stream;
 QStringList m_prefixStack;
 QString currPrefix;
 
 void computePrefix() {
	 currPrefix = "";
	 for( QStringList::iterator it = m_prefixStack.begin(); it != m_prefixStack.end() ; ++it )
		 currPrefix += *it;
 }
public:
 typedef StreamType KStreamType;
 KDDebugState();
 
 KDDebugState( StreamType stream ) : m_stream( stream ) {
 }
 
 void push( const QString & txt ) {
	 currPrefix += txt;
	 m_prefixStack.push_back( txt );
 }
 
 void pop() {
	 currPrefix.truncate( currPrefix.length() - m_prefixStack.back().length() );
	 m_prefixStack.pop_back();
	 computePrefix();
 };
 
 StreamType& dbg() {
	 m_stream << currPrefix;
	 return m_stream;
 }
 
 int depth() {
	 return m_prefixStack.size();
 }
};

#ifndef NDEBUG 
template<>
  KDDebugState<kdbgstream>::KDDebugState();
#endif

template<>
  KDDebugState<kndbgstream>::KDDebugState();

#if defined(VERBOSE) && !defined(NDEBUG)
typedef KDDebugState<kdbgstream> DBGStreamType;
#else
typedef KDDebugState<kndbgstream> DBGStreamType;
#endif

///Class to help indent the debug-output correctly
extern DBGStreamType dbgState;

extern const int completionMaxDepth;

class Debug {
private:
 DBGStreamType& m_state;
public:
 Debug( const QString& prefix = "#", DBGStreamType& st = dbgState ) : m_state( st ) {
  m_state.push( prefix );
 };
 ~Debug() {
  m_state.pop();
 }
 
 DBGStreamType::KStreamType& dbg() {
  return m_state.dbg();
 }
 
 int depth() {
  return m_state.depth();
 }
 
 operator bool() {
  bool r = depth() < completionMaxDepth;
  
  if( !r )
   dbg() << "recursion is too deep";
  
  return r;
 }
};

DBGStreamType::KStreamType& dbg();

DBGStreamType::KStreamType& dbgMajor();

}
#endif 
// kate: indent-mode csands; tab-width 4;
