/***************************************************************************
                          clibpropdlgimpl.h  -  description
                             -------------------
    begin                : Fri Mar 16 2001
    copyright            : (C) 2001 by kdevelop-team
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

#ifndef CLIBPROPDLGIMPL_H
#define CLIBPROPDLGIMPL_H

#include <clibpropdlg.h>

struct TMakefileAmInfo;

/**
* Dialog to allow user to change the library type and enter
* details for that library to be written to the Makefile.am
*
* The dialog required the makefileAm structure from CProject to be passed
* This stucture is modified if the user oks the dialog and is untouched if the
* dialog is canceled
*
* @author kdevelop-team
*/

class CLibPropDlgImpl : public CLibPropDlg
{
  Q_OBJECT

public: 
  CLibPropDlgImpl(TMakefileAmInfo* info, QWidget *parent=0, const char *name=0);
  ~CLibPropDlgImpl();

public slots:
  void accept();
  void slotLibChanged(int rbId);

private:

  /** This stucture (containing the lib details) is passed in and
  * if the dialog is OK'ed then the structure is updated for the caller.
  */
  TMakefileAmInfo* m_info;
};

#endif
