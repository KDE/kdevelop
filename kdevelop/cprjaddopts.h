/***************************************************************************
                          cprjaddopts.h  -  description
                             -------------------
    begin                : Wed Feb 19 2002
    copyright            : (C) 2002 by W. Tasin
    email                : tasin@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CPRJADDOPTS_H
#define CPRJADDOPTS_H

#include <qwidget.h>
#include "cprjaddoptsdlg.h"
#include "cprjconfchange.h"

class CPrjAddOpts : public CPrjAddOptsDlg  
{
   Q_OBJECT

public: 
   CPrjAddOpts(CPrjConfChange *confIn, QWidget *parent=0, const char *name=0);
  ~CPrjAddOpts();

  void initGUI();
  
  bool changed() const;
  void modifyConfigureIn();
  

private:
  CPrjConfChange *configureIn;
  QString old_KDE_QT_ver;
  bool exceptionsEnabled, nooptEnabled, miscTestsEnabled,
       flexEnabled, yaccEnabled;

};

#endif
