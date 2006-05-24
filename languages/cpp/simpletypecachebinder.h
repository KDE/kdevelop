/***************************************************************************
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

#ifndef __SIMPLETYPECACHEBINDER_H__
#define __SIMPLETYPECACHEBINDER_H__


#include "simpletype.h"

template <class Base>
class SimpleTypeCacheBinder : public Base {
private:
  bool secondaryActive, primaryActive;
public:
  
SimpleTypeCacheBinder( SimpleTypeCacheBinder<Base>* b ) : Base( b ), secondaryActive( b->secondaryActive ),     primaryActive( b->primaryActive ), m_locateCache( b->m_locateCache ), m_memberCache( b->m_memberCache), m_basesCache( b->m_basesCache )  {
}
  
SimpleTypeCacheBinder() : Base(), secondaryActive( true ), primaryActive( true ) {
}
  
  template<class InitType> 
    SimpleTypeCacheBinder( InitType t ) : Base ( t ), secondaryActive( true ), primaryActive( true ) {
    }
  
  template<class InitType1, class InitType2> 
    SimpleTypeCacheBinder( InitType1 t, InitType2 t2 ) : Base ( t, t2 ), secondaryActive( true ), primaryActive( true ) {
    }
  
  using Base::LocateMode;
  
  struct LocateDesc {
    TypeDesc mname;
    QString fullName;
    SimpleTypeImpl::LocateMode mmode;
    int mdir;
    SimpleTypeImpl::MemberInfo::MemberType mtypeMask;
    LocateDesc() {
    }
    
    
  LocateDesc( TypeDesc name, SimpleTypeImpl::LocateMode mode, int dir, SimpleTypeImpl::MemberInfo::MemberType typeMask )  : mname( name ), mmode( mode ) , mdir( dir ) , mtypeMask( typeMask ) {
    fullName = mname.fullNameChain();
  }
    
    int compare( const LocateDesc& rhs ) const {
      QString a = fullName; //mname.fullNameChain();
      QString b = rhs.fullName; //mname.fullNameChain();
      if( a != b ) {
        if( a < b )
          return -1;
        else
          return 1;
      }
      
      if( mmode != rhs.mmode ) {
        if( mmode < rhs.mmode )
          return -1;
        else
          return 1;
      }
      if( mdir != rhs.mdir ) {
        if( mdir < rhs.mdir )
          return -1;
        else
          return 1;
      }
      if( mtypeMask != rhs.mtypeMask ) {
        if( mtypeMask < rhs.mtypeMask )
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
    MemberFindDesc() {
    }
  MemberFindDesc( TypeDesc d, SimpleTypeImpl::MemberInfo::MemberType ft ) : m_desc( d ), findType( ft ) {
            //m_desc.makePrivate();
    fullName = m_desc.fullNameChain();
  }
    
    int compare( const MemberFindDesc& rhs ) const {
      QString a = fullName; //m_desc.fullNameChain();
      QString b = rhs.fullName; //m_desc.fullNameChain();
      if( a != b ) {
        if( a < b )
          return -1;
        else
          return 1;
      }/*
            if( int cmp = m_desc.compare( rhs.m_desc ) != 0 ) 
                return cmp;*/
      
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
    
    bool operator == ( const MemberFindDesc& rhs ) const {
      return compare( rhs ) == 0;
    }
    
    bool operator > ( const MemberFindDesc& rhs ) const {
      return compare( rhs ) == 1;
    }
    
  };
  
  
  typedef QMap<LocateDesc, SimpleType> LocateMap;
  typedef QMap<MemberFindDesc, SimpleTypeImpl::MemberInfo > MemberMap;
private:
  LocateMap m_locateCache;
  MemberMap m_memberCache;
  QValueList<SimpleType> m_basesCache;
public:
  
  virtual SimpleTypeImpl::MemberInfo findMember( TypeDesc name , SimpleTypeImpl::MemberInfo::MemberType type )  {
    if( !primaryActive ) return Base::findMember( name, type );
    MemberFindDesc key( name, type );
    typename MemberMap::iterator it = m_memberCache.find( key );
    
    if( it != m_memberCache.end() ) {
      dbg() << "\"" << Base::str() << "\" took member-info for \"" << name.fullNameChain() << "\" from the cache" << endl;
      return m_memberCache[key];
    } else {
      SimpleTypeImpl::MemberInfo mem = Base::findMember( name, type );
      m_memberCache[key] = mem;
            /*typename MemberMap::iterator it = m_memberCache.find( key );
            if( it == m_memberCache.end() ) dbg() << "\"" << Base::str() << "\"remap failed with \""<< name.fullNameChain() << "\"" << endl;*/
      
      return mem;
    }
  }
  
  
  virtual SimpleType locateType( TypeDesc name , SimpleTypeImpl::LocateMode mode, int dir,  SimpleTypeImpl::MemberInfo::MemberType typeMask )
  {
    if( !secondaryActive ) return  Base::locateType( name, mode, dir, typeMask );
    LocateDesc desc( name, mode, dir, typeMask );
    
    typename LocateMap::iterator it = m_locateCache.find( desc );
    
    if( it != m_locateCache.end() ) {
      Debug d("#lo#");
      dbg() << "\"" << Base::str() << "\" located \"" << name.fullNameChain() << "\" from the cache" << endl;
      return *it;
    } else {
      SimpleType t = Base::locateType( name, mode, dir, typeMask );
      m_locateCache[ desc ] = t;
            /*typename LocateMap::iterator it = m_locateCache.find( desc );
            if( it == m_locateCache.end() ) dbg() << "\"" << Base::str() << "\"remap failed with \""<< name.fullNameChain() << "\"" << endl;*/
      return t;
    }
  }
  
  virtual QValueList<SimpleType> getBases() {
    if( !m_basesCache.isEmpty() ) {
      dbg() << "\"" << Base::str() << "\" took base-info from the cache" << endl;
      return m_basesCache;
    } else
      return Base::getBases();
  }
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
    m_basesCache.clear();
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
  
private:
  
    /*  typedef QMap<QString, TypePointer> AliasMap;
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


#endif
// kate: indent-mode csands; tab-width 4;
