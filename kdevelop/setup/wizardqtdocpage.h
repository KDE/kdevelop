/***************************************************************************
                          wizardqtdocpage.h  -  description
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

#ifndef WIZARDQTDOCPAGE_H
#define WIZARDQTDOCPAGE_H

#include <qlineedit.h>

#include "wizardbasepage.h"

class WizardQtDocPage : public WizardBasePage
{
	Q_OBJECT
public:
	WizardQtDocPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState);
protected:
  virtual void showEvent( QShowEvent * e);
	virtual void hideEvent( QHideEvent * e);
private slots:	
	void slotQTpressed();
private:	
	QLineEdit* qt_edit;
};

#endif  // WIZARDQTDOCPAGE_H
