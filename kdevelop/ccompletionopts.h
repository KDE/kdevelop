/***************************************************************************
                          ccompletionopts.h  -  description
                             -------------------
    begin                : Mon Feb 27 2002
    copyright            : (C) 2002 by Roberto Raggi
    email                : raggi@cli.di.unipi.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CCOMPLETIONOPTS_H
#define CCOMPLETIONOPTS_H

#include "ccompletionoptsdlg.h"

class CKDevelop;

/**
  *@author Roberto Raggi
  */

class CCompletionOpts : public CCompletionOptsDlg  {
    Q_OBJECT
public:
    CCompletionOpts(CKDevelop*, QWidget *parent=0, const char *name=0);
    ~CCompletionOpts();

public slots:
    void slotSettingsChanged();

private:
    CKDevelop* m_pDevelop;
};

#endif
