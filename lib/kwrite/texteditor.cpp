#include <kdebug.h>
#include "texteditor.h"

TextEditorView::TextEditorView(TextEditorDocument *doc, QWidget *parent=0, const char *name=0)
    : KWrite(doc, parent, name, false, false)
{
    connect( this, SIGNAL(newCurPos()), this, SLOT(slotNewCurPos()) );
    connect( this, SIGNAL(newStatus()), this, SLOT(slotNewStatus()) );
}


TextEditorView::~TextEditorView()
{
    kdDebug(9000) << "Destroying text editor view" << endl;
}


TextEditorDocument *TextEditorView::editorDocument() const
{
    return static_cast<TextEditorDocument*>(document());
}


void TextEditorView::slotNewCurPos()
{
    emit cursorPositionChanged();
}


void TextEditorView::slotNewStatus()
{
    emit overwriteModeChanged();

    int state = undoState();
    emit undoAvailable(state & 1);
    emit redoAvailable(state & 2);
}


bool TextEditorView::isearch(const QString &str, bool again, bool wrap)
{
    SConfig s;
    if (wrap) {
        s.cursor.x = 0;
        s.cursor.y = 0;
    } else {
        s.cursor = kWriteView->cursor;
        if (again)
            s.cursor.x++;
    }
    s.startCursor = s.cursor;
    QString ncs = str;
    s.flags = KWrite::sfFromCursor|KWrite::sfPrompt|KWrite::sfAgain;
    s.setPattern(ncs);
    if (kWriteDoc->doSearch(s, str)) {
        PointStruc cursor = s.cursor;
        kWriteView->updateCursor(s.cursor);
        exposeFound(cursor, s.matchedLength, KWrite::ufUpdateOnScroll, false);
        return true;
    }
    return false;
}


TextEditorDocument::TextEditorDocument()
    : KWriteDoc(HlManager::self(), "")
{}


TextEditorDocument::~TextEditorDocument()
{
    kdDebug(9000) << "Destroying text editor document" << endl;
}


QList<TextEditorView> TextEditorDocument::editorViews()  const
{
    QList<TextEditorView> l;
    QList<KWrite> l2 = views;
    
    QListIterator<KWrite> it(l2);
    for (; it.current(); ++it)
        l.append(static_cast<TextEditorView*>(it.current()));
    
    return l;
}


QString TextEditorDocument::fileName() const
{
    return url().path();
}
#include "texteditor.moc"
