
#include "cppsupport_utils.h"
#include <codemodel.h>
#include <qdir.h>

#include <kdebug.h>

static void typeNameList( QStringList& path, QStringList & lst, const CodeModel * model );
static void typeNameList( QStringList& path, QStringList & lst, NamespaceDom ns );
static void typeNameList( QStringList & path, QStringList & lst, ClassDom klass );

QStringList typeNameList( const CodeModel* model )
{
	QStringList lst;
	QStringList path;
	typeNameList( path, lst, model );
	return lst;
}

static void typeNameList( QStringList& path, QStringList & lst, const CodeModel * model )
{
	const FileList fileList = model->fileList();
	for( FileList::ConstIterator it=fileList.begin(); it!=fileList.end(); ++it )
		typeNameList( path, lst, model_cast<NamespaceDom>(*it) );
}

static void typeNameList( QStringList& path, QStringList & lst, NamespaceDom ns )
{
	if( !ns->isFile() )
		path.push_back( ns->name() );
	
	const NamespaceList namespaceList = ns->namespaceList();
	for( NamespaceList::ConstIterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
		typeNameList( path, lst, *it );
	
	const ClassList classList = ns->classList();
	for( ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it )
		typeNameList( path, lst, *it );
	
	if( !ns->isFile() )
		path.pop_back();
}

static void typeNameList( QStringList & path, QStringList & lst, ClassDom klass )
{
	path.push_back( klass->name() );
	
	lst << path.join( "::" );
	
	const ClassList classList = klass->classList();
	for( ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it )
		typeNameList( path, lst, *it );
	
	path.pop_back();
}

static void typedefMap( QMap<QString, QString> & map, const CodeModel * model );
static void typedefMap( QMap<QString, QString> & map, NamespaceDom ns );
static void typedefMap( QMap<QString, QString> & map, ClassDom klass );

QMap<QString, QString> typedefMap( const CodeModel* model )
{
	QMap<QString, QString> map;
	typedefMap( map, model );
	
	/*We need to flatten the typedefs to avoid circular aliases. 
      Example:
		map["Foo"] = "int";
		map["Bar"] = "Foo";
		map["Baz"] = "Bar";*/
	
	QMap<QString, QString>::iterator it = map.begin();
	for ( ; it != map.end(); ++it )
	{
		while ( map.contains( map[ it.key() ] ) && 
		        it.key() != map[ it.key() ] )
		{
	 		map[ it.key() ] = map[ map[ it.key() ] ];
		}
	}
	
	return map;
}

static void typedefMap( QMap<QString, QString> & map, const CodeModel * model )
{
	const FileList fileList = model->fileList();
	for( FileList::ConstIterator it=fileList.begin(); it!=fileList.end(); ++it )
		typedefMap( map, model_cast<NamespaceDom>(*it) );
}

static void typedefMap( QMap<QString, QString> & map, NamespaceDom ns )
{
	const TypeAliasList aliasList = ns->typeAliasList();
	for( TypeAliasList::ConstIterator it=aliasList.begin(); it!=aliasList.end(); ++it )
		map[ ( *it )->name() ] = ( *it )->type();
	
	const NamespaceList namespaceList = ns->namespaceList();
	for( NamespaceList::ConstIterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
		typedefMap( map, *it );
	
	const ClassList classList = ns->classList();
	for( ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it )
		typedefMap( map, *it );
}

static void typedefMap( QMap<QString, QString> & map, ClassDom klass )
{
	const TypeAliasList aliasList = klass->typeAliasList();
	for( TypeAliasList::ConstIterator it=aliasList.begin(); it!=aliasList.end(); ++it )
		map[ ( *it )->name() ] = ( *it )->type();
	
	const ClassList classList = klass->classList();
	for( ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it )
		typedefMap( map, *it );
}

//kate: indent-mode csands; tab-width 4; space-indent off;
