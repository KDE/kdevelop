/*
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

#ifndef _KWVIEV_H_
#define _KWVIEV_H_

#include <qscrollbar.h>
#include <qiodevice.h>
#include <qpopupmenu.h>
#include <qkeycode.h>

#include <kfm.h>
#include <kconfig.h>

#include "kwdialog.h"
#include "kguicommand.h"

class KWriteDoc;
class Highlight;

//search flags
const int sfCaseSensitive   = 1;
const int sfWholeWords      = 2;
const int sfFromCursor      = 4;
const int sfBackward        = 8;
const int sfSelected        = 16;
const int sfPrompt          = 32;
const int sfReplace         = 64;
const int sfAgain           = 128;
const int sfWrapped         = 256;
const int sfFinished        = 512;
//dialog results
const int srYes             = QDialog::Accepted;
const int srNo              = 10;
const int srAll             = 11;
const int srCancel          = QDialog::Rejected;

//config flags
const int cfAutoIndent      = 1;
const int cfBackspaceIndent = 2;
const int cfWordWrap        = 4;
const int cfReplaceTabs     = 8;
const int cfRemoveSpaces    = 16;
const int cfWrapCursor      = 32;
const int cfAutoBrackets    = 64;

const int cfPersistent      = 128;
const int cfKeepSelection   = 256;
const int cfVerticalSelect  = 512;
const int cfDelOnInput      = 1024;
const int cfXorSelect       = 2048;

const int cfOvr             = 4096;
const int cfMark            = 8192;

//update flags
const int ufDocGeometry     = 1;
const int ufUpdateOnScroll  = 2;
const int ufPos             = 4;

//load flags
const int lfInsert          = 1;
const int lfNewFile         = 2;
const int lfNoAutoHl        = 4;

//end of line settings
const int eolUnix           = 0;
const int eolMacintosh      = 1;
const int eolDos            = 2;

//command categories
const int ctCursorCommands    = 0;
const int ctEditCommands      = 1;
const int ctFindCommands      = 2;
const int ctBookmarkCommands  = 3;
const int ctStateCommands     = 4;

//cursor movement commands
const int selectFlag        = 0x100000;
const int multiSelectFlag   = 0x200000;
const int cmLeft            = 1;
const int cmRight           = 2;
const int cmWordLeft        = 3;
const int cmWordRight       = 4;
const int cmHome            = 5;
const int cmEnd             = 6;
const int cmUp              = 7;
const int cmDown            = 8;
const int cmScrollUp        = 9;
const int cmScrollDown      = 10;
const int cmTopOfView       = 11;
const int cmBottomOfView    = 12;
const int cmPageUp          = 13;
const int cmPageDown        = 14;
const int cmCursorPageUp    = 15;
const int cmCursorPageDown  = 16;
const int cmTop             = 17;
const int cmBottom          = 18;
//edit commands
const int cmReturn          = 1;
const int cmDelete          = 2;
const int cmBackspace       = 3;
const int cmKillLine        = 4;
const int cmUndo            = 5;
const int cmRedo            = 6;
const int cmCut             = 7;
const int cmCopy            = 8;
const int cmPaste           = 9;
const int cmIndent          = 10;
const int cmUnindent        = 11;
const int cmSelectAll       = 12;
const int cmDeselectAll     = 13;
const int cmInvertSelection = 14;
//find commands
const int cmFind            = 1;
const int cmReplace         = 2;
const int cmFindAgain       = 3;
const int cmGotoLine        = 4;
//bookmark commands
const int cmSetBookmark     = 1;
const int cmAddBookmark     = 2;
const int cmClearBookmarks  = 3;
const int cmSetBookmarks    = 10;
const int cmGotoBookmarks   = 20;
//state commands
const int cmToggleInsert    = 1;
const int cmToggleVertical  = 2;


void resizeBuffer(void *user, int w, int h);

class KWriteView;
class KWrite;

struct PointStruc {
  int x;
  int y;
};

struct VConfig {
  KWriteView *view;
  PointStruc cursor;
  int cXPos;
  int flags;
  int wrapAt;
};

struct SConfig {
  PointStruc cursor;
  PointStruc startCursor;
  int flags;
};

struct LineRange {
  int start;
  int end;
};

struct BracketMark {
  PointStruc cursor;
  int sXPos;
  int eXPos;
};

class KWriteView : public QWidget {
    Q_OBJECT
    friend KWriteDoc;
    friend KWrite;
  public:
    KWriteView(KWrite *, KWriteDoc *);
    ~KWriteView();

    virtual void doCursorCommand(VConfig &, int cmdNum);
    virtual void doEditCommand(VConfig &, int cmdNum);

    void cursorLeft(VConfig &);
    void cursorRight(VConfig &);
    void wordLeft(VConfig &);
    void wordRight(VConfig &);
    void home(VConfig &);
    void end(VConfig &);
    void cursorUp(VConfig &);
    void cursorDown(VConfig &);
    void scrollUp(VConfig &);
    void scrollDown(VConfig &);
    void topOfView(VConfig &);
    void bottomOfView(VConfig &);
    void pageUp(VConfig &);
    void pageDown(VConfig &);
    void cursorPageUp(VConfig &);
    void cursorPageDown(VConfig &);
    void top(VConfig &);
    void bottom(VConfig &);

  protected slots:
    void changeXPos(int);
    void changeYPos(int);

  protected:
    void getVConfig(VConfig &);
    void update(VConfig &);
    void insLine(int line);
    void delLine(int line);
    void updateCursor();
    void updateCursor(PointStruc &newCursor);

    void lineValues(int h);
    void tagLines(int start, int end, int x1, int x2);
    void tagAll();
    void setPos(int x, int y);
    void center();

    void updateView(int flags);

    void paintTextLines(int xPos, int yPos);
    void paintCursor();
    void paintBracketMark();

    void placeCursor(int x, int y, int flags);

    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void timerEvent(QTimerEvent *);

    KWrite *kWrite;
    KWriteDoc *kWriteDoc;
    QScrollBar *xScroll;
    QScrollBar *yScroll;

    int xPos;
    int yPos;

    int mouseX;
    int mouseY;
    int scrollX;
    int scrollY;
    int scrollTimer;

    PointStruc cursor;
    bool cursorOn;
    int cursorTimer;
    int cXPos;
    int cOldXPos;

    int startLine;
    int endLine;

    bool exposeCursor;
    int updateState;
  //  int updateLines[2];
    int numLines;
    LineRange *lineRanges;
    int newXPos;
    int newYPos;

    QPixmap *drawBuffer;

    BracketMark bm;
};

class KWBookmark {
  public:
    KWBookmark();
    int xPos;
    int yPos;
    PointStruc cursor;
    QString Name;
};

/** The KWrite text editor widget. It has many options, document/view
    architecture and syntax highlight.
    @author Jochen Wilhelmy
*/

class KWrite : public QWidget {
    Q_OBJECT
    friend KWriteView;
    friend KWriteDoc;
  public:
    /** The document can be used by more than one KWrite objects
    */
    KWrite(KWriteDoc *, QWidget *parent = 0L, const char *name = 0L);
    /** The destructor does not delete the document
    */
    ~KWrite();

    static void addCursorCommands(KGuiCmdManager &);
    static void addEditCommands(KGuiCmdManager &);
    static void addFindCommands(KGuiCmdManager &);
    static void addBookmarkCommands(KGuiCmdManager &);
    static void addStateCommands(KGuiCmdManager &);

//status functions
    /** Returns the current line number, that is the line the cursor is on.
        For the first line it returns 0. Signal newCurPos() is emitted on
        cursor position changes.
    */
    int currentLine();
    /** Returns the current column number. It handles tab's correctly.
        For the first column it returns 0.
    */
    int currentColumn();
    /** Returns the number of the character, that the cursor is on (cursor x)
    */
    int currentCharNum();
    /** Sets the current cursor position
    */
    void setCursorPosition(int line, int col);
    /** Returns the config flags. See the cfXXX constants in the .h file.
    */
    int config();
    /** Sets the config flags
    */
    void setConfig(int);
  //    bool isOverwriteMode();
    /** Returns true if the document has been modified.
    */
    bool isModified();
    /** Sets the modification status of the document
    */
    void setModified(bool m = true);
    /** Returns true if this editor is the only owner of its document
    */
    bool isLastView();
    /** Returns the document object
    */
    KWriteDoc *doc();
    /** Bit 0 : undo possible, Bit 1 : redo possible.
        Used to enable/disable undo/redo menu items and toolbar buttons
    */
    int undoState();
    void copySettings(KWrite *);
  public slots:
    /** Presents a options dialog to the user
    */
    void optDlg();
    /** Presents a color dialog to the user
    */
    void colDlg();
    /** Executes state command cmdNum
    */
    void doStateCommand(int cmdNum);
    /** Toggles Insert mode
    */
    void toggleInsert();
    /** Toggles "Vertical Selections" option
    */
    void toggleVertical();
  signals:
    /** The cursor position has changed. Get the values with currentLine()
        and currentColumn()
    */
    void newCurPos();
    /** Modified flag or config flags have changed
    */
    void newStatus();
    /** The undo/redo enable status has changed
    */
    void newUndo();
    /** The marked text state has changed. This can be used to enable/disable
        cut and copy
    */
    void newMarkStatus();
    /** The file name has changed. The main window can use this to change
        its caption
    */
    void fileChanged();
    /** Emits messages for the status line
    */
    void statusMsg(const char *);
  protected:
    int configFlags;
    int wrapAt;

//text access
  public:
     /** Gets the number of text lines;
     */
     int numLines();
     /** Gets the complete document content as string
     */
     QString text();
     /** Gets the text line where the cursor is on
     */
     QString currentTextLine();
     /** Gets a text line
     */
     QString textLine(int num);
     /** Gets the word where the cursor is on
     */
     QString currentWord();
     /** Gets the word at position x, y. Can be used to find
         the word under the mouse cursor
     */
     QString word(int x, int y);
     /** Discard old text without warning and set new text
     */
     void setText(const char *);
     /** Insert text at the current cursor position. If length is a positive
         number, it restricts the number of inserted characters
     */
     void insertText(const char *, int len = -1);
     /** Queries if there is marked text
     */
     bool hasMarkedText();
     /** Gets the marked text as string
     */
     QString markedText();
     int tabWidth();
//url aware file functions
  public:
    enum fileAction{GET, PUT}; //tells us what kind of job kwrite is waiting for
    enum fileResult{OK, CANCEL, RETRY, ERROR};
    /** Loads a file from the given QIODevice. For insert = false the old
        contents will be lost.
    */
    void loadFile(QIODevice &, bool insert = false);
    /** Writes the document into the given QIODevice
    */
    void writeFile(QIODevice &);
    /** Loads the file given in name into the editor
    */
    bool loadFile(const char *name, int flags = 0);
    /** Saves the file as given in name
    */
    bool writeFile(const char *name);
    /** Loads the file given in url into the editor.
        See the lfXXX constants in the .h file.
    */
    void loadURL(const char *url, int flags = 0);
    /** Saves the file as given in url
    */
    void writeURL(const char *url, int flags = 0);
  protected slots:
    void kfmFinished();
    void kfmError(int, const char *);
  public:
    /** Returns true if the document has a filename (not counting the path).
    */
    bool hasFileName();
    /** Returns the URL of the currnet file
    */
    const char *fileName();
    /** Set the file name. This starts the automatic highlight selection.
    */
    void setFileName(const char *);
    /** Mainly for internal use. Returns true if the current document can be
        discarded. If the document is modified, the user is asked if he wants
        to save it. On "cancel" the function returns false.
    */
    bool canDiscard();
  public slots:
    /** Opens a new untitled document in the text widget. The user is given
        a chance to save the current document if the current document has
        been modified.
    */
    void newDoc();
    /** This will present an open file dialog and open the file specified by
        the user, if possible. The user will be given a chance to save the
        current file if it has been modified. This starts the automatic
        highlight selection.
    */
    void open();
    /** Calling this method will let the user insert a file at the current
        cursor position.
    */
    void insertFile();
    /** Saves the file if necessary under the current file name. If the current file
        name is Untitled, as it is after a call to newFile(), this routing will
        call saveAs().
    */
    fileResult save();
    /** Allows the user to save the file under a new name. This starts the
        automatic highlight selection.
    */
    fileResult saveAs();
  protected:
    KFM *kfm;
    QString kfmURL;
    QString kfmFile;
    fileAction kfmAction;
    int kfmFlags;

//command processors
  public slots:
    /** Does Cursor Command cmdNum
    */
    void doCursorCommand(int cmdNum);
    /** Does Edit Command cmdNum
    */
    void doEditCommand(int cmdNum);

//edit functions
  public:
    /** Clears the document without any warnings or requesters.
    */
    void clear();
  public slots:
    /** Moves the marked text into the clipboard
    */
    void cut() {doEditCommand(cmCut);}
    /** Copies the marked text into the clipboard
    */
    void copy() {doEditCommand(cmCopy);}
    /** Inserts text from the clipboard at the actual cursor position
    */
    void paste() {doEditCommand(cmPaste);}
    /** Undoes the last operation. The number of undo steps is configurable
    */
    void undo() {doEditCommand(cmUndo);}
    /** Repeats an operation which has been undone before.
    */
    void redo() {doEditCommand(cmRedo);}
    /** Moves the current line or the selection one position to the right
    */
    void indent() {doEditCommand(cmIndent);}
    /** Moves the current line or the selection one position to the left
    */
    void unIndent() {doEditCommand(cmUnindent);}
    /** Selects all text
    */
    void selectAll() {doEditCommand(cmSelectAll);}
    /** Deselects all text
    */
    void deselectAll() {doEditCommand(cmDeselectAll);}
    /** Inverts the current selection
    */
    void invertSelection() {doEditCommand(cmInvertSelection);}

//search/replace functions
  public slots:
    /** Presents a search dialog to the user
    */
    void find();
    /** Presents a replace dialog to the user
    */
    void replace();
    /** Repeasts the last search or replace operation. On replace, the
        user is prompted even if the "Prompt On Replace" option was off.
    */
    void findAgain();
    /** Presents a "Goto Line" dialog to the user
    */
    void gotoLine();
  protected:
    void initSearch(SConfig &, int flags);
    void continueSearch(SConfig &);
    void searchAgain(SConfig &);
    void replaceAgain();
    void doReplaceAction(int result, bool found = false);
    void exposeFound(PointStruc &cursor, int slen, int flags, bool replace);
    void deleteReplacePrompt();
    bool askReplaceEnd();
  protected slots:
    void replaceSlot();
  protected:
    QStrList searchForList;
//    QString replaceWith;
    QStrList replaceWithList;
    int searchFlags;
    int replaces;
    SConfig s;
    QDialog *replacePrompt;

//right mouse button popup menu
  public:
    /** Install a Popup Menu. The Popup Menu will be activated on
        a right mouse button press event.
    */
    void installRBPopup(QPopupMenu *);
  protected:
    QPopupMenu *popup;

//bookmarks
  public:
    /** Install a Bookmark Menu. The bookmark items will be added to the
        end of the menu
    */
    void installBMPopup(KGuiCmdPopup *);
    /** Sets the actual edit position as bookmark number n
    */
    void setBookmark();
    void addBookmark();
    void clearBookmarks();
    void setBookmark(int n);
    void gotoBookmark(int n);
  public slots:
    void doBookmarkCommand(int cmdNum);

    /** Shows a popup that lets the user choose the bookmark number
    */
//    void setBookmark();
    /** Adds the actual edit position to the end of the bookmark list
    */
//    void addBookmark();
    /** Clears all bookmarks
    */
//    void clearBookmarks();
    /** Sets the cursor to the bookmark n
    */
//    void gotoBookmark(int n);
  protected slots:
    /** Updates the bookmark popup menu when it emit aboutToShow()
    */
    void updateBMPopup();
  protected:
    QList<KWBookmark> bookmarks;
//    int bmEntries;

//config file / session management functions
  public:
    /** Reads config entries out of the KConfig object
    */
    void readConfig(KConfig *);
    /** Writes config entries into the KConfig object
    */
    void writeConfig(KConfig *);
    /** Reads session config out of the KConfig object. This also includes
        the actual cursor position and the bookmarks.
    */
    void readSessionConfig(KConfig *);
    /** Writes session config into the KConfig object
    */
    void writeSessionConfig(KConfig *);

//syntax highlight
  public slots:
    /** Presents the highlight defaults dialog to the user
    */
    void hlDef();
    /** Presents the highlight setup dialog to the user
    */
    void hlDlg();
    /** Gets the highlight number
    */
    int getHl();
    /** Sets the highlight number n
    */
    void setHl(int n);
    /** Get the end of line mode (Unix, Macintosh or Dos)
    */
    int getEol();
    /** Set the end of line mode (Unix, Macintosh or Dos)
    */
    void setEol(int);

//print
    void print();

//internal
  protected:
//    virtual void keyPressEvent(QKeyEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

    KWriteView *kWriteView;
    KWriteDoc *kWriteDoc;
};


#endif //KWVIEV_H

