/* This file is part of KDevelop
    Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>

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

    void wipeout();

    FileList fileList();
    const FileList fileList() const;

    bool hasFile( const QString& name ) const;

    FileDom fileByName( const QString& name );
    const FileDom fileByName( const QString& name ) const;

    bool addFile( FileDom file );
    void removeFile( FileDom file  );

    const NamespaceDom globalNamespace() const;

    virtual void read( QDataStream& stream );
    virtual void write( QDataStream& stream ) const;
    
private:
    void addNamespace( NamespaceDom target, NamespaceDom source );
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

    int kind() const { return m_kind; }
    void setKind( int kind ) { m_kind = kind; }

    QString name() const;
    void setName( const QString& name );

    FileDom file();
    const FileDom file() const;

    QString fileName() const;
    void setFileName( const QString& fileName );

    void getStartPosition( int* line, int* col ) const;
    void setStartPosition( int line, int col );

    void getEndPosition( int* line, int* col ) const;
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
    CodeModel* codeModel() { return m_model; }
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

    QStringList baseClassList() const;
    bool addBaseClass( const QString& baseClass );
    void removeBaseClass( const QString& baseClass );

    ClassList classList();
    const ClassList classList() const;
    bool hasClass( const QString& name ) const;
    ClassList classByName( const QString& name );
    const ClassList classByName( const QString& name ) const;
    bool addClass( ClassDom klass );
    void removeClass( ClassDom klass );

    FunctionList functionList();
    const FunctionList functionList() const;
    bool hasFunction( const QString& name ) const;
    FunctionList functionByName( const QString& name );
    const FunctionList functionByName( const QString& name ) const;
    bool addFunction( FunctionDom fun );
    void removeFunction( FunctionDom fun );

    FunctionDefinitionList functionDefinitionList();
    const FunctionDefinitionList functionDefinitionList() const;
    bool hasFunctionDefinition( const QString& name ) const;
    FunctionDefinitionList functionDefinitionByName( const QString& name );
    const FunctionDefinitionList functionDefinitionByName( const QString& name ) const;
    bool addFunctionDefinition( FunctionDefinitionDom fun );
    void removeFunctionDefinition( FunctionDefinitionDom fun );

    VariableList variableList();
    const VariableList variableList() const;
    bool hasVariable( const QString& name ) const;
    VariableDom variableByName( const QString& name );
    const VariableDom variableByName( const QString& name ) const;
    bool addVariable( VariableDom var );
    void removeVariable( VariableDom var );

    TypeAliasList typeAliasList();
    const TypeAliasList typeAliasList() const;
    bool hasTypeAlias( const QString& name ) const;
    TypeAliasList typeAliasByName( const QString& name );
    const TypeAliasList typeAliasByName( const QString& name ) const;
    bool addTypeAlias( TypeAliasDom typeAlias );
    void removeTypeAlias( TypeAliasDom typeAlias );
    
    EnumList enumList();
    const EnumList enumList() const;
    bool hasEnum( const QString& name ) const;
    EnumDom enumByName( const QString& name );
    const EnumDom enumByName( const QString& name ) const;
    bool addEnum( EnumDom e );
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

    NamespaceList namespaceList();
    const NamespaceList namespaceList() const;
    bool hasNamespace( const QString& name ) const;
    NamespaceDom namespaceByName( const QString& name );
    const NamespaceDom namespaceByName( const QString& name ) const;
    bool addNamespace( NamespaceDom ns );
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

    QString type() const;
    void setType( const QString& type );

    QString defaultValue() const;
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

    QStringList scope() const { return m_scope; }
    void setScope( const QStringList& scope ) { m_scope = scope; }

    int access() const;
    void setAccess( int access );

    bool isSignal() const;
    void setSignal( bool isSignal );

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

    ArgumentList argumentList();
    const ArgumentList argumentList() const;
    bool addArgument( ArgumentDom arg );
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
