/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef FCTYPEEDIT_H
#define FCTYPEEDIT_H


#include "fctypeeditbase.h"

class FCTypeEdit : public FCTypeEditBase
{
Q_OBJECT
public:
    FCTypeEdit(QWidget *parent = 0, char *name = 0);

    ~FCTypeEdit();

protected slots:
    virtual void accept();
    void slotTypeEditTextChanged( );
};

#endif
