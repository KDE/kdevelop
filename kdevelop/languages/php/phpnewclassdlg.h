/***************************************************************************
                          phpnewclassdlg.h  -  description
                             -------------------
    begin                : Sat Aug 11 2001
    copyright            : (C) 2001 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PHPNEWCLASSDLG_H
#define PHPNEWCLASSDLG_H

#include <qwidget.h>
#include "phpnewclassdlgbase.h"

/**
  *@author Sandy Meier
  */

class PHPNewClassDlg : public PHPNewClassDlgBase  {
   Q_OBJECT
public: 
   PHPNewClassDlg(const QStringList& baseClassNames,const QString& directory,QWidget *parent=0, const char *name=0);
   ~PHPNewClassDlg();
   protected slots:
     void classNameTextChanged(const QString&);
     void fileNameTextChanged(const QString&);
     void accept();
     void slotDirButtonClicked();
 protected:
   bool m_filenameModified;
};

#endif
