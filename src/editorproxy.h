#ifndef __EDITORPROXY_H__
#define __EDITORPROXY_H__


#include <qobject.h>
#include <qarray.h>

class QPopupMenu;


#include <kparts/part.h>


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
  void popupAboutToHide();
  void deletePopup();

  void activePartChanged(KParts::Part *part);

  
private:

  EditorProxy();
  
  static EditorProxy *s_instance;

  QPopupMenu *m_popup;
  QArray<int> m_popupIds;

};


#endif
