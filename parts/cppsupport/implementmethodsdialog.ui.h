/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "cppsupportpart.h"
#include "classstore.h"
#include "parsedmethod.h"
#include "parsedparent.h"

#include <kdebug.h>

using namespace std;

class MethodListItem: public QCheckListItem
{
public:
    MethodListItem( QListViewItem* parent, ParsedMethod* meth )
	: QCheckListItem( parent, QString::null, QCheckListItem::CheckBox ), m_method( meth ) {}
    
    MethodListItem( QListView* parent, ParsedMethod* meth )
	: QCheckListItem( parent, QString::null, QCheckListItem::CheckBox ), m_method( meth ) {}
    
    virtual QString text( int n ) const 
    {
	switch( n ){
	    case 0: return m_method->asString();
	    default: return QString::null;
	}
	return QString::null;
    }
    
    ParsedMethod* method() { return m_method; }
    
private:
    ParsedMethod* m_method;
};


static QValueList<ParsedMethod*> getVirtualMethodListForClassAndAncestors( CppSupportPart* part, ParsedClass* pClass )
{
    QValueList<ParsedMethod*> retVal = pClass->getVirtualMethodList( );
    
    QPtrList<ParsedParent> parentList = pClass->parents;
    for ( ParsedParent* pPClass = parentList.first( ); pPClass != 0; pPClass = parentList.next( ) ) {
	pClass = part->classStore()->getClassByName( pPClass->name() );
	if( !pClass )
	    pClass = part->ccClassStore()->getClassByName( pPClass->name( ) );

        if ( pClass )
            retVal += getVirtualMethodListForClassAndAncestors( part, pClass );
        else {
            /// @todo look in ClassStore for Namespace classes
        }
    }

    return retVal;
}

void ImplementMethodsDialog::init()
{
}

void ImplementMethodsDialog::destroy()
{
}

void ImplementMethodsDialog::setPart( CppSupportPart * part )
{
    m_part = part;
}

void ImplementMethodsDialog::accept()
{
    selectedMethods.clear();
    QListViewItem* item = MethodListView->firstChild();
    while( item != 0 ){
	kdDebug(9007) << "***" << endl;
	MethodListItem* meth = dynamic_cast<MethodListItem*>( item->firstChild() );
	while( meth ){
	    kdDebug(9007) << "+++" << endl;
	    if( meth->isOn() ){
		kdDebug(9007) << "add method " << meth->method()->asString() << endl;
		selectedMethods << meth->method();
	    }
	    meth = dynamic_cast<MethodListItem*>( item->nextSibling() );
	}
	
	item = item->nextSibling();
    }
    QDialog::accept();
}

int ImplementMethodsDialog::implementMethods( ParsedClass * klass )
{
    if( !m_part || !klass )
	return 0;
    
    selectedMethods.clear();
    MethodListView->clear();
    
    QPtrListIterator<ParsedParent> it( klass->parents );
    while( it.current() ){
	ParsedParent* parent = it.current();
	++it;
	
	ParsedClass* parentKlass = m_part->classStore()->getClassByName( parent->name() );
	if( !parentKlass )
	    parentKlass = m_part->ccClassStore()->getClassByName( parent->name() );
	
	if( !parentKlass )
	    continue;
	
	QValueList<ParsedMethod*> methods( getVirtualMethodListForClassAndAncestors(m_part, parentKlass) );
	if( !methods.size() )
	    continue;
	
	QListViewItem* item = new QListViewItem( MethodListView, parent->name() );
	item->setOpen( true );
	QValueList<ParsedMethod*>::Iterator mIt = methods.begin();
	while( mIt != methods.end() ){
	    ParsedMethod* meth = *mIt++;
	    if( meth->isVirtual() && 
		!meth->isConstructor() && !meth->isDestructor() )
		new MethodListItem( item, meth );
	}
    }

    return exec();
}
