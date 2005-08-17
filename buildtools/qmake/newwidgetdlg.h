/***************************************************************************
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jsgaarde@tdcspace.dk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NEWWIDGETDLG_H
#define NEWWIDGETDLG_H

#include "newwidgetdlgbase.h"
#include "trollprojectwidget.h"

class QStringList;
class QDomDocument;

class NewWidgetDlg : public NewWidgetDlgBase
{
public:
                        NewWidgetDlg(QStringList &newFileNames,QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
                        ~NewWidgetDlg();


public slots:
  virtual void          subclassingPressed();
  virtual void          templateSelChanged();
  virtual void          accept();

protected:
  QStringList           &m_newFileNames;
};

#endif

