/*
   Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "codecompletionhelper.h"

#include "../duchain/duchain.h"
#include "../duchain/declaration.h"
#include "../duchain/duchainlock.h"
#include "../duchain/types/functiontype.h"
#include "../duchain/types/integraltype.h"

#include <KTextEditor/Document>
#include <KTextEditor/View>

namespace KDevelop {

void insertFunctionParenText(KTextEditor::Document* document, const KTextEditor::Cursor& pos, DeclarationPointer declaration, bool jumpForbidden)
{
  bool spaceBeforeParen = false; ///@todo Take this from some astyle config or something
  bool spaceBetweenParens = false;
  bool spaceBetweenEmptyParens = false;

  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());

  bool haveArguments = false;

  if(!declaration)
    return;
  
  TypePtr< FunctionType > funcType = declaration->type<FunctionType>();
  
  if( declaration->kind() == Declaration::Type || (funcType && funcType->indexedArgumentsSize()) )
    haveArguments = true;
  
  if( declaration->kind() == Declaration::Instance && !declaration->isFunctionDeclaration())
    haveArguments = true; //probably a constructor initializer
  
  //Need to have a paren behind
  QString suffix = document->text( KTextEditor::Range( pos, pos + KTextEditor::Cursor(1, 0) ) );
  if( suffix.trimmed().startsWith('(') ) {
    //Move the cursor behind the opening paren
    if( document->activeView() )
      document->activeView()->setCursorPosition( pos + KTextEditor::Cursor( 0, suffix.indexOf('(')+1 ) );
  }else{
    //We need to insert an opening paren
    QString openingParen;
    if( spaceBeforeParen )
      openingParen = " (";
    else
      openingParen = '(';

    if( spaceBetweenParens && (haveArguments || spaceBetweenEmptyParens) )
      openingParen += ' ';

    QString closingParen;
    if( spaceBetweenParens && (haveArguments) ) {
      closingParen = " )";
    } else
      closingParen = ')';

    KTextEditor::Cursor jumpPos = pos + KTextEditor::Cursor( 0, openingParen.length() );

    // when function returns void, also add a semicolon
    if (funcType) {
      if (IntegralType::Ptr type = funcType->returnType().cast<IntegralType>()) {
        if (type->dataType() == IntegralType::TypeVoid) {
          const QChar nextChar = document->characterAt(pos);
          if (nextChar != ';' && nextChar != ')' && nextChar != ',') {
            closingParen += ';';
          }
        }
      }
    }

    //If no arguments, move the cursor behind the closing paren (or semicolon)
    if( !haveArguments )
      jumpPos += KTextEditor::Cursor( 0, closingParen.length() );


    lock.unlock();
    document->insertText( pos, openingParen + closingParen );
    if(!jumpForbidden) {
      if( document->activeView() )
        document->activeView()->setCursorPosition( jumpPos );
    }
  }

}

}
