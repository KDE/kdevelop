/***************************************************************************
                          KDevCompilerGpp.cpp  -  description
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

#include "KDevCompilerGpp.h"
#include "GppCompilerOptionsWidget.h"
#include "kdebug.h"


KDevCompilerGpp::KDevCompilerGpp(): KDevCompiler("c++", "source_cpp"){
}

KDevCompilerGpp::~KDevCompilerGpp(){
}

QWidget* KDevCompilerGpp::initOptionsWidget(QWidget* parent, QWidget* pdlg){
	GppCompilerOptionsWidget* gppw = new GppCompilerOptionsWidget(parent, "Gpp Options Widget", pdlg);
	gppw->setCompiler(this);
  setOptionsWidget(*gppw);
  return gppw;
}
