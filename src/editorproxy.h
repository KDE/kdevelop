#ifndef __EDITORPROXY_H__
#define __EDITORPROXY_H__


#include <qobject.h>
#include <qpointer.h>

#include <kparts/part.h>
#include <kdeversion.h>

#include <q3widgetstack.h>

/**
Editor proxy for delayed loading of editor parts.
*/
class EditorWrapper : public Q3WidgetStack
{
  Q_OBJECT

public:
  EditorWrapper(KTextEditor::Document* editor, bool activate, QWidget* parent, const char* name = 0L);
  virtual ~EditorWrapper();

  KTextEditor::Document* document() const;

  void setLine(int line);
  void setCol(int col);

public slots:
  virtual void show();

protected:
  virtual void focusInEvent(QFocusEvent *ev);

private:
  QPointer<KTextEditor::Document> m_doc;
  QPointer<KTextEditor::View> m_view;
  int m_line;
  int m_col;
  bool m_first;
};

class EditorProxy : public QObject
{
  Q_OBJECT

public:

  static EditorProxy *getInstance();

  void setLineNumber(KParts::Part *part, int lineNum, int col);

  void installPopup(KParts::Part *part);

  void registerEditor(EditorWrapper* wrapper);
  void deregisterEditor(EditorWrapper* wrapper);

  QWidget * widgetForPart( KParts::Part * part );
  QWidget * topWidgetForPart( KParts::Part * part );

  bool isDelayedViewCapable();

private slots:

  void popupAboutToShow();
  void showPopup();

private:

  EditorProxy();

  static EditorProxy *s_instance;

  QVector<int> m_popupIds;

  // This list is used to save line/col information for not yet activated editor views.
  QList<EditorWrapper *> m_editorParts;

  bool m_delayedViewCreationCompatibleUI;
};


#endif
