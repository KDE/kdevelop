#ifndef _API_H_
#define _API_H_


#include "kdevapi.h"


class ClassStore;
class KDevDebugger;


class API : public KDevApi
{
public:

  virtual KDevTopLevel *topLevel();
  virtual KDevPartController *partController();
  virtual KDevCore *core();
  virtual ClassStore *classStore();
  virtual ClassStore *ccClassStore();
  virtual KDevDebugger *debugger();

  
  static API *getInstance();

  ~API();

  
protected:

  API();

  
private:

  static API *s_instance;

  ClassStore *m_classStore, *m_ccClassStore;

};


#endif
