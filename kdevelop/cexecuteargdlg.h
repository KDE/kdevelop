/***************************************************************************
                          cexecuteargdlg.h  -  description                              
                             -------------------                                         
    begin                : Tue Mar 30 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    email                : Ralf.Nolden@post.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef CEXECUTEARGDLG_H
#define CEXECUTEARGDLG_H

#include <qdialog.h>

class QLineEdit;

/**gets the commandline arguments 
  *@author Ralf Nolden
  */

class CExecuteArgDlg : public QDialog  {
  Q_OBJECT

public:
  CExecuteArgDlg(QWidget *parent=0, const char *name=0,QString titel = "",QString args="");
  ~CExecuteArgDlg();
    
  QString getArguments()  {return arguments;}
    
public slots:
  void slotTextChanged(const QString& t);
  void slotSetArg();
  void slotClose();
    
//protected:
//    CProject* prj;

private:
  QLineEdit *edit;
  QString arguments;
};

#endif
