#ifndef __CPPCODECOMPLETION_H
#define __CPPCODECOMPLETION_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qstringlist.h>
#include <qvbox.h>
#include <qlistbox.h>

class CEditWidget;
class KDevArgHint;

class CompletionEntry {
public:
    QString type;
    QString text;
    QString prefix;
    QString postfix;
    QString comment;

    bool operator==( const CompletionEntry &c ) const {
        return ( c.type == type &&
                 c.text == text &&
                 c.postfix == postfix &&
                 c.prefix == prefix &&
                 c.comment == comment);
    }
};


class CppCodeCompletion : public QObject{
    Q_OBJECT
public:

    CppCodeCompletion( CEditWidget *edit );

    virtual void showArgHint ( QStringList functionList, const QString& strWrapping, const QString& strDelimiter );
    virtual void showCompletionBox(QValueList<CompletionEntry> complList,int offset=0);
    bool eventFilter( QObject *o, QEvent *e );

signals:
    void completionAborted();
    void completionDone();
    void argHintHided();

private:
    void updateBox( bool newCoordinate=false );
    KDevArgHint* m_pArgHint;
    CEditWidget *m_edit;
    QVBox *m_completionPopup;
    QListBox *m_completionListBox;
    QValueList<CompletionEntry> m_complList;
    int m_lineCursor;
    int m_colCursor;
    int m_offset;

public slots:
    void slotCursorPosChanged();
};




#endif
