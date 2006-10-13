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

#include<hashedstring.h>
#include "simpletype.h"
#include "includefiles.h"
#include <set>

class SimpleTypeNamespace : public SimpleTypeImpl {
  public:

    struct Alias {
      Alias( const QString& _alias ) : alias( _alias ) {
      }
      
      Alias( const IncludeFiles& _files, const QString& _alias ) : files( _files ), alias( _alias ) {
      }

      ///Does not respect the include-file-list, only the alias is compared
      bool operator < ( const Alias& rhs ) const {
        return alias < rhs.alias;
      }

      ///Does not respect the include-file-list, only the alias is compared
      bool operator == ( const Alias& rhs ) const {
        return alias == rhs.alias;
      }
      
      IncludeFiles files;
      QString alias;
      /*
      bool operator < ( const Alias& rhs ) const {
        if( alias < rhs.alias ) return true;
        return false;
      }
      
      bool operator == ( const Alias& rhs ) const {
        return alias == rhs.alias && files == rhs.files;
      }*/
      
    };

    typedef QPair<SimpleType, IncludeFiles> SlavePair;
    typedef QValueList<SlavePair> SlaveList;
  
  typedef std::multiset<Alias> AliasList;
  
    SimpleTypeNamespace( QStringList fakeScope, QStringList realScope = QStringList() );
  
    SimpleTypeNamespace( SimpleTypeNamespace* ns );
  
    bool isANamespace( SimpleTypeImpl* t ) {
      return dynamic_cast<SimpleTypeNamespace*>(t) != 0;
    }
    
    virtual TypePointer clone();
    
    SlaveList getSlaves();
  
    /**empty name means an import.
     * @param files Set of files that must be included for this alias-map to be active. If the set is empty, the alias will be used globally.
     */
    void addAliasMap( QString name, QString alias , const IncludeFiles& files = IncludeFiles(), bool recurse = true, bool symmetric = false );
  
    /**Takes a map of multiple aliases in form "A=B;C=D;....;" similar to the C++ "namespace A=B;" statement
     * @param files Set of files that must be included for this alias-map to be active. If the set is empty, the alias will be used globally.
     */
    void addAliases( QString map, const IncludeFiles& files = IncludeFiles() );

  private:
    QValueList< SlavePair > m_activeSlaves;
    QValueList< SimpleType > m_waitingAliases; ///For caching-reasons, it is necessary to import the aliases later after the call to addScope(because addscope is already called within the constructor, but the namespace is put into the cache after the constructor was called). This list holds all aliases to add.
    /// Maps local sub-namespace -> global namespace(multiple aliases are possible)
    typedef QMap<QString, AliasList> AliasMap;
    AliasMap m_aliases;
  
  
    void addScope( const QStringList& scope, const IncludeFiles& files = IncludeFiles() );

	void addScope( const TypePointer& t );
	
    friend class NamespaceBuildInfo;
  
    struct NamespaceBuildInfo : public TypeBuildInfo {
      QStringList m_fakeScope;
      QStringList m_realScope;
      AliasList m_imports;
	  TypePointer m_built;
    
    
      NamespaceBuildInfo( QStringList fakeScope, QStringList realScope, const AliasList& imports ) {
        m_fakeScope = fakeScope;
        m_realScope = realScope;
        m_imports = imports;
      }
    
      virtual TypePointer build();
    };
  
  protected:

    void updateAliases();

	SimpleTypeImpl::MemberInfo findMember( TypeDesc name, MemberInfo::MemberType type, std::set<SimpleTypeNamespace*>& ignore );
		
    virtual bool hasNode() const;
  
    virtual bool isNamespace() const {
      return true;
    }
    
    virtual MemberInfo findMember( TypeDesc name, MemberInfo::MemberType type =  MemberInfo::AllTypes );

	virtual QValueList<TypePointer> getMemberClasses( const TypeDesc& name ) ;

    MemberInfo setupMemberInfo( TypeDesc& subName, QStringList tscope, AliasList imports = AliasList() );
  
	QStringList locateNamespaceScope( QString alias );

	TypePointer locateNamespace( QString alias );
	
    void recurseAliasMap() ;
};


#endif
// kate: indent-mode csands; tab-width 4;

