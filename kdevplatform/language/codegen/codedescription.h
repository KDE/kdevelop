/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CODEDESCRIPTION_H
#define KDEVPLATFORM_CODEDESCRIPTION_H

#include <language/languageexport.h>
#include <language/duchain/duchainpointer.h>

#include <QString>
#include <QVariant>
#include <QVector>

/**
 * NOTE: changes in this file will quite probably also require changes
 *       in codedescriptionmetatype.h!
 */

namespace KDevelop {
/**
 * @brief Represents a variable
 *
 * A variable has two main properties: its type and name.
 **/
struct KDEVPLATFORMLANGUAGE_EXPORT VariableDescription
{
    /**
     * Creates a variable with no type and no name
     *
     **/
    VariableDescription();
    /**
     * Creates a variable with type @p type and name @p name
     *
     * @param type the type of this variable
     * @param name the name of this variable
     **/
    VariableDescription(const QString& type, const QString& name);
    /**
     * Creates a variable and determines it type and name from the @p declaration
     *
     **/
    explicit VariableDescription(const DeclarationPointer& declaration);

    /**
     * The name of this variable
     **/
    QString name;
    /**
     * The type of this variable.
     *
     * In weakly typed languages, this field can be empty.
     **/
    QString type;
    /**
     * Access specifier, only relevant for class members.
     *
     * Not all languages use these, so it can be left empty.
     **/
    QString access;
    /**
     * The default value of this variable.
     */
    QString value;
};

/**
 * List of variable descriptions
 **/
using VariableDescriptionList = QVector<VariableDescription>;

/**
 * @brief Represents a function
 *
 * A function has a name and any number of arguments and return values
 **/
struct KDEVPLATFORMLANGUAGE_EXPORT FunctionDescription
{
    /**
     * Creates a function with no name and no arguments
     *
     **/
    FunctionDescription();
    /**
     * Creates a function with name @p and specified @p arguments and @p returnArguments
     *
     * @param name the name of the new function
     * @param arguments a list of variables that are passed to this function as arguments
     * @param returnArguments a list of variables that this function returns
     **/
    FunctionDescription(const QString& name,
                        const VariableDescriptionList& arguments,
                        const VariableDescriptionList& returnArguments);
    /**
     * Creates a function and determines its properties from the @p declaration
     *
     * @param declaration a function declaration
     **/
    explicit FunctionDescription(const DeclarationPointer& declaration);

    /**
     * Convenience method, returns the type of the first variable in returnArguments
     * or an empty string if this function has no return arguments
     */
    QString returnType() const;

    /**
     * The name of this function
     **/
    QString name;
    /**
     * This function's input arguments
     **/
    QVector<VariableDescription> arguments;
    /**
     * This function's return values
     **/
    QVector<VariableDescription> returnArguments;
    /**
     * Access specifier, only relevant for class members.
     *
     * Not all languages use these, so it can be left empty.
     **/
    QString access;

    /**
     * Specifies whether this function is a class constructor
     **/
    bool isConstructor : 1;
    /**
     * Specifies whether this function is a class destructor
     */
    bool isDestructor : 1;
    /**
     * Specifies whether this function is virtual and can be overridden by subclasses
     **/
    bool isVirtual : 1;
    /**
     * Specifies whether this function is abstract and needs to be overridden by subclasses
     **/
    bool isAbstract : 1;
    /**
     * Specifies whether this function overrides a virtual method of a base class
     **/
    bool isOverriding : 1;
    /**
     * Specifies whether this function is final and cannot be overridden by subclasses
     **/
    bool isFinal : 1;
    /**
     * Specifies whether this function is static and can be called without a class instance
     **/
    bool isStatic : 1;
    /**
     * Specifies whether this function is a slot
     **/
    bool isSlot : 1;
    /**
     * Specifies whether this function is a signal
     **/
    bool isSignal : 1;
    /**
     * Specifies whether this function is constant
     **/
    bool isConst : 1;
};

/**
 * List of function descriptions
 **/
using FunctionDescriptionList = QVector<FunctionDescription>;

/**
 * Description of an inheritance relation.
 **/
struct KDEVPLATFORMLANGUAGE_EXPORT InheritanceDescription
{
    /**
     * @brief The mode of this inheritance.
     *
     * For C++ classes, mode string are the same as access specifiers (public, protected, private).
     * In other languages, the mode is used to differentiate between extends/implements
     * or other possible inheritance types.
     *
     * Some languages do not recognise distinct inheritance modes at all.
     **/
    QString inheritanceMode;
    /**
     * The name of the base class
     **/
    QString baseType;
};

/**
 * List of inheritance descriptions
 **/
using InheritanceDescriptionList = QVector<InheritanceDescription>;

/**
 * @brief Represents a class
 *
 * A class descriptions stores its name, its member variables and functions, as well as its superclasses and inheritance types.
 **/
struct KDEVPLATFORMLANGUAGE_EXPORT ClassDescription
{
    /**
     * Creates an empty class
     *
     **/
    ClassDescription();
    /**
     * Creates an empty class named @p name
     *
     * @param name the name of the new class
     **/
    explicit ClassDescription(const QString& name);

    /**
     * The name of this class
     **/
    QString name;
    /**
     * List of base classes (classes from which this one inherits) as well as inheritance types
     **/
    InheritanceDescriptionList baseClasses;
    /**
     * List of all member variables in this class
     **/
    VariableDescriptionList members;
    /**
     * List of all member functions (methods) in this class
     **/
    FunctionDescriptionList methods;
};

namespace CodeDescription {
template <class T> QVariant toVariantList(const QVector<T>& list)
{
    QVariantList ret;
    ret.reserve(list.size());
    for (const T& t : list) {
        ret << QVariant::fromValue<T>(t);
    }

    return QVariant::fromValue(ret);
}
}
}

Q_DECLARE_TYPEINFO(KDevelop::VariableDescription, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::FunctionDescription, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::InheritanceDescription, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::ClassDescription, Q_MOVABLE_TYPE);

Q_DECLARE_METATYPE(KDevelop::VariableDescription)
Q_DECLARE_METATYPE(KDevelop::VariableDescriptionList)
Q_DECLARE_METATYPE(KDevelop::FunctionDescription)
Q_DECLARE_METATYPE(KDevelop::FunctionDescriptionList)
Q_DECLARE_METATYPE(KDevelop::InheritanceDescription)
Q_DECLARE_METATYPE(KDevelop::InheritanceDescriptionList)
Q_DECLARE_METATYPE(KDevelop::ClassDescription)

#endif // KDEVPLATFORM_CODEDESCRIPTION_H
