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
#include <kmdidefines.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>

#include "uichooser_part.h"
#include "kdevmainwindow.h"
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

  int mdi = config->readNumEntry("MDIMode", KMdi::ChildframeMode);
  
  switch (mdi)
  {
  case KMdi::ChildframeMode:
    modeMDI->setChecked(true);
    break;
  case KMdi::TabPageMode:
    modeTab->setChecked(true);
    break;
  case KMdi::ToplevelMode:
    modeToplevel->setChecked(true);
    break;
  case KMdi::IDEAlMode:
    modeIDEAl->setChecked(true);
    break;
  default:
    break;
  }
}


void UIChooserWidget::save()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  config->writeEntry("MajorUIMode", "QextMDI");

  if (modeTab->isChecked())
    config->writeEntry("MDIMode", KMdi::TabPageMode);
  else if (modeToplevel->isChecked())
    config->writeEntry("MDIMode", KMdi::ToplevelMode);
  else if (modeMDI->isChecked())
    config->writeEntry("MDIMode", KMdi::ChildframeMode);
  else
    config->writeEntry("MDIMode", KMdi::IDEAlMode); // KMdi-IDEA

  config->sync();
}


void UIChooserWidget::accept()
{
  save();
  Q_ASSERT(m_pMyPart);
  
  if (modeIDEAl->isChecked()) {
      m_pMyPart->mainWindow()->setUserInterfaceMode("KMDI-IDEAl");
  }
  else if (modeTab->isChecked()) {
      m_pMyPart->mainWindow()->setUserInterfaceMode("TabPage");
  }
  else if (modeToplevel->isChecked()) {
      m_pMyPart->mainWindow()->setUserInterfaceMode("Toplevel");
  }
  else if (modeMDI->isChecked()) {
      m_pMyPart->mainWindow()->setUserInterfaceMode("Childframe");
  }
}


#include "uichooser_widget.moc"
