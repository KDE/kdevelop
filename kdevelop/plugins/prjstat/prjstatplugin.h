
#ifndef PRJSTATPLUGIN_H
#define PRJSTATPLUGIN_H

#include <kdevplugin.h>

/**
  *@author 
  */

class PrjStatPlugin : public KDevPlugin  {
 public: 
    PrjStatPlugin();
    ~PrjStatPlugin();
    virtual void start();
    virtual void stop();
};

#endif
