/***************************************************************************
                             customizedlg.cpp
                             ----------------------
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include <klocale.h>
#include "componentmanager.h"
#include "customizedlg.h"


CustomizeDialog::CustomizeDialog(QWidget *parent, const char *name)
    : KDialogBase(TreeList, i18n("Customize KDevelop"), Ok|Cancel,
                  Ok, parent, "customize dialog")
{
}


CustomizeDialog::~CustomizeDialog()
{}
