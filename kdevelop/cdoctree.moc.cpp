/****************************************************************************
** CDocTree meta object code from reading C++ file 'cdoctree.h'
**
** Created: Tue Dec 15 20:47:36 1998
**      by: The Qt Meta Object Compiler ($Revision$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 2
#elif Q_MOC_OUTPUT_REVISION != 2
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "cdoctree.h"
#include <qmetaobject.h>


const char *CDocTree::className() const
{
    return "CDocTree";
}

QMetaObject *CDocTree::metaObj = 0;


#if QT_VERSION >= 200
static QMetaObjectInit init_CDocTree(&CDocTree::staticMetaObject);

#endif

void CDocTree::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(KTreeList::className(), "KTreeList") != 0 )
	badSuperclassWarning("CDocTree","KTreeList");

#if QT_VERSION >= 200
    staticMetaObject();
}

void CDocTree::staticMetaObject()
{
    if ( metaObj )
	return;
    KTreeList::staticMetaObject();
#else

    KTreeList::initMetaObject();
#endif

    metaObj = new QMetaObject( "CDocTree", "KTreeList",
	0, 0,
	0, 0 );
}
