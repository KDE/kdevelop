
/***************************************************************************
   begin                : Sat Jul 21 2001
   copyright            : (C) 2001 by Victor R�er
   email                : victor_roeder@gmx.de
   copyright            : (C) 2002,2003 by Roberto Raggi
   email                : roberto@kdevelop.org
   copyright            : (C) 2005 by Adam Treat
   email                : manyoso@yahoo.com
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

template <class StreamType>
class KDDebugState {
private:
 StreamType m_stream;
 QStringList m_prefixStack;
 
public:
 typedef StreamType KStreamType;
 KDDebugState();
 
 KDDebugState( StreamType stream ) : m_stream( stream ) {
  
 }
 
 void push( const QString & txt ) {
  m_prefixStack.push_back( txt );
 }
 
 void pop() {
  m_prefixStack.pop_back();
 };
 
 StreamType& dbg() {
  for( QStringList::iterator it = m_prefixStack.begin(); it != m_prefixStack.end() ; ++it ) {
   m_stream << *it;
  }
  
  return m_stream;
 }
 
 int depth() {
  return m_prefixStack.size();
 }
 
} ;

#ifndef NDEBUG 
template<>
  KDDebugState<kdbgstream>::KDDebugState();
#endif

template<>
  KDDebugState<kndbgstream>::KDDebugState();

#ifdef VERBOSE
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
  
  if( !r ) {
   dbg() << "recursion is too deep";
  }
  
  return r;
 }
};


DBGStreamType::KStreamType& dbg();

DBGStreamType::KStreamType& dbgMajor();

}
#endif 
// kate: indent-mode csands; tab-width 4;
