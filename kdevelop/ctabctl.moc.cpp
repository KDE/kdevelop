/****************************************************************************
** CTabCtl meta object code from reading C++ file 'ctabctl.h'
**
** Created: Sat Jan 30 17:18:11 1999
**      by: The Qt Meta Object Compiler ($Revision$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 2
#elif Q_MOC_OUTPUT_REVISION != 2
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "ctabctl.h"
#include <qmetaobject.h>


const char *CTabCtl::className() const
{
    return "CTabCtl";
}

QMetaObject *CTabCtl::metaObj = 0;


#if QT_VERSION >= 200
static QMetaObjectInit init_CTabCtl(&CTabCtl::staticMetaObject);

#endif

void CTabCtl::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(KTabCtl::className(), "KTabCtl") != 0 )
	badSuperclassWarning("CTabCtl","KTabCtl");

#if QT_VERSION >= 200
    staticMetaObject();
}

void CTabCtl::staticMetaObject()
{
    if ( metaObj )
	return;
    KTabCtl::staticMetaObject();
#else

    KTabCtl::initMetaObject();
#endif

    metaObj = new QMetaObject( "CTabCtl", "KTabCtl",
	0, 0,
	0, 0 );
}
