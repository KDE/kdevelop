/****************************************************************************
** BugList meta object code from reading C++ file 'buglist.h'
**
** Created: Tue Dec 19 22:12:10 2000
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_BugList
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 9
#elif Q_MOC_OUTPUT_REVISION != 9
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "buglist.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *BugList::className() const
{
    return "BugList";
}

QMetaObject *BugList::metaObj = 0;

void BugList::initMetaObject()
{
    if ( metaObj )
	return;
    if ( qstrcmp(QWidget::className(), "QWidget") != 0 )
	badSuperclassWarning("BugList","QWidget");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION

QString BugList::tr(const char* s)
{
    return qApp->translate( "BugList", s, 0 );
}

QString BugList::tr(const char* s, const char * c)
{
    return qApp->translate( "BugList", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* BugList::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QWidget::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(BugList::*m1_t0)();
    typedef void(BugList::*m1_t1)();
    typedef void(BugList::*m1_t2)();
    typedef void(BugList::*m1_t3)();
    typedef void(BugList::*m1_t4)(QListViewItem*);
    typedef void(BugList::*m1_t5)();
    typedef void(BugList::*m1_t6)();
    typedef void(BugList::*m1_t7)(Bug*);
    typedef void(BugList::*m1_t8)(Bug*);
    typedef void(BugList::*m1_t9)();
    typedef void(BugList::*m1_t10)();
    m1_t0 v1_0 = Q_AMPERSAND BugList::slotCloseClicked;
    m1_t1 v1_1 = Q_AMPERSAND BugList::slotCancelClicked;
    m1_t2 v1_2 = Q_AMPERSAND BugList::slotAddClicked;
    m1_t3 v1_3 = Q_AMPERSAND BugList::slotEditClicked;
    m1_t4 v1_4 = Q_AMPERSAND BugList::slotListDoubleClicked;
    m1_t5 v1_5 = Q_AMPERSAND BugList::slotRemoveClicked;
    m1_t6 v1_6 = Q_AMPERSAND BugList::slotCompletedClicked;
    m1_t7 v1_7 = Q_AMPERSAND BugList::slotAddBug;
    m1_t8 v1_8 = Q_AMPERSAND BugList::slotUpdateBug;
    m1_t9 v1_9 = Q_AMPERSAND BugList::slotFilter;
    m1_t10 v1_10 = Q_AMPERSAND BugList::slotOwnership;
    QMetaData *slot_tbl = QMetaObject::new_metadata(11);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(11);
    slot_tbl[0].name = "slotCloseClicked()";
    slot_tbl[0].ptr = (QMember)v1_0;
    slot_tbl_access[0] = QMetaData::Public;
    slot_tbl[1].name = "slotCancelClicked()";
    slot_tbl[1].ptr = (QMember)v1_1;
    slot_tbl_access[1] = QMetaData::Public;
    slot_tbl[2].name = "slotAddClicked()";
    slot_tbl[2].ptr = (QMember)v1_2;
    slot_tbl_access[2] = QMetaData::Private;
    slot_tbl[3].name = "slotEditClicked()";
    slot_tbl[3].ptr = (QMember)v1_3;
    slot_tbl_access[3] = QMetaData::Private;
    slot_tbl[4].name = "slotListDoubleClicked(QListViewItem*)";
    slot_tbl[4].ptr = (QMember)v1_4;
    slot_tbl_access[4] = QMetaData::Private;
    slot_tbl[5].name = "slotRemoveClicked()";
    slot_tbl[5].ptr = (QMember)v1_5;
    slot_tbl_access[5] = QMetaData::Private;
    slot_tbl[6].name = "slotCompletedClicked()";
    slot_tbl[6].ptr = (QMember)v1_6;
    slot_tbl_access[6] = QMetaData::Private;
    slot_tbl[7].name = "slotAddBug(Bug*)";
    slot_tbl[7].ptr = (QMember)v1_7;
    slot_tbl_access[7] = QMetaData::Private;
    slot_tbl[8].name = "slotUpdateBug(Bug*)";
    slot_tbl[8].ptr = (QMember)v1_8;
    slot_tbl_access[8] = QMetaData::Private;
    slot_tbl[9].name = "slotFilter()";
    slot_tbl[9].ptr = (QMember)v1_9;
    slot_tbl_access[9] = QMetaData::Private;
    slot_tbl[10].name = "slotOwnership()";
    slot_tbl[10].ptr = (QMember)v1_10;
    slot_tbl_access[10] = QMetaData::Private;
    typedef void(BugList::*m2_t0)();
    m2_t0 v2_0 = Q_AMPERSAND BugList::signalDeactivate;
    QMetaData *signal_tbl = QMetaObject::new_metadata(1);
    signal_tbl[0].name = "signalDeactivate()";
    signal_tbl[0].ptr = (QMember)v2_0;
    metaObj = QMetaObject::new_metaobject(
	"BugList", "QWidget",
	slot_tbl, 11,
	signal_tbl, 1,
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

// SIGNAL signalDeactivate
void BugList::signalDeactivate()
{
    activate_signal( "signalDeactivate()" );
}
