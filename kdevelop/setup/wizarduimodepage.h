/***************************************************************************
                          wizarduimodepage.h  -  description
                             -------------------
    begin                : Sat Jun 9 2001
    copyright            : (C) 2001 by Falk Brettschneider
    email                : falk.brettschneider@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WIZARDUIMODEPAGE_H
#define WIZARDUIMODEPAGE_H

#include "wizardbasepage.h"

class WizardUIModePage : public WizardBasePage
{
	Q_OBJECT
public:
	WizardUIModePage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState);
protected slots:	
	void slotModeChanged(int userInterfaceMode);
};

#endif  // WIZARDUIMODEPAGE_H
