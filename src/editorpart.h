/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _EDITORPART_H_
#define _EDITORPART_H_

#include <kparts/part.h>

class TextEditorView;
class TextEditorDocument;
class QBoxLayout;
class QFrame;
class QHBox;
class QLabel;
class QPopupMenu;


class EditorPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
    
public:
    EditorPart( QWidget *parent=0, const char *name=0 );
    ~EditorPart();

    void gotoDocument(TextEditorDocument *doc, int lineNum);
    TextEditorView *editorView() const;
    TextEditorDocument *editorDocument() const;

signals:
    void contextMenu(QPopupMenu *popup, const QString &linestr, int col);
    void wentToSourceFile(const QString &fileName);

private slots:
    void updateUndoAvailable(bool yes);
    void updateRedoAvailable(bool yes);
    void updateOverwriteMode();
    void updateCursorPosition();
    void popupMenu(int line, int col);
    
    void setWindowCaption(const QString &caption);
    void slotUndo();
    void slotRedo();
    void slotUndoHistory();
    void slotCut();
    void slotCopy();
    void slotPaste();
    void slotFind();
    void slotFindAgain();
    void slotReplace();

private:
    virtual bool eventFilter(QObject *o, QEvent *e);
    bool openFile();

    bool isearching, isearchFailed, isearchWrapped;
    QString searchText;
    
    TextEditorView *view;
    QBoxLayout *layout;
    QFrame *statusbox;
    QLabel *locationLabel;
    QLabel *overwriteLabel;
    QLabel *cursorposLabel;
};

#endif
