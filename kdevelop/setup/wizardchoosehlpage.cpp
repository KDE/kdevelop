/***************************************************************************
                          WizardChooseHlPage.cpp  -  description
                             -------------------
    begin                : Sat Jun 2 2001
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

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <kconfig.h>

#include "ckdevinstall.h"

WizardSyntaxHlPage::WizardSyntaxHlPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState)
: WizardBasePage(parent, name, infoText, installPictPathAndFilename, pInstallState)
{
  QVBox * vbox = new QVBox(this);
  QLabel* label = new QLabel(vbox);
  label = new QLabel(i18n("Now you can choose the Syntax-Highlighting style which KDevelop will use.") + "\n" +
										 i18n("Which one do you want to use?"), vbox );
   										
  QButtonGroup* bg = new QButtonGroup(vbox);

  QGridLayout* grid = new QGridLayout(bg,2,1,15,7);
  QRadioButton* emacsStyle = new QRadioButton( i18n("Emacs style"), bg );
  QRadioButton* kWriteDefault = new QRadioButton( i18n("KWrite default"), bg );
  grid->addWidget(emacsStyle,0,0);
  grid->addWidget(kWriteDefault,1,0);
	bg->setFixedHeight(bg->sizeHint().height());

  QObject::connect(emacsStyle, SIGNAL(clicked(int)), this, SLOT(slotSetSyntaxHl(int)));
  QObject::connect(kWriteDefault, SIGNAL(clicked(int)), this, SLOT(slotSetSyntaxHl(int)));

  if (m_pInstallState->highlightStyle == 0)
	  emacsStyle->setChecked(true);
	else	
	  kWriteDefault->setChecked(true);

  label = new QLabel(vbox);
}

void WizardSyntaxHlPage::slotSetSyntaxHl(int highl_style)
{
	m_pInstallState->highlightStyle = highl_style;
}
