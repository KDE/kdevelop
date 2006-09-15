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
#define USE_HASH_MAP

#ifdef USE_HASH_MAP
#include <ext/hash_map>
#endif

#define TEST_REMAP

template <class Base>
class SimpleTypeCacheBinder : public Base {
  public:

    SimpleTypeCacheBinder( SimpleTypeCacheBinder<Base>* b ) : Base( b ), m_locateCache( b->m_locateCache ), m_memberCache( b->m_memberCache ), m_basesCache( b->m_basesCache ), secondaryActive( b->secondaryActive ), primaryActive( b->primaryActive ) {}

    SimpleTypeCacheBinder() : Base(), m_haveBasesCache( false ), secondaryActive( true ), primaryActive( true ) {}

    template <class InitType>
    SimpleTypeCacheBinder( InitType t ) : Base ( t ), m_haveBasesCache( false ) , secondaryActive( true ), primaryActive( true ) {}

    template <class InitType1, class InitType2>
    SimpleTypeCacheBinder( InitType1 t, InitType2 t2 ) : Base ( t, t2 ), m_haveBasesCache( false ), secondaryActive( true ), primaryActive( true ) {}

    using Base::LocateMode;

    struct LocateDesc {
      TypeDesc mname;
      QString fullName;
      SimpleTypeImpl::LocateMode mmode;
      int mdir;
      SimpleTypeImpl::MemberInfo::MemberType mtypeMask;

      LocateDesc() {}

      LocateDesc( TypeDesc name, SimpleTypeImpl::LocateMode mode, int dir, SimpleTypeImpl::MemberInfo::MemberType typeMask ) : mname( name ), mmode( mode ) , mdir( dir ) , mtypeMask( typeMask ) {
        fullName = mname.fullNameChain();//fullTypeStructure();
      }

      int compare( const LocateDesc& rhs ) const {
        QString a = fullName;
        QString b = rhs.fullName;
        if ( a != b ) {
          if ( a < b )
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
        return compare( rhs ) == 0;
      }

      bool operator > ( const LocateDesc& rhs ) const {
        return compare( rhs ) == 1;
      }
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
#ifndef USE_HASH_MAP
		  fullName = m_desc.fullNameChain();
#endif
      }
	    //#ifndef USE_HASH_MAP
	    
       int compare( const MemberFindDesc& rhs ) const {
	       if( fullName.isEmpty() ) const_cast<MemberFindDesc*>(this)->fullName = m_desc.fullNameChain();
	       if( rhs.fullName.isEmpty() ) const_cast<MemberFindDesc*>(&rhs)->fullName = rhs.m_desc.fullNameChain();
         const QString& a = fullName; //m_desc.fullNameChain();
         const QString& b = rhs.fullName; //m_desc.fullNameChain();
         if( a != b ) {
           if( a < b )
             return -1;
           else
             return 1;
         }
         if( findType != rhs.findType ) {
           if( findType < rhs.findType )
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
      size_t hashKey() const {
	      return m_hashKey;
      }

      bool operator == ( const MemberFindDesc& rhs ) const {
	      //return m_hashKey == rhs.m_hashKey;
	      //return m_desc == rhs.m_desc; //compare( rhs ) == 0;
	      bool ret = m_hashKey2 == rhs.m_hashKey2 && findType == rhs.findType;
	      //bool ret = compare( rhs ) == 0;
	      /*if( ret ) {
		      if( compare( rhs ) != 0 ) {
		      kdDebug( 9007 ) << "mismatched keys " << m_hashKey2 << " " << rhs.m_hashKey2 << ": " << m_desc.fullNameChain() << " " << rhs.m_desc.fullNameChain() << "(" << fullName << ", " << rhs.fullName << ")" << endl;
			      ret = false;
		      }
	      }*/
	      return ret;
      }
      /*
       bool operator > ( const MemberFindDesc& rhs ) const {
         return compare( rhs ) == 1;
       }*/
      struct hash {
        size_t operator () ( const MemberFindDesc& m ) const {
          return m.hashKey();
        }
      };
    };


    typedef QMap<LocateDesc, SimpleTypeImpl::LocateResult> LocateMap;
#ifdef USE_HASH_MAP
	typedef __gnu_cxx::hash_map<MemberFindDesc, SimpleTypeImpl::MemberInfo, typename MemberFindDesc::hash > MemberMap;
#else
	typedef QMap<MemberFindDesc, SimpleTypeImpl::MemberInfo> MemberMap;
#endif

    virtual SimpleTypeImpl::MemberInfo findMember( TypeDesc name, SimpleTypeImpl::MemberInfo::MemberType type ) {
      if ( !primaryActive )
        return Base::findMember( name, type );
      MemberFindDesc key( name, type );
      typename MemberMap::iterator it = m_memberCache.find( key );

      if ( it != m_memberCache.end() ) {
      ifVerbose( dbg() << "\"" << Base::str() << "\" took member-info for \"" << name.fullNameChain() << "\" from the cache: " << (*it).second.name << endl );
#ifdef USE_HASH_MAP
	      return (*it).second;
#else
	      return *it;
#endif
      } else {
        SimpleTypeImpl::MemberInfo mem = Base::findMember( name, type );
#ifdef USE_HASH_MAP
	      m_memberCache.insert( std::make_pair( key, mem ) );
#else
		m_memberCache.insert( key, mem );
#endif
#ifdef TEST_REMAP
		typename MemberMap::iterator it = m_memberCache.find( key );
        if( it == m_memberCache.end() ) dbgMajor() << "\"" << Base::str() << "\"remap failed with \""<< name.fullNameChain() << "\"" << endl;
#endif

        return mem;
      }
    }

    virtual SimpleTypeImpl::LocateResult locateType( TypeDesc name , SimpleTypeImpl::LocateMode mode, int dir, SimpleTypeImpl::MemberInfo::MemberType typeMask ) {
      if ( !secondaryActive )
        return Base::locateType( name, mode, dir, typeMask );
      LocateDesc desc( name, mode, dir, typeMask );

      typename LocateMap::iterator it = m_locateCache.find( desc );

      if ( it != m_locateCache.end() ) {
        Debug d( "#lo#" );
        ifVerbose( dbg() << "\"" << Base::str() << "\" located \"" << name.fullNameChain() << "\" from the cache" << endl );
        return *it;
      } else {
        SimpleTypeImpl::LocateResult t = Base::locateType( name, mode, dir, typeMask );
        m_locateCache.insert( desc, t );
#ifdef TEST_REMAP
        typename LocateMap::iterator it = m_locateCache.find( desc );
        if( it == m_locateCache.end() ) dbgMajor() << "\"" << Base::str() << "\"remap failed with \""<< name.fullNameChain() << "\"" << endl;
#endif
        return t;
      }
    }

    virtual QValueList<SimpleTypeImpl::LocateResult> getBases() {
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
    QValueList<SimpleTypeImpl::LocateResult> m_basesCache;
    bool m_haveBasesCache;
    bool secondaryActive, primaryActive;

  protected:

    virtual typename Base::TypePointer clone() {
      return new SimpleTypeCacheBinder<Base>( this );
    }

    virtual void invalidatePrimaryCache() {
      //if( !m_memberCache.isEmpty() ) dbg() << "\"" << Base::str() << "\" primary caches cleared" << endl;
      m_memberCache.clear();
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
      invalidatePrimaryCache();
      invalidateSecondaryCache();
    };

    /*
    private:

      typedef QMap<QString, TypePointer> AliasMap;
      AliasMap m_aliasCache;

      bool haveCachedAlias( const QString& type ) {
          AliasMap::iterator it = m_aliasCache.find( type );
          if( it != m_aliasCache.end() )
              return true;
          else
              return false;
      }

      void setCachedAlias( const QString& key, TypePointer val ) {
          m_aliasCache[ key ] = val;
      }

      TypePointer getCachedAlias( SimpleType type ) {
          AliasMap::iterator it = m_aliasCache.find( type );
          if( it != m_aliasCache.end() )
              return &(**it);
          else
              dbg() << "getCachedAlias: trying to get alias which is not available in \"" << str() << "\" alias: \"" << type.str() << "\"";
          return TypePointer( );
      }



      void clearAliases() {
          m_aliasCache.clear();
      }   */
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
