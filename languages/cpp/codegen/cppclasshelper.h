/*
 * KDevelop C++ Language Support
 *
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
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

#ifndef CPPCLASSHELPER_H
#define CPPCLASSHELPER_H

#include <language/interfaces/icreateclasshelper.h>
#include <language/codegen/templateclassgenerator.h>
#include <language/codegen/templateclassassistant.h>

#include "cpptypes.h"

namespace KDevelop
{
class ProjectBaseItem;
}

class CppClassHelper : public KDevelop::ICreateClassHelper
{

public:
    CppClassHelper(KDevelop::TemplateClassAssistant* assistant);
    virtual ~CppClassHelper();
    
    virtual KDevelop::ClassGenerator* generator();
    virtual KDevelop::OverridesPage* overridesPage();
    virtual KDevelop::ClassIdentifierPage* identifierPage();
    
private:
    KDevelop::TemplateClassAssistant* m_assistant;
};
class CppTemplateNewClass : public KDevelop::TemplateClassGenerator
{
  public:
    ///Specify the type of object that will be created
    enum Type
    {
      DefaultType,  //!<@todo Have the user configure the default type of container
      Class,
      Struct
    };

    CppTemplateNewClass(KDevelop::ProjectBaseItem* parentItem);
    virtual ~CppTemplateNewClass();
    
    virtual KDevelop::DocumentChangeSet generate();
    virtual QVariantHash templateVariables();

    virtual QList<KDevelop::DeclarationPointer> addBaseClass(const QString &);
    virtual void clearInheritance();
    
    virtual void setIdentifier(const QString& identifier);
    virtual QString identifier() const;

    virtual KDevelop::StructureType::Ptr objectType() const;

    void setType(Type);

  private:
    QStringList m_namespaces;
    QStringList m_baseAccessSpecifiers;
    Type m_type;

    mutable CppClassType::Ptr m_objectType;

    KDevelop::ProjectBaseItem* m_parentItem;
};

#endif // CPPCLASSHELPER_H
