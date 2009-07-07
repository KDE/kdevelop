/*
 * KDevelop C++ Language Support
 *
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef CPP_NEWCLASS_H
#define CPP_NEWCLASS_H

#include <language/codegen/createclass.h>
#include <language/codegen/overridespage.h>
#include "cppduchain/cpptypes.h"

class CppClassIdentifierPage : public KDevelop::ClassIdentifierPage
{
  Q_OBJECT

public:
  CppClassIdentifierPage(QWizard* parent);
  
  virtual KDevelop::QualifiedIdentifier parseParentClassId(const QString& inheritedObject);
};

class CppOverridesPage : public KDevelop::OverridesPage
{
  Q_OBJECT

public:
    CppOverridesPage(QWizard* parent);

    virtual void addPotentialOverride(QTreeWidgetItem* classItem, KDevelop::DeclarationPointer childDeclaration);
};

class CppNewClass : public KDevelop::ClassGenerator
{
  public:
    ///Specify the type of object that will be created
    enum Type
    {
      DefaultType,  //!<@todo Have the user configure the default type of container
      Class,
      Struct
    };
    
    CppNewClass() : m_type(DefaultType), m_objectType(new CppClassType) {};
    virtual ~CppNewClass(void) {};
    
    virtual KDevelop::DocumentChangeSet generate(KUrl url);
    
    virtual const QList<KDevelop::DeclarationPointer> & addBaseClass(const QString &);

    virtual KUrl headerUrlFromBase(KUrl baseUrl);
    virtual KUrl implementationUrlFromBase(KUrl baseUrl);
    
    virtual void identifier(const QString & identifier);
    virtual QString identifier(void) const;
    
    virtual KDevelop::StructureType::Ptr objectType() const;
    
    void setType(Type);

    KDevelop::DocumentChangeSet generateHeader(KUrl url);
    KDevelop::DocumentChangeSet generateImplementation(KUrl headerUrl, KUrl url);
  
  private:
    QStringList m_namespaces;
    QStringList m_baseAccessSpecifiers;
    Type m_type;
    
    mutable CppClassType::Ptr m_objectType;
};

//!@todo  Tag the overrided methods with the name of the parent class
class CppNewClassWizard : public KDevelop::CreateClassWizard
{
  Q_OBJECT

public:
  CppNewClassWizard(QWidget* parent, CppNewClass * generator, KUrl baseUrl = KUrl());

  virtual CppClassIdentifierPage* newIdentifierPage();
  virtual CppOverridesPage* newOverridesPage();

  KUrl implementationUrl() const { return field("implementationUrl").value<KUrl>(); }
  KUrl headerUrl() const { return field("headerUrl").value<KUrl>(); }
private:
  QStringList m_baseClasses;
  KUrl m_url;
};

#endif // CPP_NEWCLASS_H
