#ifndef _KWDOC_H_
#define _KWDOC_H_

#include <qobject.h>
#include <qlist.h>
#include <qcolor.h>
#include <qfont.h>
#include <qfontmet.h>

#ifdef QT_I18N
#include <stdio.h>
#define iseucchar(a)  (0xa0 <= (unsigned char)(a) && (unsigned char)(a) <= 0xfe)
#endif

#include "kwview.h"
#include "highlight.h"

#if defined(QT_I18N) && defined(HAVE_NKF_H)
#include <nkf.h>
#endif

class TextLine {
  public:
    TextLine(int attribute = 0, int context = CTX_UNDEF);
    ~TextLine();
    void insert(int pos, const char *, int l);
    void overwrite(int pos, const char *, int l);
    void append(char, int n = 1);
    void del(int pos,int l = 1);
    int length() const;
    void setLength(int l);

    void wrap(TextLine *nextLine, int pos);
    void unWrap(TextLine *nextLine, int pos);

    void removeTrailingWhitespace();
    int firstChar(); // find index of first non-ws char
    int lastChar(); // find index of last non-ws char
    int indentTabs(); // find count of leading tabs in indentation
    int indentSpaces(); // find count of trailing whitespace (after tabs) in indentation

    char getChar(int pos) const;

    void setAttribs(int attribute, int start, int end);
    void setAttr(int attribute);
    int getAttr(int pos) const;
    int getAttr() const;
    int getRawAttr(int pos) const;
    int getRawAttr() const;

    void setContext(int context);
    int getContext();

    const char *getString();
    const char *getText();

    void select(bool sel, int start, int end);
    void selectEol(bool sel, int pos);
    void toggleSelect(int start, int end);
    void toggleSelectEol(int pos);
    int numSelected();
    bool isSelected(int pos);
    bool isSelected();

    int findSelected(int pos);
    int findUnSelected(int pos);
    int findRevSelected(int pos);
    int findRevUnSelected(int pos);

    int cursorX(int pos, int tabChars);

    void markFound(int pos, int l);
    void unmarkFound();

    bool isVisible() { return visible; };
    void setVisible(bool status);

    void move(int pos, int offs);

    int getBPId(){ return bpID; }
    bool isBPEnabled()	{ return bpEnabled; }
    bool isBPPending()	{ return bpPending; }

    void setBPId( int id, bool enabled, bool pending) { bpID = id; bpEnabled = enabled; bpPending = pending; }
    void delBPId() { bpID = 0; }

    bool isBookmarked() { return bookmarked; }
    void toggleBookmark() { bookmarked = !bookmarked; }

  protected:
    void resize(int);

    int len;
    int size;
    char *text;
    unsigned char *attribs;
    unsigned char attr;
    int ctx;
    bool visible;

  private:
    int bpID;
    bool bpEnabled;
    bool bpPending;
    bool bookmarked;
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
    void setPrintFont(const QFont &);
    QFont font, printFont;
    QFontMetrics fm, printFM;
};

class KWAction {
  public:
    enum Action {replace, wordWrap, wordUnWrap, newLine, delLine,
      insLine, killLine};//, doubleLine, removeLine};

    KWAction(Action, PointStruc &aCursor);
    ~KWAction();
    void setData(int aLen, const char *aText, int aTextLen);
    Action action;
    PointStruc cursor;
    int len;
    const char *text;
    int textLen;
    KWAction *next;
};

class KWActionGroup {
  public:
    KWActionGroup(PointStruc &aStart);
    ~KWActionGroup();
    void insertAction(KWAction *);

    PointStruc start;
    PointStruc end;
    KWAction *action;
};

class KWriteDoc : public QObject {
    Q_OBJECT
    friend class KWriteView;
    friend class KWrite;
    friend class HlManager;
    friend class KIconBorder;
    
  public:
    KWriteDoc(HlManager *, const char *path = 0L);
    ~KWriteDoc();

    int lastLine() const;
    TextLine *textLine(int line);
    int textLength(int line);
    void tagLines(int start, int end);
    void tagAll();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);
    void readSessionConfig(KConfig *);
    void writeSessionConfig(KConfig *);

    void readFileConfig();
    void writeFileConfig();
    void readBookmarkConfig(KConfig *config);
    void writeBookmarkConfig(KConfig *config);
    
    int getTextLineCount() { return contents.count(); }

    void loadFile(QIODevice &);
    void writeFile(QIODevice &);
    void updateViews(KWriteView *exclude = 0L);
    QString fileName();
    void setFileName(const QString&);
    void setModified(bool);
    void setText(const char *);
    bool isModified();
    void clearBookmarks();
    void updateBMPopup(QPopupMenu* popup);
    void gotoBookmark(QString &text);
    KWrite* getKWrite();
    QList<KWriteView> viewList() { return views; };
    int viewCount();
    void setUpdatesEnabled( bool bEnabled ){ bUpdateEnabled = bEnabled; }

    void newBracketMark(PointStruc &, BracketMark &);

//  void inheritFileName(KWriteDoc *doc) {
//    fName = QString(doc->fName, doc->fName.findRev('/') +1);
//  }
  protected:
    void registerView(KWriteView *);
    void removeView(KWriteView *);

    int currentColumn(PointStruc &cursor);

    void insert(KWriteView *, VConfig &, const char *);
    void insertFile(KWriteView *, VConfig &, QIODevice &);
//public now    void loadFile(QIODevice &);
//public now    void writeFile(QIODevice &);

    void insertChar(KWriteView *, VConfig &, char);
#ifdef QT_I18N
    void insertChar(KWriteView *, VConfig &, char *, int len);
#endif
    int seekIndentRef(QList<TextLine> &, int & tabs, int & spaces);
    int seekIndentRef(QList<TextLine> &);
    void newLine(KWriteView *, VConfig &);
    void tab(KWriteView *, VConfig &);
    void shiftTab(KWriteView *, VConfig &);
    void commonTab(KWriteView *, VConfig &, bool add = true);
    void killLine(KWriteView *, VConfig &);
    void backspace(KWriteView *, VConfig &);
    void del(KWriteView *, VConfig &);


  protected slots:
    void clipboardChanged();
    void hlChanged();

  public:
    int getHighlight() {return hlManager->findHl(highlight);}
    int getFontHeight() {return fontHeight;}
    QCString text();

    QDateTime getLastFileModifDate() { return m_fileLastModifDate; };
    void setLastFileModifDate(QDateTime modif) { m_fileLastModifDate = modif; };
    void selectTo(PointStruc &start, PointStruc &end, int flags);

  protected:
    void setHighlight(int n);
    void setPreHighlight(int n = -1);
    void makeAttribs();
    void updateFontData();
    void setTabWidth(int);
    void setIndentLength(int length);
    void setBWPrinting(bool);
    void updateLines(int startLine = 0, int curLine = -1, int endLine = 0xffffff, int flags = 0);
    void updateMaxLength(const TextLine *);
    void updateMaxLengthSimple(  QList<TextLine> &contents );

//public now    void updateViews(KWriteView *exclude = 0L);

    int textWidth(const TextLine *, const int cursorX);
    int textWidth(PointStruc &cursor);
    int textWidth(bool wrapCursor, PointStruc &cursor, int xPos);
    int textPos(TextLine *, int xPos);

    int textWidth();
    int textHeight();

    void toggleRect(int, int, int, int);
    void clear();
    void copy(int flags);
    void paste(KWriteView *,VConfig &);
    void cut(KWriteView *, VConfig &);
    void selectAll();
    void deselectAll();
    void invertSelection();
    void selectWord(PointStruc &cursor, int flags);

    QString getWord(PointStruc &cursor);
    bool hasMarkedText() {return (selectEnd >= selectStart);}
    QString markedText(int flags);
    void delMarkedText(KWriteView *, VConfig &);

    QColor &cursorCol(int x, int y);
    void paintTextLine(QPainter &, int line, int y, int xStart, int xEnd, bool showTabs, bool printing);
    void paintTextLine(QPainter &, int line, int xStart, int xEnd, bool showTabs, bool printing=false);

    bool isLastView(int numViews);

    bool hasFileName();
    void clearFileName();

    bool doSearch(SConfig &s, const char *searchFor);
    void unmarkFound();
    void markFound(PointStruc &cursor, int len);

    void tagLine(int line);
    void insLine(int line);
    void delLine(int line);
    void optimizeSelection();
    
    void doAction(KWAction *);
    const char *doReplace(KWAction *);
    void doWordWrap(KWAction *);
    void doWordUnWrap(KWAction *);
    void doNewLine(KWAction *);
    void doDelLine(KWAction *);
    void doInsLine(KWAction *);
    void doKillLine(KWAction *);
    void newUndo();
    void recordStart(PointStruc &, bool keepModal = false);
    void recordAction(KWAction::Action, PointStruc &);
    void recordReplace(PointStruc &, int len, const char *text = 0L, int textLen = 0);
    void recordInsert(PointStruc &, const char *text = 0L, int textLen = 0);
    void recordDelete(PointStruc &, int len);
    void recordEnd(KWriteView *, VConfig &);
    void recordEnd(KWriteView *, PointStruc &, int flags);
    void doActionGroup(KWActionGroup *, int flags);
    void undo(KWriteView *, int flags);
    void redo(KWriteView *, int flags);
    void setUndoSteps(int steps);

    void setPseudoModal(QWidget *);

    void indent(KWriteView *, VConfig &);
    void unIndent(KWriteView *, VConfig &);
    void comment(KWriteView *, VConfig &);
    void unComment(KWriteView *, VConfig &);

    QList<TextLine> contents;
    QColor colors[5];
    HlManager *hlManager;
    Highlight *highlight;
    Attribute attribs[nAttribs];
    
    KConfig* kWriteConfig;

    int eolMode;

    int tabChars;
    int tabWidth, printTabWidth;
    int fontHeight, printFontHeight;
    int fontAscent, printFontAscent;
    int indentLength;
    bool bwPrinting;

    QList<KWriteView> views;
    bool newDocGeometry;

    TextLine const *longestLine;
    int maxLength;

    PointStruc select;
    PointStruc anchor;
    int selectStart;
    int selectEnd;

    bool modified;
    bool oldMarkState;
    QString fName;

    KConfig* fileConfig;

    int foundLine;

    QList<KWActionGroup> undoList;
    int currentUndo;
    int undoState;
    int undoSteps;
    int tagStart;
    int tagEnd;

    QWidget *pseudoModal;

    QDateTime m_fileLastModifDate;

#if defined(QT_I18N) && defined(HAVE_NKF_H)
    Nkf::NkfCode JPcode;
#endif
    bool bUpdateEnabled;
};

#endif //KWDOC_H
