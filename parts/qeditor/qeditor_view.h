/* $Id$
 *
 *  Copyright (C) 2001 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */
#ifndef qeditor_view_h
#define qeditor_view_h


#include <ktexteditor/view.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/clipboardinterface.h>

class QPopupMenu;
#include <ktexteditor/popupmenuinterface.h>
#include <ktexteditor/codecompletioninterface.h>

class QEditor;
class QEditorPart;
class CodeCompletion_Impl;
class LineNumberWidget;
class MarkerWidget;
class LevelWidget;
class KoFindDialog;
class KoReplaceDialog;
class KoFind;
class KoReplace;
class QTextParag;

class QEditorView:
    public KTextEditor::View,
    public KTextEditor::ClipboardInterface,
    public KTextEditor::ViewCursorInterface,
    public KTextEditor::PopupMenuInterface,
    public KTextEditor::CodeCompletionInterface
{
    Q_OBJECT
public:
    QEditorView( QEditorPart*, QWidget*, const char* =0 );
    virtual ~QEditorView();

    KTextEditor::Document* document() const;
    QEditor* editor() const { return m_editor; }

    QString currentTextLine() const;
    void insertText( const QString& );

    QString language() const;

    bool isMarkerWidgetVisible() const;
    void setMarkerWidgetVisible( bool );

    bool isLineNumberWidgetVisible() const;
    void setLineNumberWidgetVisible( bool );

    bool isLevelWidgetVisible() const;
    void setLevelWidgetVisible( bool );

    int tabStop() const;
    void setTabStop( int );

public slots:
    void gotoLine();
    void setLanguage( const QString& );
    void doFind();
    void doReplace();
    void indent();
    void expandBlock( QTextParag* );
    void collapseBlock( QTextParag* );

protected slots:
    void highlight( const QString&, int, int, const QRect& );
    void replace( const QString&, int, int, int, const QRect& );

protected:
    void proceed();
    bool find_real( QTextParag* firstParag, int firstIndex,
                    QTextParag* lastParag, int lastIndex );
    bool process( const QString& );
    void doRepaint();

private:
    QTextParag* m_currentParag;
    KoFindDialog* m_findDialog;
    KoReplaceDialog* m_replaceDialog;
    KoFind* m_find;
    KoReplace* m_replace;
    long m_options;
    int m_offset;

// ViewCursorInterface ----------------------------------------------------------------------
public:
    /**
     * Get the current cursor coordinates in pixels.
     */
    virtual QPoint cursorCoordinates ();

    /**
     * Get the cursor position
     */
    virtual void cursorPosition (unsigned int *line, unsigned int *col);

    /**
     * Get the cursor position, calculated with 1 character per tab
     */
    virtual void cursorPositionReal (unsigned int *line, unsigned int *col);

    /**
     * Set the cursor position
     */
    virtual bool setCursorPosition (unsigned int line, unsigned int col);

    /**
     * Set the cursor position, use 1 character per tab
     */
    virtual bool setCursorPositionReal (unsigned int line, unsigned int col);

    virtual unsigned int cursorLine ();
    virtual unsigned int cursorColumn ();
    virtual unsigned int cursorColumnReal ();

signals:
    void cursorPositionChanged ();

// -- ClipboardInterface ----------------------------------------------------------------------
public slots:
    /**
     * copies selected text
     */
    virtual void copy ( ) const;

    /**
     * copies selected text
     */
    virtual void cut ( );

    /**
     * copies selected text
     */
    virtual void paste ( );

// -- PopupMenuInterface ----------------------------------------------------------------------
public:
    /**
       Install a Popup Menu. The Popup Menu will be activated on
       a right mouse button press event.
    */
    virtual void installPopup ( class QPopupMenu *rmb_Menu );

// -- CodeCompletionInterface ----------------------------------------------------------------------
    /**
     * This shows an argument hint.
     */
    virtual void showArgHint (QStringList functionList, const QString& strWrapping, const QString& strDelimiter);

    /**
     * This shows a completion list. @p offset is the real start of the text that
     * should be completed. <code>0</code> means that the text starts at the current cursor
     * position. if @p casesensitive is @p true, the popup will only contain completions
     * that match the input text regarding case.
     */
    virtual void showCompletionBox (QValueList<KTextEditor::CompletionEntry> complList,int offset=0, bool casesensitive=true);

signals:
    /**
     * This signal is emitted when the completion list disappears and no completion has
     * been done. This is the case e.g. when the user presses Escape.
     *
     * IMPORTANT: Please check if a connect to this signal worked, and implement some fallback
     * when the implementation doesn't support it
     *
     * IMPORTANT FOR IMPLEMENTERS: When you don't support this signal, please just override the inherited
     * function, if you support it, declare it as a signal
     */
    void completionAborted();

    /**
     * This signal is emitted when the completion list disappears and a completion has
     * been inserted into text. This is the case e.g. when the user presses Return
     * on a selected item in the completion list.
     *
     * IMPORTANT: Please check if a connect to this signal worked, and implement some fallback
     * when the implementation doesn't support it
     *
     * IMPORTANT FOR IMPLEMENTERS: When you don't support this signal, please just override the inherited
     * function, if you support it, declare it as a signal
     */
    void completionDone();

    /**
     * This signal is the same as @ref completionDone(), but additionally it carries
     * the information which completion item was used.
     *
     * IMPORTANT: Please check if a connect to this signal worked, and implement some fallback
     * when the implementation doesn't support it
     *
     * IMPORTANT: The pointer to the CompleteionEntry, is only valid in the slots connected to this signal
     * when the connected slots are left, the data element may be destroyed, depending on the implementation
     *
     * IMPORTANT FOR IMPLEMENTERS: When you don't support this signal, please just override the inherited
     * function, if you support it, declare it as a signal.
     *
     */
    void completionDone(KTextEditor::CompletionEntry);

    /**
     * This signal is emitted when the argument hint disappears.
     * This is the case e.g. when the user moves the cursor somewhere else.
     *
     * IMPORTANT: Please check if a connect to this signal worked, and implement some fallback
     * when the implementation doesn't support it
     *
     * IMPORTANT FOR IMPLEMENTERS: When you don't support this signal, please just override the inherited
     * function, if you support it, declare it as a signal
     */
    void argHintHidden();

    /**
     * This signal is emitted just before a completion takes place.
     * You can use it to modify the @ref CompletionEntry. The modified
     * entry will not be visible in the completion list (because that has
     * just disappeared) but it will be used when the completion is
     * inserted into the text.
     *
     * IMPORTANT: Please check if a connect to this signal worked, and implement some fallback
     * when the implementation doesn't support it
     *
     * IMPORTANT FOR IMPLEMENTERS: When you don't support this signal, please just override the inherited
     * function, if you support it, declare it as a signal
     */
    void filterInsertString(KTextEditor::CompletionEntry*,QString*);

private:
    QEditorPart* m_document;
    QEditor* m_editor;
    CodeCompletion_Impl* m_pCodeCompletion;
    LineNumberWidget* m_lineNumberWidget;
    MarkerWidget* m_markerWidget;
    LevelWidget* m_levelWidget;
};

#endif
