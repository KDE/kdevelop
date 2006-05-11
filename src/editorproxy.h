#ifndef __EDITORPROXY_H__
#define __EDITORPROXY_H__


#include <QObject>
#include <qpointer.h>

#include <kparts/part.h>
#include <kdeversion.h>

#include <q3widgetstack.h>
#include "shellexport.h"


class KDEVSHELL_EXPORT EditorProxy : public QObject
{
  Q_OBJECT

public:

  static EditorProxy *getInstance();

  void setCursorPosition(KParts::Part *part, const KTextEditor::Cursor& cursor);

  void installPopup(KParts::Part *part);

  QWidget * widgetForPart( KParts::Part * part );
  QWidget * topWidgetForPart( KParts::Part * part );

private slots:
  void popupAboutToShow(KTextEditor::View* view, QMenu* menu);

private:

  EditorProxy();

  static EditorProxy *s_instance;

  //QVector<int> m_popupIds;
};


#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
