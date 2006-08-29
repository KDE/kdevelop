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

#ifndef __TYPEDESC_H__
#define __TYPEDESC_H__

#include <ktexteditor/codecompletioninterface.h>
#include <ksharedptr.h>

#include "typedecoration.h"

class TypeDesc;
class TypeDescShared;
class SimpleTypeImpl;

class TypeTrace;

typedef KSharedPtr<TypeDescShared> TypeDescPointer;
typedef KSharedPtr<SimpleTypeImpl> TypePointer;

enum ResolutionFlags {
  NoFlag = 0,
  HadTypedef = 1,
  HadTemplate = 2,
  HadAlias = 3
};

class LocateResult {
    TypeDescPointer m_desc;
    int m_resolutionCount;
    ResolutionFlags m_flags;
    int m_locateDepth; ///How far away from the beginning the type was found(counting steps upwards and steps into base-classes. Counter is stopped on the first typedef.)
    TypeTrace* m_trace; ///pointer to the previous type in the trace-chain

  public:
    LocateResult();
    LocateResult( const TypeDesc& desc );

    LocateResult( const TypeDescPointer& desc );
    LocateResult( TypeDescShared* desc );
    LocateResult( const LocateResult& rhs );
    ~LocateResult();
    /*
    LocateResult& operator = ( const TypeDesc& rhs ) {
    	m_desc = new rhs;
    	return *this;
    }*/

    operator const TypeDesc&() const;

    operator TypeDesc&() ;

    TypeDesc& desc();

    int depth() const {
      return m_locateDepth;
    }

    ///This may be used to simply increase the depth while returning a LocateResult
    LocateResult& increaseDepth() {
      m_locateDepth++;
      return *this;
    }

    ///This may be used to simply reset the depth while returning a LocateResult
    LocateResult& resetDepth() {
      m_locateDepth = 0;
      return *this;
    }

    LocateResult& operator * () {
      return * this;
    }

    const LocateResult& operator * () const {
      return * this;
    }

    operator TypeDescPointer();

    operator bool() const;

    LocateResult& operator = ( const LocateResult& rhs );

    bool operator >( const LocateResult& rhs ) const {
      return m_resolutionCount > rhs.m_resolutionCount;
    }

    const TypeDesc* operator ->() const;

    TypeDesc* operator ->();

    int resolutionCount() const {
      return m_resolutionCount;
    }

    void increaseResolutionCount() {
      m_resolutionCount++;
    }

    void addResolutionFlag( ResolutionFlags flag );

    bool hasResolutionFlag( ResolutionFlags flag ) const;

    TypeTrace* trace();
};

    ///These flags have no internal use, they are set and read from the outside
enum TypeDescFlags {
	Standard = 0,
		ResolutionTried = 1  ///means that the resolution was tried, and should not be retried.
};


struct TypeDescData : public KShared {
    typedef QValueList<LocateResult> TemplateParams;
    QString m_cleanName;
    int m_pointerDepth;
    int m_functionDepth;
    TemplateParams m_templateParams;
    TypeDescPointer m_nextType;
    TypePointer m_resolved;
    TypeDecoration m_dec;
    TypeDescFlags m_flags;

	TypeDescData() : m_hashValid( false ), m_hash2Valid( false ) {
	}
	void invalidateKey() {
		m_hashValid = false;
		m_hash2Valid = false;
	}

	size_t hashKey();
	size_t hashKey2(); 
private:
	bool m_hashValid;
	uint m_hashKey;
	bool m_hash2Valid;
	uint m_hashKey2;
};

class TypeDesc {
  public:
    typedef QValueList<LocateResult> TemplateParams;

	TypeDesc();

	TypeDesc( const QString& str );
	
    TypeDesc( const TypeDesc& rhs );

    bool deeper( const TypeDesc& rhs ) const {
      return depth() > rhs.depth();
    }

    bool longer( const TypeDesc& rhs ) const {
      return length() > rhs.length();
    }

    TypeDesc& operator = ( const TypeDesc& rhs );

    TypeDesc& operator = ( const QString& rhs ) {
      init( rhs );
      return *this;
    }

    TypeDesc firstType() const {
      TypeDesc ret = *this;
      ret.setNext( 0 );
      return ret;
    }

	uint hashKey() const;

	/**Returns a hash-key that is computed in a different way than the first.
	 * If both keys match, it is pretty sure that it is the same type.
	 * */
	uint hashKey2() const;

///this function must be remade
    bool isValidType() const ;

    int depth() const;

    int length() const ;

    ///Something is wrong with this function.. so i use the string-comparison
    int compare ( const TypeDesc& rhs ) const;

    bool operator < ( const TypeDesc& rhs ) const {
      return compare( rhs ) == -1;
    }

    bool operator > ( const TypeDesc& rhs ) const {
      return compare( rhs ) == 1;
    }

    bool operator == ( const TypeDesc& rhs ) const {
      return compare( rhs ) == 0;
    }

    QString nameWithParams() const;

    ///returns the type including template-parameters and pointer-depth
    QString fullName( ) const;

    /**returns the type include template-parameters, pointer-depth, and possible sub-types.
       Example "A::B": A is the type, and B is the subtype */
    QString fullNameChain( ) const ;

    ///Returns the type-structure(full name-chain without any instance-info)
    QString fullTypeStructure() const;

    int pointerDepth() const {
	    if( !m_data ) return 0;
	    
      return m_data->m_pointerDepth;
    }

    void setPointerDepth( int d ) {
      makeDataPrivate();
      m_data->m_pointerDepth = d;
    }

    void decreasePointerDepth() {
	    maybeInit();
	    
      if ( m_data->m_pointerDepth > 0 ) {
        makeDataPrivate();
        m_data->m_pointerDepth--;
      }
    }

    ///returns a list include the full name of this type, and all subtypes
    QStringList fullNameList( ) const;

    QString name() const {
	    if( !m_data ) return "";
      return m_data->m_cleanName;
    };

    void setName( QString name ) {
      makeDataPrivate();
      m_data->m_cleanName = name;
    }

    /** The template-params may be changed in-place
        this list is local, but the params pointed by them not(call makePrivate before changing) */
    TemplateParams& templateParams();

    const TemplateParams& templateParams() const;

    ///clears the current template-parameters, and extracts those from the given string
    void takeTemplateParams( const QString& string );

    /**makes all references/pointers private, so everything about this structure may be changed without side-effects*/
    TypeDesc& makePrivate();

    operator bool () const {
	    if( !m_data ) return false;
	    
      return !m_data->m_cleanName.isEmpty();
    }

    TypeDescPointer next();

    bool hasTemplateParams() const ;

    void setNext( TypeDescPointer type );

    void append( TypeDescPointer type );

    TypePointer resolved() const ;

    void setResolved( TypePointer resolved );

    void resetResolved();

    ///Resets the resolved-pointers of this type, and all template-types
    void resetResolvedComplete();

    void increaseFunctionDepth();

    void decreaseFunctionDepth();

    int functionDepth() const;

    static const char* functionMark;

    void setFlag( TypeDescFlags flag ) {
	    makeDataPrivate();
	    m_data->m_flags = ( TypeDescFlags ) ( m_data->m_flags | flag );
    }

    bool hasFlag( TypeDescFlags flag ) {
	    if( !m_data ) return false;
	    
	    return ( bool ) ( m_data->m_flags & flag );
    }

    ///instance-information consists of things like the pointer-depth and the decoration
    void takeInstanceInfo( const TypeDesc& rhs );

    void clearInstanceInfo();


  private:
	void takeData( const QString& string );
    void makeDataPrivate();
    KSharedPtr<TypeDescData> m_data;

	void  maybeInit();
  	void init( QString stri );

};

class TypeDescShared : public TypeDesc, public KShared {
  public:


    TypeDescShared( const TypeDescShared& rhs ) : TypeDesc( rhs ), KShared() {}

    TypeDescShared( const TypeDesc& rhs ) : TypeDesc( rhs ), KShared() {}

    TypeDescShared& operator = ( const TypeDesc& rhs ) {
      ( *( TypeDesc* ) this ) = rhs;
      return *this;
    }

	TypeDescShared( const QString& name ) : TypeDesc( name ) {}
	TypeDescShared() : TypeDesc() {}
}
;

extern TypeDesc operator + ( const TypeDesc& lhs, const TypeDesc& rhs );

#endif 
// kate: indent-mode csands; tab-width 4;
