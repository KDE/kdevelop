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
#ifndef SIMPLETYPENAMESPACE_H
#define SIMPLETYPENAMESPACE_H

#include "simpletype.h"

class SimpleTypeNamespace : public SimpleTypeImpl {
  public:
  
    SimpleTypeNamespace( QStringList fakeScope, QStringList realScope = QStringList() );
  
    SimpleTypeNamespace( SimpleTypeNamespace* ns );
  
    bool isANamespace( SimpleTypeImpl* t ) {
      return dynamic_cast<SimpleTypeNamespace*>(t) != 0;
    }
    
    virtual TypePointer clone();
    
    QValueList<SimpleType> getSlaves() {
      return m_activeSlaves;
    }
  
    ///empty name means an import
    void addAliasMap( QString name, QString alias , bool recurse = true, bool symmetric = false );
  
    ///Takes a map of multiple aliases in form "A=B;C=D;....;" similar to the C++ "namespace A=B;" statement
    void addAliases( QString map );

  private:
    QValueList<SimpleType> m_activeSlaves;
    /// Maps local sub-namespace -> global namespace(multiple aliases are possible)
    typedef QMap<QString, QStringList > AliasMap;
    AliasMap m_aliases;
  
  
	void addScope( const QStringList& scope );

	void addScope( const TypePointer& t );
	
    friend class NamespaceBuildInfo;
  
    struct NamespaceBuildInfo : public TypeBuildInfo {
      QStringList m_fakeScope;
      QStringList m_realScope;
      QValueList<QStringList> m_imports;
    
    
      NamespaceBuildInfo( QStringList fakeScope, QStringList realScope, const QValueList<QStringList>& imports ) {
        m_fakeScope = fakeScope;
        m_realScope = realScope;
        m_imports = imports;
      }
    
      virtual TypePointer build();
    };
  
  protected:

	SimpleTypeImpl::MemberInfo findMember( TypeDesc name, MemberInfo::MemberType type, std::set <QString>& ignore );
		
    virtual bool hasNode() const;
  
    virtual bool isNamespace() const {
      return true;
    }
    
    virtual MemberInfo findMember( TypeDesc name, MemberInfo::MemberType type =  MemberInfo::AllTypes);
  
    MemberInfo setupMemberInfo( TypeDesc& subName, QStringList tscope, QValueList<QStringList> imports = QValueList<QStringList>() );
  
	QStringList locateNamespaceScope( QString alias );

	TypePointer locateNamespace( QString alias );
	
    void recurseAliasMap() ;
};


#endif
// kate: indent-mode csands; tab-width 4;

