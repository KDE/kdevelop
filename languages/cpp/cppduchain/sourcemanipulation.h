/*
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
class KDEVCPPDUCHAIN_EXPORT SourceCodeInsertion : public KShared
{
public:
  SourceCodeInsertion(KDevelop::TopDUContext* topContext);
  virtual ~SourceCodeInsertion();
  
  ///Set a position before which any code must be inserted
  virtual void setInsertBefore(KDevelop::SimpleCursor position);
  ///Set context into which code must be inserted. This must be called before calling modifier functions.
  virtual void setContext(KDevelop::DUContext* context);
  ///Set optional sub-scope into which the code should be inserted.
  virtual void setSubScope(KDevelop::QualifiedIdentifier scope);
  ///Set optional access-policy for the inserted items
  virtual void setAccess(KDevelop::Declaration::AccessPolicy access);

  ///Adds a variable declaration using the parameters given before
  virtual bool insertVariableDeclaration(KDevelop::Identifier name, KDevelop::AbstractType::Ptr type);
  
  KDevelop::DocumentChangeSet& changes();
  
  protected:
    virtual QString accessString() const;
    virtual QString indentation() const;
    
    KDevelop::DocumentChangeSet m_changeSet;
    KDevelop::SimpleCursor m_insertBefore;
    KDevelop::DUContext* m_context;
    KDevelop::QualifiedIdentifier m_scope;
    KDevelop::Declaration::AccessPolicy m_access;
    KDevelop::TopDUContext* m_topContext;
    //Represents the whole code of the manipulated top-context for reading.
    //Changes should be done as transactions to m_changeSet.
    ///@warning This must be checked for zero before using. It is zero if the file could not be read.
    const KDevelop::CodeRepresentation* m_codeRepresentation;
};
}

namespace Cpp {

class KDEVCPPDUCHAIN_EXPORT SourceCodeInsertion : public KDevelop::SourceCodeInsertion {
  public:
  SourceCodeInsertion(KDevelop::TopDUContext* topContext);
  ///setContext(..) must have been called before with the class-context
  virtual bool insertSlot(QString name, QString normalizedSignature);
};

}


#endif // CPP_SOURCEMANIPULATION_H
 