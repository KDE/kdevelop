#ifndef __EDITORPROXY_H__
#define __EDITORPROXY_H__


#include <qobject.h>
#include <qmemarray.h>

class QPopupMenu;

#include <kparts/part.h>
#include <ktexteditor/markinterface.h>
#include <kdeversion.h>
#if (KDE_VERSION > 305)
# include <ktexteditor/markinterfaceextension.h>
#else
# include "kde30x_markinterfaceextension.h"
#endif

class EditorProxy : public QObject
{
  Q_OBJECT

public:

  static EditorProxy *getInstance();

  void setLineNumber(KParts::Part *part, int lineNum, int col);

  void installPopup(KParts::Part *part);
  
private slots:

  void popupAboutToShow();
  void showPopup();

private:

  EditorProxy();

  static EditorProxy *s_instance;

  QMemArray<int> m_popupIds;

};


#endif
