/***************************************************************************
                          wizarduimodepage.cpp  -  description
                             -------------------
    begin                : Mon Jun 4 2001
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

#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kstddirs.h>

#include "ckdevinstallstate.h"
#include "wizarduimodepage.h"

WizardUIModePage::WizardUIModePage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState)
: WizardBasePage(parent, name, infoText, installPictPathAndFilename, pInstallState)
{
  m_vbox = new QVBox(this);

  QLabel * label = new QLabel("", m_vbox);

  KGlobal::instance()->iconLoader()->loadIcon( "window_list", KIcon::NoGroup, KIcon::SizeMedium );

  label = new QLabel(i18n("These are possible user interface modes, all of them are types of MDI.") + "\n" +
                     i18n("Use the question mark button to get more information about each style!:"),m_vbox);

  QButtonGroup* bg = new QButtonGroup(m_vbox);
  QObject::connect(bg, SIGNAL(clicked(int)), SLOT(slotModeChanged(int)) );
  QGridLayout* innerGrid = new QGridLayout(bg,3,2,15,7);
  QPixmap pm;

  QRadioButton* toplevel = new QRadioButton( i18n("Toplevel mode"), bg );
  innerGrid->addWidget(toplevel,1,0);
  QLabel* pictureLabelTL = new QLabel(bg);
  pm.load(locate("appdata", "pics/toplevel.png"));
  pictureLabelTL->setPixmap(pm);
  innerGrid->addWidget(pictureLabelTL,1,1);

  QRadioButton* childframe = new QRadioButton( i18n("Childframe mode"), bg );
  innerGrid->addWidget(childframe,0,0);
  QLabel* pictureLabelCF = new QLabel(bg);
  pm.load(locate("appdata", "pics/childfrm.png"));
  pictureLabelCF->setPixmap(pm);
  innerGrid->addWidget(pictureLabelCF,0,1);

  QRadioButton* tabpage = new QRadioButton( i18n("Tab page mode"), bg );
  innerGrid->addWidget(tabpage,2,0);
  QLabel* pictureLabelTP = new QLabel(bg);
  pm.load(locate("appdata", "pics/tabpage.png"));
  pictureLabelTP->setPixmap(pm);
  innerGrid->addWidget(pictureLabelTP,2,1);

  childframe->setChecked(false);
  toplevel->setChecked(false);
  tabpage->setChecked(false);

  switch (m_pInstallState->userInterfaceMode) {
  case 0:
    toplevel->setChecked(true);
    break;
  case 1:
    childframe->setChecked(true);
    break;
  case 2:
    tabpage->setChecked(true);
    break;
  default:
    break;
  }

  bg->setFrameStyle(QFrame::Raised|QFrame::Box);
  bg->setMargin(8);
  bg->setFixedHeight(bg->sizeHint().height());

  QString cfTxt = i18n("All tool views are initially docked to the mainframe.\n"
                       "Editor and browser views will live like toplevel windows within a view area of the mainframe.\n"
                       "A typical example of this MDI mode is MS Visual Studio 6.0.");
  QWhatsThis::add(childframe, cfTxt);
  QString tlTxt = i18n("All editor, browser and tool views will be toplevel windows (directly on desktop).\n"
                       "The main widget contains the menu, toolbars and statusbar, only.\n"
                       "A typical example of this MDI mode is the image manipulation application Gimp");
  QWhatsThis::add(toplevel, tlTxt);
  QString tpTxt = i18n("All tool views are initially docked to the mainframe.\n"
                       "Editor and browser views will be stacked in a tab window.\n"
                       "A typical example of this MDI mode is KDEStudio, our friend C++-IDE in the world of KDE.");
  QWhatsThis::add(tabpage, tpTxt);

  label = new QLabel("", m_vbox);
}

void WizardUIModePage::slotModeChanged(int userInterfaceMode)
{
  m_pInstallState->userInterfaceMode = userInterfaceMode;
}

#include "wizarduimodepage.moc"
