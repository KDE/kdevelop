#ifndef _DBGMANAGER_H_
#define _DBGMANAGER_H_

#include "kdevcomponent.h"

class BreakpointManager;

class DbgManager : public KDevComponent
{
  Q_OBJECT

public:
  DbgManager( QObject *parent=0, const char *name=0 );
  virtual ~DbgManager();

protected:
  virtual void setupGUI();
  virtual void compilationAborted();
  virtual void projectSpaceClosed();
  virtual void projectSpaceOpened(ProjectSpace *pProjectSpace);
  
private slots:
  void slotDebugStart();
  void slotDebugExamineCore();
  void slotDebugNamedFile();
  void slotDebugAttach();
  void slotDebugExecuteWithArgs();
  void slotDebugRun();
  void slotDebugRunToCursor();
  void slotDebugStepOver();
  void slotDebugStepOverInstr();
  void slotDebugStepInto();
  void slotDebugStepIntoInstr();
  void slotDebugStepOut();
  void slotDebugInterrupt();
  void slotDebugStop();

private:
  BreakpointManager* m_BPManager;
};

#endif
