/***************************************************************************
                          wizardbasepage.cpp  -  description
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
#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qwidget.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kstddirs.h>

#include "ckdevinstall.h"
#include "wizardbasepage.h"

WizardBasePage::WizardBasePage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState)
: QHBox(parent, name)
	,m_pInstallState(pInstallState)
{
  QWidget* box = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout(box);
  m_labelText = new QLabel(infoText, box);
  QLabel* labelPict = new QLabel(box);
  m_labelText->setMinimumHeight(50);

  QPixmap pm;

  if (pm.load(locate("appdata", installPictPathAndFilename))) {
    box->setFixedWidth(pm.width());
    labelPict->setBackgroundPixmap(pm);
    labelPict->setFixedSize(pm.size());
    m_labelText->setBackgroundColor(QColor(206,102,57));
    m_labelText->setFixedWidth(pm.width());
  }
  else {
    box->setFixedWidth(120);
    labelPict->setBackgroundColor(QColor(206,102,57));
    m_labelText->setBackgroundColor(QColor(206,102,57));
    labelPict->setFixedWidth(120);
    m_labelText->setFixedWidth(120);
  }

  setSpacing(8);
  m_labelText->setAlignment(AlignCenter | WordBreak);
  m_labelText->setIndent(8);

  if (layout != 0L) {
    layout->add( labelPict);
    layout->add( m_labelText);
  }
}

void WizardBasePage::setValid( bool state )
{
  m_valid = state;
  emit validData(this, state);
}

void WizardBasePage::setBackButtonEnabled( bool state)
{
  m_backButtonEnabled = state;
  emit enableBackButton(this, state);
}

void WizardBasePage::setHelpTextBackgroundColor( const QColor& bgcolor)
{
   m_labelText->setBackgroundColor( bgcolor);
}

#include "wizardbasepage.moc"
