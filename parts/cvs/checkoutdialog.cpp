/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcombobox.h>
#include <qlineedit.h>
#include <qbutton.h>

#include <kpassdlg.h>
#include <knuminput.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "checkoutdialog.h"

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

const QString SSS( ":" );  // Server String Separator :)

///////////////////////////////////////////////////////////////////////////////
// class CheckoutDialog
///////////////////////////////////////////////////////////////////////////////

CheckoutDialog::CheckoutDialog( QWidget *parent, const char *name, WFlags f )
    : CheckoutDialogBase( parent, name ? name : "checkoutdialog", f )
{
    setWFlags( WDestructiveClose | getWFlags() );  // Auto-delete this window when closed

 //   connect( fetchModulesButton, SIGNAL(clicked()), this, SLOT(slotFetchModulesList()) );
}

///////////////////////////////////////////////////////////////////////////////

CheckoutDialog::~CheckoutDialog()
{
}

///////////////////////////////////////////////////////////////////////////////

QString CheckoutDialog::cvsRsh() const
{
    return cvsRshComboBox->text( cvsRshComboBox->currentItem() );
}

///////////////////////////////////////////////////////////////////////////////

QString CheckoutDialog::serverPath() const
{
    return serverPathLineEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::slotFetchModulesList()
{
}

//#include "serverconfigurationwidget.moc.cpp"
