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

#include <list>

#include <qobject.h>
#include <qlist.h>
#include <qcolor.h>
#include <qfont.h>
#include <qfontmetrics.h>

#include "kwview.h"
#include "highlight.h"

/** The TextLine represents a line of text. A text line that contains the
    text, an attribute for each character, an attribute for the free space
    behind the last character and a context number for the syntax highlight.
    The attribute stores the index to a table that contains fonts and colors
    and also if a character is selected.
The text will be changed from char * to QChar *
*/
class TextLine {
  public:
    /** Creates an empty text line with given attribute and syntax highlight
        context
    */
    TextLine(int attribute = 0, int context = 0);
    ~TextLine();

    /** Returns the length
    */
    int length() const {return len;}
    /** Universal text manipulation method. It can be used to insert, delete
        or replace text.
    */
    void replace(int pos, int delLen, const QChar *insText, int insLen,
      uchar *insAttribs = 0L);

    /** Appends a string of length l to the textline
    */
    void append(const QChar *s, int l) {replace(len, 0, s, l);}
    /** Wraps the text from the given position to the end to the next line
    */
    void wrap(TextLine *nextLine, int pos);
    /** Wraps the text of given length from the beginning of the next line to
        this line at the given position
    */
    void unWrap(int pos, TextLine *nextLine, int len);
    /** Removes trailing spaces
    */
    void removeSpaces();
    /** Returns the position of the first character which has a value > 32
    */
    int firstChar() const;
    /** Gets the char at the given position
    */
    QChar getChar(int pos) const;

    /** Sets the attributes from start to end -1
    */
    void setAttribs(int attribute, int start, int end);
    /** Sets the attribute for the free space behind the last character
    */
    void setAttr(int attribute);
    /** Gets the attribute at the given position
    */
    int getAttr(int pos) const;
    /** Gets the attribute for the free space behind the last character
    */
    int getAttr() const;
    /** Gets the attribute, including the select state, at the given position
    */
    int getRawAttr(int pos) const;
    /** Gets the attribute, including the select state, for the free space
        behind the last character
    */
    int getRawAttr() const;

    /** Sets the syntax highlight context number
    */
    void setContext(int context);
    /** Gets the syntax highlight context number
    */
    int getContext() const;

    /** Gets a C-like null terminated string
    */
    const QChar *getString();
    /** Gets the text. WARNING: it is not null terminated
    */
    const QChar *getText() const;

    /** Sets the select state from start to end -1
    */
    void select(bool sel, int start, int end);
    /** Sets the select state from the given position to the end, including
        the free space behind the last character
    */
    void selectEol(bool sel, int pos);
    /** Toggles the select state from start to end -1
    */
    void toggleSelect(int start, int end);
    /** Toggles the select state from the given position to the end, including
        the free space behind the last character
    */
    void toggleSelectEol(int pos);
    /** Returns the number of selected characters
    */
    int numSelected() const;
    /** Returns if the character at the given position is selected
    */
    bool isSelected(int pos) const;
    /** Returns true if the free space behind the last character is selected
    */
    bool isSelected() const;
    /** Finds the next selected character, starting at the given position
    */
    int findSelected(int pos) const;
    /** Finds the next unselected character, starting at the given position
    */
    int findUnselected(int pos) const;
    /** Finds the previous selected character, starting at the given position
    */
    int findRevSelected(int pos) const;
    /** Finds the previous unselected character, starting at the given position
    */
    int findRevUnselected(int pos) const;

    /** Returns the x position of the cursor at the given position, which
        depends on the number of tab characters
    */
    int cursorX(int pos, int tabChars) const;

    /** Marks the text from the given position and length as found
    */
    void markFound(int pos, int l);
    /** Removes the found marks
    */
    void unmarkFound();

  protected:
    /** Length of the text line
    */
    int len;
    /** Memory Size of the text line
    */
    int size;
    /** The text *)
    */
    QChar *text;
    /** The attributes
    */
    uchar *attribs;
    /** The attribute of the free space behind the end
    */
    uchar attr;
    /** The syntax highlight context
    */
    int ctx;
};



const int nAttribs = 32;

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

    static QString typeName(int type);

    PointStruc start;
    PointStruc end;
    KWAction *action;
    int undoType;
};

/** The text document. It contains the textlines, controls the
    document changing operations and does undo/redo. WARNING: do not change
    the text contents directly in methods where this is not explicitly
    permitted. All changes have to be made with some basic operations,
    which are recorded by the undo/redo system.
    @see TextLine
    @author Jochen Wilhelmy
*/
class KWriteDoc : QObject {
    Q_OBJECT
    friend KWriteView;
    friend KWrite;

  public:
    KWriteDoc(HlManager *, const QString &path = QString::null);
    ~KWriteDoc();

    /** gets the number of lines */
    int numLines() const {return (int) contents.count();}
    /** gets the last line number (numLines() -1) */
    int lastLine() const {return (int) contents.count() -1;}
    TextLine *textLine(int line);
    int textLength(int line);
    void tagLineRange(int line, int x1, int x2);
    void tagLines(int start, int end);
    void tagAll();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);
    void readSessionConfig(KConfig *);
    void writeSessionConfig(KConfig *);
  protected:
    void registerView(KWriteView *);
    void removeView(KWriteView *);

    int currentColumn(PointStruc &cursor);

    void insert(VConfig &, const QString &);
    void insertFile(VConfig &, QIODevice &);
    void loadFile(QIODevice &);
    void writeFile(QIODevice &);

    bool insertChars(VConfig &, const QString &chars);
    void newLine(VConfig &);
    void killLine(VConfig &);
    void backspace(VConfig &);
    void del(VConfig &);

    Highlight *getHighlight() {return highlight;}
  protected slots:
    void clipboardChanged();
    void hlChanged();

  public:
    int getHighlightNum() {return hlManager->findHl(highlight);}
  protected:
    void setHighlight(int n);
    void makeAttribs();
    void updateFontData();
    void setTabWidth(int);
    void updateLines(int startLine = 0, int endLine = 0xffffff, int flags = 0);
    void updateMaxLength(TextLine *);
    void updateViews(KWriteView *exclude = 0L);

    int textWidth(TextLine *, int cursorX);
    int textWidth(PointStruc &cursor);
    int textWidth(bool wrapCursor, PointStruc &cursor, int xPos);
    int textPos(TextLine *, int xPos);
//    int textPos(TextLine *, int xPos, int &newXPos);

    int textWidth();
    int textHeight();

    void toggleRect(int, int, int, int);
    void selectTo(VConfig &c, PointStruc &cursor, int cXPos);
    void clear();
    void copy(int flags);
    void paste(VConfig &);
    void cut(VConfig &);
    void selectAll();
    void deselectAll();
    void invertSelection();
    void selectWord(PointStruc &cursor, int flags);

    QString text();
    QString getWord(PointStruc &cursor);
    void setText(const QString &);
    bool hasMarkedText() {return (selectEnd >= selectStart);}
    QString markedText(int flags);
    void delMarkedText(VConfig &/*, bool undo = true*/);

    QColor &cursorCol(int x, int y);
    void paintTextLine(QPainter &, int line, int xStart, int xEnd);
    void printTextLine(QPainter &, int line, int xEnd, int y);

    void setReadOnly(bool);
    bool isReadOnly();
    void setModified(bool);
    bool isModified();

    bool isLastView(int numViews);

    bool hasFileName();
    const QString fileName();
    void setFileName(const QString&);
    void clearFileName();

    bool doSearch(SConfig &s, const QString &searchFor);
    void unmarkFound();
    void markFound(PointStruc &cursor, int len);

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
    void recordInsert(PointStruc &, const QString &text);
    void recordDelete(PointStruc &, int len);
    void recordReplace(PointStruc &, int len, const QString &text);
    void recordEnd(VConfig &);
    void recordEnd(KWriteView *, PointStruc &, int flags);
//  void recordReset();
    void doActionGroup(KWActionGroup *, int flags, bool undo = false);
    int nextUndoType();
    int nextRedoType();
    void undoTypeList(std::list<int> &lst);
    void redoTypeList(std::list<int> &lst);
    void undo(VConfig &, int count = 1);
    void redo(VConfig &, int count = 1);
    void clearRedo();
    void setUndoSteps(int steps);


    void setPseudoModal(QWidget *);

    void indent(VConfig &c) {doIndent(c, 1);}
    void unIndent(VConfig &c) {doIndent(c, -1);}
    void cleanIndent(VConfig &c) {doIndent(c, 0);}
    // called by indent/unIndent/cleanIndent
    // just does some setup and then calls optimizeLeadingSpace()
    void doIndent(VConfig &, int change);
    // optimize leading whitespace on a single line - see kwdoc.cpp for full description
//    bool optimizeLeadingSpace(VConfig &, TextLine *, int, bool);
    void optimizeLeadingSpace(int line, int flags, int change);

    void newBracketMark(PointStruc &, BracketMark &);

    QList<TextLine> contents;
    QColor colors[5];
    HlManager *hlManager;
    Highlight *highlight;
    Attribute attribs[nAttribs];

    int eolMode;

    int tabChars;
    int tabWidth;
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

    bool readOnly;
    bool modified;
    QString fName;

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
};

#endif //KWDOC_H
