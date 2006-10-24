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
#include <list>

class SimpleTypeNamespace : public SimpleTypeImpl {
  public:

    struct Import {
      Import( const TypeDesc& _import ) : import( _import ) {
      }
      
      Import( const IncludeFiles& _files, const TypeDesc& _import ) : files( _files ), import( _import ) {
      }

      ///Does not respect the include-file-list, only the import-name is compared
      bool operator < ( const Import& rhs ) const {
        return import.name() < rhs.import.name();
      }

      ///Does not respect the include-file-list, only the import-name is compared
      bool operator == ( const Import& rhs ) const {
        return import.name() == rhs.import.name();
      }
      
      IncludeFiles files;
      TypeDesc import;
      /*
      bool operator < ( const Alias& rhs ) const {
        if( alias < rhs.alias ) return true;
        return false;
      }
      
      bool operator == ( const Alias& rhs ) const {
        return alias == rhs.alias && files == rhs.files;
      }*/
      
    };

    typedef std::list<TypeDesc> SlaveList;
  
  typedef std::multiset<Import> ImportList;
  
    SimpleTypeNamespace( const QStringList& fakeScope, const QStringList& realScope );

    SimpleTypeNamespace( const QStringList& fakeScope );
    
    SimpleTypeNamespace( SimpleTypeNamespace* ns );
  
    bool isANamespace( SimpleTypeImpl* t ) {
      return dynamic_cast<SimpleTypeNamespace*>(t) != 0;
    }
    
    virtual TypePointer clone();

    ///Returns a list of all slave-namespaces that have an effect with the given set of include-files(the returned typedescs should be resolved)
    SlaveList getSlaves( const IncludeFiles& includeFiles );
  
    /**empty name means an import.
     * @param files Set of files that must be included for this alias-map to be active. If the set is empty, the alias will be used globally.
     */
    void addAliasMap( const TypeDesc& name, const TypeDesc& alias , const IncludeFiles& files = IncludeFiles(), bool recurse = true, bool symmetric = false );
  
    /**Takes a map of multiple aliases in form "A=B;C=D;....;" similar to the C++ "namespace A=B;" statement
     * @param files Set of files that must be included for this alias-map to be active. If the set is empty, the alias will be used globally.
     */
    void addAliases( QString map, const IncludeFiles& files = IncludeFiles() );

  private:
    SlaveList m_activeSlaves;
    typedef QMap<QString, ImportList> AliasMap;
    AliasMap m_aliases;

    //Inserts all aliases necessary fo handling a request using the given IncludeFiles
    void updateAliases( const IncludeFiles& files );
    
//     LocateResult locateSlave( const SlaveList::const_iterator& it, const IncludeFiles& includeFiles );

    void addImport( const TypeDesc& import, const IncludeFiles& files = IncludeFiles() );

    friend class NamespaceBuildInfo;
  
    struct NamespaceBuildInfo : public TypeBuildInfo {
      QStringList m_fakeScope;
      ImportList m_imports;
	  TypePointer m_built;
    
    
      NamespaceBuildInfo( QStringList fakeScope, const ImportList& imports ) {
        m_fakeScope = fakeScope;
        m_imports = imports;
      }
    
      virtual TypePointer build();
    };

    explicit SimpleTypeNamespace( const SimpleTypeNamespace& rhs ) {
    }
  
  protected:

    //void updateAliases( const HashedStringSet& files );

    SimpleTypeImpl::MemberInfo findMember( TypeDesc name, MemberInfo::MemberType type, std::set<SimpleTypeImpl*>& ignore );
		
    virtual bool hasNode() const;
  
    virtual bool isNamespace() const {
      return true;
    }
    
    virtual MemberInfo findMember( TypeDesc name, MemberInfo::MemberType type =  MemberInfo::AllTypes );

	virtual QValueList<TypePointer> getMemberClasses( const TypeDesc& name ) ;

    MemberInfo setupMemberInfo( const QStringList& subName, const ImportList& imports );
  
    //TypePointer locateNamespace( const TypeDesc& alias );
	
    //void recurseAliasMap() ;
};


#endif
// kate: indent-mode csands; tab-width 4;

