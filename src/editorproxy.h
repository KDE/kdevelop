#ifndef __EDITORPROXY_H__
#define __EDITORPROXY_H__


#include <qobject.h>
#include <qarray.h>

class QPopupMenu;


#include <kparts/part.h>
#include <ktexteditor/markinterface.h>

class EditorProxy : public QObject
{
  Q_OBJECT

public:

  static EditorProxy *getInstance();

  void setLineNumber(KParts::Part *part, int lineNum);

  void removeBreakpoint(KParts::Part *part, int lineNum);
  void setBreakpoint(KParts::Part *part, int lineNum, bool enabled, bool pending);

  void clearExecutionPoint();
  void setExecutionPoint(KParts::Part *part, int lineNum);

  void installPopup(KParts::Part *part, QPopupMenu *popup);
 

private slots:

  void popupAboutToShow();

  void activePartChanged(KParts::Part *part);

  
private:
  enum MarkType {
    Bookmark           = KTextEditor::MarkInterface::markType01,
    ActiveBreakpoint   = KTextEditor::MarkInterface::markType02,
    ReachedBreakpoint  = KTextEditor::MarkInterface::markType03,
    InactiveBreakpoint = KTextEditor::MarkInterface::markType04,
    ExecutionPoint     = KTextEditor::MarkInterface::markType05
  };

  EditorProxy();
  
  static EditorProxy *s_instance;

  QArray<int> m_popupIds;

};


#endif
