/****************************************************************************
** BugListComponent meta object code from reading C++ file 'buglistcomponent.h'
**
** Created: Tue Dec 19 22:12:20 2000
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_BugListComponent
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 9
#elif Q_MOC_OUTPUT_REVISION != 9
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "buglistcomponent.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *BugListComponent::className() const
{
    return "BugListComponent";
}

QMetaObject *BugListComponent::metaObj = 0;

void BugListComponent::initMetaObject()
{
    if ( metaObj )
	return;
    if ( qstrcmp(KDevComponent::className(), "KDevComponent") != 0 )
	badSuperclassWarning("BugListComponent","KDevComponent");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION

QString BugListComponent::tr(const char* s)
{
    return qApp->translate( "BugListComponent", s, 0 );
}

QString BugListComponent::tr(const char* s, const char * c)
{
    return qApp->translate( "BugListComponent", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* BugListComponent::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) KDevComponent::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(BugListComponent::*m1_t0)();
    typedef void(BugListComponent::*m1_t1)();
    m1_t0 v1_0 = Q_AMPERSAND BugListComponent::slotActivate;
    m1_t1 v1_1 = Q_AMPERSAND BugListComponent::slotWidgetClosed;
    QMetaData *slot_tbl = QMetaObject::new_metadata(2);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(2);
    slot_tbl[0].name = "slotActivate()";
    slot_tbl[0].ptr = (QMember)v1_0;
    slot_tbl_access[0] = QMetaData::Public;
    slot_tbl[1].name = "slotWidgetClosed()";
    slot_tbl[1].ptr = (QMember)v1_1;
    slot_tbl_access[1] = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"BugListComponent", "KDevComponent",
	slot_tbl, 2,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    metaObj->set_slot_access( slot_tbl_access );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    return metaObj;
}
