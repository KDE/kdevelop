/*
   Copyright 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>
   
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

#ifndef CPP_SOURCEMANIPULATION_H
#define CPP_SOURCEMANIPULATION_H

#include <language/duchain/declaration.h>
#include <language/codegen/documentchangeset.h>
#include <language/codegen/coderepresentation.h>
#include "cppduchainexport.h"

namespace KDevelop {

//Through the whole lifetime of this object, the duchain must be locked
//Note: _nothing_ will happen until you apply the resulting changes, that can be retrieved through changes()
class KDEVCPPDUCHAIN_EXPORT SourceCodeInsertion : public QSharedData
{
public:
  SourceCodeInsertion(KDevelop::TopDUContext* topContext);
  virtual ~SourceCodeInsertion();
  
  ///Set a position before which any code must be inserted
  virtual void setInsertBefore(KTextEditor::Cursor position);
  ///Set context into which code must be inserted. This must be called before calling modifier functions.
  virtual void setContext(KDevelop::DUContext* context);
  ///Set optional sub-scope into which the code should be inserted, under 'context'
  virtual void setSubScope(KDevelop::QualifiedIdentifier scope);
  ///Set optional access-policy for the inserted items
  virtual void setAccess(KDevelop::Declaration::AccessPolicy access);
  ///Adds a variable declaration using the parameters given before
  virtual bool insertVariableDeclaration(KDevelop::Identifier name, KDevelop::AbstractType::Ptr type);

  struct SignatureItem {
    AbstractType::Ptr type;
    QString name;
  };

  ///@param body Optional function-body, including parens
  virtual bool insertFunctionDeclaration(KDevelop::Identifier name, KDevelop::AbstractType::Ptr returnType, QList< KDevelop::SourceCodeInsertion::SignatureItem > signature, bool isConstant = false, QString body = QString());
  
  ///Use the returned change-set to eventually let the user review the changes, and apply them.
  KDevelop::DocumentChangeSet& changes();

  ///Moves the given line-number to a position that is not part of a comment, is behind the preprocessor/#ifdef code at top of a file,
  ///and is before or equal @param line
  virtual int firstValidCodeLineBefore(int line) const;
  
  protected:
    enum InsertionKind {
      Variable,
      Function,
      Slot
    };
    
    struct InsertionPoint {
      int line;
      QString prefix;
    };
    
    ///Returns the exact position where the item should be inserted so it is in the given line.
    ///The inserted item has to start with a newline, and does not need to end with a newline.
    KTextEditor::Range insertionRange(int line);
    
    ///Returns a pair: (line, prefix) for inserting the given kind of declaration with the given access policy
    InsertionPoint findInsertionPoint(KDevelop::Declaration::AccessPolicy policy, InsertionKind kind) const;
    //Should apply m_scope to the given declaration string
    virtual QString applySubScope(QString decl) const;
    virtual QString accessString() const;
    virtual QString indentation() const;
    virtual QString applyIndentation(QString decl) const;

    ///Returns an end-cursor that is guaranteed to fit into the current document. The top-context may have an invalid end-cursor if
    ///the document is not open.
    KTextEditor::Cursor end() const;
    
    KDevelop::DocumentChangeSet m_changeSet;
    KTextEditor::Cursor m_insertBefore;
    KDevelop::DUContext* m_context;
    KDevelop::QualifiedIdentifier m_scope;
    KDevelop::Declaration::AccessPolicy m_access;
    KDevelop::TopDUContext* m_topContext;
    //Represents the whole code of the manipulated top-context for reading.
    //Changes should be done as transactions to m_changeSet.
    ///@warning This must be checked for zero before using. It is zero if the file could not be read.
    const KDevelop::CodeRepresentation::Ptr m_codeRepresentation;
};
}

namespace Cpp {

class KDEVCPPDUCHAIN_EXPORT SourceCodeInsertion : public KDevelop::SourceCodeInsertion {
  public:
  SourceCodeInsertion(KDevelop::TopDUContext* topContext);
  ///setContext(..) must have been called before with the class-context
  virtual bool insertSlot(QString name, QString normalizedSignature);
  ///If this is used, only setInsertBefore(..) needs to be called before.
  virtual bool insertForwardDeclaration(KDevelop::Declaration* decl);
};

}


#endif // CPP_SOURCEMANIPULATION_H 
