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
#include <qwhatsthis.h>

#include <klocale.h>
#include <kconfig.h>

#include "ckdevinstallstate.h"
#include "wizardchoosehlpage.h"

WizardSyntaxHlPage::WizardSyntaxHlPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState)
: WizardBasePage(parent, name, infoText, installPictPathAndFilename, pInstallState)
{
  m_vbox = new QVBox(this);
  QLabel* label = new QLabel(m_vbox);
  label = new QLabel(i18n("Now you can choose the Syntax-Highlighting style which KDevelop will use.") + "\n" +
										 i18n("Which one do you want to use?"), m_vbox );
   										
  QButtonGroup* bg = new QButtonGroup(m_vbox);

  QGridLayout* grid = new QGridLayout(bg,3,1,15,7);
  QRadioButton* newKDevelop20Style = new QRadioButton( i18n("KDevelop 2.0 style"), bg );
  QRadioButton* emacsStyle = new QRadioButton( i18n("Emacs style"), bg );
  QRadioButton* kWriteDefault = new QRadioButton( i18n("KWrite default"), bg );
  grid->addWidget(newKDevelop20Style,0,0);
  grid->addWidget(emacsStyle,1,0);
  grid->addWidget(kWriteDefault,2,0);
	bg->setFixedHeight(bg->sizeHint().height());

  QObject::connect(bg, SIGNAL(clicked(int)), this, SLOT(slotSetSyntaxHl(int)));
  QObject::connect(bg, SIGNAL(clicked(int)), this, SLOT(slotSetSyntaxHl(int)));
  QObject::connect(bg, SIGNAL(clicked(int)), this, SLOT(slotSetSyntaxHl(int)));

  switch (m_pInstallState->highlightStyle) {
  case 0:
    newKDevelop20Style->setChecked(true);
    break;
  case 1:
	  emacsStyle->setChecked(true);
    break;
  case 2:
	  kWriteDefault->setChecked(true);
    break;
  default:
    break;
  }

  QWhatsThis::add(newKDevelop20Style, i18n("The source files are almost like MS Visual C++ but a little bit more colored."
                                           " The rest is like in KWrite."));
  QWhatsThis::add(emacsStyle, i18n("Like used from Emacs."));
  QWhatsThis::add(kWriteDefault, i18n("Like used from the KWrite editor."));

  label = new QLabel(m_vbox);
}

void WizardSyntaxHlPage::slotSetSyntaxHl(int highl_style)
{
	m_pInstallState->highlightStyle = highl_style;
}

#include "wizardchoosehlpage.moc"
