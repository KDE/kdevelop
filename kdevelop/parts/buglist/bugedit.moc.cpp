/****************************************************************************
** BugEdit meta object code from reading C++ file 'bugedit.h'
**
** Created: Tue Dec 19 21:15:05 2000
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_BugEdit
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 9
#elif Q_MOC_OUTPUT_REVISION != 9
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "bugedit.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *BugEdit::className() const
{
    return "BugEdit";
}

QMetaObject *BugEdit::metaObj = 0;

void BugEdit::initMetaObject()
{
    if ( metaObj )
	return;
    if ( qstrcmp(QTabDialog::className(), "QTabDialog") != 0 )
	badSuperclassWarning("BugEdit","QTabDialog");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION

QString BugEdit::tr(const char* s)
{
    return qApp->translate( "BugEdit", s, 0 );
}

QString BugEdit::tr(const char* s, const char * c)
{
    return qApp->translate( "BugEdit", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* BugEdit::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QTabDialog::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(BugEdit::*m1_t0)();
    typedef void(BugEdit::*m1_t1)();
    m1_t0 v1_0 = Q_AMPERSAND BugEdit::closeClicked;
    m1_t1 v1_1 = Q_AMPERSAND BugEdit::cancelClicked;
    QMetaData *slot_tbl = QMetaObject::new_metadata(2);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(2);
    slot_tbl[0].name = "closeClicked()";
    slot_tbl[0].ptr = (QMember)v1_0;
    slot_tbl_access[0] = QMetaData::Private;
    slot_tbl[1].name = "cancelClicked()";
    slot_tbl[1].ptr = (QMember)v1_1;
    slot_tbl_access[1] = QMetaData::Private;
    typedef void(BugEdit::*m2_t0)(Bug*);
    typedef void(BugEdit::*m2_t1)(Bug*);
    m2_t0 v2_0 = Q_AMPERSAND BugEdit::sigAddBug;
    m2_t1 v2_1 = Q_AMPERSAND BugEdit::sigUpdateBug;
    QMetaData *signal_tbl = QMetaObject::new_metadata(2);
    signal_tbl[0].name = "sigAddBug(Bug*)";
    signal_tbl[0].ptr = (QMember)v2_0;
    signal_tbl[1].name = "sigUpdateBug(Bug*)";
    signal_tbl[1].ptr = (QMember)v2_1;
    metaObj = QMetaObject::new_metaobject(
	"BugEdit", "QTabDialog",
	slot_tbl, 2,
	signal_tbl, 2,
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

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL sigAddBug
void BugEdit::sigAddBug( Bug* t0 )
{
    // No builtin function for signal parameter type Bug*
    QConnectionList *clist = receivers("sigAddBug(Bug*)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef void (QObject::*RT1)(Bug*);
    RT0 r0;
    RT1 r1;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
#ifdef Q_FP_CCAST_BROKEN
		r0 = reinterpret_cast<RT0>(*(c->member()));
#else
		r0 = (RT0)*(c->member());
#endif
		(object->*r0)();
		break;
	    case 1:
#ifdef Q_FP_CCAST_BROKEN
		r1 = reinterpret_cast<RT1>(*(c->member()));
#else
		r1 = (RT1)*(c->member());
#endif
		(object->*r1)(t0);
		break;
	}
    }
}

// SIGNAL sigUpdateBug
void BugEdit::sigUpdateBug( Bug* t0 )
{
    // No builtin function for signal parameter type Bug*
    QConnectionList *clist = receivers("sigUpdateBug(Bug*)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef void (QObject::*RT1)(Bug*);
    RT0 r0;
    RT1 r1;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
#ifdef Q_FP_CCAST_BROKEN
		r0 = reinterpret_cast<RT0>(*(c->member()));
#else
		r0 = (RT0)*(c->member());
#endif
		(object->*r0)();
		break;
	    case 1:
#ifdef Q_FP_CCAST_BROKEN
		r1 = reinterpret_cast<RT1>(*(c->member()));
#else
		r1 = (RT1)*(c->member());
#endif
		(object->*r1)(t0);
		break;
	}
    }
}
