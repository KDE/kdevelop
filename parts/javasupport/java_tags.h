//
//
// Java Interface: java_tags
//
// Description: 
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef JAVA_TAGS_H
#define JAVA_TAGS_H

#include <qstring.h>
#include <qstringlist.h>

template <class Tag>
class JavaBaseClass
{
public:
    JavaBaseClass( Tag& tag )
	: m_tag( tag ) { m_info.flags = tag.flags(); }
    
    operator Tag& () { return asTag(); }
    Tag& asTag() { return m_tag; }
        
    operator const Tag& () const { return asTag(); }
    const Tag& asTag() const { return m_tag; }
    
    static QString format( const Tag& tag ) { return QString::null; }	
    
    int access() const 
    { 
	return m_info.data.access; 
    }
    
    void setAccess( int access )
    { 
	m_info.data.access = access;
	m_tag.setFlags( m_info.flags );
    }
    
    bool isVirtual() const
    {
	return m_info.data.isVirtual;
    }
    
    void setVirtual( bool b )
    {
	m_info.data.isVirtual = b;
	m_tag.setFlags( m_info.flags );
    }
        
    QString name() const
    {
	return m_tag.name();
    }
    
    void setName( const QString& name )
    {
	m_tag.setName( name );
    }
    
    QString baseClass() const
    {
	return m_tag.attribute( "b" ).toString();
    }
    
    void setBaseClass( const QString& baseClass )
    {
	m_tag.setAttribute( "b", baseClass );
    }
        
private:
    Tag& m_tag;
    union
    {
	unsigned long flags;
	struct
	{
	    unsigned long access: 3;
	    unsigned long isVirtual: 1;
	} data;
    } m_info;
};

template <class Tag>
class JavaVariable
{
public:
    JavaVariable( Tag& tag )
	: m_tag( tag ) { m_info.flags = tag.flags(); }
    
    operator Tag& () { return asTag(); }
    Tag& asTag() { return m_tag; }
        
    operator const Tag& () const { return asTag(); }
    const Tag& asTag() const { return m_tag; }
    
    static QString format( const Tag& tag ) { return QString::null; }	
    
    int access() const 
    { 
	return m_info.data.access; 
    }
    
    void setAccess( int access )
    { 
	m_info.data.access = access;
	m_tag.setFlags( m_info.flags );
    }
    
    bool isFriend() const
    {
	return m_info.data.isFriend;
    }
    
    void setFriend( bool b )
    {
	m_info.data.isFriend = b;
	m_tag.setFlags( m_info.flags );
    }
        
    bool isStatic() const
    {
	return m_info.data.isStatic;
    }
    
    void setStatic( bool b )
    {
	m_info.data.isStatic = b;
	m_tag.setFlags( m_info.flags );
    }
    
    QString type() const
    {
	return m_tag.attribute( "t" ).toString();
    }
    
    void setType( const QString& type )
    {
	m_tag.setAttribute( "t", type );
    }
    
    QString name() const
    {
	return m_tag.name();
    }
    
    void setName( const QString& name )
    {
	m_tag.setName( name );
    }
        
private:
    Tag& m_tag;
    union
    {
	unsigned long flags;
	struct
	{
	    unsigned long access: 3;
	    unsigned long isStatic: 1;
	    unsigned long isFriend: 1;
	} data;
    } m_info;
};

template <class Tag>
class JavaFunction
{
public:
    JavaFunction( Tag& tag )
	: m_tag( tag ) { m_info.flags = tag.flags(); }
    
    operator Tag& () { return asTag(); }
    Tag& asTag() { return m_tag; }
        
    operator const Tag& () const { return asTag(); }
    const Tag& asTag() const { return m_tag; }
    
    static QString format( const Tag& tag ) { return QString::null; }	
    
    int access() const 
    { 
	return m_info.data.access; 
    }
    
    void setAccess( int access )
    { 
	m_info.data.access = access;
	m_tag.setFlags( m_info.flags );
    }
    
    bool isInline() const
    {
	return m_info.data.isInline;
    }
    
    void setInline( bool b )
    {
	m_info.data.isInline = b;
	m_tag.setFlags( m_info.flags );
    }
    
    bool isPure() const
    {
	return m_info.data.isPure;
    }
    
    void setPure( bool b )
    {
	m_info.data.isPure = b;
	m_tag.setFlags( m_info.flags );
    }
    
    bool isFriend() const
    {
	return m_info.data.isFriend;
    }
    
    void setFriend( bool b )
    {
	m_info.data.isFriend = b;
	m_tag.setFlags( m_info.flags );
    }
    
    bool isConst() const
    {
	return m_info.data.isConst;
    }
    
    void setConst( bool b )
    {
	m_info.data.isConst = b;
	m_tag.setFlags( m_info.flags );
    }    
    
    bool isVolatile() const
    {
	return m_info.data.isVolatile;
    }
    
    void setVolatile( bool b )
    {
	m_info.data.isVolatile = b;
	m_tag.setFlags( m_info.flags );
    }
    
    bool isVirtual() const
    {
	return m_info.data.isVirtual;
    }
    
    void setVirtual( bool b )
    {
	m_info.data.isVirtual = b;
	m_tag.setFlags( m_info.flags );
    }
    
    bool isStatic() const
    {
	return m_info.data.isStatic;
    }
    
    void setStatic( bool b )
    {
	m_info.data.isStatic = b;
	m_tag.setFlags( m_info.flags );
    }
    
    
    QString type() const
    {
	return m_tag.attribute( "t" ).toString();
    }
    
    void setType( const QString& type )
    {
	m_tag.setAttribute( "t", type );
    }
    
    QString name() const
    {
	return m_tag.name();
    }
    
    void setName( const QString& name )
    {
	m_tag.setName( name );
    }
    
    QStringList arguments() const
    {
	return m_tag.attribute( "a" ).toStringList();
    }
    
    void setArguments( const QStringList args )
    {
	m_tag.setAttribute( "a", args );
    }
    
    QStringList argumentNames() const
    {
	return m_tag.attribute( "an" ).toStringList();
    }
    
    void setArgumentNames( const QStringList args )
    {
	m_tag.setAttribute( "an", args );
    }
    
private:
    Tag& m_tag;
    union
    {
	unsigned long flags;
	struct
	{
	    unsigned long access: 3;
	    unsigned long isInline: 1;
	    unsigned long isVirtual: 1;
	    unsigned long isStatic: 1;
	    unsigned long isPure: 1;
	    unsigned long isFriend: 1;
	    unsigned long isConst: 1;
	    unsigned long isValile: 1;
	} data;
    } m_info;
};

#endif
