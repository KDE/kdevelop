/***************************************************************************
  uichooser_widget.cpp - ?
			     -------------------
    begin                : ?
    copyright            : (C) 2003 by the KDevelop team
    email                : team@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qradiobutton.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>

#include "uichooser_part.h"
#include "kdevtoplevel.h"
#include "uichooser_widget.h"

UIChooserWidget::UIChooserWidget(QWidget *parent, const char *name)
  : UIChooser(parent, name)
  ,m_pMyPart(0L)
{
  load();
}

void UIChooserWidget::setPart(UIChooserPart* pMyPart)
{
  m_pMyPart = pMyPart;
}

void UIChooserWidget::load()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  if (config->readEntry("MajorUIMode", "IDEAl") == "IDEAl")
  {
    modeIDEA->setChecked(true);
    return;
  }

  int mdi = config->readNumEntry("MDIMode", 1);

  switch (mdi)
  {
  case 2:
    modeTab->setChecked(true);
    break;
  case 0:
    modeToplevel->setChecked(true);
    break;
  default:
    modeMDI->setChecked(true);
    break;
  }
}


void UIChooserWidget::save()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  if (modeIDEA->isChecked())
  {
    config->writeEntry("MajorUIMode", "IDEAl");
  }
  else
  {
    config->writeEntry("MajorUIMode", "QextMDI");

    if (modeTab->isChecked())
      config->writeEntry("MDIMode", 2);
    else if (modeToplevel->isChecked())
      config->writeEntry("MDIMode", 0);
    else
      config->writeEntry("MDIMode", 1);
  }

  config->sync();
}


void UIChooserWidget::accept()
{
  save();
  Q_ASSERT(m_pMyPart);
  
  if (modeIDEA->isChecked()) { // immediate switch not supported yet 
  }
  else if (modeTab->isChecked()) {
      m_pMyPart->topLevel()->setUserInterfaceMode("TabPage");
  }
  else if (modeToplevel->isChecked()) {
      m_pMyPart->topLevel()->setUserInterfaceMode("Toplevel");
  }
  else if (modeMDI->isChecked()) {
      m_pMyPart->topLevel()->setUserInterfaceMode("Childframe");
  }
}


#include "uichooser_widget.moc"
