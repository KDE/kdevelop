#ifndef _TEXTEDITOR_H
#define _TEXTEDITOR_H

#include "kwview.h"
#include "kwdoc.h"


class TextEditorDocument;


class TextEditorView : public KWrite
{
    Q_OBJECT
    
public:
    TextEditorView(TextEditorDocument *doc, QWidget *parent, const char *name);
    ~TextEditorView();

    TextEditorDocument *editorDocument() const;
    bool isearch(const QString &str, bool again, bool wrap);
    
signals:
    void cursorPositionChanged();
    void popupMenu(int line, int col);
    void overwriteModeChanged();
    void undoAvailable(bool yes);
    void redoAvailable(bool yes);
    
private slots:
    void slotNewCurPos();
    void slotNewStatus();
};


class TextEditorDocument : public KWriteDoc
{
    Q_OBJECT
    
public:
    TextEditorDocument();
    ~TextEditorDocument();

    QString fileName() const;
    QList<TextEditorView> editorViews() const;
};

#endif
