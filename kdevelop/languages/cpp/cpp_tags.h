//
//
// C++ Interface: cpp_tags
//
// Description:
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CPP_TAGS_H
#define CPP_TAGS_H

#include <qstring.h>
#include <qstringlist.h>

template <class Tag>
class CppBaseClass
{
public:
	CppBaseClass( Tag& tag )
			: m_tag( tag )
	{
		m_info.flags = tag.flags();
	}

	operator Tag& ()
	{
		return asTag();
	}
	Tag& asTag()
	{
		return m_tag;
	}

	operator const Tag& () const
	{
		return asTag();
	}
	const Tag& asTag() const
	{
		return m_tag;
	}

	static QString format( const Tag& tag )
	{
		return QString::null;
	}

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
		unsigned long access:
			3;
		unsigned long isVirtual:
			1;
		}
		data;
	} m_info;
};

template <class Tag>
class CppVariable
{
public:
	CppVariable( Tag& tag )
			: m_tag( tag )
	{
		m_info.flags = tag.flags();
	}

	operator Tag& ()
	{
		return asTag();
	}
	Tag& asTag()
	{
		return m_tag;
	}

	operator const Tag& () const
	{
		return asTag();
	}
	const Tag& asTag() const
	{
		return m_tag;
	}

	static QString format( const Tag& tag )
	{
		return QString::null;
	}

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
		unsigned long access:
			3;
		unsigned long isStatic:
			1;
		unsigned long isFriend:
			1;
		}
		data;
	} m_info;
};

template <class Tag>
class CppFunction
{
public:
	CppFunction( Tag& tag )
			: m_tag( tag )
	{
		m_info.flags = tag.flags();
	}

	operator Tag& ()
	{
		return asTag();
	}
	Tag& asTag()
	{
		return m_tag;
	}

	operator const Tag& () const
	{
		return asTag();
	}
	const Tag& asTag() const
	{
		return m_tag;
	}

	static QString format( const Tag& tag )
	{
		return QString::null;
	}

	int access() const
	{
		return m_info.data.access;
	}

	void setAccess( int access )
	{
		m_info.data.access = access;
		m_tag.setFlags( m_info.flags );
	}

	bool isSignal() const
	{
		return bool( m_info.data.isSignal );
	}

	void setSignal( bool isSignal )
	{
		m_info.data.isSignal = isSignal;
		m_tag.setFlags( m_info.flags );
	}

	bool isSlot() const
	{
		return bool( m_info.data.isSlot );
	}

	void setSlot( bool isSlot )
	{
		m_info.data.isSlot = isSlot;
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
		unsigned long access:
			3;
		unsigned long isInline:
			1;
		unsigned long isVirtual:
			1;
		unsigned long isStatic:
			1;
		unsigned long isPure:
			1;
		unsigned long isFriend:
			1;
		unsigned long isConst:
			1;
		unsigned long isValile:
			1;
		unsigned long isSlot:
			1;
		unsigned long isSignal:
			1;
		}
		data;
	} m_info;
};

#endif 
// kate: indent-mode csands; tab-width 4;

