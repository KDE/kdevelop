#ifndef __EDITORPROXY_H__
#define __EDITORPROXY_H__


#include <qobject.h>
#include <qarray.h>

class QPopupMenu;

#include <kparts/part.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/markinterfaceextension.h>

class EditorProxy : public QObject
{
  Q_OBJECT

public:

  static EditorProxy *getInstance();

  void setLineNumber(KParts::Part *part, int lineNum);

  void installPopup(KParts::Part *part, QPopupMenu *popup);
 
private slots:

  void popupAboutToShow();

private:
  
  EditorProxy();
  
  static EditorProxy *s_instance;

  QArray<int> m_popupIds;

};


#endif
