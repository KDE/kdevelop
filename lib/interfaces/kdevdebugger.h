#ifndef _KDEVDEBUGGER_H_
#define _KDEVDEBUGGER_H_


#include <qobject.h>
#include <qpixmap.h>


#include <kurl.h>


class KDevDebugger : public QObject
{
  Q_OBJECT
    
public:
    
  KDevDebugger(QObject *parent=0, const char *name=0);
  ~KDevDebugger();

  /**
   * Sets a breakpoint in the editor document belong to fileName.
   * If id==-1, the breakpoint is deleted.
   */
  virtual void setBreakpoint(const QString &fileName, int lineNum,
                             int id, bool enabled, bool pending) = 0;
   
  /**
   * Goes to a given location in a source file and marks the line.
   * This is used by the debugger to mark the location where the
   * the debugger has stopped.
   */
  virtual void gotoExecutionPoint(const KURL &url, int lineNum=0) = 0;

  /**
   * Clear the execution point. Usefull if debugging has ended.
   */
  virtual void clearExecutionPoint() = 0;

  static const QPixmap* inactiveBreakpointPixmap();
  static const QPixmap* activeBreakpointPixmap();
  static const QPixmap* reachedBreakpointPixmap();
  static const QPixmap* disabledBreakpointPixmap();
  static const QPixmap* executionPointPixmap();
  
signals:

  /**
   * The user has toggled a breakpoint.
   */
  void toggledBreakpoint(const QString &fileName, int lineNum);

  /*
   * The user wants to edit the properties of a breakpoint.
   */
  void editedBreakpoint(const QString &fileName, int lineNum);
  
  /**
   * The user wants to enable/disable a breakpoint.
   */
  void toggledBreakpointEnabled(const QString &fileName, int lineNum);
    
};


#endif
