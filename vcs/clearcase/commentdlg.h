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

#ifndef _CLEARCASECOMMENTDLG_H_
#define _CLEARCASECOMMENTDLG_H_

#include <qcheckbox.h>
#include <qdialog.h>
#include <q3multilineedit.h>

class CcaseCommentDlg : public QDialog
{
Q_OBJECT
public:
  CcaseCommentDlg(bool = FALSE);
  QString logMessage() { return _edit->text(); };
  bool isReserved() { return (_check) ?  _check->isChecked() : FALSE;  };

private:
  Q3MultiLineEdit *_edit;
  QCheckBox* _check;
};

#endif
