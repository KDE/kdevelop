/* This file is part of KDevelop
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2004 Matt Rogers <mattr@kde.org> 
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef CODEMODEL_H
#define CODEMODEL_H

/**
@file codemodel.h
Code Model - a memory symbol store.
*/


#include <qmap.h>
#include <qstringlist.h>
#include <ksharedptr.h>
#include <qvaluevector.h>

#include <iostream>
#include <ostream>
#include <string>
#include <sstream>

using namespace std;

class CodeModel;
class CodeModelItem;
class FileModel;
class NamespaceModel;
class ClassModel;
class FunctionModel;
class FunctionDefinitionModel;
class VariableModel;
class ArgumentModel;
class EnumModel;
class EnumeratorModel;
class TypeAliasModel;


/**@class ItemDom
Safe pointer to the @ref CodeModelItem.
This is a type definition: @code typedef KSharedPtr<CodeModelItem> ItemDom; @endcode
@sa KSharedPtr*/
typedef KSharedPtr<CodeModelItem> ItemDom;
/**@class FileDom
Safe pointer to the @ref FileModel.
This is a type definition: @code typedef KSharedPtr<FileModel> FileDom; @endcode
@sa KSharedPtr*/
typedef KSharedPtr<FileModel> FileDom;
/**@class NamespaceDom
Safe pointer to the @ref NamespaceModel.
This is a type definition: @code typedef KSharedPtr<NamespaceModel> NamespaceDom; @endcode
@sa KSharedPtr*/
typedef KSharedPtr<NamespaceModel> NamespaceDom;
/**@class ClassDom
Safe pointer to the @ref ClassModel.
This is a type definition: @code typedef KSharedPtr<ClassModel> ClassDom; @endcode
@sa KSharedPtr*/
typedef KSharedPtr<ClassModel> ClassDom;
/**@class FunctionDom
Safe pointer to the @ref FunctionModel.
This is a type definition: @code typedef KSharedPtr<FunctionModel> FunctionDom; @endcode
@sa KSharedPtr*/
typedef KSharedPtr<FunctionModel> FunctionDom;
/**@class FunctionDefinitionDom
Safe pointer to the @ref FunctionDefinitionModel.
This is a type definition: @code typedef KSharedPtr<FunctionDefinitionModel> FunctionDefinitionDom; @endcode
@sa KSharedPtr*/
typedef KSharedPtr<FunctionDefinitionModel> FunctionDefinitionDom;
/**@class VariableDom
Safe pointer to the @ref VariableModel.
This is a type definition: @code typedef KSharedPtr<VariableModel> VariableDom; @endcode
@sa KSharedPtr*/
typedef KSharedPtr<VariableModel> VariableDom;
/**@class ArgumentDom
Safe pointer to the @ref ArgumentModel.
This is a type definition: @code typedef KSharedPtr<ArgumentModel> ArgumentDom; @endcode
@sa KSharedPtr*/
typedef KSharedPtr<ArgumentModel> ArgumentDom;
/**@class EnumDom
Safe pointer to the @ref EnumModel.
This is a type definition: @code typedef KSharedPtr<EnumModel> EnumDom; @endcode
@sa KSharedPtr*/
typedef KSharedPtr<EnumModel> EnumDom;
/**@class TypeAliasDom
Safe pointer to the @ref TypeAliasModel.
This is a type definition: @code typedef KSharedPtr<TypeAliasModel> TypeAliasDom; @endcode
@sa KSharedPtr*/
typedef KSharedPtr<TypeAliasModel> TypeAliasDom;
/**@class EnumeratorDom
Safe pointer to the @ref EnumeratorModel.
This is a type definition: @code typedef KSharedPtr<EnumeratorModel> EnumeratorDom; @endcode
@sa KSharedPtr*/
typedef KSharedPtr<EnumeratorModel> EnumeratorDom;

/**@class ItemList
The list of code model items.
This is a type definition: @code typedef QValueList<ItemDom> ItemList; @endcode
@sa QValueList*/
typedef QValueList<ItemDom> ItemList;
/**@class FileList
The list of code model files.
This is a type definition: @code typedef QValueList<FileDom> FileList; @endcode
@sa QValueList*/
typedef QValueList<FileDom> FileList;
/**@class NamespaceList
The list of code model namespaces.
This is a type definition: @code typedef QValueList<NamespaceDom> NamespaceList; @endcode
@sa QValueList*/
typedef QValueList<NamespaceDom> NamespaceList;
/**@class ClassList
The list of code model classes.
This is a type definition: @code typedef QValueList<ClassDom> ClassList; @endcode
@sa QValueList*/
typedef QValueList<ClassDom> ClassList;
/**@class FunctionList
The list of code model functions.
This is a type definition: @code typedef QValueList<FunctionDom> FunctionList; @endcode
@sa QValueList*/
typedef QValueList<FunctionDom> FunctionList;
/**@class FunctionDefinitionList
The list of code model function definitions.
This is a type definition: @code typedef QValueList<FunctionDefinitionDom> FunctionDefinitionList; @endcode
@sa QValueList*/
typedef QValueList<FunctionDefinitionDom> FunctionDefinitionList;
/**@class VariableList
The list of code model variables.
This is a type definition: @code typedef QValueList<VariableDom> VariableList; @endcode
@sa QValueList*/
typedef QValueList<VariableDom> VariableList;
/**@class ArgumentList
The list of code model arguments.
This is a type definition: @code typedef QValueList<ArgumentDom> ArgumentList; @endcode
@sa QValueList*/
typedef QValueList<ArgumentDom> ArgumentList;
/**@class EnumList
The list of code model enums.
This is a type definition: @code typedef QValueList<EnumDom> EnumList; @endcode
@sa QValueList*/
typedef QValueList<EnumDom> EnumList;
/**@class TypeAliasList
The list of code model type aliases.
This is a type definition: @code typedef QValueList<TypeAliasDom> TypeAliasList; @endcode
@sa QValueList*/
typedef QValueList<TypeAliasDom> TypeAliasList;
/**@class EnumeratorList
The list of code model enumerators.
This is a type definition: @code typedef QValueList<EnumeratorDom> EnumeratorList; @endcode
@sa QValueList*/
typedef QValueList<EnumeratorDom> EnumeratorList;

/**Iterates through @p lst and creates sorted list of code model item names.
Can be used, for example, to get the list of classes in the store:
@code
QStringList classList = sortedNameList(codeModel()->globalNamespace()->classList());
@endcode
@param lst The list to iterate.
@return Sorted list of code model item names.*/
template <class ItemList>
QStringList sortedNameList( const ItemList& lst )
{
    QStringList nameList;

    typename ItemList::ConstIterator it = lst.begin();
    while( it != lst.end() ){
        if( !(*it)->name().isEmpty() )
            nameList << (*it)->name();
        ++it;
    }

    nameList.sort();
    return nameList;
}

/**Casts safe code model pointers (@p KSharedPtr<T> objects like
FileDom, NamespaceDom, etc.) to the @p Result type.

Example:
@code
//ns is of type NamespaceDom
ClassDom cl = model_cast<ClassDom>(ns);
@endcode
@param x Object to cast.*/
template <class Result, class T>
Result model_cast( KSharedPtr<T> x )
{
    Result r( static_cast<T*>(x) );
    return r;
}

/**Casts code model pointers (objects like
FileModel, NamespaceModel, etc.) to the @p Result type.

Example:
@code
//ns is of type NamespaceModel*
ClassDom cl = model_cast<ClassDom>(ns);
@endcode
@param x Object to cast.*/
template <class Result, class T>
Result model_cast( T* x )
{
    Result r( static_cast<T*>(x) );
    return r;
}


/**
Code Model - a memory symbol store.
Symbol store (aka class store) is a database of symbols
found in code with the important information about those symbols.

For example, programming language support plugins use symbol store
to remember information about classes, functions, etc. For each type
of symbol a certain information can be stored - symbol name, the
location in source file, etc.

@sa codemodel.h documentation for a list of typedefs and other convenience functions.

@sa codemodel_utils.h documentation for an additional code model utility functions and classes reference.
*/
class CodeModel
{
public:
    /**Constructor.*/
    CodeModel();
    /**Destructor.*/
    virtual ~CodeModel();

    /**Creates a code model item. This should be used to create
    code model items.
    
    For example, to create a class model somewhere in your plugin, use:
    @code
    klass = codeModel()->create<ClassModel>();
    klass->setName("ClassName");
    klass->setFileName("FileName");
    klass->setStartPosition(line, column);
    @endcode
    @return Created code model item.*/
    template <class T> typename T::Ptr create()
    {
        typename T::Ptr ptr( new T(this) );
        return ptr;
    }

    /**Resets the CodeModel.*/
    void wipeout();

    /**Gets the list of files in the store.
    @return The FileList object that contains the list of files.*/
    FileList fileList();

    /**Gets the list of files in the store.
    This is a const version for convenience.
    @return The FileList object that contains the list of files.*/
    const FileList fileList() const;

    /**Checks to see if a file is in the store.
    @return true if @p name is in the file list.*/
    bool hasFile( const QString& name ) const;

    /**Gets the FileDom object for a file.
    @param name The name of the file to get the FileDom object for.*/
    FileDom fileByName( const QString& name );

    /**Gets the FileDom object for a file.
    This is a const version provided for convenience.
    @param name the name of the file to get the FileDom object for.*/
    const FileDom fileByName( const QString& name ) const;

    /**Adds a file to the store.
    @param file The FileDom object to add to the store.
    @return true if the file was added successfully.*/
    bool addFile( FileDom file );

    /**Removes a file from the store.
    @param file the FileDom object to remove from the store.*/
    void removeFile( FileDom file  );

    /**Gets the global namespace
    @return The NamespaceDom object that represents the global namespace.*/
    const NamespaceDom globalNamespace() const;

    /**Reads the model from a stream.
    Use this to save the memory symbol store to a file.
    
    Language support plugins usually save symbols from projects before the project is 
    closed to avoid reparsing when the project is opened next time.
    @param stream Stream to read from.
    @return whether the read succeeded(may fail when the store-format is deprecated).*/
    virtual void read( QDataStream& stream );
    /**Writes the model to a stream.
    Use this to restore the memory symbol store to a file.
    
    Language support plugins usually save symbols from projects before the project is 
    closed to avoid reparsing when the project is opened next time.
    @param stream Stream to write to.*/
    virtual void write( QDataStream& stream ) const;
    
    /** this will dump the whole tree into dot-file-format so it can be inspected, not ready yet*/
    virtual void dump( std::ostream& file, QString Info="" );
    
    /** Merges two groups, by changing the group-ids of the files.
    Returns the id of the new group, or 0 on fail.
    @param g1 first group
    @param g2 second group */
    int mergeGroups( int g1, int g2 );
    
    /** Returns all files within the given group 
    it should be preferred calling FileModel::wholeGroup and 
    FileModel::wholeGroupStrings because those return in constant
    time if they are the only member of the group */
    FileList getGroup( int gid ) const;
    
    FileList getGroup( const FileDom& file) const;
    
    /** Same as above, but returns the names instead of the objects */
    virtual QStringList getGroupStrings( int gid ) const;
    
private:
    /**Adds a namespace to the store.
    @param target The NamespaceDom object that the namespace will be added to.
    @param source The NamespaceDom object that contains the namespace to remove.*/
    void addNamespace( NamespaceDom target, NamespaceDom source );

    /**Removes a namespace from the store.
    @param target The NamespaceDom object that the namespace will be removed from.
    @param source The NamespaceDom object that contains the namespace to remove.*/
    void removeNamespace( NamespaceDom target, NamespaceDom source );

private:
    QMap<QString, FileDom> m_files;
    NamespaceDom m_globalNamespace;

    virtual int newGroupId();
    ///the groups were introduced to represent dependencies between different files.
    ///Files can have slaves that are owned by other files within the same group.
    ///While parsing, whole groups should always be parsed/reparsed together.
    int m_currentGroupId;   ///normally, each file has its own group.
    
private:
    CodeModel( const CodeModel& source );
    void operator = ( const CodeModel& source );
    friend class CodeModelItem;
    friend class FileModel;
};


/**
Item in code model (symbol store).
Item is a symbol in a store. Code model provides several predefined classes
for predefined item types (files, namespaces, classes, functions and function definitions,
variables, arguments, enums and enumerators, type aliases.

Instanses of this class should be created using @ref CodeModel::create method but usually
it is better to create instances of derived classes like ClassModel, NamespaceModel, FileModel, etc.
*/
class CodeModelItem: public KShared
{
public:
    /**A definition of safe pointer to the code model item.*/
    typedef ItemDom Ptr;

    /**A type of a code model item.*/
    enum Kind
    {
        File,                /**<File.*/
        Namespace,           /**<Namespace.*/
        Class,               /**<Class.*/
        Function,            /**<Function or class method.*/
        Variable,            /**<Variable.*/
        Argument,            /**<Function or method parameter.*/
        FunctionDefinition,  /**<Function definition.*/
        Enum,                /**<Enum.*/
        Enumerator,          /**<Enumerator - a member of an Enum (example: @code enum Type { A, B, C} @endcode 
                                Type will be an Enum; A, B and C - Enumerators.*/
        TypeAlias,           /**<Type alias (aka typedef in c++).*/
    
        Custom = 1000        /**<Custom model items should have type greater than 1000*/
    };

    /**An access to the code model item.*/
    enum Access
    {
        Public,        /**<Public.*/
        Protected,     /**<Protected.*/
        Private        /**<Private.*/
    };

protected:
    /**Constructor.
    @param kind The type, see also @ref CodeModelItem::Kind.
    @param model Code model which stores this item.*/
    CodeModelItem( int kind, CodeModel* model );

    
public:
    /**Destructor.*/
    virtual ~CodeModelItem();

    /**@return The type (kind) of item.*/
    int kind() const { return m_kind; }

    /**Sets the type (kind) of item.
    @param kind The type, see also @ref CodeModelItem::Kind.*/
    void setKind( int kind ) { m_kind = kind; }

    /**@return The name of the item.*/
    QString name() const;

    QString comment() const {
        return m_comment;
    }
    
    void setComment( QString comment ) {
        m_comment = comment;
    }
    
    /**Sets the name of the item.
    @param name The name.*/
    void setName( const QString& name );

    /**Gets the file of the item.
    @return The FileDom object for the item.*/
    FileDom file();

    /**Gets the file of the item
    This is a const version provided for convenience.
    @return The FileDom object for the item.*/
    const FileDom file() const;

    /**@return The filename of the item.*/
    QString fileName() const;

    /**Sets the filename of the item.
    @param fileName The file name.*/
    void setFileName( const QString& fileName );

    /**Gets the start position of the item.
    @param line Will be set to the line number of the items start position. Pass 0 if line number is not necessary.
    @param col Will be set to the column number of the items start position. Pass 0 if column number is not necessary.*/
    void getStartPosition( int* line, int* col ) const;

    /**Sets the start position of the item.
    @param line Line number.
    @param col Column number.*/
    void setStartPosition( int line, int col );

    /**Get the end position of the item.
    @param line Will be set to the line number of the items end position. Pass 0 if line number is not necessary.
    @param col Will be set to the column number of the items end position. Pass 0 if column number is not necessary.*/
    void getEndPosition( int* line, int* col ) const;

    /**Set the end position of the item.
    @param line Line number.
    @param col Column number.*/
    void setEndPosition( int line, int col );

    /**@return true if an item is a FileModel.*/
    virtual bool isFile() const { return false; }
    /**@return true if an item is a NamespaceModel.*/
    virtual bool isNamespace() const { return false; }
    /**@return true if an item is a ClassModel.*/
    virtual bool isClass() const { return false; }
    /**@return true if an item is a FunctionModel.*/
    virtual bool isFunction() const { return false; }
    /**@return true if an item is a FileDefinitionModel.*/
    virtual bool isFunctionDefinition() const { return false; }
    /**@return true if an item is a VariableModel.*/
    virtual bool isVariable() const { return false; }
    /**@return true if an item is an ArgumentModel.*/
    virtual bool isArgument() const { return false; }
    /**@return true if an item is a EnumModel.*/
    virtual bool isEnum() const { return false; }
    /**@return true if an item is a EnumeratorModel.*/
    virtual bool isEnumerator() const { return false; }
    /**@return true if an item is a TypeAliasModel.*/
    virtual bool isTypeAlias() const { return false; }
    /**@return true if an item is a custom item.*/
    virtual bool isCustom() const { return false; }
    
    virtual bool isTemplateable() const { return false; }

    /**Reads an item from the stream.
    @param stream The stream to read from.*/
    virtual void read( QDataStream& stream );
    /**Writes an item to the stream.
    @param stream The stream to write to.*/
    virtual void write( QDataStream& stream ) const;

    virtual void dump( std::ostream& file, bool recurse=false, QString Info="" );
    
protected:

    /**@return The code model for this item.*/
    CodeModel* codeModel() { return m_model; }

    /**@note This is a const version provided for convenience.
    @return The code model for this item*/ 
    const CodeModel* codeModel() const { return m_model; }

private:
    int m_kind;
    CodeModel* m_model;
    QString m_name;
    QString m_fileName;
    QString m_comment; ///not stored yet
    int m_startLine, m_startColumn;
    int m_endLine, m_endColumn;

private:
    CodeModelItem( const CodeModelItem& source );
    void operator = ( const CodeModelItem& source );
};



class TemplateModelItem {
    public:
        typedef QPair< QString, QString > ParamPair;
        typedef QValueVector< ParamPair > ParamMap; ///The first is the name, and the second the default-parameter, or "" if there is none.
        
        virtual const ParamMap& getTemplateParams() {
            return m_params;
        }
        
        virtual void addTemplateParam( QString name, QString def = "" ) {
            m_params.push_back( ParamPair( name, def ) );
        }
        
        virtual void clearTemplateParams() {
            m_params.clear();
        }
        
        ///returns -1 if the parameter does not exist
        virtual int findTemplateParam( const QString& name ) const {
            for( unsigned int a = 0; a< m_params.size(); a++) 
                if( m_params[a].first == name ) return a;
            return -1;
       }
        
       const ParamPair getParam( int index ) const {
           return m_params[index];
       }
       
       virtual bool isTemplateable() const  { return true; }
    
       void write(  QDataStream & stream ) const {
           stream << (int)m_params.size();
           for( ParamMap::const_iterator it = m_params.begin(); it != m_params.end(); ++it ) {
               stream << (*it).first;
               stream << (*it).second;
           }
       }
       
       void read(  QDataStream & stream )  {
           int count;
           stream >> count;
           for( int a = 0; a < count; a++ ) {
               ParamPair tmp;
               stream >> tmp.first;
               stream >> tmp.second;
               m_params.push_back( tmp );
           }
       }
    
    protected:
        ParamMap m_params;
};



/**
Class model.
Represents a class in the code model.

Instanses of this class should be created using @ref CodeModel::create method.
*/
class ClassModel: public CodeModelItem, public TemplateModelItem
{
protected:
    /**Constructor.
    @param model Code model which stores this item.*/
    ClassModel( CodeModel* model );

public:
    /**A definition of safe pointer to the class model.*/
    typedef ClassDom Ptr;

    virtual bool isClass() const { return true; }

    /**@return The scope of the class. Scope is a string list composed from names of parent classes and namespaces.*/
    QStringList scope() const { return m_scope; }
    /**Sets the scope of this class.
    @param scope The scope - a list of parent classes and namespaces.*/
    void setScope( const QStringList& scope ) { m_scope = scope; }

    /**@return The list of base class names.*/
    QStringList baseClassList() const;

    /**Adds a base class to the list of base classes.
    @param baseClass The base class name.*/
    bool addBaseClass( const QString& baseClass );

    /**Removes a base class from the list of base classes.
    @param baseClass The base class name.*/
    void removeBaseClass( const QString& baseClass );

    /**@return The list of (sub)classes in this model.*/
    ClassList classList();

    /**@note This is a const version provided for convenience.
    @return The list of (sub)classes in this model.*/
    const ClassList classList() const;

    /**Checks if the class specified by @p name is in this model.
    @param name The name of a class to look for.
    @return true if the model has a class.*/
    bool hasClass( const QString& name ) const;

    /**@param name The name of a class.
    @return A list of classes that match the name given by @p name.*/
    ClassList classByName( const QString& name );

    /**@param name The name of a class.
    @return A list of classes that match the name given by @p name.
    @note This is a const version provided for convenience.*/
    const ClassList classByName( const QString& name ) const;

    /**Adds a class to the model.
    @param klass The class model to add.
    @return true if addition was successful.*/
    bool addClass( ClassDom klass );

    /**Removes a class from the model.
    @param klass The class model to remove.*/    
    void removeClass( ClassDom klass );

    /**@return A list of functions in the model.*/
    FunctionList functionList();

    /**@return A list of functions in the model.
    @note This is a const version provided for convenience.*/
    const FunctionList functionList() const;

    /**Check if the function specified by @p name is in the model.
    @param name The name of a function to look for.
    @return true if the model has a class.*/
    bool hasFunction( const QString& name ) const;

    /**@param name The name of a function to look for.
    @return A list of functions that match the name given by @p name.*/
    FunctionList functionByName( const QString& name );

    /**@param name The name of a function to look for.
    @return A list of functions that match the name given by @p name.
    @note This is a const version provided for convenience.*/
    const FunctionList functionByName( const QString& name ) const;

    /**Adds a function to the class model.
    @param fun The function model to add.
    @return true if addition was successful.*/
    bool addFunction( FunctionDom fun );

    /**Removes a function from the class model.
    @param fun The FunctionDom object to remove from the model.*/
    void removeFunction( FunctionDom fun );

    /**@return The list of function definitions in the model.*/
    FunctionDefinitionList functionDefinitionList();

    /**@return The list of function definitions
    @note This is a const version provided for convenience.*/
    const FunctionDefinitionList functionDefinitionList() const;

    /**Checks if the function definition specified by \p name is in the model.
    @param name The name of a function definition to look for.
    @return true if the function definition was found.*/
    bool hasFunctionDefinition( const QString& name ) const;

    /**Gets the list of functions that match the name given by \p name.
    If there are no matches, then the list returned is empty.
    @param name The name of a function definition to look for.
    @return The FunctionDefinitionList object containing the definitions that match.*/
    FunctionDefinitionList functionDefinitionByName( const QString& name );

    /**Gets the list of functions that match the name given by \p name.
    If there are no matches, then the list returned is empty.
    @param name The name of a function definition to look for.
    @return The FunctionDefinitionList object containing the definitions that match.
    @note This is a const version provided for convenience.*/
    const FunctionDefinitionList functionDefinitionByName( const QString& name ) const;

    /**Adds a function definition to the model.
    @param fun The function fefinition model to add to the model.
    @return true if the addition was successful.*/
    bool addFunctionDefinition( FunctionDefinitionDom fun );

    /**Removes a function definition from the model.
    @param fun The function fefinition model to remove from the model.*/
    void removeFunctionDefinition( FunctionDefinitionDom fun );

    /**@return The list of variables in the model.*/
    VariableList variableList();

    /**@return The list of variables in the model.
    @note This is a const version provided for convenience.*/
    const VariableList variableList() const;

    /**Checks if the variable specified by @p name is in the model.
    @param name The name of a variable.
    @return true if the variable was found.*/
    bool hasVariable( const QString& name ) const;

    /**Gets the variable specified by @p name.
    If there are no matches, then the VariableDom object returned is empty.
    @param name The name of a variable.
    @return A VariableDom object that matches the name specified.*/
    VariableDom variableByName( const QString& name );

    /**Gets the variable specified by @p name.
    If there are no matches, then the VariableDom object returned is empty.
    @param name The name of a variable.
    @return A VariableDom object that matches the name specified.
    @note This is a const version provided for convenience.*/
    const VariableDom variableByName( const QString& name ) const;

    /**Adds a variable to the model.
    @param var The variable model to add to the model.
    @return true if the addition was successful.*/
    bool addVariable( VariableDom var );

    /**Removes a variable from the model.
    @param var The variable model to remove from the model.*/
    void removeVariable( VariableDom var );

    /**@return The type alias list for this model.*/
    TypeAliasList typeAliasList();

    /**@return The type alias list for this model.
    @note This is a const version provided for convenience.*/
    const TypeAliasList typeAliasList() const;

    /**Checks if the type alias specified by @p name is in the model.
    @param name The name of a type alias.
    @return true if the type alias was found.*/
    bool hasTypeAlias( const QString& name ) const;

    /**Gets the list of type aliases that match @p name.
    If there are no matches, the TypeAliasList object is empty.
    @param name The name of a type alias.
    @return A TypeAliasList object that contains the matches.*/
    TypeAliasList typeAliasByName( const QString& name );

    /**Gets the list of type aliases that match @p name.
    If there are no matches, the TypeAliasList object is empty.
    @param name The name of a type alias.
    @return A TypeAliasList object that contains the matches.
    @note This is a const version provided for convenience.*/
    const TypeAliasList typeAliasByName( const QString& name ) const;

    /**Adds a type alias to the model.
    @param typeAlias The type alias model to add to the model.
    @return true if the addition was successful.*/
    bool addTypeAlias( TypeAliasDom typeAlias );

    /**Removes a type alias from the model.
    @param typeAlias The TypeAliasDom object to remove from the model.*/
    void removeTypeAlias( TypeAliasDom typeAlias );

    /**@return The list of enums in the model.*/
    EnumList enumList();

    /**@return The list of enums in the model.
    @note This is a const version provided for convenience.*/
    const EnumList enumList() const;

    /**Checks if the enum specified by @p name is in the model.
    @param name The name of an enum.
    @return true if the enum was found.*/
    bool hasEnum( const QString& name ) const;

    /**Gets the enum specified by @p name.
    The EnumDom object returned will be empty if no match is found.
    @param name The name of an enum.
    @return The EnumDom object that contains the match.*/
    EnumDom enumByName( const QString& name );

    /**Gets the enum specified by @p name.
    The EnumDom object returned will be empty if no match is found.
    @param name The name of an enum.
    @return The EnumDom object that contains the match.*/
    const EnumDom enumByName( const QString& name ) const;

    /**Adds an enum to the model.
    @param e The enum model to add to the model.
    @return true if the addition was successful.*/
    bool addEnum( EnumDom e );

    /**Removes an enum from the model.
    @param e The enum model to remove from the model.*/
    void removeEnum( EnumDom e );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

    virtual void dump( std::ostream& file, bool recurse=false, QString Info="" );
    
private:
    QStringList m_scope;
    QStringList m_baseClassList;
    QMap<QString, ClassList> m_classes;
    QMap<QString, FunctionList> m_functions;
    QMap<QString, FunctionDefinitionList> m_functionDefinitions;
    QMap<QString, VariableDom> m_variables;
    QMap<QString, TypeAliasList> m_typeAliases;
    QMap<QString, EnumDom> m_enumerators;

private:
    ClassModel( const ClassModel& source );
    void operator = ( const ClassModel& source );
    friend class CodeModel;
};


/**
Namespace model.
Represents a namespace in the code model.
Namespace model can represent either usual c++ namespaces
and packages or modules from other languages.

Instanses of this class should be created using @ref CodeModel::create method.
*/
class NamespaceModel: public ClassModel
{
protected:
    /**Constructor.
    @param model Code model which stores this item.*/
    NamespaceModel( CodeModel* model );

public:
    /**A definition of safe pointer to the namespace model.*/
    typedef NamespaceDom Ptr;

    virtual bool isClass() const { return false; }
    virtual bool isNamespace() const { return true; }

    /**@return The list of namespaces in this model.*/
    NamespaceList namespaceList();

    /**@return The list of namespaces in this model.
    @note This is a const version provided for convenience.*/
    const NamespaceList namespaceList() const;

    /**Checks if the namespace referenced by @p name is in the model.
    @param name The name of a namespace.
    @return true if the namespace was found.*/
    bool hasNamespace( const QString& name ) const;

    /**Gets the namespace specified by @p name.
    If there are no matches, then the NamespaceDom object returned is empty.
    @param name The name of a namespace.
    @return The NamespaceDom object that contains the match.*/
    NamespaceDom namespaceByName( const QString& name );

    /**Gets the namespace specified by @p name.
    If there are no matches, then the NamespaceDom object returned is empty.
    @param name The name of a namespace.
    @return The NamespaceDom object that contains the match.
    @note This is a const version provided for convenience.*/
    const NamespaceDom namespaceByName( const QString& name ) const;

    /**Adds a namespace to the model.
    @param ns The namespace model to add to the model.
    @return true if addition was successful.*/
    bool addNamespace( NamespaceDom ns );

    /**Removes the namespace from the model.
    @param ns The namespace model to remove from the model.*/
    void removeNamespace( NamespaceDom ns );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

    virtual void dump( std::ostream& file, bool recurse=false, QString Info="" );
    
private:
    QMap<QString, NamespaceDom> m_namespaces;

private:
    NamespaceModel( const NamespaceModel& source );
    void operator = ( const NamespaceModel& source );
    friend class CodeModel;
};




/**
File model.
Represents a file in the code model.
Files in general contain classes, namespaces, functions,
types, etc. Therefore FileModel is derived from NamespaceModel.

Instanses of this class should be created using @ref CodeModel::create method.
*/
class FileModel: public NamespaceModel
{
protected:
    /**Constructor.
    @param model Code model which stores this item.*/
    FileModel( CodeModel* model );

public:
    /**A definition of safe pointer to the file model.*/
    typedef FileDom Ptr;

    virtual bool isFile() const { return true; }

    virtual int groupId() const {
        return m_groupId;
    }
    
    virtual void setGroupId(int newId) {
        m_groupId = newId;
    }
    
    /** This function additionally does version-checking and
        should be used instead of read when read should be called
        from outside.
    @return whether the read was successful */
    
    virtual void write( QDataStream& stream ) const;

    FileList wholeGroup() ;
    
    QStringList wholeGroupStrings() const;
    
    virtual void read( QDataStream& stream );
private:
    int m_groupId;
    FileModel( const FileModel& );
    void operator = ( const FileModel& );
    friend class CodeModel;
};


/**
Function (procedure) argument model.
Represents an argument in the function.

Instanses of this class should be created using @ref CodeModel::create method.
*/
class ArgumentModel: public CodeModelItem
{
protected:
    ArgumentModel( CodeModel* model );

public:
    /**A definition of safe pointer to the argument model.*/
    typedef ArgumentDom Ptr;

    virtual bool isArgument() const { return true; }

    /**@return The type of this argument.*/
    QString type() const;

    /**Sets the type of this argument.
    @param type The type to set.*/
    void setType( const QString& type );

    /**@return The default value of this argument.*/
    QString defaultValue() const;

    /**Sets the default value of this argument.
    @param defaultValue The default value to set.*/
    void setDefaultValue( const QString& defaultValue );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

    virtual void dump( std::ostream& file, bool recurse=false, QString Info="" );
    
private:
    QString m_type;
    QString m_defaultValue;

private:
    ArgumentModel( const ArgumentModel& source );
    void operator = ( const ArgumentModel& source );
    friend class CodeModel;
};


/**
Function model.
Represents:
- functions;
- procedures;
- class methods;
.
In languages that have separate function declarations and definitions (c++)
this represents only function declarations. @see FunctionDefinitionModel 
for a model of function definitions.

Instanses of this class should be created using @ref CodeModel::create method.
*/
class FunctionModel: public CodeModelItem, public TemplateModelItem
{
protected:
    /**Constructor.
    @param model Code model which stores this item.*/
    FunctionModel( CodeModel* model );

public:
    /**A definition of safe pointer to the function model.*/
    typedef FunctionDom Ptr;

    virtual bool isFunction() const { return true; }

    /**@return The scope of the function. Scope is a string list composed 
    from names of parent functions, classes and namespaces.*/
    QStringList scope() const { return m_scope; }

    /**Sets the scope of the function.
    @param scope The scope to set.*/
    void setScope( const QStringList& scope ) { m_scope = scope; }

    /**@return The access level of the function. Can return either values of type @ref CodeModelItem::Access or 
    other integers if the function has other access level (for example pascal methods can have "published"
    access level).*/
    int access() const;

    /**Sets the access level of the function.
    @param access The access level.*/
    void setAccess( int access );

    /**@return true if the function is a signal.*/
    bool isSignal() const;
    /**Sets the function to be a signal.
    @param isSignal The signal flag.*/
    void setSignal( bool isSignal );

    /**@return true if the function is a slot.*/
    bool isSlot() const;
    /**Sets the function to be a slot.
    @param isSlot The slot flag.*/
    void setSlot( bool isSlot );

    /**@return true if the function is a virtual function.*/
    bool isVirtual() const;
    /**Sets the function to be a virtual function.
    @param isVirtual The virtual flag.*/
    void setVirtual( bool isVirtual );

    /**@return true if the function is a static function.*/
    bool isStatic() const;
    /**Sets the function to be a static function.
    @param isStatic The static flag.*/
    void setStatic( bool isStatic );

    /**@return true if the function is an inline function.*/
    bool isInline() const;
    /**Sets the function to be an inline function.
    @param isInline The inline flag.*/
    void setInline( bool isInline );

    /**@return true if the function is a constant function.*/
    bool isConstant() const;
    /**Sets the function to be a constant function.
    @param isConstant The constant flag.*/
    void setConstant( bool isConstant );

    /**@return true if the function is an abstract function.*/
    bool isAbstract() const;
    /**Sets the function to be an inline function.
    @param isAbstract The abstract flag.*/
    void setAbstract( bool isAbstract );

    /**@return The result type of a function.*/
    QString resultType() const;
    /**Sets the result type of a function.
    @param type The type of a function result.*/
    void setResultType( const QString& type );

    /**Gets the list of arguments being passed to the function.
    If there are no arguments, then the list is empty.
    @return The ArgumentList object that contains the arguments for this function.*/
    ArgumentList argumentList();

    /**Gets the list of arguments being passed to the function.
    If there are no arguments, then the list is empty.
    @return The ArgumentList object that contains the arguments for this function.
    @note This is a const version provided for convenience.*/
    const ArgumentList argumentList() const;

    /**Adds an argument to the function.
    @param arg The argument model to add as an argument to the function.
    @return true if the addition was successful.*/
    bool addArgument( ArgumentDom arg );

    /**Removes an argument from the function.
    @param arg The argument model to remove from the function.*/
    void removeArgument( ArgumentDom arg );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

    virtual void dump( std::ostream& file, bool recurse=false, QString Info="" );
    
private:
    QStringList m_scope;
    int m_access;

    union {
        struct {
            int m_signal : 1;
            int m_slot : 1;
            int m_virtual : 1;
            int m_static : 1;
            int m_inline : 1;
            int m_constant : 1;
            int m_abstract : 1;
        } v;
        int flags;
    } d;

    QString m_resultType;
    ArgumentList m_arguments;

private:
    FunctionModel( const FunctionModel& source );
    void operator = ( const FunctionModel& source );
    friend class CodeModel;
};

/**
Function model.
Represents function definition for languages that have such.

Instanses of this class should be created using @ref CodeModel::create method.
*/
class FunctionDefinitionModel: public FunctionModel
{
protected:
    /**Constructor.
    @param model Code model which stores this item.*/
    FunctionDefinitionModel( CodeModel* model );

public:
    /**A definition of safe pointer to the function definition model.*/
    typedef FunctionDefinitionDom Ptr;

    virtual bool isFunctionDefinition() const { return true; }

private:
    FunctionDefinitionModel( const FunctionDefinitionModel& source );
    void operator = ( const FunctionDefinitionModel& source );
    friend class CodeModel;
};


/**
Variable model.
Represents variables and class attributes.

Instanses of this class should be created using @ref CodeModel::create method.
*/
class VariableModel: public CodeModelItem
{
protected:
    /**Constructor.
    @param model Code model which stores this item.*/
    VariableModel( CodeModel* model );

public:
    /**A definition of safe pointer to the variable model.*/
    typedef VariableDom Ptr;

    virtual bool isVariable() const { return true; }

    /**@return The access level of the variable. Can return either values of type @ref CodeModelItem::Access or 
    other integers if the variable has other access level (for example pascal attributes can have "published"
    access level).*/
    int access() const;
    /**Sets the access level of the variable.
    @param access The access level.*/
    void setAccess( int access );

    /**@return true if the variable is a static variable.*/
    bool isStatic() const;
    /**Sets the variable to be a static variable.
    @param isStatic The static flag.*/
    void setStatic( bool isStatic );

    /**@return A type of the variable.*/
    QString type() const;
    /**Sets the type of the variable.
    @param type The type name.*/
    void setType( const QString& type );

		/**@return If this is an enumerator, the enum it is part of, else an empty string. This is just a hack, necessary because EnumeratorModel is not used at all by the cpp-code-model. */
		bool isEnumeratorVariable() const;

		void setEnumeratorVariable( bool b );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

    virtual void dump( std::ostream& file, bool recurse=false, QString Info="" );
    
private:
    int m_access;
    int m_static;
    QString m_type;
		int m_isEnumeratorVariable;

private:
    VariableModel( const VariableModel& source );
    void operator = ( const VariableModel& source );
    friend class CodeModel;
};


/**
Enum model.
Represents enums.

Instanses of this class should be created using @ref CodeModel::create method.
*/
class EnumModel: public CodeModelItem
{
protected:
    /**Constructor.
    @param model Code model which stores this item.*/
    EnumModel( CodeModel* model );

public:
    /**A definition of safe pointer to the enum model.*/
    typedef EnumDom Ptr;

    virtual bool isEnum() const { return true; }

    /**@return The access level of the enum. Can return either values 
    of type @ref CodeModelItem::Access or other integers if the enum has other access level.*/
    int access() const;
    /**Sets the access level of the enum.
    @param access The access level.*/
    void setAccess( int access );
    
    /**@return The list of enumerators in this enum.*/
    EnumeratorList enumeratorList();
    /**@return The list of enumerators in this enum.
    @note This is a const version provided for convenience.*/
    const EnumeratorList enumeratorList() const;
    /**Adds an enumerator to the model.
    @param e The enumerator model to add.*/
    void addEnumerator( EnumeratorDom e );
    /**Removes an enumerator from the model.
    @param e The enumerator model to remove.*/
    void removeEnumerator( EnumeratorDom e );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

    ///The dump-function is not ready yet
    virtual void dump( std::ostream& file, bool recurse=false, QString Info="" );
    
private:
    int m_access;
    QMap<QString, EnumeratorDom> m_enumerators;

private:
    EnumModel( const EnumModel& source );
    void operator = ( const EnumModel& source );
    friend class CodeModel;
};


/**
Enumerator model.
Represents enumerators. Enums consist of enumerators, for example in code:
@code
enum Type { A, B, C};
@endcode
Type is represented as EnumModel;\n
A, B, C are represented with EnumeratorModel.

Instanses of this class should be created using @ref CodeModel::create method.
*/
class EnumeratorModel: public CodeModelItem
{
protected:
    /**Constructor.
    @param model Code model which stores this item.*/
    EnumeratorModel( CodeModel* model );

public:
    /**A definition of safe pointer to the enumerator model.*/
    typedef EnumeratorDom Ptr;

    virtual bool isEnumerator() const { return true; }
    
    /**@return The value of an enumerator.*/
    QString value() const;
    /**Sets the value of an enumerator.
    @param value The value.*/
    void setValue( const QString& value );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

    virtual void dump( std::ostream& file, bool recurse=false, QString Info="" );
    
private:
    QString m_value;
    
private:
    EnumeratorModel( const EnumeratorModel& source );
    void operator = ( const EnumeratorModel& source );
    friend class CodeModel;
};


/**
Type alias model.
Represents type aliases (like aliases in Ada and typedefs in c++).
*/
class TypeAliasModel: public CodeModelItem
{
protected:
    /**Constructor.
    @param model Code model which stores this item.*/
    TypeAliasModel( CodeModel* model );

public:
    /**A definition of safe pointer to the type alias model.*/
    typedef TypeAliasDom Ptr;

    virtual bool isTypeAlias() const { return true; }
    
    /**@return The actual type of an alias.*/
    QString type() const;
    /**Sets the type of an alias.
    @param type The type name.*/
    void setType( const QString& type );
    
    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

    
    virtual void dump( std::ostream& file, bool recurse=false, QString Info="" );
    
private:
    QString m_type;
    
private:
    TypeAliasModel( const TypeAliasModel& source );
    void operator = ( const TypeAliasModel& source );
    friend class CodeModel;
};

#endif
