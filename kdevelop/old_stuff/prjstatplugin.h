
#ifndef PRJSTATPLUGIN_H
#define PRJSTATPLUGIN_H

#include <kdevplugin.h>

/**
  *@author Sandy Meier
  */

class PrjStatPlugin : public KDevPlugin {

Q_OBJECT

 public: 
    PrjStatPlugin();
    ~PrjStatPlugin();
    virtual void start();
    virtual void stop();
   public slots:	 
    virtual void slotMenuActivated();
};

#endif
