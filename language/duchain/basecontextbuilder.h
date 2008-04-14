/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef BASECONTEXTBUILDER_H
#define BASECONTEXTBUILDER_H

#include "topducontext.h"
#include "duchainpointer.h"

class KUrl;

namespace KDevelop
{
class EditorIntegrator;
class TopDUContext;
class DUContext;
class DUChainBase;
class QualifiedIdentifier;

template<typename T>
class BaseContextBuilder
{
public:
  BaseContextBuilder();
  BaseContextBuilder( EditorIntegrator* editor );
  ~BaseContextBuilder();

  TopDUContext buildContexts( const KUrl& url, T* ast,
                                        const TopDUContextPointer&
                                                updateContext = TopDUContextPointer() );
protected:
  
  virtual void supportBuild( T* node ) = 0;
  virtual void setContextOnNode( T* node, KDevelop::DUContext* ctx ) = 0;
  virtual DUContext* contextFromNode( T* node ) = 0;
  virtual KTextEditor::Range editorFindRange( T* fromRange, T* toRange ) = 0;
  virtual const QualifiedIdentifier identifierForNode( T* ) = 0;

  DUContext * currentContext();
  void smartenContext( TopDUContext* topLevelContext );
  KDevelop::DUContext* buildSubContexts( const KUrl& url, T *node,
                                         KDevelop::DUContext* parent );
  void deleteContextOnNode( T* );
  void smartenContext( TopDUContext* topLevelContext );
  void openContext( DUContext* newContext );
  DUContext* openContext( T* rangeNode, DUContext::ContextType type, T* identifier );
  DUContext* openContext( T* rangeNode, DUContext::ContextType type, const QualifiedIdentifier& identifier );
  DUContext* openContext( T* fromRange, T* toRange, DUContext::ContextType type, const QualifiedIdentifier& identifier );
  void closeContext();
  void setEncountered( DUChainBase* item );
  bool wasEncountered( DUChainBase* item );
  void smartenContext( TopDUContext* topLevelContext );
  
  EditorIntegrator* editor();
private:
  class BaseContextBuilderPrivate* const d;
};

}

#endif
