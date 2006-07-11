#ifndef __EDITORPROXY_H__
#define __EDITORPROXY_H__


#include <qobject.h>
#include <qmemarray.h>
#include <qguardedptr.h>

class QPopupMenu;
class MultiBuffer;

#include <kparts/part.h>
#include <ktexteditor/markinterface.h>
#include <kdeversion.h>
# include <ktexteditor/markinterfaceextension.h>

class EditorProxy : public QObject
{
  Q_OBJECT

public:

  static EditorProxy *getInstance();

  void setLineNumber(KParts::Part *part, int lineNum, int col);

  void installPopup(KParts::Part *part);

  void registerEditor(MultiBuffer* wrapper);
  void deregisterEditor(MultiBuffer* wrapper);

  QWidget * widgetForPart( KParts::Part * part );
  QWidget * topWidgetForPart( KParts::Part * part );

  bool isDelayedViewCapable();

  QWidget * findPartWidget( KParts::Part * part );

private slots:

  void popupAboutToShow();
  void showPopup();

private:

  EditorProxy();

  static EditorProxy *s_instance;

  QMemArray<int> m_popupIds;

  // This list is used to save line/col information for not yet activated editor views.
  QValueList< MultiBuffer* > m_editorParts;

  bool m_delayedViewCreationCompatibleUI;
};


#endif
