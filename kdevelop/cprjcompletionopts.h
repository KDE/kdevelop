/***************************************************************************
                          cprjcompletionopts.h  -  description
                             -------------------
    begin                : Mon Feb 25 2002
    copyright            : (C) 2002 by kdevelop-team
    email                : kdevelop-team@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CPRJCOMPLETIONOPTS_H
#define CPRJCOMPLETIONOPTS_H

#include "cprjcompletionoptsdlg.h"

class CProject;

/**
  *@author kdevelop-team
  */

class CPrjCompletionOpts : public CPrjCompletionOptsDlg  {
   Q_OBJECT
public: 
	CPrjCompletionOpts(CProject*, QWidget *parent=0, const char *name=0);
	~CPrjCompletionOpts();

public slots:
  void slotSettingsChanged();

private:
  CProject* m_pProject;
};

#endif
