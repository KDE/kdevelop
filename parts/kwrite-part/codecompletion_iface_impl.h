#ifndef __CODECOMPLETION_IFACE_IMPL_H__
#define __CODECOMPLETION_IFACE_IMPL_H__


#include "keditor/codecompletion_iface.h"
#include <qvaluelist.h>
#include <qvbox.h>
#include <qlistbox.h>

class KWrite;
class KDevArgHint;

class CodeCompletionIfaceImpl : public KEditor::CodeCompletionDocumentIface {
  Q_OBJECT
    public:
  
  CodeCompletionIfaceImpl(KWrite *edit,KEditor::Document *parent, KEditor::Editor *editor);
 
  virtual void showArgHint ( QValueList <QString> functionList, QString strWrapping, QString strDelimiter );
  virtual void showCompletionBox(QValueList<KEditor::CompletionEntry> complList,int offset=0);
  bool eventFilter( QObject *o, QEvent *e );
  
private:
  void updateBox(bool newCoordinate=false);
  KDevArgHint* m_pArgHint;
  KWrite *m_edit;
  QVBox *m_completionPopup;
  QListBox *m_completionListBox;
  QValueList<KEditor::CompletionEntry> m_complList;
  int m_lineCursor;
  int m_colCursor;
  int m_offset;

public slots:
	void slotCursorPosChanged();
};
  
  


#endif
