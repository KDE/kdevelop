/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qfontmetrics.h>

#include <kdebug.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kparts/part.h>
#include <ktexteditor/viewcursorinterface.h>


#include "statusbar.h"
#include "partcontroller.h"


StatusBar::StatusBar(QWidget *parent, const char *name)
    : KStatusBar(parent, name), m_cursorIface(0), m_activePart(0)
{
  QLineEdit *le = new QLineEdit(this);
  //    le->setFont(KGlobalSettings::fixedFont());
  setFixedHeight(le->sizeHint().height());
  delete le;

  // stretcher
  addWidget(new QWidget(this), 1);

  _status = new QLabel(this);
  _status->setFont(KGlobalSettings::fixedFont());
  _status->setMinimumWidth(_status->fontMetrics().width("OVR, ro"));
  _status->setAlignment(QWidget::AlignCenter);
  addWidget(_status, 0, true);

  _cursorPosition = new QLabel(this);
  _cursorPosition->setAlignment(AlignCenter);
  // Same string as in setCursorPosition()
  QString s2 = i18n(" Line: %1 Col: %2 ").arg("xxxx").arg("xxxx");
  _cursorPosition->setMinimumWidth(_cursorPosition->fontMetrics().width(s2));
  addWidget(_cursorPosition, 0, true);

  _modified = new QLabel(this);
  _modified->setFixedWidth(_modified->fontMetrics().width("*"));
  addWidget(_modified, 0, true);

  setEditorStatusVisible(false);

  connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)),
          this, SLOT(activePartChanged(KParts::Part*)));
}


StatusBar::~StatusBar()
{}


void StatusBar::activePartChanged(KParts::Part *part)
{
  if (m_activePart)
    disconnect(m_activePart, 0, this, 0);

  m_activePart = part;
  m_cursorIface = 0;

  if (part && part->widget())
  {
    m_cursorIface = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
    if (m_cursorIface)
    {
      connect(part->widget(), SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));

      cursorPositionChanged();
    }
  }
  else
    _cursorPosition->setText("");
}


void StatusBar::cursorPositionChanged()
{
  if (m_cursorIface)
  {
    uint line, col;
    m_cursorIface->cursorPosition(&line, &col);
    setCursorPosition(line, col);
  }
}


void StatusBar::setEditorStatusVisible(bool visible)
{
  // Note: I tried to hide/show the widgets here, but that
  // causes flicker, so I just set them to be empty.

  if (!visible)
	{
	  _status->setText("");
	  _modified->setText("");
	  _cursorPosition->setText("");
	}
}


void StatusBar::setStatus(const QString &str)
{
  _status->setText(str);
}


void StatusBar::setCursorPosition(int line, int col)
{
  _cursorPosition->setText(i18n(" Line: %1 Col: %2 ").arg(line+1).arg(col));
}


void StatusBar::setModified(bool isModified)
{
  _modified->setText(isModified? "*" : "");
}

#include "statusbar.moc"
