/* This file is part of KDevelop
    Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2004 Matt Rogers <mattr@kde.org> 

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

#ifndef _CODEMODEL_H_
#define _CODEMODEL_H_

#include <qmap.h>
#include <qstringlist.h>
#include <ksharedptr.h>

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

typedef KSharedPtr<CodeModelItem> ItemDom;
typedef KSharedPtr<FileModel> FileDom;
typedef KSharedPtr<NamespaceModel> NamespaceDom;
typedef KSharedPtr<ClassModel> ClassDom;
typedef KSharedPtr<FunctionModel> FunctionDom;
typedef KSharedPtr<FunctionDefinitionModel> FunctionDefinitionDom;
typedef KSharedPtr<VariableModel> VariableDom;
typedef KSharedPtr<ArgumentModel> ArgumentDom;
typedef KSharedPtr<EnumModel> EnumDom;
typedef KSharedPtr<TypeAliasModel> TypeAliasDom;
typedef KSharedPtr<EnumeratorModel> EnumeratorDom;

typedef QValueList<ItemDom> ItemList;
typedef QValueList<FileDom> FileList;
typedef QValueList<NamespaceDom> NamespaceList;
typedef QValueList<ClassDom> ClassList;
typedef QValueList<FunctionDom> FunctionList;
typedef QValueList<FunctionDefinitionDom> FunctionDefinitionList;
typedef QValueList<VariableDom> VariableList;
typedef QValueList<ArgumentDom> ArgumentList;
typedef QValueList<EnumDom> EnumList;
typedef QValueList<TypeAliasDom> TypeAliasList;
typedef QValueList<EnumeratorDom> EnumeratorList;

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

template <class Result, class T>
Result model_cast( KSharedPtr<T> x )
{
    Result r( static_cast<T*>(x) );
    return r;
}

template <class Result, class T>
Result model_cast( T* x )
{
    Result r( static_cast<T*>(x) );
    return r;
}

class CodeModel
{
public:
    CodeModel();
    virtual ~CodeModel();

    template <class T> typename T::Ptr create()
    {
	typename T::Ptr ptr( new T(this) );
	return ptr;
    }

    /**
     * Reset the CodeModel
     */
    void wipeout();

    //! Get the list of files in this code model
    //! \return the FileList object that contains the list of files
    FileList fileList();

    //! Get the list of files in this code model
    //! This is a const version for convenience
    //! \return the FileList object that contains the list of files
    const FileList fileList() const;

    //! Check to see if a file is in this code model
    //! \return true if @p name is in the file list
    bool hasFile( const QString& name ) const;

    //! Get the FileDom object for a file
    //! \param name the name of the file to get the FileDom object for
    FileDom fileByName( const QString& name );

    /**
     * Get the FileDom object for a file
     * This is a const version provided for convenience
     * \param name the name of the file to get the FileDom object for
     */
    const FileDom fileByName( const QString& name ) const;

    /**
     * Add a file to the code model
     * \param file the FileDom object to add to the code model
     * \return true if the file was added successfully
     */
    bool addFile( FileDom file );

    //! Remove a file from the code model
    //! \param file the FileDom object to add to the code model
    void removeFile( FileDom file  );

    //! Get the global namespace
    //! \return the NamespaceDom object that represents the global namespace
    const NamespaceDom globalNamespace() const;

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;
    
private:
    /**
     * Add a namespace to the code model
     * \param target the NamespaceDom object that the namespace will be added to
     * \param source the NamespaceDom object that contains the namespace to remove
     */
    void addNamespace( NamespaceDom target, NamespaceDom source );

    /**
     * Remove a namespace from the code model
     * \param target the NamespaceDom object that the namespace will be removed from
     * \param source the NamespaceDom object that contains the namespace to remove
     */
    void removeNamespace( NamespaceDom target, NamespaceDom source );

private:
    QMap<QString, FileDom> m_files;
    NamespaceDom m_globalNamespace;

private:
    CodeModel( const CodeModel& source );
    void operator = ( const CodeModel& source );
    friend class CodeModelItem;
};

class CodeModelItem: public KShared
{
public:
    typedef ItemDom Ptr;

    enum
    {
	File,
	Namespace,
	Class,
	Function,
	Variable,
	Argument,
	FunctionDefinition,
	Enum,
	Enumerator,
	TypeAlias,

	Custom = 1000
    };

    enum
    {
	Public,
	Protected,
	Private
    };

protected:
    CodeModelItem( int kind, CodeModel* model );

public:
    virtual ~CodeModelItem();

    //! Get the kind of item
    int kind() const { return m_kind; }

    //! Set the kind of item
    void setKind( int kind ) { m_kind = kind; }

    //! Get the name of the item
    QString name() const;

    //! Set the name of the item
    void setName( const QString& name );

    //! Get the file of the item
    //! \return the FileDom object for the item
    FileDom file();

    //! Get the file of the item
    //! This is a const version provided for convenience
    //! \return the FileDom object for the item
    const FileDom file() const;

    //! Get the filename of the item
    QString fileName() const;

    //! Set the filename of the item
    void setFileName( const QString& fileName );

    //! Get the start position of the item
    void getStartPosition( int* line, int* col ) const;

    //! Set the start position of the item
    void setStartPosition( int line, int col );

    //! Get the end position of the item
    void getEndPosition( int* line, int* col ) const;

    //! Set the end position of the item
    void setEndPosition( int line, int col );

    virtual bool isFile() const { return false; }
    virtual bool isNamespace() const { return false; }
    virtual bool isClass() const { return false; }
    virtual bool isFunction() const { return false; }
    virtual bool isFunctionDefinition() const { return false; }
    virtual bool isVariable() const { return false; }
    virtual bool isArgument() const { return false; }
    virtual bool isEnum() const { return false; }
    virtual bool isEnumerator() const { return false; }
    virtual bool isTypeAlias() const { return false; }
    virtual bool isCustom() const { return false; }

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

protected:

    //! Get the code model for this item
    CodeModel* codeModel() { return m_model; }

    //! Get the code model for this item
    //! This is a const version provided for convenience
    const CodeModel* codeModel() const { return m_model; }

private:
    int m_kind;
    CodeModel* m_model;
    QString m_name;
    QString m_fileName;
    int m_startLine, m_startColumn;
    int m_endLine, m_endColumn;

private:
    CodeModelItem( const CodeModelItem& source );
    void operator = ( const CodeModelItem& source );
};

class ClassModel: public CodeModelItem
{
protected:
    ClassModel( CodeModel* model );

public:
    typedef ClassDom Ptr;

    virtual bool isClass() const { return true; }

    QStringList scope() const { return m_scope; }
    void setScope( const QStringList& scope ) { m_scope = scope; }

    //! Get the list of base classes
    QStringList baseClassList() const;

    //! Add a base class to the model
    bool addBaseClass( const QString& baseClass );

    //! Remove a base class from the model
    void removeBaseClass( const QString& baseClass );

    //! Get the list of classes in the model
    ClassList classList();

    //! Get the list of classes in the model
    //! This is a const version provided for convenience
    const ClassList classList() const;

    //! Check if the class specified by \p name is in this model
    bool hasClass( const QString& name ) const;

    //! Get a list of classes that match the name given by \p name
    ClassList classByName( const QString& name );

    //! Get a list of classes that match the name given by \p name
    //! This is a const version provided for convenience
    const ClassList classByName( const QString& name ) const;

    //! Add a class to the model
    bool addClass( ClassDom klass );

    //! Remove a class from the model
    void removeClass( ClassDom klass );

    //! Get a list of functions in the model
    FunctionList functionList();

    //! Get a list of functions in the model
    //! This is a const version provided for convenience
    const FunctionList functionList() const;

    //! Check if the function specified by \p name is in the model
    bool hasFunction( const QString& name ) const;

    //! Get a list of functions that match the name given by \p name
    FunctionList functionByName( const QString& name );

    //! Get a list of functions that match the name given by \p name
    //! This is a const version provided for convenience
    const FunctionList functionByName( const QString& name ) const;

    //! Add a function to the class model
    //! \return true if adding the function was successful
    //! \param fun the FunctionDom object to add to the model
    bool addFunction( FunctionDom fun );

    //! Remove a function from the class model
    //! \param fun the FunctionDom object to remove from the model
    void removeFunction( FunctionDom fun );

    //! Get the list of function definitions
    FunctionDefinitionList functionDefinitionList();

    //! Get the list of function definitions
    //! This is a const version provided for convenience
    const FunctionDefinitionList functionDefinitionList() const;

    //! Check if the function definition specified by \p name is in the model
    //! \return true if the function definition was found
    bool hasFunctionDefinition( const QString& name ) const;

    //! Get the list of functions that match the name given by \p name
    //! If there are no matches, then the list returned is empty
    //! \return the FunctionDefinitionList object containing the definitions that match
    FunctionDefinitionList functionDefinitionByName( const QString& name );

    //! Get the list of functions that match the name given by \p name
    //! If there are no matches, then the list returned is empty
    //! This is a const version provided for convenience
    //! \return the FunctionDefinitionList object containing the definitions that match
    const FunctionDefinitionList functionDefinitionByName( const QString& name ) const;

    //! Add a function definition to the model
    //! \param fun the FunctionDefinitionDom object to add to the model
    //! \return true if the addition was successful
    bool addFunctionDefinition( FunctionDefinitionDom fun );

    //! Remove a function definition from the model
    //! \param fun the FunctionDefinitionDom object to remove from the model
    void removeFunctionDefinition( FunctionDefinitionDom fun );

    //! Get the list of variables in the model
    VariableList variableList();

    //! Get the list of variables in the model
    //! This is a const version provided for convenience
    const VariableList variableList() const;

    //! Check if the variable specified by \p name is in the model
    bool hasVariable( const QString& name ) const;

    //! Get the variable specified by \p name
    //! If there are no matches, then the VariableDom object returned is empty
    //! \return a VariableDom object that matches the name specified
    VariableDom variableByName( const QString& name );

    //! Get the variable specified by \p name
    //! If there are no matches, then the VariableDom object returned is empty
    //! \return a VariableDom object that matches the name specified
    const VariableDom variableByName( const QString& name ) const;

    //! Add a variable to the model
    //! \param var the VariableDom object to add to the model
    //! \return true if the addition was successful
    bool addVariable( VariableDom var );

    //! Remove a variable from the model
    //! \param var the VariableDom object to remove from the model
    void removeVariable( VariableDom var );

    //! Get the type alias list for this model
    TypeAliasList typeAliasList();

    //! Get the type alias list for this model
    //! This is a const version provided for convenience
    const TypeAliasList typeAliasList() const;

    //! Check if the type alias specified by \p name is in the model
    bool hasTypeAlias( const QString& name ) const;

    //! Get the list of type aliases that match \p name
    //! If there are no matches, the TypeAliasList object is empty
    //! \return a TypeAliasList object that contains the matches
    TypeAliasList typeAliasByName( const QString& name );

    //! Get the list of type aliases that match \p name
    //! If there are no matches, the TypeAliasList object is empty
    //! \return a TypeAliasList object that contains the matches
    const TypeAliasList typeAliasByName( const QString& name ) const;

    //! Add a type alias to the model
    //! \param typeAlias the TypeAliasDom object to add to the model
    //! \return true if the addition was successful
    bool addTypeAlias( TypeAliasDom typeAlias );

    //! Remove a type alias from the model
    //! \param typeAlias the TypeAliasDom object to remove from the model
    void removeTypeAlias( TypeAliasDom typeAlias );

    //! Get the list of enums in the model
    EnumList enumList();

    //! Get the list of enums in the model
    //! This is a const version provided for convenience
    const EnumList enumList() const;

    //! Check if the enum specified by \p name is in the model
    bool hasEnum( const QString& name ) const;

    //! Get the enum specified by \p name
    //! The EnumDom object returned will be empty if no match is found
    //! \return the EnumDom object that contains the match
    EnumDom enumByName( const QString& name );

    //! Get the enum specified by \p name
    //! The EnumDom object returned will be empty if no match is found
    //! \return the EnumDom object that contains the match
    const EnumDom enumByName( const QString& name ) const;

    //! Add an enum to the model
    //! \param e the EnumDom object to add to the model
    //! \return true if the addition was successful
    bool addEnum( EnumDom e );

    //! Remove an enum from the model
    //! \param e the EnumDom object to remove from the model
    void removeEnum( EnumDom e );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

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

class NamespaceModel: public ClassModel
{
protected:
    NamespaceModel( CodeModel* model );

public:
    typedef NamespaceDom Ptr;

    virtual bool isClass() const { return false; }
    virtual bool isNamespace() const { return true; }

    //! Get the list of namespaces in this model
    NamespaceList namespaceList();

    //! Get the list of namespaces in this model
    //! This is a const version provided for convenience
    const NamespaceList namespaceList() const;

    //! Check if the namespace referenced by \p name is in the model
    bool hasNamespace( const QString& name ) const;

    //! Get the namespace specified by \p name
    //! If there are no matches, then the NamespaceDom object returned is empty
    //! \return the NamespaceDom object that contains the match
    NamespaceDom namespaceByName( const QString& name );

    //! Get the namespace specifed by \p name
    //! If there are no matches, then the NamespaceDom object returned is empty
    //! This is a const version provided for convenience
    //! \return the NamespaceDom object that contains the match
    const NamespaceDom namespaceByName( const QString& name ) const;

    //! Add a namespace to the model
    //! \param ns the NamespaceDom object to add to the model
    bool addNamespace( NamespaceDom ns );

    //! Remove the namespace from the model
    //! \param ns the NamespaceDom object to remove from the model
    void removeNamespace( NamespaceDom ns );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

private:
    QMap<QString, NamespaceDom> m_namespaces;

private:
    NamespaceModel( const NamespaceModel& source );
    void operator = ( const NamespaceModel& source );
    friend class CodeModel;
};

class FileModel: public NamespaceModel
{
protected:
    FileModel( CodeModel* model );

public:
    typedef FileDom Ptr;

    virtual bool isFile() const { return true; }

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

private:
    FileModel( const FileModel& );
    void operator = ( const FileModel& );
    friend class CodeModel;
};

class ArgumentModel: public CodeModelItem
{
protected:
    ArgumentModel( CodeModel* model );

public:
    typedef ArgumentDom Ptr;

    virtual bool isArgument() const { return true; }

    //! Get the type of this argument
    QString type() const;

    //! Set the type of this argument
    void setType( const QString& type );

    //! Get the default value of this argument
    QString defaultValue() const;

    //! Set the default value of this argument
    void setDefaultValue( const QString& defaultValue );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

private:
    QString m_type;
    QString m_defaultValue;

private:
    ArgumentModel( const ArgumentModel& source );
    void operator = ( const ArgumentModel& source );
    friend class CodeModel;
};

class FunctionModel: public CodeModelItem
{
protected:
    FunctionModel( CodeModel* model );

public:
    typedef FunctionDom Ptr;

    virtual bool isFunction() const { return true; }

    //! Get the scope of the function
    QStringList scope() const { return m_scope; }

    //! Set the scope of the function
    void setScope( const QStringList& scope ) { m_scope = scope; }

    /**
     * Get the access level of the function
     * Can be one of the following:
     * \li Public
     * \li Protected
     * \li Private
     */
    int access() const;

    /**
     * Set the access level of the function
     * Can be one of the following:
     * \li Public
     * \li Protected
     * \li Private
     */
    void setAccess( int access );

    //! Check if the function is a Qt signal
    bool isSignal() const;

    //! Set the function to be a virtual function
    void setSignal( bool isSignal );

    //! Check if the function is a Qt slot
    bool isSlot() const;
    void setSlot( bool isSlot );

    bool isVirtual() const;
    void setVirtual( bool isVirtual );

    bool isStatic() const;
    void setStatic( bool isStatic );

    bool isInline() const;
    void setInline( bool isInline );

    bool isConstant() const;
    void setConstant( bool isConstant );

    bool isAbstract() const;
    void setAbstract( bool isAbstract );

    QString resultType() const;
    void setResultType( const QString& type );

    //! Get the list of arguments being passed to the function
    //! If there are no arguments, then the list is empty
    //! \return the ArgumentList object that contains the arguments for this function
    ArgumentList argumentList();

    //! Get the list of arguments being passed to the function
    //! If there are no arguments, then the list is empty
    //! \return the ArgumentList object that contains the arguments for this function
    //! This is a const version provided for convenience
    const ArgumentList argumentList() const;

    //! Add an argument to the function
    //! \param arg the ArgumentDom object to add as an argument to the function
    //! \return true if the addition was successful
    bool addArgument( ArgumentDom arg );

    //! Remove an argument from the function
    //! \param arg the ArgumentDom object to remove from the function
    void removeArgument( ArgumentDom arg );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

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

class FunctionDefinitionModel: public FunctionModel
{
protected:
    FunctionDefinitionModel( CodeModel* model );

public:
    typedef FunctionDefinitionDom Ptr;

    virtual bool isFunctionDefinition() const { return true; }

private:
    FunctionDefinitionModel( const FunctionDefinitionModel& source );
    void operator = ( const FunctionDefinitionModel& source );
    friend class CodeModel;
};


class VariableModel: public CodeModelItem
{
protected:
    VariableModel( CodeModel* model );

public:
    typedef VariableDom Ptr;

    virtual bool isVariable() const { return true; }

    int access() const;
    void setAccess( int access );

    bool isStatic() const;
    void setStatic( bool isStatic );

    QString type() const;
    void setType( const QString& type );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

private:
    int m_access;
    int m_static;
    QString m_type;

private:
    VariableModel( const VariableModel& source );
    void operator = ( const VariableModel& source );
    friend class CodeModel;
};

class EnumModel: public CodeModelItem
{
protected:
    EnumModel( CodeModel* model );

public:
    typedef EnumDom Ptr;

    virtual bool isEnum() const { return true; }

    int access() const;
    void setAccess( int access );
    
    EnumeratorList enumeratorList();
    const EnumeratorList enumeratorList() const;
    void addEnumerator( EnumeratorDom e );
    void removeEnumerator( EnumeratorDom e );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

private:
    int m_access;
    QMap<QString, EnumeratorDom> m_enumerators;

private:
    EnumModel( const EnumModel& source );
    void operator = ( const EnumModel& source );
    friend class CodeModel;
};


class EnumeratorModel: public CodeModelItem
{
protected:
    EnumeratorModel( CodeModel* model );

public:
    typedef EnumeratorDom Ptr;

    virtual bool isEnumerator() const { return true; }
    
    QString value() const;
    void setValue( const QString& value );

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

private:
    QString m_value;
    
private:
    EnumeratorModel( const EnumeratorModel& source );
    void operator = ( const EnumeratorModel& source );
    friend class CodeModel;
};

class TypeAliasModel: public CodeModelItem
{
protected:
    TypeAliasModel( CodeModel* model );

public:
    typedef TypeAliasDom Ptr;

    virtual bool isTypeAlias() const { return true; }
    
    QString type() const;
    void setType( const QString& type );
    
    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;

private:
    QString m_type;
    
private:
    TypeAliasModel( const TypeAliasModel& source );
    void operator = ( const TypeAliasModel& source );
    friend class CodeModel;
};


#endif // _CODEMODEL_H_

