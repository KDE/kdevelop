#ifndef _API_H_
#define _API_H_


#include "kdevapi.h"


class API : public KDevApi
{
public:

  static void createInstance();
  static API *getInstance();

  ~API();

  
protected:

  API();

  
private:

  static API *s_instance;

};


#endif
