/***************************************************************************
                          KDevLinkerLd.cpp  -  description
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

#include "KDevLinkerLd.h"
#include "LdLinkerOptionsWidget.h"
#include "kdebug.h"

KDevLinkerLd::KDevLinkerLd(): KDevLinker("ld", "blockdevice"){
}

KDevLinkerLd::~KDevLinkerLd(){
}

QWidget* KDevLinkerLd::initOptionsWidget(QWidget* parent, QWidget* pdlg){
	LdLinkerOptionsWidget* ldw = new LdLinkerOptionsWidget(parent, "Ld Options Widget", pdlg);
	ldw->setLinker(this);
  setOptionsWidget(*ldw);
  return ldw;
}
