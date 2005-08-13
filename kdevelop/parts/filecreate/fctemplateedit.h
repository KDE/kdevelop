/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef FCTEMPLATEEDIT_H
#define FCTEMPLATEEDIT_H

#include <qwidget.h>
#include "fctemplateeditbase.h"

class FCTemplateEdit : public FCTemplateEditBase
{
Q_OBJECT
public:
    FCTemplateEdit(QWidget *parent=0, const char *name=0);

    ~FCTemplateEdit();

protected slots:
    virtual void accept();
    void slotTemplateNameChanged( );
};

#endif
