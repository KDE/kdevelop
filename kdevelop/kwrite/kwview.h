#ifndef _KWVIEV_H_
#define _KWVIEV_H_

#include <qpoint.h>

#include <qscrbar.h>
#include <qiodev.h>
#include <kpopupmenu.h>
#include <kconfig.h>

#include "kwdialog.h"

class KWriteDoc;
class Highlight;
class KURL;

//icon-paint
const int iconBorderWidth		= 16;
const int iconBorderHeight	= 800;

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
const int cfAutoIndent        = 1;
const int cfBackspaceIndent   = 2;
const int cfWordWrap          = 4;
const int cfReplaceTabs       = 8;
const int cfRemoveSpaces      = 16;
const int cfWrapCursor        = 32;
const int cfAutoBrackets      = 64;

const int cfPersistent        = 128;
const int cfKeepSelection     = 256;
const int cfVerticalSelect    = 512;
const int cfDelOnInput        = 1024;
const int cfXorSelect         = 2048;

const int cfOvr               = 4096;
const int cfMark              = 8192;

const int cfAutoCopy          = 16384;

const int cfShowTabs          = 32768;
const int cfHighlightBrackets = 65536;
const int cfTabIndent         = 1L << 17;

const int cfIndentBraces      = 1L << 18;
const int cfIndentParentheses = 1L << 19;


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

void resizeBuffer(void *user, int w, int h);

struct PointStruc {
  PointStruc() {}
  PointStruc(int _x, int _y) : x(_x), y(_y) {}
  int x;
  int y;
};

struct BracketMark {
  PointStruc cursor;
  int sXPos;
  int eXPos;
};

struct VConfig {
  PointStruc cursor;
  int flags;
  int wrapAt;
};

struct SConfig {
  PointStruc cursor;
  PointStruc startCursor;
  int flags;
};

extern int kw_bookmark_keys[];

class KWrite;
class KWriteView;

class KIconBorder : public QWidget {
    Q_OBJECT
  public:
    KIconBorder(KWrite *, KWriteDoc *, KWriteView *);
    ~KIconBorder();

    /** Paints line */
    void paintLine(int line);
    /** Clear line*/
    void clearLine(int line);
    /** Clear the icon border */
    void clearAll();
    /** Paint a bookmark on the line if needed */
    void paintBookmark(int line);
    /** Paint a breakpoint on the line if needed */
    void paintBreakpoint(int line);
    /** Painst an icon where the debugger has stopped */
    void paintDbgPosition(int line);

    /**  */
    virtual void mousePressEvent(QMouseEvent* e);
    /**  */
    virtual void paintEvent(QPaintEvent* e);

protected:
    /** Clear some pixel lines */
    void clearPixelLines(int startPixelLine, int numberPixelLine);
    /** Paint an icon to y */
    void showIcon(const QPixmap& icon, int y);

  protected slots:
    /**  */
    void slotGetRange();
    /**  */
    void slotToggleBookmark();
    /**  */
    void slotLMBMenuToggle();
    /**  */
    void slotEditBreakpoint();
    /** */
    void slotToggleBPEnabled();

  private:
    KWrite *kWrite;
    KWriteDoc *kWriteDoc;
    KWriteView *kWriteView;

    KPopupMenu selectMenu;
    /** action when user left clicks in border */
    bool LMBIsBreakpoint;
    /** cursor is on line number */
    int cursorOnLine;
    /** the checkable menu items */
    int menuId_LMBBrkpoint;
    int menuId_LMBBookmark;
    int menuId_editBrkpoint;
    int menuId_enableBrkpoint;
};

class KWriteView : public QWidget {
    Q_OBJECT
    friend class KWriteDoc;
    friend class KWrite;
    friend class KIconBorder;
  public:
    KWriteView(KWrite *, KWriteDoc *);
    ~KWriteView();

    void cursorLeft(VConfig &);
    void cursorRight(VConfig &);
    void cursorLeftWord(VConfig &);
    void cursorRightWord(VConfig &);
    void cursorUp(VConfig &);
    void cursorDown(VConfig &);
    void home(VConfig &);
    void end(VConfig &);
    void pageUp(VConfig &);
    void pageDown(VConfig &);
    void top(VConfig &);
    void bottom(VConfig &);

    int getXPos() { return xPos;}
    int getYPos() { return yPos;}
    int getRange(int midline);
    QPoint cursorPosition() { return QPoint(cursor.x, cursor.y); };
    QPoint getCursorCoordinates() const;

protected slots:
    void changeXPos(int);
    void changeYPos(int);

  protected:
    virtual bool event ( QEvent * );

    void paintBracketMark();

    void getVConfig(VConfig &);
    void update(VConfig &);
//    void updateCursor(PointStruc &start, PointStruc &end, bool insert);
    void insLine(int line);
    void delLine(int line);
    void updateCursor();
    void updateCursor(PointStruc &newCursor);
    void updateView(int flags, int newXPos = 0, int newYPos = 0);
    void scroll(int, int);
//  void scroll2(int, int);
    void wheelEvent(QWheelEvent *);
    void tagLines(int start, int end);
    void tagAll();

    void paintTextLines(int xPos, int yPos);
    void paintCursor();

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
    virtual bool focusNextPrevChild( bool next);

    KWrite *kWrite;
    KWriteDoc *kWriteDoc;
    QScrollBar *xScroll;
    QScrollBar *yScroll;

    KIconBorder *leftBorder;

    int xPos;
    int yPos;

    int mouseX;
    int mouseY;
    int scrollX;
    int scrollY;
    int scrollTimer;

    BracketMark bm;

    PointStruc cursor;
    bool cursorOn;
    int cursorTimer;
    int cXPos;
    int cOldXPos;
    bool exposeCursor;//cursorMoved;

    int startLine;
    int endLine;
    int updateState;
    int updateLines[2];
    bool bIsPainting;

    QPixmap *drawBuffer;
	  bool m_hasFocus;
};

/*
class KWBookmark {
  public:
    KWBookmark();
    int xPos;
    int yPos;
    PointStruc cursor;
    QString Name;
};
*/

/** An edit widget with many options, document/view architecture and syntax
    highlight.
    @author Jochen Wilhelmy
*/

class KWrite : public QWidget {
    Q_OBJECT
    friend class KWriteView;
    friend class KWriteDoc;
    friend class KIconBorder;
  public:
    /** The document can be used by more than one KWrite objects
    */
    KWrite(KWriteDoc *, QWidget *parent = 0L, const char *name = 0L);
    /** The destructor does not delete the document
    */
    ~KWrite();

// dbg functions
  void setBreakpoint( int line, int id, bool enabled = true, bool pending = false );
  void delBreakpoint( int line );

  void setStepLine( int line );
  void clearStepLine();
  int getStepLine(){ return stepLine; }

  /** */
  QString textLine( int ) const;

  /** */
  void insertText( const QString& text, bool mark=FALSE );

  QPoint getCursorCoordinates() const;

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
    void setModified(bool);
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

    int flags() const		{ return configFlags; }

  public slots:
    /** Presents a options dialog to the user
    */
    void optDlg();
    /** Presents a color dialog to the user
    */
    void colDlg();
    /** Toggles "Vertical Selections" option
    */
    void toggleVertical();
    /** Toggles Overwrite mode
    */
    void toggleOverwrite();

    /**  */
    void slotToggleBreakpoint();
    /**  */
    void pasteStr(QString s);
    /**  */
    void delMarkedText();

  signals:
    /** The cursor position has changed. Get the values with currentLine()
        and currentColumn()
    */
    void newCurPos();
    /** Modified flag or config flags have changed
    */
    void newStatus();
    /** reflects the mark state
    */
    void markStatus(KWriteView *, bool);
    /** reflects the 'text in clipboard' state
    */
    void clipboardStatus(KWriteView *, bool);
    /** Emits messages for the status line
    */
    void statusMsg(const char *);
    /** The file name has changed. The main window can use this to change
        its caption
    */
    void newCaption();
    /** The undo/redo enable status has changed
    */
    void newUndo();

    // dbg
    void toggleBreakpoint( const QString&, int );
    void toggleBPEnabled( const QString&, int );
    void editBreakpoint( const QString&, int );
    void deleteLine( int );
    void clearAllBreakpoints();

  protected:
    int configFlags;
    int wrapAt;

//text access
  public:
     /** Gets the complete document content as string
     */
     QCString text();
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
     /** Gets the marked text as string
     */
     QString markedText();

//url aware file functions
  public:
    enum action{GET, PUT}; //tells us what kind of job kwrite is waiting for
    /** Loads a file from the given QIODevice. For insert = false the old
        contents will be lost.
    */
    void loadFile(QIODevice &, bool insert = false);
    /** Writes the document into the given QIODevice
    */
    void writeFile(QIODevice &);
    /** Loads the file given in name into the editor
    */
    bool loadFile(const QString& name, int flags = 0);
    /** Saves the file as given in name
    */
    bool writeFile(const QString& name);
    /** Loads the file given in url into the editor.
        See the lfXXX constants in the .h file.
    */
    void loadURL(const KURL& url, int flags = 0);
    /** Saves the file as given in url
    */
    void writeURL(const KURL& url, int flags = 0);

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
    void setFileName(const QString&);
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
    void save();
    /** Allows the user to save the file under a new name. This starts the
        automatic highlight selection.
    */
    void saveAs();
  protected:
    //KFM *kfm;
    QString kfmURL;
    QString kfmFile;
    action kfmAction;
    int kfmFlags;

//edit functions
  public:
    /** Clears the document without any warnings or requesters.
    */
    void clear();
  public slots:
    /** Moves the marked text into the clipboard
    */
    void cut();
    /** Copies the marked text into the clipboard
    */
    void copy();
    /** Inserts text from the clipboard at the actual cursor position
    */
    void paste();
    /** Undoes the last operation. The number of undo steps is configurable
    */
    void undo();
    /** Repeats an operation which has been undone before.
    */
    void redo();
    /** Moves the current line or the selection one position to the right
    */
    void indent();
    /** Moves the current line or the selection one position to the left
    */
    void unIndent();
    /** Adds "//" at the beginning of the current line or of each line in the selection
    */
    void comment();
    /** Removes "//" at the beginning of the current line or of each line in the selection
    */
    void unComment();
    /** Selects all text
    */
    void selectAll();
    /** Deselects all text
    */
    void deselectAll();
    /** Inverts the current selection
    */
    void invertSelection();

//search/replace functions
  public slots:
    /** Presents a search dialog to the user
    */
    void search();
    /** Presents a replace dialog to the user
    */
    void replace();
    /** Repeasts the last search or replace operation. On replace, the
        user is prompted even if the "Prompt On Replace" option was off.
    */
    void searchAgain(bool back=false);
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
		void gotoPos(int cursorXPos, int cursorYPos);
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

////right mouse button popup menu
//  public:
//    /** Install a Popup Menu. The Popup Menu will be activated on
//        a right mouse button press event.
//    */
////    void installRBPopup(QPopupMenu *);
//  protected:
////    QPopupMenu *popup;

//bookmarks
  public:
    /** Install a Bookmark Menu. The bookmark items will be added to the
        end of the menu
    */
    void installBMPopup(QPopupMenu *);
    /** Sets the actual edit position as bookmark number n
    */
//    void setBookmark(int n);
    /** Is there a bookmark on the line?
    */
    bool bookmarked(int line);
    /** Get a pointer to the bookmark list
    */
//    QList<KWBookmark>* getBookmarks() { return &bookmarks; }
public slots:
    /** Shows a popup that lets the user choose the bookmark number
    */
//    void setBookmark();
    /** Toggle a bookmark at the actual edit position
    */
    void toggleBookmark();
    /** Toggle a bookmark at line
    */
    void toggleBookmark(int line);
    /** Go to the next bookmark
    */
    void nextBookmark();
    /** Go to the previous bookmark
    */
    void previousBookmark();
    /** Clears all bookmarks
    */
    void clearBookmarks();
    /** Sets the cursor to the bookmark n
    */
    void gotoBookmark(int n);
  protected slots:
    void updateBMPopup();
//  protected:
//    QList<KWBookmark> bookmarks;
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
    virtual void keyPressEvent(QKeyEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

    KWriteView *kWriteView;
    KWriteDoc *kWriteDoc;

    int stepLine;
};


#endif //KWVIEV_H


