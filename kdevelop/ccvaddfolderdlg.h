/***************************************************************************
               cclassaddattributedlg.h  -  description

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


#ifndef CCVADDFOLDERDLG_H
#define CCVADDFOLDERDLG_H

#include <qdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>

/** */
class CCVAddFolderDlg : public QDialog
{
  Q_OBJECT

public: // Constructor & Destructor

  CCVAddFolderDlg( QWidget *parent=0, const char *name=0 );

public: // Public widgets

  QLabel folderLbl;
  QLineEdit folderEdit;

  QPushButton okBtn;
  QPushButton cancelBtn;

protected:
 protected slots:
  void OK();
 
private: // Private methods

  void setWidgetValues();
  void setCallbacks();
};

#endif

