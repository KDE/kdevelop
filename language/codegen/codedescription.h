/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVELOP_CODEDESCRIPTION_H
#define KDEVELOP_CODEDESCRIPTION_H

#include "../languageexport.h"
#include <language/duchain/duchainpointer.h>

#include <grantlee/metatype.h>

#include <QString>
#include <QList>
#include <QAbstractItemModel>

#define GRANTLEE_LOOKUP_PROPERTY(name)      \
if (property == #name) return QVariant::fromValue(object.name);

namespace KDevelop {

/**
 * @brief Represents a variable
 *
 * A variable has two main properties: its type and name.
 * 
 **/
struct VariableDescription
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
    VariableDescription(const DeclarationPointer& declaration);

    /**
     * The name of this variable
     **/
    QString name;
    /**
     * The type of this variable.
     * 
     * In weekly typed languages, this field can be empty. 
     **/
    QString type;
};

/**
 * List of variable descriptions
 **/
typedef QList<VariableDescription> VariableDescriptionList;

/**
 * @brief Represents a function
 * 
 * A function has a name and any number of arguments and return values
 **/
struct FunctionDescription
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
    FunctionDescription(const QString& name, const VariableDescriptionList& arguments, const VariableDescriptionList& returnArguments);
    /**
     * Creates a function and determines its properties from the @p declaration
     *
     * @param declaration a function declaration
     **/
    FunctionDescription(const DeclarationPointer& declaration);

    /**
     * The name of this function
     **/
    QString name;
    /**
     * This function's input arguments
     **/
    QList<VariableDescription> arguments;
    /**
     * This function's return values
     **/
    QList<VariableDescription> returnArguments;

    /**
     * Specifies whether this function is a class constructor
     **/
    bool isConstructor;
    /**
     * Specifies whether this function is a class destructor
     */
    bool isDestructor;
    /**
     * Specifies whether this function is virtual and can be overridden by subclasses
     **/
    bool isVirtual;
    /**
     * Specifies whether this function is static and can be called without a class instance
     **/
    bool isStatic;
    /**
     * Specifies whether this function is a slot
     **/
    bool isSlot;
    /**
     * Specifies whether this function is a signal
     **/
    bool isSignal;
    /**
     * Specifies whether this function is constant
     **/
    bool isConst;

    /**
     * Convenience method, returns the type of the first variable in returnArguments
     * or an empty string if this function has no return arguments
     */
    QString returnType() const;
};

/**
 * List of function descriptions
 **/
typedef QList<FunctionDescription> FunctionDescriptionList;

/**
 * Description of an inheritance relation. 
 **/
struct InheritanceDescription
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
typedef QList<InheritanceDescription> InheritanceDescriptionList;

/**
 * @brief Represents a class
 * 
 * A class descriptions stores its name, its member variables and functions, as well as its superclasses and inheritance types. 
 **/
struct ClassDescription
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
    ClassDescription(const QString& name);

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

/**
 * @brief A data model that represents a single ClassDescription
 * 
 * This model can be used in Qt views. 
 * 
 * It is implemented as a tree. 
 * Every property of the class description (name, inheritance, members, methods) is a top level branch.
 * Their child items are the actual base classes, data members and methods. 
 **/
class ClassDescriptionModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(KDevelop::ClassDescription description READ description WRITE setDescription)

public:
    /**
     * Enumerates the top-level branches of the tree model
     **/
    enum TopLevelRow
    {
        ClassNameRow = 0, ///< The class name
        InheritanceRow, ///< Base classes and respective inheritance types
        MembersRow, ///< Data members
        FunctionsRow, ///< Class functions (methods)
        TopLevelRowCount ///< The number of top-level branches
    };

    /**
     * Creates a new description model and sets its description to @p description
     *
     * @param description the class description displayed by this class
     * @param parent parent object, defaults to 0.
     **/
    explicit ClassDescriptionModel(const ClassDescription& description, QObject* parent = 0);
    /**
     * Creates a new description model with an empty class description
     * 
     * @param parent parent object, defaults to 0.
     **/
    explicit ClassDescriptionModel(QObject* parent = 0);
    /**
     * Destructor
     **/
    virtual ~ClassDescriptionModel();

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    /**
     * @property description
     * 
     * The class description this model represents
     **/
    ClassDescription description() const;
    void setDescription(const ClassDescription& description);

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

    /**
     * Moves the row at position @p source to position @p destination under index @p parent
     *
     * @param source position from where to take the row
     * @param destination position where to put the row
     * @param parent the parent index of both old and new positions
     **/
    void moveRow(int source, int destination, const QModelIndex& parent);

private:
    class ClassDescriptionModelPrivate* const d;
};

}

Q_DECLARE_METATYPE(KDevelop::VariableDescription)
Q_DECLARE_METATYPE(KDevelop::VariableDescriptionList)
Q_DECLARE_METATYPE(KDevelop::FunctionDescription)
Q_DECLARE_METATYPE(KDevelop::FunctionDescriptionList)
Q_DECLARE_METATYPE(KDevelop::InheritanceDescription)
Q_DECLARE_METATYPE(KDevelop::InheritanceDescriptionList)
Q_DECLARE_METATYPE(KDevelop::ClassDescription)

GRANTLEE_BEGIN_LOOKUP(KDevelop::VariableDescription)
    GRANTLEE_LOOKUP_PROPERTY(name)
    GRANTLEE_LOOKUP_PROPERTY(type)
GRANTLEE_END_LOOKUP

GRANTLEE_BEGIN_LOOKUP(KDevelop::FunctionDescription)
    GRANTLEE_LOOKUP_PROPERTY(name)
    GRANTLEE_LOOKUP_PROPERTY(arguments)
    GRANTLEE_LOOKUP_PROPERTY(returnArguments)
    GRANTLEE_LOOKUP_PROPERTY(isConstructor)
    GRANTLEE_LOOKUP_PROPERTY(isDestructor)
    GRANTLEE_LOOKUP_PROPERTY(isVirtual)
    GRANTLEE_LOOKUP_PROPERTY(isStatic)
    if (property == "returnType")
    {
        return object.returnType();
    }
GRANTLEE_END_LOOKUP

GRANTLEE_BEGIN_LOOKUP(KDevelop::InheritanceDescription)
    GRANTLEE_LOOKUP_PROPERTY(inheritanceMode)
    GRANTLEE_LOOKUP_PROPERTY(baseType)
GRANTLEE_END_LOOKUP

GRANTLEE_BEGIN_LOOKUP(KDevelop::ClassDescription)
    GRANTLEE_LOOKUP_PROPERTY(name)
    GRANTLEE_LOOKUP_PROPERTY(baseClasses)
    GRANTLEE_LOOKUP_PROPERTY(members)
    GRANTLEE_LOOKUP_PROPERTY(methods)
GRANTLEE_END_LOOKUP

#endif // KDEVELOP_CODEDESCRIPTION_H
