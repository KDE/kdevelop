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

#ifndef _KWVIEV_H_
#define _KWVIEV_H_

#include <qscrollbar.h>
#include <qiodevice.h>
#include <qpopupmenu.h>
#include <qkeycode.h>
#include <qintdict.h>
#include <qstring.h>
#include <qdropsite.h>
#include <qdragobject.h>
#include <qregexp.h>

#include <ktempfile.h>
#include <kconfig.h>
#include <kspell.h>
#include <ksconfig.h>
#include <ktexteditor.h>

class KAction;
class KToggleAction;
class KRecentFilesAction;
class KSelectAction;

class QPrinter;

namespace KIO { class FileCopyJob; }

class KTempFile;
class KWriteDoc;
class KWIconBorder;
class Highlight;

/*
//dialog results
const int srYes               = QDialog::Accepted;
const int srNo                = 10;
const int srAll               = 11;
const int srCancel            = QDialog::Rejected;
*/
// --- config flags ---
// indent

enum Select_flags {
  selectFlag          = 0x100000,
  multiSelectFlag     = 0x200000
};
//state commands
enum State_commands {
  cmToggleInsert      = 1,
  cmToggleVertical    = 2
};


void resizeBuffer(void *user, int w, int h);

class KWriteView;
class KWrite;

struct PointStruc {
  int x;
  int y;
};

struct VConfig {
  KWrite *view;
  PointStruc cursor;
  int cXPos;
  int flags;
  int wrapAt;
};

struct SConfig {
  PointStruc cursor;
  PointStruc startCursor;
  int flags;

  // Set the pattern to be used for searching.
  void setPattern(QString &newPattern);

  // Search the given string.
  int search(QString &text, int index);

  // The length of the last match found using pattern or regExp.
  int matchedLength;

private:
  QString m_pattern;

  // The regular expression corresponding to pattern. Only guaranteed valid if
  // flags has sfRegularExpression set.
  QRegExp m_regExp;
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
    friend class KWriteDoc;
    friend class KWrite;
    friend class KWIconBorder;
    friend class TextEditorView;
    friend class CodeCompletionIfaceImpl;
  public:
    // a drop-aware container should set HandleOwnURIDrops = false and handle all URI drops
    // KWriteView will otherwise handle URI drops, but is slightly limited
    // KWriteView always handles text drops
    KWriteView(KWrite *, KWriteDoc *, bool HandleOwnURIDrops);
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
    void top_home(VConfig &c);
    void bottom_end(VConfig &c);
    PointStruc getCursorPosition(){
      return cursor;
    }
    // get the real CursorCoodinates
    QPoint getCursorCoordinates(){
      return QPoint(xCoord,yCoord);
    }

  protected slots:
    void changeXPos(int);
    void changeYPos(int);

  protected:
    void getVConfig(VConfig &);
    void changeState(VConfig &);
    void insLine(int line);
    void delLine(int line);
    void updateCursor();
    void updateCursor(PointStruc &newCursor);
    void updateCursor(PointStruc &newCursor, int flags);

    void clearDirtyCache(int height);
    void tagLines(int start, int end, int x1, int x2);
    void tagAll();
    void setPos(int x, int y);
    void center();

    void updateView(int flags);

    void paintTextLines(int xPos, int yPos);
    void paintCursor();
    void paintBracketMark();

    void placeCursor(int x, int y, int flags = 0);
    bool isTargetSelected(int x, int y);

    void doDrag();

    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void wheelEvent( QWheelEvent *e );
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void timerEvent(QTimerEvent *);

    virtual bool focusNextPrevChild(bool next);
    
    virtual void dragEnterEvent( QDragEnterEvent * );
//  virtual void dragMoveEvent( QDragMoveEvent * );
//  virtual void dragLeaveEvent( QDragLeaveEvent * );
    virtual void dropEvent( QDropEvent * );

    KWrite *kWrite;
    KWriteDoc *kWriteDoc;
    QScrollBar *xScroll;
    QScrollBar *yScroll;
    KWIconBorder *leftBorder;
    int xCoord;
    int yCoord;

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

    bool HandleURIDrops;

    enum DragState { diNone, diPending, diDragging };

    struct _dragInfo {
      DragState       state;
      PointStruc      start;
      QTextDrag       *dragObject;
    } dragInfo;

  signals:
    // emitted when KWriteView is not handling its own URI drops
    void dropEventPass(QDropEvent*);
};

class KWBookmark {
  public:
    KWBookmark();
    int xPos;
    int yPos;
    PointStruc cursor;
    QString Name;
};

/**
  The KWrite text editor widget. It has many options, document/view
  architecture and syntax highlight.
  @author Jochen Wilhelmy
*/

class KWrite : public KTextEditor::View {
    Q_OBJECT
    friend class KWriteView;
    friend class KWriteDoc;
    friend class KWIconBorder;
  public:
    /**
      The document can be used by more than one KWrite objects.
      HandleOwnURIDrops should be set to false for a container that can handle URI drops
      better than KWriteView does.
    */
    KWrite(KWriteDoc *, QWidget *parent = 0L, const char * name = 0, bool HandleOwnURIDrops = true, bool deleteDoc = true);
    /**
      The destructor does not delete the document
    */
    ~KWrite();

    virtual void setCursorPosition( int line, int col, bool mark = false );
    virtual void getCursorPosition( int *line, int *col );

    virtual bool isOverwriteMode() const;
    virtual void setOverwriteMode( bool b );

    void setupActions();
//status and config functions
    /**
      Returns the current line number, that is the line the cursor is on.
      For the first line it returns 0. Signal newCurPos() is emitted on
      cursor position changes.
    */
    int currentLine();
    /**
      Returns the current column number. It handles tab's correctly.
      For the first column it returns 0.
    */
    int currentColumn();
    /**
      Returns the number of the character, that the cursor is on (cursor x)
    */
    int currentCharNum();
    /**
      Sets the current cursor position
    */
    void setCursorPositionInternal(int line, int col);
    /**
      Returns the config flags. See the cfXXX constants in the .h file.
    */
    int config();// {return configFlags;}
    /**
      Sets the config flags
    */
    void setConfig(int);

    int wordWrapAt() {return wrapAt;}
    void setWordWrapAt(int at) {wrapAt = at;}
    int tabWidth();
    void setTabWidth(int);
    int undoSteps();
    void setUndoSteps(int);

  //    bool isOverwriteMode();
    /**
      Returns true if the document is in read only mode.
    */
    bool isReadOnly();
    /**
      Returns true if the document has been modified.
    */
    bool isModified();
    /**
      Sets the read-only flag of the document
    */
    void setReadOnly(bool);
    /**
      Sets the modification status of the document
    */
    void setModified(bool m = true);
    /**
      Returns true if this editor is the only owner of its document
    */
    bool isLastView();
    /**
      Returns the document object
    */
    KWriteDoc *doc();

    /*
      Bit 0 : undo possible, Bit 1 : redo possible.
      Used to enable/disable undo/redo menu items and toolbar buttons
    */
    int undoState();
    /**
      Returns the type of the next undo group.
    */
    int nextUndoType();
    /**
      Returns the type of the next redo group.
    */
    int nextRedoType();
    /**
      Returns a list of all available undo types, in undo order.
    */
    void undoTypeList(QValueList<int> &lst);
    /**
      Returns a list of all available redo types, in redo order.
    */
    void redoTypeList(QValueList<int> &lst);
    /**
      Returns a short text description of the given undo type,
      which is obtained with nextUndoType(), nextRedoType(), undoTypeList(), and redoTypeList(),
      suitable for display in a menu entry.  It is not translated;
      use i18n() before displaying this string.
    */
    const char * undoTypeName(int undoType);

    void copySettings(KWrite *);

    QColor* getColors();
    void applyColors();

  public slots:

    void slotOpenRecent( const KURL & );

    /**
      Toggles Insert mode
    */
    void toggleInsert();
    /**
      Toggles "Vertical Selections" option
    */
    void toggleVertical();
  signals:
    /**
      The cursor position has changed. Get the values with currentLine()
      and currentColumn()
    */
    void newCurPos();
    /**
      Modified flag or config flags have changed
    */
    void newStatus();
    /**
      The undo/redo enable status has changed
    */
    void newUndo();
    /**
      The marked text state has changed. This can be used to enable/disable
      cut and copy
    */
    void newMarkStatus();
    /**
      The file name has changed. The main window can use this to change
      its caption
    */
    void fileChanged();
    /**
      Emits messages for the status line
    */
    void statusMsg(const QString &);

    // emitted when saving a remote URL with KIO::NetAccess. In that case we have to disable the UI.
    void enableUI( bool enable );
    void popupMenu(int line, int col);
    void toggledBreakpoint(int line);
    void editedBreakpoint(int line);
    void toggledBreakpointEnabled(int line);

  protected:
    virtual void keyPressEvent( QKeyEvent *ev );
    virtual void customEvent( QCustomEvent *ev );

    int configFlags;
    int wrapAt;

    /*
     * The source, the destination of the copy, and the flags
     * for each job being run (job id is the dict key).
     */
    struct NetData
    {
      KURL m_url;
      QByteArray m_data;
      int m_flags;
    };
    QMap<KIO::Job *, NetData> m_mapNetData;

    KTempFile *m_tempSaveFile;

    /*
     * Check if the given URL already exists. Currently used by both save() and saveAs()
     *
     * Asks the user for permission and returns the message box result and defaults to
     * KMessageBox::Yes in case of doubt
     */
    int checkOverwrite( KURL u );

//text access
  public:
     /**
       Gets the number of text lines;
     */
     int numLines();
     /**
       Gets the complete document content as string
     */
     QString text();
     /**
       Gets the text line where the cursor is on
     */
     QString currentTextLine();
     /**
       Gets a text line
     */
     QString textLine(int num);
     /**
       Gets the word where the cursor is on
     */
     QString currentWord();
     /**
       Gets the word at position x, y. Can be used to find
       the word under the mouse cursor
     */
     QString word(int x, int y);
     /**
       Discard old text without warning and set new text
     */
     void setText(const QString &);
     /**
       Insert text at the current cursor position. If length is a positive
       number, it restricts the number of inserted characters
     */
     virtual void insertText(const QString &, bool mark = false);
     /**
       Queries if there is marked text
     */
     bool hasMarkedText();
     /**
       Gets the marked text as string
     */
     QString markedText();

//url aware file functions
  public:
//    enum fileAction{GET, PUT}; //tells us what kind of job kwrite is waiting for
    enum fileResult { OK, CANCEL, RETRY, ERROR };

#ifdef NEW_CODE
    /**
     * Loads a file @p file using @p codec. 
     * If insert = false the old contents will be deleted.
     */
    void loadFile(const QString &file, QTextCodec *codec, bool insert);
#else
    void loadFile(QIODevice &, bool insert = false);
#endif
    /**
      Writes the document into the given QIODevice
    */
    void writeFile(QIODevice &);
    /**
      Loads the file given in name into the editor
    */
    bool loadFile(const QString &name, int flags = 0);
    /**
      Saves the file as given in name
    */
    bool writeFile(const QString &name);
    /**
      Loads the file given in url into the editor.
      See the lfXXX constants in the .h file.
    */
    void loadURL(const KURL &url, int flags = 0);
    /**
      Saves the file as given in url
    */
    void writeURL(const KURL &url, int flags = 0);

    void loadInternal( const QByteArray &data, const KURL &url, int flags );
  protected slots:
    void slotJobReadResult( KIO::Job *job );
    void slotJobData( KIO::Job *job, const QByteArray &data );

    void slotUpdate();
    void slotFileStatusChanged(); // something is wrong here! the corresponding signal should be in the doc! (Simon)
    void slotNewUndo();
    void slotHighlightChanged();

  public:
    void init();
    /**
      Mainly for internal use. Returns true if the current document can be
      discarded. If the document is modified, the user is asked if he wants
      to save it. On "cancel" the function returns false.
    */
    bool canDiscard();
  public slots:
    /**
      Opens a new untitled document in the text widget. The user is given
      a chance to save the current document if the current document has
      been modified.
    */
    void newDoc();
    /**
      This will present an open file dialog and open the file specified by
      the user, if possible. The user will be given a chance to save the
      current file if it has been modified. This starts the automatic
      highlight selection.
    */
    void open();
    /**
      Calling this method will let the user insert a file at the current
      cursor position.
    */
    void insertFile();
    /**
      Saves the file if necessary under the current file name. If the current file
      name is Untitled, as it is after a call to newFile(), this routing will
      call saveAs().
    */
    fileResult save();
    /**
      Allows the user to save the file under a new name. This starts the
      automatic highlight selection.
    */
    fileResult saveAs();
  protected:
//    KFM *kfm;
//    QString kfmURL;
//    QString kfmFile;
//    fileAction kfmAction;
//    int kfmFlags;

//command processors
  public slots:


//edit functions
  public:
    /**
      Clears the document without any warnings or requesters.
    */
    void clear();
  public slots:
    /**
      Moves the marked text into the clipboard
    */
    void cut() {doEditCommand(KWrite::cmCut);}
    /**
      Copies the marked text into the clipboard
    */
    void copy() {doEditCommand(KWrite::cmCopy);}
    /**
      Inserts text from the clipboard at the actual cursor position
    */
    void paste() {doEditCommand(KWrite::cmPaste);}
    /**
      Undoes the last operation. The number of undo steps is configurable
    */
    void undo() {doEditCommand(KWrite::cmUndo);}
    /**
      Repeats an operation which has been undone before.
    */
    void redo() {doEditCommand(KWrite::cmRedo);}
    /**
      Undoes <count> operations.
      Called by slot undo().
    */
    void undoMultiple(int count);
    /**
      Repeats <count> operation which have been undone before.
      Called by slot redo().
    */
    void redoMultiple(int count);
    /**
      Displays the undo history dialog
    */
    void undoHistory();
    /**
      Moves the current line or the selection one position to the right
    */
    void indent() {doEditCommand(KWrite::cmIndent);};
    /**
      Moves the current line or the selection one position to the left
    */
    void unIndent() {doEditCommand(KWrite::cmUnindent);};
    /**
      Optimizes the selected indentation, replacing tabs and spaces as needed
    */
    void cleanIndent() {doEditCommand(KWrite::cmCleanIndent);};
    /**
      Selects all text
    */
    void selectAll() {doEditCommand(KWrite::cmSelectAll);}
    /**
      Deselects all text
    */
    void deselectAll() {doEditCommand(KWrite::cmDeselectAll);}
    /**
      Inverts the current selection
    */
    void invertSelection() {doEditCommand(KWrite::cmInvertSelection);}
    /**
      comments out current line
    */
    void comment() {doEditCommand(KWrite::cmComment);};
    /**
      removes comment signs in the current line
    */
    void uncomment() {doEditCommand(KWrite::cmUncomment);};

    void keyReturn() {doEditCommand(KWrite::cmReturn);};
    void keyDelete() {doEditCommand(KWrite::cmDelete);};
    void backspace() {doEditCommand(KWrite::cmBackspace);};
    void killLine() {doEditCommand(KWrite::cmKillLine);};

// cursor commands...

    void cursorLeft() {doCursorCommand(KWrite::cmLeft);};
    void shiftCursorLeft() {doCursorCommand(KWrite::cmLeft | selectFlag);};
    void cursorRight() {doCursorCommand(KWrite::cmRight);}
    void shiftCursorRight() {doCursorCommand(KWrite::cmRight | selectFlag);}
    void wordLeft() {doCursorCommand(KWrite::cmWordLeft);};
    void shiftWordLeft() {doCursorCommand(KWrite::cmWordLeft | selectFlag);};
    void wordRight() {doCursorCommand(KWrite::cmWordRight);};
    void shiftWordRight() {doCursorCommand(KWrite::cmWordRight | selectFlag);};
    void home() {doCursorCommand(KWrite::cmHome);};
    void shiftHome() {doCursorCommand(KWrite::cmHome | selectFlag);};
    void end() {doCursorCommand(KWrite::cmEnd);};
    void shiftEnd() {doCursorCommand(KWrite::cmEnd | selectFlag);};
    void up() {doCursorCommand(KWrite::cmUp);};
    void shiftUp() {doCursorCommand(KWrite::cmUp | selectFlag);};
    void down() {doCursorCommand(KWrite::cmDown);};
    void shiftDown() {doCursorCommand(KWrite::cmDown | selectFlag);};
    void scrollUp() {doCursorCommand(KWrite::cmScrollUp);};
    void scrollDown() {doCursorCommand(KWrite::cmScrollDown);};
    void topOfView() {doCursorCommand(KWrite::cmTopOfView);};
    void bottomOfView() {doCursorCommand(KWrite::cmBottomOfView);};
    void pageUp() {doCursorCommand(KWrite::cmPageUp);};
    void shiftPageUp() {doCursorCommand(KWrite::cmPageUp | selectFlag);};
    void pageDown() {doCursorCommand(KWrite::cmPageDown);};
    void shiftPageDown() {doCursorCommand(KWrite::cmPageDown | selectFlag);};
    void top() {doCursorCommand(KWrite::cmTop);};
    void shiftTop() {doCursorCommand(KWrite::cmTop | selectFlag);};
    void bottom() {doCursorCommand(KWrite::cmBottom);};
    void shiftBottom() {doCursorCommand(KWrite::cmBottom | selectFlag);};

//search/replace functions
  public slots:
    /**
      Presents a search dialog to the user
    */
    void find();
    /**
      Presents a replace dialog to the user
    */
    void replace();
    /**
      Repeasts the last search or replace operation. On replace, the
      user is prompted even if the "Prompt On Replace" option was off.
    */
    void findAgain();
    /**
      Presents a "Goto Line" dialog to the user
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
    int searchFlags;
    int replaces;
    SConfig s;
    QDialog *replacePrompt;

//right mouse button popup menu & bookmark menu
  public:
    /**
      Install a Popup Menu. The Popup Menu will be activated on
      a right mouse button press event.
    */
    void installPopup(QPopupMenu *rmb_Menu);

  public slots:
    void setBookmark(int n);
    void gotoBookmark(int n);
    void slotGotoBookmark(); // CAREFUL: relies on sender() !

    /**
      Shows a popup that lets the user choose the bookmark number
    */
    void setBookmark();
    /**
      Adds the actual edit position to the end of the bookmark list
    */
    void addBookmark();
    /**
      Clears all bookmarks
    */
    void clearBookmarks();
    /**
      Sets the cursor to the bookmark n
    */
//    void gotoBookmark(int n);
  protected slots:
    /**
      Updates the bookmark popup menu when it emit aboutToShow()
    */
    void updateBookmarks();

  protected:
    QList<KWBookmark> bookmarks;
    QPopupMenu *rmbMenu;
    QList<KAction> bookmarkActionList;

  signals:
    void bookAddChanged(bool enabled);
    void bookClearChanged(bool enabled);

//config file / session management functions
  public:
    /**
      Reads config entries out of the KConfig object
    */
    void readConfig(KConfig *);
    /**
      Writes config entries into the KConfig object
    */
    void writeConfig(KConfig *);
    /**
      Reads session config out of the KConfig object. This also includes
      the actual cursor position and the bookmarks.
    */
    void readSessionConfig(KConfig *);
    /**
      Writes session config into the KConfig object
    */
    void writeSessionConfig(KConfig *);

  // printing
  public slots:
    void printDlg ();

  protected:
    QPrinter *printer;

  // syntax highlight
  public slots:
    /**
      Presents the highlight setup dialog to the user
    */
    void hlDlg();
    /**
      Gets the highlight number
    */
    int getHl();
    /**
      Sets the highlight number n
    */
    void setHl(int n);
    /**
      Get the end of line mode (Unix, Macintosh or Dos)
    */
    int getEol();
    /**
      Set the end of line mode (Unix, Macintosh or Dos)
    */
    void setEol(int);
    
    // quick hack!!!! by smeier@kdevelop.org
    KWriteView* view(){return kWriteView;}

//internal
  protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

    void doCursorCommand(int cmdNum);
    void doEditCommand(int cmdNum);

    KWriteView *kWriteView;
    KWriteDoc *kWriteDoc;

//spell checker
  public:
    /**
     * Returns the KSpellConfig object
     */
    KSpellConfig *ksConfig(void) {return kspell.ksc;}
    /**
     * Sets the KSpellConfig object.  (The object is
     *  copied internally.)
     */
    void setKSConfig (const KSpellConfig _ksc) {*kspell.ksc=_ksc;}

  public slots:    //please keep prototypes and implementations in same order
    void spellcheck();
    void spellcheck2(KSpell*);
    void misspelling (QString word, QStringList *, unsigned pos);
    void corrected (QString originalword, QString newword, unsigned pos);
    void spellResult (const QString &newtext);
    void spellCleanDone();
  signals:
    /** This says spellchecking is <i>percent</i> done.
      */
    void  spellcheck_progress (unsigned int percent);
    /** Emitted when spellcheck is complete.
     */
    void spellcheck_done ();

  protected:
    // all spell checker data stored in here
    struct _kspell {
      KSpell *kspell;
      KSpellConfig *ksc;
      QString spell_tmptext;
      bool kspellon;              // are we doing a spell check?
      int kspellMispellCount;     // how many words suggested for replacement so far
      int kspellReplaceCount;     // how many words actually replaced so far
      bool kspellPristine;        // doing spell check on a clean document?
    } kspell;

    KAction *editInsert, 
            *editReplace, *editUndoHist,
            *toolsIndent, *toolsUnindent, *toolsCleanIndent,
            *toolsComment, *toolsUncomment, *toolsSpell;

    KToggleAction *setVerticalSelection;
    KRecentFilesAction *fileRecent;
    KSelectAction *setHighlight, *setEndOfLine;
    bool m_singleViewMode;
    bool myDeleteDoc;


    // some kwriteview stuff
  protected:
    void insLine(int line) { kWriteView->insLine(line); };
    void delLine(int line) { kWriteView->delLine(line); };
    void updateCursor() { kWriteView->updateCursor(); };
    void updateCursor(PointStruc &newCursor) { kWriteView->updateCursor(newCursor); };
    void updateCursor(PointStruc &newCursor, int flags) { kWriteView->updateCursor(newCursor, flags); };

    void clearDirtyCache(int height) { kWriteView->clearDirtyCache(height); };
    void tagLines(int start, int end, int x1, int x2) { kWriteView->tagLines(start, end, x1, x2); };
    void tagAll() { kWriteView->tagAll(); };
    void setPos(int x, int y) { kWriteView->setPos(x, y); };
    void center() { kWriteView->center(); };

    void updateView(int flags) { kWriteView->updateView(flags); };

  protected slots:
    // to send dropEventPass
    void dropEventPassEmited (QDropEvent* e);


   signals:
    // emitted when KWriteView is not handling its own URI drops
    void dropEventPass(QDropEvent*);
  // end of kwriteview stuff


  public:
    enum Config_flags {
      cfAutoIndent= 0x1,
      cfBackspaceIndents= 0x2,
      cfWordWrap= 0x4,
      cfReplaceTabs= 0x8,
      cfRemoveSpaces = 0x10,
      cfWrapCursor= 0x20,
      cfAutoBrackets= 0x40,
      cfPersistent= 0x80,
      cfKeepSelection= 0x100,
      cfVerticalSelect= 0x200,
      cfDelOnInput= 0x400,
      cfXorSelect= 0x800,
      cfOvr= 0x1000,
      cfMark= 0x2000,
      cfGroupUndo= 0x4000,
      cfKeepIndentProfile= 0x8000,
      cfKeepExtraSpaces= 0x10000,
      cfMouseAutoCopy= 0x20000,
      cfSingleSelection= 0x40000,
      cfTabIndents= 0x80000,
      cfPageUDMovesCursor= 0x100000,
      cfShowTabs= 0x200000,
      cfSpaceIndent= 0x400000,
      cfSmartHome = 0x800000};

    enum Dialog_results {
      srYes=QDialog::Accepted,
      srNo=10,
      srAll,
      srCancel=QDialog::Rejected};

//search flags
    enum Search_flags {
     sfCaseSensitive=1,
     sfWholeWords=2,
     sfFromCursor=4,
     sfBackward=8,
     sfSelected=16,
     sfPrompt=32,
     sfReplace=64,
     sfAgain=128,
     sfWrapped=256,
     sfFinished=512,
     sfRegularExpression=1024};

//update flags
    enum Update_flags {
     ufDocGeometry=1,
     ufUpdateOnScroll=2,
     ufPos=4};

//load flags
    enum Load_flags {
     lfInsert=1,
     lfNewFile=2,
     lfNoAutoHl=4};

//end of line settings
    enum Eol_settings {eolUnknown,eolUnix,eolMacintosh,eolDos};

//cursor movement commands
    enum Cursor_commands
	   { cmLeft,cmRight,cmWordLeft,cmWordRight,
       cmHome,cmEnd,cmUp,cmDown,
       cmScrollUp,cmScrollDown,cmTopOfView,cmBottomOfView,
       cmPageUp,cmPageDown,cmCursorPageUp,cmCursorPageDown,
       cmTop,cmBottom};
//edit commands
    enum Edit_commands {
		    cmReturn=1,cmDelete,cmBackspace,cmKillLine,cmUndo,
        cmRedo,cmCut,cmCopy,cmPaste,cmIndent,cmUnindent,cmCleanIndent,
        cmSelectAll,cmDeselectAll,cmInvertSelection,cmComment,
        cmUncomment};
//find commands
    enum Find_commands { cmFind=1,cmReplace,cmFindAgain,cmGotoLine};

//bookmark commands
    enum Bookmark_commands {
        cmSetBookmark=1,cmAddBookmark,cmClearBookmarks,
        cmSetBookmarks=10,cmGotoBookmarks=20 };
};


class KWIconBorder : public QWidget
{
public:
    KWIconBorder(KWrite *write, KWriteView *view);
    ~KWIconBorder();

    void paintLine(int i);

protected:
    void paintEvent(QPaintEvent* e);
    void mousePressEvent(QMouseEvent* e);

private:
    
    KWrite *kWrite;
    KWriteView *kWriteView;
    bool lmbSetsBreakpoints;
};

#endif //KWVIEV_H

