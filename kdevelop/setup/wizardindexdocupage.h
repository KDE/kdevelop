/***************************************************************************
                          wizardindexdocupage.h  -  description
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

#ifndef WIZARDINDEXDOCUPAGE_H
#define WIZARDINDEXDOCUPAGE_H

#include "wizardbasepage.h"

class CCreateDocDatabaseDlg;

class WizardIndexDocuPage : public WizardBasePage
{
	Q_OBJECT
public:
	WizardIndexDocuPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState);

protected slots:	
  void indexingFinished(const QString&);
  void slotDisableButton();
  virtual void showEvent(QShowEvent * e);

private:
  CCreateDocDatabaseDlg* pDlg;
};

#endif  // WIZARDINDEXDOCUPAGE_H
