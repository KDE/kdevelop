/*
  $Id$

   Copyright (C) 1998, 1999 Jochen Wilhelmy
                            digisnap@cs.tu-berlin.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _KWDOC_H_
#define _KWDOC_H_

#include <qobject.h>
#include <qlist.h>
#include <qcolor.h>
#include <qfont.h>
#include <qfontmetrics.h>

#include <kparts/browserextension.h>

#include "kwview.h"
#include "highlight.h"
#include "kidetexteditor.h"

/**
  The TextLine represents a line of text. A text line that contains the
  text, an attribute for each character, an attribute for the free space
  behind the last character and a context number for the syntax highlight.
  The attribute stores the index to a table that contains fonts and colors
  and also if a character is selected.
*/
class TextLine {
  public:
    /**
      Creates an empty text line with given attribute and syntax highlight
      context
    */
    TextLine(int attribute = 0, int context = 0);
    ~TextLine();

    /**
      Returns the length
    */
    int length() const {return len;}
    /**
      Universal text manipulation method. It can be used to insert, delete
      or replace text.
    */
    void replace(int pos, int delLen, const QChar *insText, int insLen,
      uchar *insAttribs = 0L);

    /**
      Appends a string of length l to the textline
    */
    void append(const QChar *s, int l) {replace(len, 0, s, l);}
    /**
      Wraps the text from the given position to the end to the next line
    */
    void wrap(TextLine *nextLine, int pos);
    /**
      Wraps the text of given length from the beginning of the next line to
      this line at the given position
    */
    void unWrap(int pos, TextLine *nextLine, int len);
    /**
      Truncates the textline to the new length
    */
    void truncate(int newLen) {if (newLen < len) len = newLen;}
    /**
      Returns the position of the first character which is not a white space
    */
    int firstChar() const;
    /**
      Returns the position of the last character which is not a white space
    */
    int lastChar() const;
    /**
      Removes trailing spaces
    */
    void removeSpaces();
    /**
      Gets the char at the given position
    */
    QChar getChar(int pos) const;
    /**
      Gets the text. WARNING: it is not null terminated
    */
    QChar *getText() const {return text;}
    /**
      Gets a C-like null terminated string
    */
    const QChar *getString();
    /**
      Returns the x position of the cursor at the given position, which
      depends on the number of tab characters
    */
    int cursorX(int pos, int tabChars) const;
    /**
      Is the line starting with the given string
    */
    bool startingWith(QString& match);
    /**
      Is the line ending with the given string
    */
    bool endingWith(QString& match);

    /**
      Sets the attributes from start to end -1
    */
    void setAttribs(int attribute, int start, int end);
    /**
      Sets the attribute for the free space behind the last character
    */
    void setAttr(int attribute);
    /**
      Gets the attribute at the given position
    */
    int getAttr(int pos) const;
    /**
      Gets the attribute for the free space behind the last character
    */
    int getAttr() const;
    /**
      Gets the attribute, including the select state, at the given position
    */
    int getRawAttr(int pos) const;
    /**
      Gets the attribute, including the select state, for the free space
      behind the last character
    */
    int getRawAttr() const;

    /**
      Sets the syntax highlight context number
    */
    void setContext(int context);
    /**
      Gets the syntax highlight context number
    */
    int getContext() const;

    /**
      Sets the select state from start to end -1
    */
    void select(bool sel, int start, int end);
    /**
      Sets the select state from the given position to the end, including
      the free space behind the last character
    */
    void selectEol(bool sel, int pos);
    /**
      Toggles the select state from start to end -1
    */
    void toggleSelect(int start, int end);
    /**
      Toggles the select state from the given position to the end, including
      the free space behind the last character
    */
    void toggleSelectEol(int pos);
    /**
      Returns the number of selected characters
    */
    int numSelected() const;
    /**
      Returns if the character at the given position is selected
    */
    bool isSelected(int pos) const;
    /**
      Returns true if the free space behind the last character is selected
    */
    bool isSelected() const;
    /**
      Finds the next selected character, starting at the given position
    */
    int findSelected(int pos) const;
    /**
      Finds the next unselected character, starting at the given position
    */
    int findUnselected(int pos) const;
    /**
      Finds the previous selected character, starting at the given position
    */
    int findRevSelected(int pos) const;
    /**
      Finds the previous unselected character, starting at the given position
    */
    int findRevUnselected(int pos) const;

    /**
      Marks the text from the given position and length as found
    */
    void markFound(int pos, int l);
    /**
      Removes the found marks
    */
    void unmarkFound();

  protected:
    /**
      Length of the text line
    */
    int len;
    /**
      Memory Size of the text line
    */
    int size;
    /**
      The text
    */
    QChar *text;
    /**
      The attributes
    */
    uchar *attribs;
    /**
      The attribute of the free space behind the end
    */
    uchar attr;
    /**
      The syntax highlight context
    */
    int ctx;
};



class Attribute {
  public:
    Attribute();
//    Attribute(const char *aName, const QColor &, const QColor &, const QFont &);
//    QString name;
    QColor col;
    QColor selCol;
    void setFont(const QFont &);
    QFont font;
    QFontMetrics fm;
    //workaround for slow QFontMetrics::width()
    int width(QChar c) {return (fontWidth < 0) ? fm.width(c) : fontWidth;}
    int width(QString s) {return (fontWidth < 0) ? fm.width(s) : s.length()*fontWidth;}
  protected:
    int fontWidth;
};

class KWAction {
  public:
    enum Action {replace, wordWrap, wordUnWrap, newLine, delLine,
      insLine, killLine};//, doubleLine, removeLine};

    KWAction(Action, PointStruc &cursor, int len = 0,
      const QString &text = QString::null);

    Action action;
    PointStruc cursor;
    int len;
    QString text;
    KWAction *next;
};

class KWActionGroup {
  public:
    // the undo group types
    enum {  ugNone,         //
            ugPaste,        // paste
            ugDelBlock,     // delete/replace selected text
            ugIndent,       // indent
            ugUnindent,     // unindent
            ugComment,      // comment
            ugUncomment,    // uncomment
            ugReplace,      // text search/replace
            ugSpell,        // spell check
            ugInsChar,      // char type/deleting
            ugDelChar,      // ''  ''
            ugInsLine,      // line insert/delete
            ugDelLine       // ''  ''
         };

    KWActionGroup(PointStruc &aStart, int type = ugNone);
    ~KWActionGroup();
    void insertAction(KWAction *);

    static const char * typeName(int type);

    PointStruc start;
    PointStruc end;
    KWAction *action;
    int undoType;
};

/**
  The text document. It contains the textlines, controls the
  document changing operations and does undo/redo. WARNING: do not change
  the text contents directly in methods where this is not explicitly
  permitted. All changes have to be made with some basic operations,
  which are recorded by the undo/redo system.
  @see TextLine
  @author Jochen Wilhelmy
*/
class KWriteDoc : public KTextEditor::Document {
    Q_OBJECT
    friend KWriteView;
    friend KWrite;

  public:
    KWriteDoc(HlManager *, const QString &path = QString::null,
              bool bSingleViewMode = false, bool bBrowserView = false, QWidget *parentWidget = 0, const char *widgetName = 0,
              QObject *parent = 0, const char *name = 0);
    ~KWriteDoc();

    virtual bool openFile();
    virtual bool saveFile();

    virtual KTextEditor::View *createView( QWidget *parent, const char *name );
    virtual QString textLine( int line ) const;

    virtual void insertLine( const QString &s, int line = -1 );
    virtual void insertAt( const QString &s, int line, int col, bool mark = FALSE );
    virtual void removeLine( int line );
    virtual int length() const;

    virtual void setSelection( int row_from, int col_from, int row_to, int col_t );
    virtual bool hasSelection() const;
    virtual QString selection() const;

    bool isSingleViewMode() const { return m_bSingleViewMode; }

    public:

// public interface
    /**
      gets the number of lines
    */
    virtual int numLines() const {return (int) contents.count();}
    /**
      gets the last line number (numLines() -1)
    */
    int lastLine() const {return (int) contents.count() -1;}
    /**
      gets the given line
      @return  the TextLine object at the given line
      @see     TextLine
    */
    TextLine *getTextLine(int line) const;
    /**
      get the length in pixels of the given line
    */
    int textLength(int line);

    void setTabWidth(int);
    int tabWidth() {return tabChars;}
    void setReadOnly(bool);
    bool isReadOnly() const;
    virtual void setReadWrite( bool );
    virtual bool isReadWrite() const;
    virtual void setModified(bool);
    virtual bool isModified() const;
    void setSingleSelection(bool ss) {m_singleSelection = ss;}
    bool singleSelection() {return m_singleSelection;}

    void readConfig(KConfig *);
    void writeConfig(KConfig *);
    void readSessionConfig(KConfig *);
    void writeSessionConfig(KConfig *);

  signals:
    void selectionChanged();
    void highlightChanged();

// highlight
  public:
    Highlight *highlight() {return m_highlight;}
    int highlightNum() {return hlManager->findHl(m_highlight);}
    int numAttribs() {return m_numAttribs;}
    Attribute *attribs() {return m_attribs;}
  protected:
    void setHighlight(int n);
    void makeAttribs();
    void updateFontData();
  protected slots:
    void hlChanged();

// view interaction
  public:
    virtual void addView(KTextEditor::View *);
    virtual void removeView(KTextEditor::View *);
    bool ownedView(KWriteView *);
    bool isLastView(int numViews);

    int textWidth(TextLine *, int cursorX);
    int textWidth(PointStruc &cursor);
    int textWidth(bool wrapCursor, PointStruc &cursor, int xPos);
    int textPos(TextLine *, int xPos);
//    int textPos(TextLine *, int xPos, int &newXPos);
    int textWidth();
    int textHeight();

    void insert(VConfig &, const QString &);
    void insertFile(VConfig &, QIODevice &);
    void loadFile(QIODevice &);
    void writeFile(QIODevice &);

    int currentColumn(PointStruc &cursor);
    bool insertChars(VConfig &, const QString &chars);
    void newLine(VConfig &);
    void killLine(VConfig &);
    void backspace(VConfig &);
    void del(VConfig &);
    void clear();
    void cut(VConfig &);
    void copy(int flags);
    void paste(VConfig &);

    void toggleRect(int, int, int, int);
    void selectTo(VConfig &c, PointStruc &cursor, int cXPos);
    void selectAll();
    void deselectAll();
    void invertSelection();
    void selectWord(PointStruc &cursor, int flags);

    void indent(VConfig &c) {doIndent(c, 1);}
    void unIndent(VConfig &c) {doIndent(c, -1);}
    void cleanIndent(VConfig &c) {doIndent(c, 0);}
    // called by indent/unIndent/cleanIndent
    // just does some setup and then calls optimizeLeadingSpace()
    void doIndent(VConfig &, int change);
    // optimize leading whitespace on a single line - see kwdoc.cpp for full description
//    bool optimizeLeadingSpace(VConfig &, TextLine *, int, bool);
    void optimizeLeadingSpace(int line, int flags, int change);

    void comment(VConfig &c) {doComment(c, 1);}
    void unComment(VConfig &c) {doComment(c, -1);}
    void doComment(VConfig &, int change);
    void doCommentLine(PointStruc &cursor);
    void doUncommentLine(PointStruc &cursor);

    virtual QString text() const;
    QString getWord(PointStruc &cursor);
    public slots:
    virtual void setText(const QString &);
    public:
    bool hasMarkedText() {return (selectEnd >= selectStart);}
    QString markedText(int flags);
    void delMarkedText(VConfig &/*, bool undo = true*/);

    void tagLineRange(int line, int x1, int x2);
    void tagLines(int start, int end);
    void tagAll();
    void updateLines(int startLine = 0, int endLine = 0xffffff, int flags = 0,
      int cursorY = -1);
    void updateMaxLength(TextLine *);
    void updateViews(KWriteView *exclude = 0L);

    QColor &cursorCol(int x, int y);
    QFont &getTextFont(int x, int y);
    void paintTextLine(QPainter &, int line, int xStart, int xEnd, bool showTabs);
//    void printTextLine(QPainter &, int line, int xEnd, int y);

    void setURL( const KURL &url, bool updateHighlight );
    void clearFileName();

    bool doSearch(SConfig &s, const QString &searchFor);
    void unmarkFound();
    void markFound(PointStruc &cursor, int len);

// internal
    void tagLine(int line);
    void insLine(int line);
    void delLine(int line);
    void optimizeSelection();

    void doAction(KWAction *);
    void doReplace(KWAction *);
    void doWordWrap(KWAction *);
    void doWordUnWrap(KWAction *);
    void doNewLine(KWAction *);
    void doDelLine(KWAction *);
    void doInsLine(KWAction *);
    void doKillLine(KWAction *);
    void newUndo();

    void recordStart(VConfig &, int newUndoType);
    void recordStart(KWriteView *, PointStruc &, int flags, int newUndoType,
      bool keepModal = false, bool mergeUndo = false);
    void recordAction(KWAction::Action, PointStruc &);
    void recordInsert(VConfig &, const QString &text);
    void recordReplace(VConfig &, int len, const QString &text);
    void recordInsert(PointStruc &, const QString &text);
    void recordDelete(PointStruc &, int len);
    void recordReplace(PointStruc &, int len, const QString &text);
    void recordEnd(VConfig &);
    void recordEnd(KWriteView *, PointStruc &, int flags);
//  void recordReset();
    void doActionGroup(KWActionGroup *, int flags, bool undo = false);

    int nextUndoType();
    int nextRedoType();
    void undoTypeList(QValueList<int> &lst);
    void redoTypeList(QValueList<int> &lst);
    void undo(VConfig &, int count = 1);
    void redo(VConfig &, int count = 1);
    void clearRedo();
    void setUndoSteps(int steps);

    void setPseudoModal(QWidget *);

    void newBracketMark(PointStruc &, BracketMark &);

  protected:
    virtual void guiActivateEvent( KParts::GUIActivateEvent *ev );

  protected slots:
    void clipboardChanged();

  private slots:
    void slotViewDestroyed();

// member variables
  protected:
    QList<TextLine> contents;
    QColor colors[5];
    HlManager *hlManager;
    Highlight *m_highlight;
    int m_numAttribs;
    static const int maxAttribs = 32;
    Attribute m_attribs[maxAttribs];

    int eolMode;

    int tabChars;
    int m_tabWidth;
    int fontHeight;
    int fontAscent;

    QList<KWriteView> views;
    bool newDocGeometry;

    TextLine *longestLine;
    int maxLength;

    PointStruc select;
    PointStruc anchor;
    int aXPos;
    int selectStart;
    int selectEnd;
    bool oldMarkState;
    bool m_singleSelection; // false: windows-like, true: X11-like

    bool readOnly;
    bool modified;

    int foundLine;

    QList<KWActionGroup> undoList;
    int currentUndo;
    int undoState;
    int undoSteps;
    int tagStart;
    int tagEnd;

//    int undoType;           // what kind of undo is active
    int undoCount;          //counts merged undo steps
//    bool undoReported;      // true if the current undo has been reported to the views
//    KWriteView* undoView;   // the KWriteView that owns the undo group

    QWidget *pseudoModal;   //the replace prompt is pseudo modal

    bool m_bSingleViewMode;
};

class KWriteBrowserExtension : public KParts::BrowserExtension
{
  Q_OBJECT
public:
  KWriteBrowserExtension( KWriteDoc *doc );

private slots:
  void copy();
  void slotSelectionChanged();

private:
  KWriteDoc *m_doc;
};

#endif //KWDOC_H
