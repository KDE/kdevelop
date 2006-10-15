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

#include "codemodel.h"
#include <kdebug.h>
#include <kdatastream.h>
#include "driver.h" //Not a nice solution

CodeModel::CodeModel()
{
    wipeout();
    m_currentGroupId = 1;   ///0 stands for invalid group
}

CodeModel::~ CodeModel( )
{
}

int CodeModel::newGroupId() {
    return (m_currentGroupId++) * 2;
}

inline bool isSingleGroup( const int group ) {
    return (group % 2) == 0;
}

QStringList CodeModel::getGroupStrings(int gid) const {
    QStringList ret;
    for(QMap<QString, FileDom>::ConstIterator it = m_files.begin(); it != m_files.end(); ++it) {
        if((*it)->groupId() == gid) ret.append( (*it)-> name() );
    }
    return ret;
}


FileList CodeModel::getGroup(int gid) const {
    FileList ret;
    for(QMap<QString, FileDom>::ConstIterator it = m_files.begin(); it != m_files.end(); ++it) {
        if((*it)->groupId() == gid) ret.append(*it);
    }
    return ret;
}

FileList CodeModel::getGroup( const FileDom& dom) const {
    return getGroup( dom->groupId() );
}

int CodeModel::mergeGroups( int g1, int g2) {
    if( !g1 || !g2 ) return 0;
    if( g1 == g2 ) return g1;
    int ng = isSingleGroup( g1 ) ? g2 : g1;
    if( isSingleGroup( ng ) )
        ng = newGroupId() + 1;
    
    for( QMap<QString, FileDom>::iterator it = m_files.begin(); it != m_files.end(); ++it ) {
        if( (*it)->groupId() == g2 || (*it)->groupId() == g1 ) (*it)->setGroupId( ng );
    }
    return ng;
}

template<class Type> static void dumpMap( std::ostream& file, QMap<QString, Type>& map ) {
    typename QMap<QString, Type>::Iterator it = map.begin();
    for( ; it != map.end(); ++it) {
        typename Type::Iterator it2 = (*it).begin();
        for( ; it2 != (*it).end(); ++it2) {
            (*it2) -> dump( file, true );
        }
    }
}

template<class Type> static void dumpMapDirect( std::ostream& file, QMap<QString, Type>& map ) {
    typename QMap<QString, Type>::Iterator it = map.begin();
    for( ; it != map.end(); ++it) {
        (*it) -> dump( file, true );
    }
}

void CodeModelItem::dump( std::ostream& file, bool recurse, QString Info ) 
{
    ostringstream str( ostringstream::out );
    
    str << "name: " << name().ascii() << "\n";
    str << "kind: " << m_kind << "  ";
    
    if( isFile() ) str << "isFile ";
    if( isNamespace() ) str << "isNamespace ";
    if( isClass() ) str << "isClass ";
    if( isFunction() ) str << "isFunction ";
    if( isFunctionDefinition() ) str << "isFunctionDefinition ";
    if( isVariable() ) str << "isVariable ";
    if( isArgument() ) str << "isArgument ";
    if( isEnum() ) str << "isEnum ";
    if( isEnumerator() ) str << "isEnumerator ";
    if( isTypeAlias() ) str << "isTypeAlias ";
    if( isCustom() ) str << "isCustom ";
    str << "\n";
    str << "File: " << fileName().ascii() << " ";
    int line, col;
    getStartPosition( &line, &col );
    str << "s:(" << line << ", " << col << ") ";
    getEndPosition( &line, &col );
    str << "e:(" << line << ", " << col << ")\n";
    
    
    Info.prepend( str.str().c_str() );
    
    file << Info.ascii() << "\n";
    if(recurse) {} ///just to get rid of the warning
}

void ClassModel::dump( std::ostream& file, bool recurse, QString Info ) 
{
    ostringstream str( ostringstream::out );
    
    
    str << "scope: " << m_scope.join("::").ascii() << "\n";
    str << "bases: " << m_baseClassList.join(" ").ascii() << "\n";
    
    Info.prepend( str.str().c_str() );
    
    CodeModelItem::dump( file, false, Info );
    
    if( recurse ) {
        dumpMap( file, m_classes );
    }
}

void NamespaceAliasModel::read( QDataStream& stream ) {
  QString tempFileName;
  stream >> m_name >> m_aliasName >> tempFileName;
  m_fileName = HashedString( tempFileName );
}

void NamespaceAliasModel::write( QDataStream& stream ) const {
  stream << m_name << m_aliasName << m_fileName.str();
}

void NamespaceImportModel::read( QDataStream& stream ) {
  QString tempFileName;
  stream >> m_name >> tempFileName;
  m_fileName = HashedString( tempFileName );
}

void NamespaceImportModel::write( QDataStream& stream ) const {
  stream << m_name << m_fileName.str();
}

void NamespaceModel::dump( std::ostream& file, bool recurse, QString Info ) 
{
    ostringstream str( ostringstream::out );

    Info.prepend( str.str().c_str() );

    ClassModel::dump( file, false, Info );

    if( recurse ) {
        dumpMapDirect( file, m_namespaces );
    }
}    

void ArgumentModel::dump( std::ostream& file, bool recurse, QString Info ) 
{
    ostringstream str( ostringstream::out );

    str << "type: " << m_type.ascii() << " default: " << m_defaultValue.ascii() << "\n";
    
    Info.prepend( str.str().c_str() );

    CodeModelItem::dump( file, false, Info );
    
    if(recurse) {} ///just to get rid of the warning
}

void FunctionModel::dump( std::ostream& file, bool recurse, QString Info ) 
{
    ostringstream str( ostringstream::out );

    str << "access: " << m_access;
        
    str << " scope: " << m_scope.join("::").ascii() << "\n";
        
    if(isAbstract()) str << "isAbstract ";
    if(isConstant()) str << "isConstant ";
    if(isFunction()) str << "isFunction ";
    if(isInline()) str << "isInline ";
    if(isSignal()) str << "isSignal ";
    if(isSlot()) str << "isSlot ";
    if(isStatic()) str << "isStatic ";
    if(isVirtual()) str << "isVirtual ";
        
    str << "\n";
    str << "result-type: " << resultType().ascii() << "\n";
        
    Info.prepend( str.str().c_str() );

    CodeModelItem::dump( file, false, Info );

    if(recurse) {
        for( ArgumentList::iterator it = m_arguments.begin(); it != m_arguments.end(); ++it) {
            (*it) -> dump( file, true );
        }
    }
}

void VariableModel::dump( std::ostream& file, bool recurse, QString Info ) 
{
    ostringstream str( ostringstream::out );

    str << "access: " << m_access << "type: " << m_type.ascii() << "\n";
    
    if(isStatic()) str << "isStatic ";

    str << "\n";

    Info.prepend( str.str().c_str() );

    CodeModelItem::dump( file, false, Info );
    
    if(recurse) {} ///just to get rid of the warning
}

void CodeModel::dump( std::ostream& file, QString Info ) {
    ostringstream str(ostringstream::out);
            
    Info.prepend( str.str().c_str() );
    
    file << Info.ascii() << "\n";
    
    QMap<QString, FileDom>::iterator it = m_files.begin();
    for(; it != m_files.end(); ++it) {
        (*it) -> dump( file, true );
    }
}

void EnumModel::dump( std::ostream& file, bool recurse, QString Info ) 
{
    ostringstream str( ostringstream::out );

    str << "access: " << m_access << "\n";
    
    Info.prepend( str.str().c_str() );

    CodeModelItem::dump( file, false, Info );
    
    if( recurse ) {
        dumpMapDirect( file, m_enumerators );
    }
}

void EnumeratorModel::dump( std::ostream& file, bool recurse, QString Info ) 
{
    ostringstream str( ostringstream::out );

    str << "value: " << m_value.ascii() << "\n";
    
    Info.prepend( str.str().c_str() );

    CodeModelItem::dump( file, false, Info );
    
    if(recurse) {} ///just to get rid of the warning
}

void TypeAliasModel::dump( std::ostream& file, bool recurse, QString Info ) {
    ostringstream str( ostringstream::out );

    str << "type: " << m_type.ascii() << "\n";
    
    Info.prepend( str.str().c_str() );

    CodeModelItem::dump( file, false, Info );
    
    if(recurse) {} ///just to get rid of the warning
}

void CodeModel::wipeout()
{
    m_files.clear();
    NamespaceDom ns = create<NamespaceModel>();
    ns->setName( "::" );

    m_globalNamespace = ns;
}

FileList CodeModel::fileList( )
{
    return m_files.values();
}

const FileList CodeModel::fileList( ) const
{
    return m_files.values();
}

bool CodeModel::hasFile( const QString & name ) const
{
    return m_files.contains( name );
}

FileDom CodeModel::fileByName( const QString & name )
{
    return m_files.contains(name) ? m_files[ name ] : FileDom();
}

const FileDom CodeModel::fileByName( const QString & name ) const
{
    return m_files.contains(name) ? m_files[ name ] : FileDom();
}

void CodeModel::addNamespace( NamespaceDom target, NamespaceDom source )
{
    if( source->name().isEmpty() ){
	return;
    } else if( !target->hasNamespace(source->name()) ){
	NamespaceDom ns = this->create<NamespaceModel>();
	ns->setName( source->name() );
	ns->setFileName( source->fileName() ); /// \FIXME ROBE
	ns->setScope( source->scope() );
	target->addNamespace( ns );
    }

    NamespaceDom ns = target->namespaceByName( source->name() );

    NamespaceList namespaceList = source->namespaceList();
    ClassList classList = source->classList();
    FunctionList functionList = source->functionList();
    FunctionDefinitionList functionDefinitionList = source->functionDefinitionList();
    VariableList variableList = source->variableList();
    EnumList enumList = source->enumList();
    TypeAliasList typeAliasList = source->typeAliasList();
    const NamespaceModel::NamespaceAliasModelList& namespaceAliases = source->namespaceAliases();
    const NamespaceModel::NamespaceImportModelList& namespaceImports = source->namespaceImports();

    for( NamespaceList::Iterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	addNamespace( ns, *it );
    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	ns->addClass( *it );
    for( FunctionList::Iterator it=functionList.begin(); it!=functionList.end(); ++it )
	ns->addFunction( *it );
    for( FunctionDefinitionList::Iterator it=functionDefinitionList.begin(); it!=functionDefinitionList.end(); ++it )
	ns->addFunctionDefinition( *it );
    for( VariableList::Iterator it=variableList.begin(); it!=variableList.end(); ++it )
	ns->addVariable( *it );
    for( EnumList::Iterator it=enumList.begin(); it!=enumList.end(); ++it )
	ns->addEnum( *it );
    for( TypeAliasList::Iterator it=typeAliasList.begin(); it!=typeAliasList.end(); ++it )
	ns->addTypeAlias( *it );
  for( NamespaceModel::NamespaceAliasModelList::const_iterator it=namespaceAliases.begin(); it != namespaceAliases.end(); ++it )
    ns->addNamespaceAlias( *it );
  for( NamespaceModel::NamespaceImportModelList::const_iterator it=namespaceImports.begin(); it != namespaceImports.end(); ++it )
    ns->addNamespaceImport( *it );
}

void CodeModel::removeNamespace( NamespaceDom target, NamespaceDom source )
{
    if( source->name().isEmpty() || !target->hasNamespace(source->name()) )
	return;

    NamespaceDom ns = target->namespaceByName( source->name() );

    NamespaceList namespaceList = source->namespaceList();
    ClassList classList = source->classList();
    FunctionList functionList = source->functionList();
    FunctionDefinitionList functionDefinitionList = source->functionDefinitionList();
    VariableList variableList = source->variableList();
    EnumList enumList = source->enumList();
    TypeAliasList typeAliasList = source->typeAliasList();
    const NamespaceModel::NamespaceAliasModelList& namespaceAliases = source->namespaceAliases();
    const NamespaceModel::NamespaceImportModelList& namespaceImports = source->namespaceImports();
  
    for( NamespaceList::Iterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	removeNamespace( ns, *it );
    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	ns->removeClass( *it );
    for( FunctionList::Iterator it=functionList.begin(); it!=functionList.end(); ++it )
	ns->removeFunction( *it );
    for( FunctionDefinitionList::Iterator it=functionDefinitionList.begin(); it!=functionDefinitionList.end(); ++it )
	ns->removeFunctionDefinition( *it );
    for( VariableList::Iterator it=variableList.begin(); it!=variableList.end(); ++it )
	ns->removeVariable( *it );
    for( EnumList::Iterator it=enumList.begin(); it!=enumList.end(); ++it )
	ns->removeEnum( *it );
    for( TypeAliasList::Iterator it=typeAliasList.begin(); it!=typeAliasList.end(); ++it )
	ns->removeTypeAlias( *it );
  for( NamespaceModel::NamespaceAliasModelList::const_iterator it=namespaceAliases.begin(); it != namespaceAliases.end(); ++it )
    ns->removeNamespaceAlias( *it );
  for( NamespaceModel::NamespaceImportModelList::const_iterator it=namespaceImports.begin(); it != namespaceImports.end(); ++it )
    ns->removeNamespaceImport( *it );
  
    if( ns->namespaceList().isEmpty() && 
    	ns->classList().isEmpty() && 
	ns->functionList().isEmpty() && 
	ns->functionDefinitionList().isEmpty() && 
	ns->variableList().isEmpty() &&
	ns->enumList().isEmpty() && 
	ns->typeAliasList().isEmpty() &&
  ns->namespaceImports().empty() &&
  ns->namespaceAliases().empty() )
    {
        target->removeNamespace( ns );
    }
}

bool CodeModel::addFile( FileDom file )
{
    if( file->name().isEmpty() )
		return false;

		if( m_files.find( file->name() ) != m_files.end() ) {
			///the error-channel is set to 9007 because this problem appears with the cpp-support, so it is needed while debugging it
			kdDebug(9007) << "file " << file->name() << " was added to code-model without removing it before! \n" << kdBacktrace() << endl;
			removeFile( fileByName( file->name() ) );
		}

    // update global namespace
    NamespaceList namespaceList = file->namespaceList();
    ClassList classList = file->classList();
    FunctionList functionList = file->functionList();
    FunctionDefinitionList functionDefinitionList = file->functionDefinitionList();
    VariableList variableList = file->variableList();
    EnumList enumList = file->enumList();
    TypeAliasList typeAliasList = file->typeAliasList();
    const NamespaceModel::NamespaceAliasModelList& namespaceAliases = file->namespaceAliases();
    const NamespaceModel::NamespaceImportModelList& namespaceImports = file->namespaceImports();
  
    for( NamespaceList::Iterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	addNamespace( m_globalNamespace, *it );
    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	m_globalNamespace->addClass( *it );
    for( FunctionList::Iterator it=functionList.begin(); it!=functionList.end(); ++it )
	m_globalNamespace->addFunction( *it );
    for( FunctionDefinitionList::Iterator it=functionDefinitionList.begin(); it!=functionDefinitionList.end(); ++it )
	m_globalNamespace->addFunctionDefinition( *it );
    for( VariableList::Iterator it=variableList.begin(); it!=variableList.end(); ++it )
	m_globalNamespace->addVariable( *it );
    for( EnumList::Iterator it=enumList.begin(); it!=enumList.end(); ++it )
	m_globalNamespace->addEnum( *it );
    for( TypeAliasList::Iterator it=typeAliasList.begin(); it!=typeAliasList.end(); ++it )
	m_globalNamespace->addTypeAlias( *it );
  for( NamespaceModel::NamespaceAliasModelList::const_iterator it=namespaceAliases.begin(); it != namespaceAliases.end(); ++it )
    m_globalNamespace->addNamespaceAlias( *it );
  for( NamespaceModel::NamespaceImportModelList::const_iterator it=namespaceImports.begin(); it != namespaceImports.end(); ++it )
    m_globalNamespace->addNamespaceImport( *it );
  
    m_files.insert( file->name(), file );
    return true;
}

void CodeModel::removeFile( FileDom file )
{
    // update global namespace
    NamespaceList namespaceList = file->namespaceList();
    ClassList classList = file->classList();
    FunctionList functionList = file->functionList();
    FunctionDefinitionList functionDefinitionList = file->functionDefinitionList();
    VariableList variableList = file->variableList();
    EnumList enumList = file->enumList();
    TypeAliasList typeAliasList = file->typeAliasList();
    const NamespaceModel::NamespaceAliasModelList& namespaceAliases = file->namespaceAliases();
    const NamespaceModel::NamespaceImportModelList& namespaceImports = file->namespaceImports();
  
    for( NamespaceList::Iterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	removeNamespace( m_globalNamespace, *it );
    for( ClassList::Iterator it=classList.begin(); it!=classList.end(); ++it )
	m_globalNamespace->removeClass( *it );
    for( FunctionList::Iterator it=functionList.begin(); it!=functionList.end(); ++it )
	m_globalNamespace->removeFunction( *it );
    for( FunctionDefinitionList::Iterator it=functionDefinitionList.begin(); it!=functionDefinitionList.end(); ++it )
	m_globalNamespace->removeFunctionDefinition( *it );
    for( VariableList::Iterator it=variableList.begin(); it!=variableList.end(); ++it )
	m_globalNamespace->removeVariable( *it );
    for( EnumList::Iterator it=enumList.begin(); it!=enumList.end(); ++it )
	m_globalNamespace->removeEnum( *it );
    for( TypeAliasList::Iterator it=typeAliasList.begin(); it!=typeAliasList.end(); ++it )
	m_globalNamespace->removeTypeAlias( *it );
  for( NamespaceModel::NamespaceAliasModelList::const_iterator it=namespaceAliases.begin(); it != namespaceAliases.end(); ++it )
    m_globalNamespace->removeNamespaceAlias( *it );
  for( NamespaceModel::NamespaceImportModelList::const_iterator it=namespaceImports.begin(); it != namespaceImports.end(); ++it )
    m_globalNamespace->removeNamespaceImport( *it );
  
    m_files.remove( file->name() );
}

// ------------------------------------------------------------------------
CodeModelItem::CodeModelItem( int kind, CodeModel* model )
    : m_kind( kind ), m_model( model )
{
    //kdDebug() << "CodeModelItem::CodeModelItem()" << endl;
    m_startLine = 0;
    m_startColumn = 0;
    m_endLine = 0;
    m_endColumn = 0;
}

CodeModelItem::~ CodeModelItem( )
{
}

QString CodeModelItem::name( ) const
{
    return m_name;
}

void CodeModelItem::setName( const QString & name )
{
    m_name = name;
}

const FileDom CodeModelItem::file( ) const
{
    return m_model->fileByName( m_fileName );
}

FileDom CodeModelItem::file( )
{
    return m_model->fileByName( m_fileName );
}

QString CodeModelItem::fileName() const
{
    return m_fileName;
}

void CodeModelItem::setFileName( const QString& fileName )
{
    m_fileName = fileName;
}

void CodeModelItem::getStartPosition( int * line, int * column ) const
{
    if( line ) *line = m_startLine;
    if( column ) *column = m_startColumn;
}

void CodeModelItem::setStartPosition( int line, int column )
{
    m_startLine = line;
    m_startColumn = column;
}

void CodeModelItem::getEndPosition( int * line, int * column ) const
{
    if( line ) *line = m_endLine;
    if( column ) *column = m_endColumn;
}

void CodeModelItem::setEndPosition( int line, int column )
{
    m_endLine = line;
    m_endColumn = column;
}

// ------------------------------------------------------------------------
NamespaceModel::NamespaceModel( CodeModel* model )
    : ClassModel( model )
{
    setKind( Namespace );
}

NamespaceList NamespaceModel::namespaceList( )
{
    return m_namespaces.values();
}

const NamespaceList NamespaceModel::namespaceList( ) const
{
    return m_namespaces.values();
}

NamespaceDom NamespaceModel::namespaceByName( const QString & name )
{
    return m_namespaces.contains( name ) ? m_namespaces[ name ] : NamespaceDom();
}

const NamespaceDom NamespaceModel::namespaceByName( const QString & name ) const
{
    return m_namespaces.contains( name ) ? m_namespaces[ name ] : NamespaceDom();
}

bool NamespaceModel::hasNamespace( const QString & name ) const
{
    return m_namespaces.contains( name );
}

bool NamespaceModel::addNamespace( NamespaceDom ns )
{
    if( ns->name().isEmpty() )
	return false;

    m_namespaces[ ns->name() ] = ns;
    return true;
}

void NamespaceModel::removeNamespace( NamespaceDom ns )
{
    m_namespaces.remove( ns->name() );
}

// ------------------------------------------------------------------------
FileModel::FileModel( CodeModel* model )
    : NamespaceModel( model ), m_groupId( model->newGroupId() ), m_parseResult( 0 )
{
}

// ------------------------------------------------------------------------
ClassModel::ClassModel( CodeModel* model )
    : CodeModelItem( Class, model)
{
}

QStringList ClassModel::baseClassList( ) const
{
    return m_baseClassList;
}

bool ClassModel::addBaseClass( const QString & baseClass )
{
    m_baseClassList.push_back( baseClass );
    return true;
}

void ClassModel::removeBaseClass( const QString & baseClass )
{
    m_baseClassList.remove( baseClass );
}

ClassList ClassModel::classList( )
{
    ClassList l;
    QMap<QString, ClassList>::Iterator it = m_classes.begin();
    while( it != m_classes.end() ){
	l += *it;
	++it;
    }

    return l;
}

const ClassList ClassModel::classList( ) const
{
    ClassList l;
    QMap<QString, ClassList>::ConstIterator it = m_classes.begin();
    while( it != m_classes.end() ){
	l += *it;
	++it;
    }

    return l;
}

bool ClassModel::hasClass( const QString & name ) const
{
    return m_classes.contains( name );
}

ClassList ClassModel::classByName( const QString & name )
{
    return m_classes.contains( name ) ? m_classes[ name ] : ClassList();
}

const ClassList ClassModel::classByName( const QString & name ) const
{
    return m_classes.contains( name ) ? m_classes[ name ] : ClassList();
}

bool ClassModel::addClass( ClassDom klass )
{
    if( klass->name().isEmpty() )
	return false;

    m_classes[ klass->name() ].push_back( klass );
    return true;
}

void ClassModel::removeClass( ClassDom klass )
{
    m_classes[ klass->name() ].remove( klass );

    if( m_classes[klass->name()].isEmpty() )
	m_classes.remove( klass->name() );
}

FunctionList ClassModel::functionList( )
{
    FunctionList l;
    QMap<QString, FunctionList>::Iterator it = m_functions.begin();
    while( it != m_functions.end() ){
	l += *it;
	++it;
    }

    return l;
}

const FunctionList ClassModel::functionList( ) const
{
    FunctionList l;
    QMap<QString, FunctionList>::ConstIterator it = m_functions.begin();
    while( it != m_functions.end() ){
	l += *it;
	++it;
    }

    return l;
}

bool ClassModel::hasFunction( const QString & name ) const
{
    return m_functions.contains( name );
}

FunctionList ClassModel::functionByName( const QString & name )
{
    return m_functions.contains( name ) ? m_functions[ name ] : FunctionList();
}

const FunctionList ClassModel::functionByName( const QString & name ) const
{
    return m_functions.contains( name ) ? m_functions[ name ] : FunctionList();
}

bool ClassModel::addFunction( FunctionDom fun )
{
    if( fun->name().isEmpty() )
	return false;

    m_functions[ fun->name() ].push_back( fun );
    return true;
}

void ClassModel::removeFunction( FunctionDom fun )
{
    m_functions[ fun->name() ].remove( fun );

    if( m_functions[fun->name()].isEmpty() )
	m_functions.remove( fun->name() );
}

FunctionDefinitionList ClassModel::functionDefinitionList( )
{
    FunctionDefinitionList l;
    QMap<QString, FunctionDefinitionList>::Iterator it = m_functionDefinitions.begin();
    while( it != m_functionDefinitions.end() ){
	l += *it;
	++it;
    }

    return l;
}

const FunctionDefinitionList ClassModel::functionDefinitionList( ) const
{
    FunctionDefinitionList l;
    QMap<QString, FunctionDefinitionList>::ConstIterator it = m_functionDefinitions.begin();
    while( it != m_functionDefinitions.end() ){
	l += *it;
	++it;
    }

    return l;
}

bool ClassModel::hasFunctionDefinition( const QString & name ) const
{
    return m_functionDefinitions.contains( name );
}

FunctionDefinitionList ClassModel::functionDefinitionByName( const QString & name )
{
    return m_functionDefinitions.contains( name ) ? m_functionDefinitions[ name ] : FunctionDefinitionList();
}

const FunctionDefinitionList ClassModel::functionDefinitionByName( const QString & name ) const
{
    return m_functionDefinitions.contains( name ) ? m_functionDefinitions[ name ] : FunctionDefinitionList();
}

bool ClassModel::addFunctionDefinition( FunctionDefinitionDom fun )
{
    if( fun->name().isEmpty() )
	return false;

    m_functionDefinitions[ fun->name() ].push_back( fun );
    return true;
}

void ClassModel::removeFunctionDefinition( FunctionDefinitionDom fun )
{
    m_functionDefinitions[ fun->name() ].remove( fun );

    if( m_functionDefinitions[fun->name()].isEmpty() )
	m_functionDefinitions.remove( fun->name() );
}

VariableList ClassModel::variableList( )
{
    return m_variables.values();
}

const VariableList ClassModel::variableList( ) const
{
    return m_variables.values();
}

VariableDom ClassModel::variableByName( const QString & name )
{
    return m_variables.contains( name ) ? m_variables[ name ] : VariableDom();
}

const VariableDom ClassModel::variableByName( const QString & name ) const
{
    return m_variables.contains( name ) ? m_variables[ name ] : VariableDom();
}

bool ClassModel::hasVariable( const QString & name ) const
{
    return m_variables.contains( name );
}

bool ClassModel::addVariable( VariableDom var )
{
    if( var->name().isEmpty() )
	return false;

    m_variables.insert( var->name(), var );
    return true;
}

void ClassModel::removeVariable( VariableDom var )
{
    m_variables.remove( var->name() );
}

EnumList ClassModel::enumList( )
{
    return m_enumerators.values();
}

const EnumList ClassModel::enumList( ) const
{
    return m_enumerators.values();
}

EnumDom ClassModel::enumByName( const QString & name )
{
    return m_enumerators.contains( name ) ? m_enumerators[ name ] : EnumDom();
}

const EnumDom ClassModel::enumByName( const QString & name ) const
{
    return m_enumerators.contains( name ) ? m_enumerators[ name ] : EnumDom();
}

bool ClassModel::hasEnum( const QString & name ) const
{
    return m_enumerators.contains( name );
}

bool ClassModel::addEnum( EnumDom e )
{
    if( e->name().isEmpty() )
	return false;

    m_enumerators.insert( e->name(), e );
    return true;
}

void ClassModel::removeEnum( EnumDom e )
{
    m_enumerators.remove( e->name() );
}

TypeAliasList ClassModel::typeAliasList( )
{
    TypeAliasList l;
    QMap<QString, TypeAliasList>::Iterator it = m_typeAliases.begin();
    while( it != m_typeAliases.end() ){
	l += *it;
	++it;
    }

    return l;
}

const TypeAliasList ClassModel::typeAliasList( ) const
{
    TypeAliasList l;
    QMap<QString, TypeAliasList>::ConstIterator it = m_typeAliases.begin();
    while( it != m_typeAliases.end() ){
	l += *it;
	++it;
    }

    return l;
}

bool ClassModel::hasTypeAlias( const QString & name ) const
{
    return m_typeAliases.contains( name );
}

TypeAliasList ClassModel::typeAliasByName( const QString & name )
{
    return m_typeAliases.contains( name ) ? m_typeAliases[ name ] : TypeAliasList();
}

const TypeAliasList ClassModel::typeAliasByName( const QString & name ) const
{
    return m_typeAliases.contains( name ) ? m_typeAliases[ name ] : TypeAliasList();
}

bool ClassModel::addTypeAlias( TypeAliasDom typeAlias )
{
    if( typeAlias->name().isEmpty() )
	return false;

    m_typeAliases[ typeAlias->name() ].push_back( typeAlias );
    return true;
}

void ClassModel::removeTypeAlias( TypeAliasDom typeAlias )
{
    m_typeAliases[ typeAlias->name() ].remove( typeAlias );

    if( m_typeAliases[typeAlias->name()].isEmpty() )
	m_typeAliases.remove( typeAlias->name() );
}



// ------------------------------------------------------------------------
ArgumentModel::ArgumentModel( CodeModel* model )
    : CodeModelItem( Argument, model)
{
}

QString ArgumentModel::type( ) const
{
    return m_type;
}

void ArgumentModel::setType( const QString& type )
{
    m_type = type;
}

QString ArgumentModel::defaultValue( ) const
{
    return m_defaultValue;
}

void ArgumentModel::setDefaultValue( const QString & defaultValue )
{
    m_defaultValue = defaultValue;
}

// ------------------------------------------------------------------------
FunctionModel::FunctionModel( CodeModel* model )
    : CodeModelItem( Function, model)
{
    m_access = Public;
    d.v.m_signal = false;
    d.v.m_slot = false;
    d.v.m_virtual = false;
    d.v.m_static = false;
    d.v.m_inline = false;
    d.v.m_constant = false;
    d.v.m_abstract = false;
}

bool FunctionModel::isVirtual( ) const
{
    return d.v.m_virtual;
}

void FunctionModel::setVirtual( bool isVirtual )
{
    d.v.m_virtual = isVirtual;
}

bool FunctionModel::isStatic( ) const
{
    return d.v.m_static;
}

void FunctionModel::setStatic( bool isStatic )
{
    d.v.m_static = isStatic;
}

bool FunctionModel::isInline( ) const
{
    return d.v.m_inline;
}

void FunctionModel::setInline( bool isInline )
{
    d.v.m_inline = isInline;
}

bool FunctionModel::isConstant( ) const
{
    return d.v.m_constant;
}

void FunctionModel::setConstant( bool isConstant )
{
    d.v.m_constant = isConstant;
}

bool FunctionModel::isAbstract( ) const
{
    return d.v.m_abstract;
}

void FunctionModel::setAbstract( bool isAbstract )
{
    d.v.m_abstract = isAbstract;
}

QString FunctionModel::resultType( ) const
{
    return m_resultType;
}

void FunctionModel::setResultType( const QString& type )
{
    m_resultType = type;
}

ArgumentList FunctionModel::argumentList( )
{
    return m_arguments;
}

const ArgumentList FunctionModel::argumentList( ) const
{
    return m_arguments;
}

bool FunctionModel::addArgument( ArgumentDom arg )
{
    m_arguments.push_back( arg );
    return true;
}

void FunctionModel::removeArgument( ArgumentDom arg )
{
    m_arguments.remove( arg );
}

// ------------------------------------------------------------------------
VariableModel::VariableModel( CodeModel* model )
    : CodeModelItem( Variable, model)
{
    m_access = Public;
    m_static = false;
		m_isEnumeratorVariable = false;
}

bool VariableModel::isStatic( ) const
{
    return m_static;
}

void VariableModel::setStatic( bool isStatic )
{
    m_static = isStatic;
}

QString VariableModel::type( ) const
{
    return m_type;
}

void VariableModel::setType( const QString& type )
{
    m_type = type;
}

bool VariableModel::isEnumeratorVariable() const {
	return m_isEnumeratorVariable;
}

void VariableModel::setEnumeratorVariable( bool b) {
	m_isEnumeratorVariable = b;
}

int FunctionModel::access( ) const
{
    return m_access;
}

void FunctionModel::setAccess( int access )
{
    m_access = access;
}

bool FunctionModel::isSignal( ) const
{
    return d.v.m_signal;
}

void FunctionModel::setSignal( bool isSignal )
{
    d.v.m_signal = isSignal;
}

bool FunctionModel::isSlot( ) const
{
    return d.v.m_slot;
}

void FunctionModel::setSlot( bool isSlot )
{
    d.v.m_slot = isSlot;
}

FunctionDefinitionModel::FunctionDefinitionModel( CodeModel* model )
    : FunctionModel( model )
{
}

int VariableModel::access( ) const
{
    return m_access;
}

void VariableModel::setAccess( int access )
{
    m_access = access;
}

const NamespaceDom CodeModel::globalNamespace( ) const
{
    return m_globalNamespace;
}

void CodeModelItem::read( QDataStream & stream )
{
    stream
	>> m_kind
	>> m_name
	>> m_fileName
	>> m_startLine
	>> m_startColumn
	>> m_endLine
	>> m_endColumn
        >> m_comment;
    
    if( isTemplateable() ) {
        TemplateModelItem* t = (TemplateModelItem*)( this );
        
        t->read( stream );
    }
}

void CodeModelItem::write( QDataStream & stream ) const
{
    stream
	<< m_kind
	<< m_name
	<< m_fileName
	<< m_startLine
	<< m_startColumn
	<< m_endLine
	<< m_endColumn
        << m_comment;
    
    if( isTemplateable() ) {
        TemplateModelItem* t = (TemplateModelItem*)( this );
        t-> write( stream );
    }
}

void ClassModel::read( QDataStream & stream )
{
    CodeModelItem::read( stream );

		TemplateModelItem::read( stream );
		
    stream >> m_scope >> m_baseClassList;

    int n;

    m_classes.clear();
    stream >> n;
    for( int i=0; i<n; ++i ){
	ClassDom klass = codeModel()->create<ClassModel>();
	klass->read( stream );
	addClass( klass );
    }

    m_functions.clear();
    stream >> n;
    for( int i=0; i<n; ++i ){
	FunctionDom fun = codeModel()->create<FunctionModel>();
	fun->read( stream );
	addFunction( fun );
    }

    m_functionDefinitions.clear();
    stream >> n;
    for( int i=0; i<n; ++i ){
	FunctionDefinitionDom fun = codeModel()->create<FunctionDefinitionModel>();
	fun->read( stream );
	addFunctionDefinition( fun );
    }

    m_variables.clear();
    stream >> n;
    for( int i=0; i<n; ++i ){
	VariableDom var = codeModel()->create<VariableModel>();
	var->read( stream );
	addVariable( var );
    }
    
    m_enumerators.clear();
    stream >> n;
    for( int i=0; i<n; ++i ){
	EnumDom e = codeModel()->create<EnumModel>();
	e->read( stream );
	addEnum( e );
    }

    m_typeAliases.clear();
    stream >> n;
    for( int i=0; i<n; ++i ){
	TypeAliasDom typeAlias = codeModel()->create<TypeAliasModel>();
	typeAlias->read( stream );
	addTypeAlias( typeAlias );
    }
}

void ClassModel::write( QDataStream & stream ) const
{
    CodeModelItem::write( stream );

		TemplateModelItem::write( stream );
		
    stream << m_scope << m_baseClassList;

    const ClassList class_list = classList();
    stream << int( class_list.size() );
    for( ClassList::ConstIterator it = class_list.begin(); it!=class_list.end(); ++it )
	(*it)->write( stream );

    const FunctionList function_list = functionList();
    stream << int( function_list.size() );
    for( FunctionList::ConstIterator it = function_list.begin(); it!=function_list.end(); ++it )
	(*it)->write( stream );

    const FunctionDefinitionList function_definition_list = functionDefinitionList();
    stream << int( function_definition_list.size() );
    for( FunctionDefinitionList::ConstIterator it = function_definition_list.begin(); it!=function_definition_list.end(); ++it )
	(*it)->write( stream );

    const VariableList variable_list = variableList();
    stream << int( variable_list.size() );
    for( VariableList::ConstIterator it = variable_list.begin(); it!=variable_list.end(); ++it )
	(*it)->write( stream );
    
    const EnumList enum_list = enumList();
    stream << int( enum_list.size() );
    for( EnumList::ConstIterator it = enum_list.begin(); it!=enum_list.end(); ++it )
	(*it)->write( stream );    
	
    const TypeAliasList type_alias_list = typeAliasList();
    stream << int( type_alias_list.size() );
    for( TypeAliasList::ConstIterator it = type_alias_list.begin(); it!=type_alias_list.end(); ++it )
	(*it)->write( stream );    
	
}

void NamespaceModel::read( QDataStream & stream )
{
    ClassModel::read( stream );

    int n;

    m_namespaces.clear(); m_namespaceAliases.clear(); m_namespaceImports.clear();
    stream >> n;
    for( int i=0; i<n; ++i ){
	NamespaceDom ns = codeModel()->create<NamespaceModel>();
	ns->read( stream );
	addNamespace( ns );
    }

  stream >> n;
  for( int a = 0; a < n; a++ ) {
    NamespaceAliasModel m;
    m.read( stream );
    m_namespaceAliases.insert( m );
  }
  stream >> n;
  for( int a = 0; a < n; a++ ) {
    NamespaceImportModel m;
    m.read( stream );
    m_namespaceImports.insert( m );
  }
}

void NamespaceModel::addNamespaceImport( const NamespaceImportModel& import ) {
  m_namespaceImports.insert( import );
}

void NamespaceModel::addNamespaceAlias( const NamespaceAliasModel& alias ) {
  m_namespaceAliases.insert( alias );
}

void NamespaceModel::removeNamespaceImport( const NamespaceImportModel& import ) {
  m_namespaceImports.erase( import );
}

void NamespaceModel::removeNamespaceAlias( const NamespaceAliasModel& alias ) {
  m_namespaceAliases.erase( alias );
}

void NamespaceModel::write( QDataStream & stream ) const
{
    ClassModel::write( stream );

    const NamespaceList namespace_list = namespaceList();
    stream << int( namespace_list.size() );
    for( NamespaceList::ConstIterator it = namespace_list.begin(); it!=namespace_list.end(); ++it )
	(*it)->write( stream );

  stream << int( m_namespaceAliases.size() );
  for( NamespaceAliasModelList::const_iterator it = m_namespaceAliases.begin(); it != m_namespaceAliases.end(); ++it )
    (*it).write( stream );
  stream << int( m_namespaceImports.size() );
  for( NamespaceImportModelList::const_iterator it = m_namespaceImports.begin(); it != m_namespaceImports.end(); ++it )
    (*it).write( stream );
}

void FileModel::read( QDataStream & stream )
{
    stream >> m_groupId;
    bool b;
    stream >> b;
    if( b ) {
      int i;
      stream >> i;
      ParsedFileType t( (ParsedFileType) i );
      switch( t ) {
      case CppParsedFile:
        m_parseResult = (AbstractParseResult*)(new ParsedFile());
        break;
      }
        m_parseResult->read( stream );
    }

    NamespaceModel::read( stream );
}

void FileModel::write( QDataStream & stream ) const
{
    stream << m_groupId;
    bool b = m_parseResult;
    stream << b;
    if( b ) {
        int i = m_parseResult->type();
        stream << i;
        m_parseResult->write( stream );
    }
    
    NamespaceModel::write( stream );
}

void ArgumentModel::read( QDataStream & stream )
{
    CodeModelItem::read( stream );
  

    stream >> m_type >> m_defaultValue;
}

void ArgumentModel::write( QDataStream & stream ) const
{
    CodeModelItem::write( stream );

    stream << m_type << m_defaultValue;
}

void FunctionModel::read( QDataStream & stream )
{
    CodeModelItem::read( stream );
		TemplateModelItem::read( stream );

    stream >> m_scope;
    stream >> d.flags;

    int n;

    m_arguments.clear();
    stream >> n;
    for( int i=0; i<n; ++i ){
	ArgumentDom arg = codeModel()->create<ArgumentModel>();
	arg->read( stream );
	addArgument( arg );
    }

    stream
	>> m_resultType;
}

void FunctionModel::write( QDataStream & stream ) const
{
    CodeModelItem::write( stream );
		TemplateModelItem::write( stream );

    stream << m_scope;
    stream << d.flags;

    const ArgumentList argument_list = argumentList();
    stream << int( argument_list.size() );
    for( ArgumentList::ConstIterator it = argument_list.begin(); it!=argument_list.end(); ++it )
	(*it)->write( stream );

    stream
	<< m_resultType;
}

void CodeModel::read( QDataStream & stream )
{
    int n;

    m_files.clear();

    stream >> n;
    for( int i=0; i<n; ++i ){
	FileDom file = this->create<FileModel>();
	file->read( stream );
	addFile( file );
    }
}

void CodeModel::write( QDataStream & stream ) const
{
    const FileList file_list = fileList();
    stream << int( file_list.size() );
    for( FileList::ConstIterator it = file_list.begin(); it!=file_list.end(); ++it )
	(*it)->write( stream );
}

void VariableModel::read( QDataStream & stream )
{
    CodeModelItem::read( stream );
		stream >> m_access >> m_static >> m_type >> m_isEnumeratorVariable;
}

void VariableModel::write( QDataStream & stream ) const
{
    CodeModelItem::write( stream );
		stream << m_access << m_static << m_type << m_isEnumeratorVariable;
}

// -------------------------------------------------------
EnumModel::EnumModel( CodeModel * model )
    : CodeModelItem( Enum, model)
{
}

int EnumModel::access( ) const
{
    return m_access;
}

void EnumModel::setAccess( int access )
{
    m_access = access;
}

EnumeratorList EnumModel::enumeratorList( )
{
    return m_enumerators.values();
}

const EnumeratorList EnumModel::enumeratorList( ) const
{
    return m_enumerators.values();
}

void EnumModel::addEnumerator( EnumeratorDom enumerator )
{
    m_enumerators.insert( enumerator->name(), enumerator );
}

void EnumModel::read( QDataStream & stream )
{
    CodeModelItem::read( stream );
    stream >> m_access;
    
    int n;
    stream >> n;
    for( int i=0; i<n; ++i ){
	EnumeratorDom e = codeModel()->create<EnumeratorModel>();
	e->read( stream );
	addEnumerator( e );
    }
}

void EnumModel::write( QDataStream & stream ) const
{
    CodeModelItem::write( stream );
    
    stream << m_access;
    const EnumeratorList enumerator_list = enumeratorList();
    stream << int( enumerator_list.size() );
    for( EnumeratorList::ConstIterator it = enumerator_list.begin(); it!=enumerator_list.end(); ++it )
	(*it)->write( stream );
}

EnumeratorModel::EnumeratorModel( CodeModel * model )
    : CodeModelItem( Enumerator, model )
{
}

QString EnumeratorModel::value( ) const
{
    return m_value;
}

void EnumeratorModel::setValue( const QString & value )
{
    m_value = value;
}

void EnumeratorModel::read( QDataStream & stream )
{
    CodeModelItem::read( stream );
    stream >> m_value;
}

void EnumeratorModel::write( QDataStream & stream ) const
{
    CodeModelItem::write( stream );
    stream << m_value;
}

void EnumModel::removeEnumerator( EnumeratorDom e )
{
    m_enumerators.remove( e->name() );
}

// ---------------------------------------------------------------
TypeAliasModel::TypeAliasModel( CodeModel * model )
    : CodeModelItem( TypeAlias, model )
{
}

void TypeAliasModel::read( QDataStream & stream )
{
    CodeModelItem::read( stream );
    
    stream >> m_type;
}

void TypeAliasModel::write( QDataStream & stream ) const
{
    CodeModelItem::write( stream );
    
    stream << m_type;
}

QString TypeAliasModel::type( ) const
{
    return m_type;
}

void TypeAliasModel::setType( const QString & type )
{
    m_type = type;
}

FileList FileModel::wholeGroup() {
    if( isSingleGroup( m_groupId ) ) return ( FileList() << FileDom(this) );
    return codeModel()->getGroup( m_groupId );
}

QStringList FileModel::wholeGroupStrings() const {
    if( isSingleGroup( m_groupId ) ) return (QStringList() << name() );
    return codeModel()->getGroupStrings( m_groupId );
}

ParseResultPointer FileModel::parseResult() const {
    return m_parseResult;
}

void FileModel::setParseResult( const ParseResultPointer& result ) {
    m_parseResult = result;
}
