/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "propertyobject.h"
#include "metadatabase.h"
#include <q3ptrvector.h>
#include <q3ptrlist.h>
#include <qmetaobject.h>
#include <qvariant.h>

PropertyObject::PropertyObject( const QWidgetList &objs )
    : QObject(), objects( objs ), mobj( 0 )
{
    Q3PtrVector<Q3PtrList<QMetaObject> > v;
    v.resize( objects.count() );
    v.setAutoDelete( TRUE );

    for ( QObject *o = objects.first(); o; o = objects.next() ) {
	const QMetaObject *m = o->metaObject();
	Q3PtrList<QMetaObject> *mol = new Q3PtrList<QMetaObject>;
	while ( m ) {
	    mol->insert( 0, m );
	    m = m->superClass();
	}
	v.insert( v.count(), mol );
    }

    int numObjects = objects.count();
    int minDepth = v[0]->count();
    int depth = minDepth;

    for ( int i = 0; i < numObjects; ++i ) {
	depth = (int)v[i]->count();
	if ( depth < minDepth )
	    minDepth = depth;
    }

    const QMetaObject *m = v[0]->at( --minDepth );
    
    for ( int j = 0; j < numObjects; ++j ) {
	if ( v[j]->at( minDepth ) != m ) {
	    m = v[0]->at( --minDepth );
	    j = 0;
	}
    }

    mobj = m;
    
    Q_ASSERT( mobj );
}

bool PropertyObject::setProperty( const char *name, const QVariant& value )
{
    for ( QObject *o = objects.first(); o; o = objects.next() )
	o->setProperty( name, value );

    return TRUE;
}

QVariant PropertyObject::property( const char *name ) const
{
    return ( (PropertyObject*)this )->objects.first()->property( name );
}

void PropertyObject::mdPropertyChanged( const QString &property, bool changed )
{
    for ( QObject *o = objects.first(); o; o = objects.next() )
	MetaDataBase::setPropertyChanged( o, property, changed );
}

bool PropertyObject::mdIsPropertyChanged( const QString &property )
{
    for ( QObject *o = objects.first(); o; o = objects.next() ) {
	if ( MetaDataBase::isPropertyChanged( o, property ) )
	    return TRUE;
    }
    return FALSE;
}

void PropertyObject::mdSetPropertyComment( const QString &property, const QString &comment )
{
    for ( QObject *o = objects.first(); o; o = objects.next() )
	MetaDataBase::setPropertyComment( o, property, comment );
}

QString PropertyObject::mdPropertyComment( const QString &property )
{
    return MetaDataBase::propertyComment( objects.first(), property );
}

void PropertyObject::mdSetFakeProperty( const QString &property, const QVariant &value )
{
    for ( QObject *o = objects.first(); o; o = objects.next() )
	MetaDataBase::setFakeProperty( o, property, value );
}

QVariant PropertyObject::mdFakeProperty( const QString &property )
{
    return MetaDataBase::fakeProperty( objects.first(), property );
}

void PropertyObject::mdSetCursor( const QCursor &c )
{
    for ( QObject *o = objects.first(); o; o = objects.next() ) {
	if ( o->isWidgetType() )
	    MetaDataBase::setCursor( (QWidget*)o, c );
    }
}

QCursor PropertyObject::mdCursor()
{
    return MetaDataBase::cursor( objects.first() );
}

void PropertyObject::mdSetPixmapKey( int pixmap, const QString &arg )
{
    for ( QObject *o = objects.first(); o; o = objects.next() )
	MetaDataBase::setPixmapKey( o, pixmap, arg );
}

QString PropertyObject::mdPixmapKey( int pixmap )
{
    return MetaDataBase::pixmapKey( objects.first(), pixmap );
}

void PropertyObject::mdSetExportMacro( const QString &macro )
{
    for ( QObject *o = objects.first(); o; o = objects.next() )
	MetaDataBase::setExportMacro( o, macro );
}

QString PropertyObject::mdExportMacro()
{
    return MetaDataBase::exportMacro( objects.first() );
}
