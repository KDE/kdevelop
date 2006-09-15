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
#ifndef SIMPLETYPECATALOG_H
#define SIMPLETYPECATALOG_H

#include "simpletype.h"

class SimpleTypeCatalog : public SimpleTypeImpl {
  
public:
  
  SimpleTypeCatalog() {
  };
  
  virtual DeclarationInfo getDeclarationInfo();
  
  SimpleTypeCatalog( SimpleTypeCatalog* rhs ) : SimpleTypeImpl( rhs ), m_tag( rhs->m_tag ) {
  };
  
  static CppCodeCompletion* data;
  
  virtual bool isNamespace() const {
    return m_tag.kind() == Tag::Kind_Namespace;
  }
  
    /** empty scope means global scope */
  SimpleTypeCatalog( const QStringList& scope ) : SimpleTypeImpl( scope ) {
    init();
  }
    
  SimpleTypeCatalog( SimpleTypeImpl* rhs ) : SimpleTypeImpl( rhs ) {
    init();
  };
  
  SimpleTypeCatalog( Tag& tag ) {
    m_tag = tag;
    initFromTag();
  }
  
  virtual QString comment() const {
    return m_tag.comment();
  };
  
  virtual TypePointer clone();
  
  virtual Repository rep() const {
    return Catalog;
  }
  
  virtual bool hasNode() const {
    return (bool)m_tag;
  };
  
  virtual QValueList<LocateResult> getBases();
  
  virtual TemplateParamInfo getTemplateParamInfo();
  
  virtual const LocateResult findTemplateParam( const QString& name );
  
private:
  Tag m_tag;
  
  int pointerDepthFromString( const QString& str ) {
    QRegExp ptrRx( "(\\*|\\&)" );
    QString ptr = str.mid( str.find( ptrRx ) );
    QStringList ptrList = QStringList::split( "", ptr );
    return ptrList.size();
  }
  
  Tag findSubTag( const QString& name );
  
  QValueList<Tag> getBaseClassList();
  
  void initFromTag();
  
  void init();
  
protected:
  const Tag& tag() {
    return m_tag;
  }
  
  struct CatalogBuildInfo : public TypeBuildInfo {
    Tag m_tag;
    TypeDesc m_desc;
    TypePointer m_parent;
    
    CatalogBuildInfo( Tag tag , TypeDesc& desc, TypePointer parent ) : m_tag( tag ) , m_desc( desc ), m_parent( parent )   {
    }
    
    virtual TypePointer build();
  };
  
  virtual MemberInfo findMember( TypeDesc name, MemberInfo::MemberType type = MemberInfo::AllTypes);
};




#endif
// kate: indent-mode csands; tab-width 4;

