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

#ifndef __TYPEDESC_H__
#define __TYPEDESC_H__

#include <ktexteditor/codecompletioninterface.h>
#include <ksharedptr.h>


#include "typedecoration.h"

class TypeDescShared;
class SimpleTypeImpl;

typedef KSharedPtr<TypeDescShared> TypeDescPointer;
typedef KSharedPtr<SimpleTypeImpl> TypePointer;

///TODO: Das KShared muss weg (??? English, please)
class TypeDesc {
public:
 typedef QValueList<TypeDescPointer> TemplateParams;
 static const char* functionMark;
private:
 QString m_cleanName;
 int m_pointerDepth;
 int m_functionDepth;
 TemplateParams m_templateParams;
 TypeDescPointer m_nextType;
 TypePointer m_resolved;
 TypeDecoration m_dec;

 
 void init( QString stri );
public:
 void takeTemplateParams( const QString& string );
 
 TypeDesc( const QString& name = "" );
 
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
 
 QString fullName( ) const;
 
 QString fullNameChain( ) const ;
 
 int pointerDepth() const {
  return m_pointerDepth;
 }
 
 void setPointerDepth( int d ) {
  m_pointerDepth = d;
 }
 
 void decreasePointerDepth() {
  if( m_pointerDepth > 0 )
   m_pointerDepth--;
 }
 
 QStringList fullNameList( ) const;
 
 QString name() const {
  return m_cleanName;
 };
 
 void setName( QString name ) {
  m_cleanName = name;
 }
 
 /// The template-params may be changed in-place
 /// this list is local, but the params pointed by them not
 TemplateParams& templateParams();
 
 const TemplateParams& templateParams() const;
 /*
 inline operator QString() const {
  return name();
 }*/
 
 ///makes all template-params private so changing them will not affect any other TypeDesc-structures.
 TypeDesc& makePrivate();
 
 operator bool () const {
  return !m_cleanName.isEmpty();
 }
 
 TypeDescPointer next();
 
 bool hasTemplateParams() const ;
 
 void setNext( TypeDescPointer type );
 
 void append( TypeDescPointer type );
 
 TypePointer resolved();
 
 void setResolved( TypePointer resolved );
 
 void resetResolved();
 
 ///Resets the resolved-pointers of this type, and all template-types
 void resetResolvedComplete();
 
 ///these might be changed in future to an own data-member
 void increaseFunctionDepth();
 
 void decreaseFunctionDepth();
 
 int functionDepth() const;
 
 void takeInstanceInfo( const TypeDesc& rhs );
 
 void clearInstanceInfo();
};

class TypeDescShared : public TypeDesc, public KShared {
public:
 
 
 TypeDescShared( const TypeDescShared& rhs ) : TypeDesc(rhs), KShared() {
 }
 
 TypeDescShared( const TypeDesc& rhs ) : TypeDesc(rhs), KShared() {
 }
 
 TypeDescShared& operator = ( const TypeDesc& rhs ) {
  (*(TypeDesc*)this) = rhs;
  return *this;
 } 

 TypeDescShared( const QString& name = "" ) : TypeDesc( name ) {
 }
};

extern TypeDesc operator + ( const TypeDesc& lhs, const TypeDesc& rhs );

#endif
// kate: indent-mode csands; tab-width 4;
