#ifndef _CORE_H_
#define _CORE_H_


#include <kparts/partmanager.h>


#include "kdevcore.h"


class Core : public KDevCore
{
  Q_OBJECT

public:

  static Core *getInstance();

  ~Core();

  virtual void gotoExecutionPoint(const QString &fileName, int lineNum=0);
  virtual void setBreakpoint(const QString &fileName, int lineNum,
                             int id, bool enabled, bool pending);
  virtual void running(KDevPart *which, bool runs);
  virtual void fillContextMenu(QPopupMenu *popup, const Context *context);
  virtual void message(const QString &str);
  virtual void openProject(const QString& projectFileName);

  
  void doEmitProjectOpened() { emit projectOpened(); }
  void doEmitProjectClosed() { emit projectClosed(); }
  void doEmitCoreInitialized() { emit coreInitialized(); }
  void doEmitProjectConfigWidget(KDialogBase *base) { emit projectConfigWidget(base); }
  void doEmitConfigWidget(KDialogBase *base) { emit configWidget(base); };
  

protected:

  Core();

  
private:

  static Core *s_instance;
  
};


#endif
