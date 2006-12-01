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

#ifndef __SIMPLETYPECACHEBINDER_H__
#define __SIMPLETYPECACHEBINDER_H__

#include "simpletypenamespace.h"

///In case the hashing-stuff brings some regressions, it can be removed again using this switch.
#include <ext/hash_map>
#include <map>

//#define TEST_REMAP

template <class Base>
class SimpleTypeCacheBinder : public Base {
 public:

  SimpleTypeCacheBinder( SimpleTypeCacheBinder<Base>* b ) : Base( b ), m_locateCache( b->m_locateCache ), m_memberCache( b->m_memberCache ), m_basesCache( b->m_basesCache ), secondaryActive( b->secondaryActive ), m_classListCache( b->m_classListCache ), primaryActive( b->primaryActive ), m_haveBasesCache( b->m_haveBasesCache ) {}

  SimpleTypeCacheBinder() : Base(), m_haveBasesCache( false ), secondaryActive( true ), primaryActive( true ) {}

  template <class InitType>
  SimpleTypeCacheBinder( InitType t ) : Base ( t ), m_haveBasesCache( false ) , secondaryActive( true ), primaryActive( true ) {}

  template <class InitType1, class InitType2>
  SimpleTypeCacheBinder( InitType1 t, InitType2 t2 ) : Base ( t, t2 ), m_haveBasesCache( false ), secondaryActive( true ), primaryActive( true ) {}

  using Base::LocateMode;

  struct LocateDesc {
   TypeDesc mname;
   //QString fullName;
   SimpleTypeImpl::LocateMode mmode;
   int mdir;
   SimpleTypeImpl::MemberInfo::MemberType mtypeMask;
   size_t m_hashKey;

   LocateDesc() {}

   LocateDesc( const TypeDesc& name, SimpleTypeImpl::LocateMode mode, int dir, SimpleTypeImpl::MemberInfo::MemberType typeMask ) : mname( name ), mmode( mode ) , mdir( dir ) , mtypeMask( typeMask ), m_hashKey( name.hashKey() + 11*int(mode) + 13*dir + 17*int(typeMask) ) {
    //fullName = mname.fullNameChain();//fullTypeStructure();

   }

   ///@todo this should use hashing too
   int compare( const LocateDesc& rhs ) const {
    if ( m_hashKey != rhs.m_hashKey ) {
     if ( m_hashKey < rhs.m_hashKey )
      return -1;
     else
      return 1;
    }
    if ( mname.hashKey2() != rhs.mname.hashKey2() ) {
     if ( mname.hashKey2() < rhs.mname.hashKey2() )
      return -1;
     else
      return 1;
    }
    if ( mmode != rhs.mmode ) {
     if ( mmode < rhs.mmode )
      return -1;
     else
      return 1;
    }
    if ( mdir != rhs.mdir ) {
     if ( mdir < rhs.mdir )
      return -1;
     else
      return 1;
    }
    if ( mtypeMask != rhs.mtypeMask ) {
     if ( mtypeMask < rhs.mtypeMask )
      return -1;
     else
      return 1;
    }

    return 0;
   }

   bool operator < ( const LocateDesc& rhs ) const {
    return compare( rhs ) == -1;
   }

   bool operator == ( const LocateDesc& rhs ) const {
       //return compare( rhs ) == 0;
       return m_hashKey == rhs.m_hashKey && mname.hashKey2() == rhs.mname.hashKey2();
   }

   bool operator > ( const LocateDesc& rhs ) const {
    return compare( rhs ) == 1;
   }

   inline size_t hashKey() const {
    return m_hashKey;
   }

   struct hash {
    inline size_t operator () ( const LocateDesc& m ) const {
     return m.hashKey();
    }

   };
  };

   struct MemberFindDesc {
    TypeDesc m_desc;
    QString fullName;
    SimpleTypeImpl::MemberInfo::MemberType findType;
    size_t m_hashKey; //in time check why it is necessary to cache this key
    size_t m_hashKey2;

    MemberFindDesc() : m_hashKey( 0 ) {}

    MemberFindDesc( TypeDesc d, SimpleTypeImpl::MemberInfo::MemberType ft ) : m_desc( d ), findType( ft ), m_hashKey( d.hashKey() + findType ), m_hashKey2( d.hashKey2() + findType ) {
     //m_desc.makePrivate();
    }

    int compare( const MemberFindDesc& rhs ) const {
     if ( fullName.isEmpty() ) const_cast<MemberFindDesc*>(this)->fullName = m_desc.fullNameChain();
     if ( rhs.fullName.isEmpty() ) const_cast<MemberFindDesc*>(&rhs)->fullName = rhs.m_desc.fullNameChain();
     const QString& a = fullName; //m_desc.fullNameChain();
     const QString& b = rhs.fullName; //m_desc.fullNameChain();
     if ( a != b ) {
      if ( a < b )
       return -1;
      else
       return 1;
     }
     if ( findType != rhs.findType ) {
      if ( findType < rhs.findType )
       return -1;
      else
       return 1;
     }

     return 0;
    }

    bool operator < ( const MemberFindDesc& rhs ) const {
     return compare( rhs ) == -1;
    }
    //#endif
    inline size_t hashKey() const {
     return m_hashKey;
    }

    bool operator == ( const MemberFindDesc& rhs ) const {
     bool ret = m_hashKey2 == rhs.m_hashKey2 && findType == rhs.findType;
     return ret;
    }
    /*
     bool operator > ( const MemberFindDesc& rhs ) const {
       return compare( rhs ) == 1;
     }*/
    struct hash {
     inline size_t operator () ( const MemberFindDesc& m ) const {
      return m.hashKey();
     }
    };
   };

   typedef __gnu_cxx::hash_map<LocateDesc, LocateResult, typename LocateDesc::hash > LocateMap;
   typedef __gnu_cxx::hash_map<MemberFindDesc, SimpleTypeImpl::MemberInfo, typename MemberFindDesc::hash > MemberMap;
   typedef __gnu_cxx::hash_map<MemberFindDesc, QValueList<TypePointer>, typename MemberFindDesc::hash > ClassListMap;

   virtual SimpleTypeImpl::MemberInfo findMember( TypeDesc name, SimpleTypeImpl::MemberInfo::MemberType type ) {
    if ( !primaryActive )
     return Base::findMember( name, type );
    MemberFindDesc key( name, type );
    typename MemberMap::iterator it = m_memberCache.find( key );

    if ( it != m_memberCache.end() ) {
     ifVerbose( dbg() << "\"" << Base::str() << "\" took member-info for \"" << name.fullNameChain() << "\" from the cache: " << (*it).second.name << endl );
     return (*it).second;
    } else {
     SimpleTypeImpl::MemberInfo mem;

     m_memberCache.insert( std::make_pair( key, mem ) ); //This is done to prevent expensive endless recursion

     mem = Base::findMember( name, type );

     std::pair< typename MemberMap::iterator, bool > r = m_memberCache.insert( std::make_pair( key, mem ) );
     if ( !r.second ) {
      (*r.first).second = mem;
     }

#ifdef TEST_REMAP
     typename MemberMap::iterator it = m_memberCache.find( key );
     if ( it == m_memberCache.end() ) dbgMajor() << "\"" << Base::str() << "\"remap failed with \"" << name.fullNameChain() << "\"" << endl;
#endif

     return mem;
    }
   }

   virtual QValueList<TypePointer> getMemberClasses( const TypeDesc& name ) {
    if ( !primaryActive )
     return Base::getMemberClasses( name );
    MemberFindDesc key( name, Base::MemberInfo::NestedType );
    typename ClassListMap::iterator it = m_classListCache.find( key );

    if ( it != m_classListCache.end() ) {
     /*ifVerbose( dbg() << "\"" << Base::str() << "\" took member-info for \"" << name.fullNameChain() << "\" from the cache: " << (*it).second. << endl );*/
     return (*it).second;
    } else {
     QValueList<TypePointer> mem;

     m_classListCache.insert( std::make_pair( key, mem ) );

     mem = Base::getMemberClasses( name );
     std::pair<typename ClassListMap::iterator, bool> r = m_classListCache.insert( std::make_pair( key, mem ) );
     if ( !r.second )
      (*r.first).second = mem;

#ifdef TEST_REMAP
     typename ClassListMap::iterator it = m_classListCache.find( key );
     if ( it == m_classListCache.end() ) dbgMajor() << "\"" << Base::str() << "\"remap failed with \"" << name.fullNameChain() << "\"" << endl;
#endif

     return mem;
    }
   }

   virtual LocateResult locateType( TypeDesc name , SimpleTypeImpl::LocateMode mode, int dir, SimpleTypeImpl::MemberInfo::MemberType typeMask ) {

    if ( !secondaryActive )
     return Base::locateType( name, mode, dir, typeMask );
    LocateDesc desc( name, mode, dir, typeMask );

    typename LocateMap::iterator it = m_locateCache.find( desc );

    if ( it != m_locateCache.end() ) {
     Debug d( "#lo#" );
     ifVerbose( dbg() << "\"" << Base::str() << "\" located \"" << name.fullNameChain() << "\" from the cache" << endl );
     return (*it).second;
    } else {
     LocateResult t;
     m_locateCache.insert( std::make_pair( desc, t ) ); //Done to prevent expensive recursion.
     t = Base::locateType( name, mode, dir, typeMask );
     std::pair< typename LocateMap::iterator, bool > r = m_locateCache.insert( std::make_pair( desc, t ) );
     if ( !r.second )
      (*r.first).second = t;

#ifdef TEST_REMAP
     typename LocateMap::iterator it = m_locateCache.find( desc );
     if ( it == m_locateCache.end() ) dbgMajor() << "\"" << Base::str() << "\"remap failed with \"" << name.fullNameChain() << "\"" << endl;
#endif
     return t;
    }
   }

   virtual QValueList<LocateResult> getBases() {
    if ( m_haveBasesCache ) {
     ifVerbose( dbg() << "\"" << Base::str() << "\" took base-info from the cache" << endl );
     return m_basesCache;
    } else {
     m_basesCache = Base::getBases();
     m_haveBasesCache = true;
     return m_basesCache;
    }
   }

  private:
   LocateMap m_locateCache;
   MemberMap m_memberCache;
   ClassListMap m_classListCache;
   QValueList<LocateResult> m_basesCache;
   bool m_haveBasesCache;
   bool secondaryActive, primaryActive;

  protected:

   virtual typename Base::TypePointer clone() {
    return new SimpleTypeCacheBinder<Base>( this );
   }

   virtual void invalidatePrimaryCache( bool  onlyNegative ) {
    //if( !m_memberCache.isEmpty() ) dbg() << "\"" << Base::str() << "\" primary caches cleared" << endl;
    if ( !onlyNegative)
     m_memberCache.clear();
    else {
     for ( typename MemberMap::iterator it = m_memberCache.begin(); it != m_memberCache.end(); ) {
      if ( (*it).second.memberType == SimpleTypeImpl::MemberInfo::NotFound )
       m_memberCache.erase( it++ );
      else
       ++it;
     }
    }
    m_classListCache.clear();
   }
   virtual void invalidateSecondaryCache() {
    //if( !m_locateCache.isEmpty() ) dbg() << "\"" << Base::str() << "\" secondary caches cleared" << endl;
    m_locateCache.clear();
    m_haveBasesCache = false;
   }

   virtual void setSecondaryCacheActive( bool active ) {
    secondaryActive = active;
   }

   virtual void setPrimaryCacheActive( bool active ) {
    primaryActive = active;
   }

   virtual void invalidateCache() {
    invalidatePrimaryCache( false );
    invalidateSecondaryCache();
   };
  };


//typedef SimpleTypeCacheBinder<SimpleTypeImpl> SimpleTypeImpl;

  class SimpleTypeCodeModel;
  class SimpleTypeCatalog;
  class SimpleTypeNamespace;
  class SimpleTypeCodeModelFunction;
  class SimpleTypeCatalogFunction;

  typedef SimpleTypeCacheBinder<SimpleTypeCodeModel> SimpleTypeCachedCodeModel;
  typedef SimpleTypeCacheBinder<SimpleTypeCatalog> SimpleTypeCachedCatalog;
  typedef SimpleTypeCacheBinder<SimpleTypeNamespace> SimpleTypeCachedNamespace;
  typedef SimpleTypeCacheBinder<SimpleTypeCodeModelFunction> SimpleTypeCachedCodeModelFunction;
  typedef SimpleTypeCacheBinder<SimpleTypeCatalogFunction> SimpleTypeCachedCatalogFunction;


#endif
// kate: indent-mode csands; tab-width 4;
