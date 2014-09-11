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

#include <QUrl>

class CppClassHelper : public KDevelop::ICreateClassHelper
{

public:
    CppClassHelper();
    virtual ~CppClassHelper();

    virtual KDevelop::TemplateClassGenerator* createGenerator(const QUrl &baseUrl);
    virtual QList< KDevelop::DeclarationPointer > defaultMethods(const QString& name) const;
};

/**
 * @brief A C++ specific template class generator
 * 
 * This class adds additional variables to the template context. 
 * @sa extraVariables()
 * 
 * Additionally, it attempts to add the class to a target after it is generated. 
 * 
 */
class CppTemplateNewClass : public KDevelop::TemplateClassGenerator
{
  public:
    CppTemplateNewClass(const QUrl &url);
    virtual ~CppTemplateNewClass();

    virtual KDevelop::DocumentChangeSet generate();
    /**
     * In addition to the variables provided by the base class,
     * it groups member variables and functions by access policy. 
     * The variables are of type VariableDescriptionList or FunctionDescriptionList
     * and are called @c private_members, @c public_functions, etc.
     * 
     * Signals and slots are not included in the above variables. Instead, they are listed in
     * @c private_slots, @c protected_slots, @c public_slots and @c signals. 
     * 
     * For convenience, another variable named @c needs_qobject_macro is also provided.
     * It is set to @c true if the class contains at least one signal or slot, and @c false otherwise. 
     * 
     * It also adds a @c namespaces variable which holds a list of all namespaces
     * in which the class is nested. For a class identified by Foo::Bar::ExampleClass,
     * @c namespaces holds two strings: "Foo" and "Bar". 
     *
     */
    QVariantHash extraVariables();

    void addBaseClass(const QString& base);
};

#endif // CPPCLASSHELPER_H
