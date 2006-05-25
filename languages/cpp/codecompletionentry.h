/***************************************************************************
                         cppcodecompletion.cpp  -  description
                            -------------------
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

#ifndef __CODECOMPLETIONENTRY_H__
#define __CODECOMPLETIONENTRY_H__

#include <ktexteditor/codecompletioninterface.h>

//this is just a little helper-class to allow custom sorting, it must stay binary compatible with KTextEditor::CompletionEntry!!
class CodeCompletionEntry : public KTextEditor::CompletionEntry
{
public:
 CodeCompletionEntry() : KTextEditor::CompletionEntry() {
 }
 CodeCompletionEntry( const CodeCompletionEntry& rhs ) : KTextEditor::CompletionEntry( rhs ) {
 }
 CodeCompletionEntry( const KTextEditor::CompletionEntry& rhs ) : KTextEditor::CompletionEntry( rhs ) {
 }
 
 bool operator < ( const CodeCompletionEntry& rhs ) {
  return userdata < rhs.userdata;
 }
 bool operator == ( const CodeCompletionEntry& rhs ) {
  return userdata == rhs.userdata;
 }
 bool operator > ( const CodeCompletionEntry& rhs ) {
  return userdata > rhs.userdata;
 }
 
 CodeCompletionEntry& operator = ( const KTextEditor::CompletionEntry& rhs ) {
  (*(KTextEditor::CompletionEntry*)this) = rhs;
  return *this;
 }
};

#endif
// kate: indent-mode csands; tab-width 4;
