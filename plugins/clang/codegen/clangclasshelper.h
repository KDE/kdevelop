/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CLANGCLASSHELPER_H
#define CLANGCLASSHELPER_H

#include "clangprivateexport.h"

#include <language/interfaces/icreateclasshelper.h>
#include <language/codegen/templateclassgenerator.h>

class KDEVCLANGPRIVATE_EXPORT ClangClassHelper : public KDevelop::ICreateClassHelper
{
public:
    ClangClassHelper();
    ~ClangClassHelper() override;

    KDevelop::TemplateClassGenerator* createGenerator(const QUrl& baseUrl) override;
    QList<KDevelop::DeclarationPointer> defaultMethods(const QString& name) const override;
};

/**
 * @brief A C++ specific template class generator
 *
 * This class adds additional variables to the template context.
 * @sa extraVariables()
 *
 * Additionally, it attempts to add the class to a target after it is generated.
 */
class ClangTemplateNewClass : public KDevelop::TemplateClassGenerator
{
  public:
    explicit ClangTemplateNewClass(const QUrl& url);
    ~ClangTemplateNewClass() override;

    KDevelop::DocumentChangeSet generate() override;

private:
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
     */
    QVariantHash extraVariables() const;
};

#endif // CLANGCLASSHELPER_H
