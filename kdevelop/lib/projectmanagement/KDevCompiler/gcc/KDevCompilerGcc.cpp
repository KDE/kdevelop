/***************************************************************************
                          KDevCompilerGCC.cpp  -  description
                             -------------------
    begin                : Thu Feb 1 2001
    copyright            : (C) 2001 by Omid Givi
    email                : omid@givi.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KDevCompilerGcc.h"
#include "GccCompilerOptionsWidget.h"
#include "kdebug.h"


void KDevCompiler::initGcc(void){
  setName("gcc");
  setIcon("source_c");
}

QWidget* KDevCompiler::initGccOptionsWidget(QWidget* parent, const char* name, QWidget* pdlg){
	GccCompilerOptionsWidget* gccw = new GccCompilerOptionsWidget(parent, name, pdlg);
  KDevCompiler::setOptionsWidget(*gccw);
	
}
