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

KDevCompilerGcc::KDevCompilerGcc(): KDevCompiler("gcc", "source_c"){
}

KDevCompilerGcc::~KDevCompilerGcc(){
}

QWidget* KDevCompilerGcc::initOptionsWidget(QWidget* parent, QWidget* pdlg){
	GccCompilerOptionsWidget* gccw = new GccCompilerOptionsWidget(parent, "Gcc Option Widget", pdlg);
  setOptionsWidget(*gccw);
  return gccw;
}

#include "KDevCompilerGcc.moc"
