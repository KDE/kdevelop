
#include "cppsupport_utils.h"
#include <codemodel.h>
#include <qdir.h>

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

//kate: indent-mode csands; tab-width 4; space-indent off;
