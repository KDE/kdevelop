#ifndef __CODECOMPLETION_IFACE_IMPL_H__
#define __CODECOMPLETION_IFACE_IMPL_H__


#include <qvaluelist.h>
#include "keditor/codecompletion_iface.h"

class QEditor;
class QVBox;
class QListBox;
class KDevArgHint;


class CodeCompletionDocumentIfaceImpl : public KEditor::CodeCompletionDocumentIface
{
    Q_OBJECT

public:

    CodeCompletionDocumentIfaceImpl(QEditor* edit, KEditor::Document *parent, KEditor::Editor *editor);
    virtual ~CodeCompletionDocumentIfaceImpl();
    static CodeCompletionDocumentIface *interface(KEditor::Document *doc);

    virtual void showArgHint ( QStringList functionList, const QString& strWrapping, const QString& strDelimiter );
    virtual void showCompletionBox(QValueList<KEditor::CompletionEntry> complList,int offset=0);

    bool eventFilter( QObject *o, QEvent *e );
    void updateBox( bool =FALSE );

signals:
    void completionAborted();
    void completionDone();
    void argHintHided();

private slots:
    void slotCursorPosChanged( int, int );


private:
    QEditor* m_edit;
    QVBox *m_completionPopup;
    QListBox *m_completionListBox;
    QValueList<KEditor::CompletionEntry> m_complList;
    int m_lineCursor;
    int m_colCursor;
    int m_offset;
    KDevArgHint* m_pArgHint;
};




#endif
