#include "wzconnectdlgimpl.h"

/* 
 *  Constructs a wzConnectDlgImpl which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The wizard will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal wizard.
 */
wzConnectDlgImpl::wzConnectDlgImpl( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : wzConnectDlg( parent, name, modal, fl )
{
}

/*  
 *  Destroys the object and frees any allocated resources
 */
wzConnectDlgImpl::~wzConnectDlgImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

/* 
 * public slot
 */
void wzConnectDlgImpl::slotDoCreateMethod()
{
    qWarning( "wzConnectDlgImpl::slotDoCreateMethod() not yet implemented!" ); 
}

/* 
 * protected slot
 */
void wzConnectDlgImpl::slotDoCreateSlot()
{
    qWarning( "wzConnectDlgImpl::slotDoCreateSlot() not yet implemented!" ); 
}
/* 
 * protected slot
 */
void wzConnectDlgImpl::slotMemberSelected(const QString&)
{
    qWarning( "wzConnectDlgImpl::slotMemberSelected(const QString&) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void wzConnectDlgImpl::slotMethodChanged()
{
    qWarning( "wzConnectDlgImpl::slotMethodChanged() not yet implemented!" ); 
}
/* 
 * protected slot
 */
void wzConnectDlgImpl::slotMethodSelected(const QString&)
{
    qWarning( "wzConnectDlgImpl::slotMethodSelected(const QString&) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void wzConnectDlgImpl::slotNewSlotChanged()
{
    qWarning( "wzConnectDlgImpl::slotNewSlotChanged() not yet implemented!" ); 
}
/* 
 * protected slot
 */
void wzConnectDlgImpl::slotSignalSelected(const QString&)
{
    qWarning( "wzConnectDlgImpl::slotSignalSelected(const QString&) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void wzConnectDlgImpl::slotSlotSelected(const QString&)
{
    qWarning( "wzConnectDlgImpl::slotSlotSelected(const QString&) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void wzConnectDlgImpl::slotSourceClassSelected(const QString&)
{
    qWarning( "wzConnectDlgImpl::slotSourceClassSelected(const QString&) not yet implemented!" ); 
}

