/***************************************************************************
                          ctagsdialog_impl.cpp
                          --------------------
    begin                : Wed April 26 2001
    copyright            : (C) 2001 by rokrau, the kdevelop-team
    email                : rokrau@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ctagsdialog_impl.h"

/* 
 *  Constructs a searchTagsDialogImpl which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
searchTagsDialogImpl::searchTagsDialogImpl( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : searchTagsDialog( parent, name, modal, fl )
{
}

/*  
 *  Destroys the object and frees any allocated resources
 */
searchTagsDialogImpl::~searchTagsDialogImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

/* 
 * public slot
 */
void searchTagsDialogImpl::slotLBItemSelected()
{
    qWarning( "searchTagsDialogImpl::slotLBItemSelected() not yet implemented!" ); 
}
/* 
 * public slot
 */
void searchTagsDialogImpl::slotClear()
{
    qWarning( "searchTagsDialogImpl::slotClear() not yet implemented!" ); 
}
/* 
 * public slot
 */
void searchTagsDialogImpl::slotGotoTag()
{
    qWarning( "searchTagsDialogImpl::slotGotoTag() not yet implemented!" ); 
}
/* 
 * public slot
 */
void searchTagsDialogImpl::slotSearchTag()
{
    qWarning( "searchTagsDialogImpl::slotSearchTag() not yet implemented!" ); 
}

