
#include <qmessagebox.h>
#include "prjstatplugin.h"
#include <iostream.h>

extern "C" {
  PrjStatPlugin* create();
}

PrjStatPlugin* create(){
  return(new PrjStatPlugin());
}


PrjStatPlugin::PrjStatPlugin(){
    name = i18n("Project Statistic Plugin (example)");
    email = i18n("kdevelop-team@barney.cs.uni-potsdam.de");
    author = i18n("KDevelop Team");
    version = i18n("v0.1");
    homepage = i18n("http://www.kdevelop.org");
    copyright = i18n("(C) 1999, published under GPL");
    description = i18n("This plugin display some information about the current\n project."); 
}
PrjStatPlugin::~PrjStatPlugin(){
}
void PrjStatPlugin::start(){
  KDevPlugin::start();
  cerr << "\nSTART";

  plugin_menu->insertItem( "MenuPlugin", this, SLOT( slotMenuActivated() ) );
}

void PrjStatPlugin::stop(){
 KDevPlugin::stop();
  cerr << "\nSTOP";
}
void PrjStatPlugin::slotMenuActivated() {
QMessageBox::information( kdev, "Test1", "test2" );
}          
