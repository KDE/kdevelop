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

#ifndef CHECKOUTDIALOG_H
#define CHECKOUTDIALOG_H

#include <checkoutdialogbase.h>

/**
* Encapsulates user input while configuring access to remote CVS repository
* and provide some useful signal and methods to possible parent widgets.
*
* @author Mario Scalas
*/
class CheckoutDialog : public CheckoutDialogBase
{
    Q_OBJECT
public:
    CheckoutDialog( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~CheckoutDialog();

    QString cvsRsh() const;
    QString serverPath() const;

public slots:
    void slotFetchModulesList();
};

#endif
