/***************************************************************************
                          GccCompiler.h  -  description
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

#ifndef KDEVCOMPILERGCC_H
#define KDEVCOMPILERGCC_H

#include <KDevCompiler.h>
/**
  *@author Omid Givi
  */

class KDevCompilerGcc : public KDevCompiler{
	
public:
	KDevCompilerGcc();
	~KDevCompilerGcc();

  QWidget* initOptionsWidget(QWidget* parent=0, QWidget* pdlg=0);
};



#endif
