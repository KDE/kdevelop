/***************************************************************************
               cclassaddmethoddlg.h  -  description

                             -------------------

    begin                : Fri Mar 19 1999

    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef CADDCLASSMETHODDLG_H
#define CADDCLASSMETHODDLG_H

#include <qdialog.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qmultilinedit.h>
#include "./classparser/ParsedMethod.h"

/** Dialog to create a new method for a class. */
class CAddClassMethodDlg : public QDialog
{
  Q_OBJECT
public: // Constructor & Destructor

  CAddClassMethodDlg( QWidget *parent=0, const char *name=0 );

public: // Public queries

  CParsedMethod *asSystemObj();

protected: // Private widgets

  QButtonGroup modifierGrp;
  QButtonGroup functionGrp;
  QButtonGroup accessGrp;

  QLabel typeLbl;
  QLineEdit typeEdit;
  QLabel declLbl;
  QLineEdit declEdit;
  QLabel docLbl;
  QMultiLineEdit docEdit;

  QRadioButton publicRb;
  QRadioButton protectedRb;
  QRadioButton privateRb;

  QCheckBox virtualCb;
  QCheckBox staticCb;
  QCheckBox constCb;

  QPushButton okBtn;
  QPushButton cancelBtn;

 protected slots:

  void OK();

private: // Private methods

  void setWidgetValues();
  void setCallbacks();

};

#endif


