#ifndef _TEXTEDITOR_H
#define _TEXTEDITOR_H

#include <sys/stat.h>
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

    QList<TextEditorView> editorViews() const;
    QString fileName() const;
    bool modifiedOnDisk() const;
    /**
     * This method compares the _real_ identity of files. That means,
     * it returns true even if otherURL is just a symbolic link to
     * the edited file, or a different file name (which happens often
     * on NFS systems).
     */
    bool isEditing(const KURL &otherURL) const;
    bool openURL(const KURL &url);
    bool save();
    bool saveAs(const KURL &url);

private:
    dev_t dev;
    ino_t ino;
    time_t mtime;
};

#endif
