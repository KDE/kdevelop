/***************************************************************************
                          cmakemanualdlg.cpp  -  description                              
                             -------------------                                         
    begin                : Wed Jul 14 1999                                           
    copyright            : (C) 1999 by The KDevelop Team                         
    email                :                                      
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "cmakemanualdlg.h"

CMakeManualDlg::CMakeManualDlg(QWidget *parent, const char *name) : QDialog(parent,name,true){
	initDialog();
}

CMakeManualDlg::~CMakeManualDlg(){
}
