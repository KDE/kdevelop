/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _APPWIZARDDIALOG_H_
#define _APPWIZARDDIALOG_H_

#include <kassistantdialog.h>

class AppWizardDialog: public KAssistantDialog {
public:
    explicit AppWizardDialog(QWidget *parent = 0, Qt::WFlags flags = 0);

};

#endif

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
