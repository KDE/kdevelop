
#include <qmessagebox.h>

#include "prjstatplugin.h"
#include <iostream.h>


PrjStatPlugin::PrjStatPlugin(){
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
