/***************************************************************************
                          testprj.cpp - testframework for projectmanagment
                             -------------------
    begin                : Fri Aug 4 2000
    copyright            : (C) 2000 by Sandy Meier (KDevelop Team)
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// yes, many paths are hardcoded, but this is a testframework  
// and I commited it, so Makefile.am changing all the time isn't needed.

#include "projectspace.h"
#include "ctoolclass.h"
#include "project.h"
#include <kapp.h>
#include <kcmdlineargs.h>
#include <iostream.h>
#include <filepropspage.h>

int main(int argc, char* argv[]){
  KCmdLineArgs::init( argc, argv,"testprj","","");
  KApplication a;
  FilePropsPage* page = new FilePropsPage();
  page->show();
  
  ProjectSpace* space = new ProjectSpace();
    space->setName("SPACE");
  space->setAbsolutePath("/home/smeier/testprj/jk");

  Project* prj = new Project();
  prj->setName("PROJECT");
  prj->setAbsolutePath("/home/smeier/testprj/jk/tetris");

  Project* prj1 = new Project();

  prj->addFile("/home/smeier/main.cpp");
  space->addProject(prj);
  space->writeConfig();
  
  space->readConfig("/home/smeier/testprj/jk/SPACE.kdevpsp");
  

  // test some function in ProjectSpace
  QString rel_path = CToolClass::getRelativePath("/home/smeier/","/usr/bin/");
  cerr << endl << "ABS:" << CToolClass::getAbsolutePath("/home/smeier/",rel_path) << endl;
  
  a.setMainWidget(page);
  a.exec();
}
