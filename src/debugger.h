#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__


#include "kdevdebugger.h"


class Debugger : public KDevDebugger
{
  Q_OBJECT

public:

  static Debugger *getInstance();
  
  void setBreakpoint(const QString &fileName, int lineNum,
                     int id, bool enabled, bool pending);

  void gotoExecutionPoint(const KURL &url, int lineNum=-1);
  void clearExecutionPoint();


protected:

  Debugger();
  ~Debugger();


private:

  static Debugger *s_instance;
  
};


#endif
