/***************************************************************************
 *   Copyright (C) 2003 by Ajay Guleria                                    *
 *   ajay_guleria at yahoo dot com                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "commentdlg.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdguiitem.h>

CcaseCommentDlg::CcaseCommentDlg(bool bCheckin)
  : QDialog(0, "", true)
{
  setCaption( i18n("Clearcase Comment") );

  QBoxLayout *layout = new QVBoxLayout(this, 10);

  QLabel *messagelabel = new QLabel(i18n("Enter log message:"), this);
  messagelabel->setMinimumSize(messagelabel->sizeHint());
  layout->addWidget(messagelabel, 0);

  _edit = new QMultiLineEdit(this);
  QFontMetrics fm(_edit->fontMetrics());
  _edit->setMinimumSize(fm.width("0")*40, fm.lineSpacing()*3);
  layout->addWidget(_edit, 10);

  QBoxLayout *layout2 = new QHBoxLayout(layout);
  if(bCheckin) {
    _check = new QCheckBox("Reserve", this);
    layout2->addWidget(_check);
  }

  KButtonBox *buttonbox = new KButtonBox(this);
  buttonbox->addStretch();
#if KDE_IS_VERSION( 3, 2, 90 )
  QPushButton *ok = buttonbox->addButton(KStdGuiItem::ok());
  QPushButton *cancel = buttonbox->addButton(KStdGuiItem::cancel());
#else
  QPushButton *ok = buttonbox->addButton(i18n("OK"));
  QPushButton *cancel = buttonbox->addButton(i18n("Cancel"));
#endif
  connect(ok, SIGNAL(clicked()), SLOT(accept()) );
  connect(cancel, SIGNAL(clicked()), SLOT(reject()) );
  ok->setDefault(true);
  buttonbox->layout();
  layout2->addWidget(buttonbox, 0);

  layout->activate();
  adjustSize();
}



#include "commentdlg.moc"
