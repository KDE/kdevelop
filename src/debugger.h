#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__


#include "kdevdebugger.h"

#include <kdeversion.h>

#include <kparts/part.h>
#include <ktexteditor/markinterface.h>
#if (KDE_VERSION > 305)
# include <ktexteditor/markinterfaceextension.h>
#else
# include "kde30x_markinterfaceextension.h"
#endif

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
  
private slots:

  void partAdded( KParts::Part* part );
  void markChanged( KTextEditor::Mark, KTextEditor::MarkInterfaceExtension::MarkChangeAction );

private:
  enum MarkType {
    Bookmark           = KTextEditor::MarkInterface::markType01,
    Breakpoint         = KTextEditor::MarkInterface::markType02,
    ActiveBreakpoint   = KTextEditor::MarkInterface::markType03,
    ReachedBreakpoint  = KTextEditor::MarkInterface::markType04,
    DisabledBreakpoint = KTextEditor::MarkInterface::markType05,
    ExecutionPoint     = KTextEditor::MarkInterface::markType06
  };

  static Debugger *s_instance;
  
};


#endif
