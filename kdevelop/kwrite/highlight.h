#ifndef _HIGHLIGHT_H_
#define _HIGHLIGHT_H_

#include <qlist.h>
//#include <qlistbox.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <qdialog.h>

#include <kcolorbtn.h>

class TextLine;
class Attribute;

bool testWw(char c); //whole word check: false for '_','0'-'9','A'-'Z','a'-'z'

class HlItem {
  public:
    HlItem(int attribute, int context);
    virtual bool startEnable(char) {return true;}
    virtual bool endEnable(char) {return true;}
    virtual const char *checkHgl(const char *) = 0;
    int attr;
    int ctx;
};

class HlItemWw : public HlItem {
  public:
    HlItemWw(int attribute, int context);
    virtual bool startEnable(char c) {return testWw(c);}
    virtual bool endEnable(char c) {return testWw(c);}
};


class HlCharDetect : public HlItem {
  public:
    HlCharDetect(int attribute, int context, char);
    virtual const char *checkHgl(const char *);
  protected:
    char sChar;
};

class Hl2CharDetect : public HlItem {
  public:
    Hl2CharDetect(int attribute, int context, const char *);
    virtual const char *checkHgl(const char *);
  protected:
    char sChar[2];
};

class HlStringDetect : public HlItem {
  public:
    HlStringDetect(int attribute, int context, const char *);
    virtual ~HlStringDetect();
    virtual const char *checkHgl(const char *);
  protected:
    char *str;
    int len;
};

class HlRangeDetect : public HlItem {
  public:
    HlRangeDetect(int attribute, int context, const char *);
    virtual const char *checkHgl(const char *);
  protected:
    char sChar[2];
};


class KeywordData {
  public:
    KeywordData(const char *);
    ~KeywordData();
    char *s;
    int len;
};

class HlKeyword : public HlItemWw {
  public:
    HlKeyword(int attribute, int context);
    virtual ~HlKeyword();
    void addWord(const char *);
    void addList(char **);
    virtual const char *checkHgl(const char *);
  protected:
    QList<KeywordData> words;
};

class HlInt : public HlItemWw {
  public:
    HlInt(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlFloat : public HlItemWw {
  public:
    HlFloat(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlCInt : public HlInt {
  public:
    HlCInt(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlCOct : public HlItemWw {
  public:
    HlCOct(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlCHex : public HlItemWw {
  public:
    HlCHex(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlCFloat : public HlFloat {
  public:
    HlCFloat(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlLineContinue : public HlItem {
  public:
    HlLineContinue(int attribute, int context);
    virtual bool endEnable(char c) {return c == '\0';}
    virtual const char *checkHgl(const char *);
};

class HlCStringChar : public HlItem {
  public:
    HlCStringChar(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlCChar : public HlItemWw {
  public:
    HlCChar(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

class HlCPrep : public HlItem {
  public:
    HlCPrep(int attribute, int context);
    virtual bool startEnable(char c) {return c == '\0';}
    virtual const char *checkHgl(const char *);
};

class HlHtmlTag : public HlItem {
  public:
    HlHtmlTag(int attribute, int context);
    virtual bool startEnable(char c) {return c == '<';}
    virtual const char *checkHgl(const char *);
};

class HlHtmlValue : public HlItem {
  public:
    HlHtmlValue(int attribute, int context);
    virtual bool startEnable(char c) {return c == '=';}
    virtual const char *checkHgl(const char *);
};

class HlShellComment : public HlCharDetect {
  public:
    HlShellComment(int attribute, int context);
    virtual bool startEnable(char c) {return testWw(c);}
};

//modula 2 hex
class HlMHex : public HlItemWw {
  public:
    HlMHex(int attribute, int context);
    virtual const char *checkHgl(const char *);
};


//ada decimal
class HlAdaDec : public HlItemWw {
  public:
    HlAdaDec(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

//ada base n
class HlAdaBaseN : public HlItemWw {
  public:
    HlAdaBaseN(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

//ada float
class HlAdaFloat : public HlItemWw {
  public:
    HlAdaFloat(int attribute, int context);
    virtual const char *checkHgl(const char *);
};

//ada char
class HlAdaChar : public HlItemWw {
  public:
    HlAdaChar(int attribute, int context);
    virtual const char *checkHgl(const char *);
};


//Item Style: color, selected color, bold italic
class ItemStyle {
  public:
    ItemStyle();
//    ItemStyle(const ItemStyle &);
    ItemStyle(const QColor &, const QColor &, bool bold, bool italic);
//    void setData(const ItemStyle &);
    QColor col;
    QColor selCol;
    int bold;   //boolean value
    int italic; //boolean value
};

typedef QList<ItemStyle> ItemStyleList;

//Item Font: family, size, charset
class ItemFont {
  public:
    ItemFont();
//    ItemFont(const ItemFont &);
//    ItemFont(const char *family, int size, const char *charset);
//    void setData(const ItemFont &);
    QString family;
    int size;
    QString charset;
};

//Item Properties: name, Item Style, Item Font
class ItemData : public ItemStyle, public ItemFont {
  public:
    ItemData(const char *name, int defStyleNum);
    ItemData(const char *name, int defStyleNum,
      const QColor&, const QColor&, bool bold, bool italic);

    QString name;
    int defStyleNum;
    int defStyle; //boolean value
    int defFont;  //boolean value
};

typedef QList<ItemData> ItemDataList;

class HlData {
  public:
    HlData(const char *wildcards, const char *mimetypes);
    ItemDataList itemDataList;
    QString wildcards;
    QString mimetypes;
};

typedef QList<HlData> HlDataList;

class HlManager;
class KConfig;

class Highlight {
    friend HlManager;
  public:
    Highlight(const char *name);
    virtual ~Highlight();
    KConfig *getKConfig();
    void getWildcards(QString &);
    void getMimetypes(QString &);
    HlData *getData();
    void setData(HlData *);
    void getItemDataList(ItemDataList &);
    virtual void getItemDataList(ItemDataList &, KConfig *);
    virtual void setItemDataList(ItemDataList &, KConfig *);
    const char *name();
//    const char *extensions();
//    const char *mimetypes();
    void use();
    void release();
    virtual bool isInWord(char c) {return !testWw(c);}
    virtual void doHighlight(int ctxNum, TextLine *textLine);
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void init();
    virtual void done();

    QString iName;
    QString dw;
    QString dm;
//    QString iMimetypes;
    int refCount;
};


//context
class HlContext {
  public:
    HlContext(int attribute, int lineEndContext);
    QList<HlItem> items;
    int attr;
    int ctx;
};

class GenHighlight : public Highlight {
  public:
    GenHighlight(const char *name);

    virtual void doHighlight(int ctxNum, TextLine *);
  protected:
    virtual void makeContextList() = 0;
    virtual void init();
    virtual void done();
    HlContext *contextList[32];
};



class CHighlight : public GenHighlight {
  public:
    CHighlight(const char *name);
    virtual ~CHighlight();
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
    virtual void setKeywords(HlKeyword *keyword, HlKeyword *dataType);
};

class CppHighlight : public CHighlight {
  public:
    CppHighlight(const char *name);
    virtual ~CppHighlight();
  protected:
    virtual void setKeywords(HlKeyword *keyword, HlKeyword *dataType);
};

class JavaHighlight : public CHighlight {
  public:
    JavaHighlight(const char *name);
    virtual ~JavaHighlight();
  protected:
    virtual void setKeywords(HlKeyword *keyword, HlKeyword *dataType);
};

class HtmlHighlight : public GenHighlight {
  public:
    HtmlHighlight(const char *name);
    virtual ~HtmlHighlight();
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
};

class BashHighlight : public GenHighlight {
  public:
    BashHighlight(const char *name);
    virtual ~BashHighlight();
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
};

class ModulaHighlight : public GenHighlight {
  public:
    ModulaHighlight(const char *name);
    virtual ~ModulaHighlight();
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
};

class AdaHighlight : public GenHighlight {
  public:
    AdaHighlight(const char *name);
    virtual ~AdaHighlight();
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
};

class PythonHighlight : public GenHighlight {
  public:
    PythonHighlight(const char *name);
    virtual ~PythonHighlight();
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
};

class PerlHighlight : public Highlight {
  public:
    PerlHighlight(const char *name);

    virtual void doHighlight(int ctxNum, TextLine *);
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void init();
    virtual void done();
    HlKeyword *keyword;
};


class KWriteDoc;

class HlManager : public QObject {
    Q_OBJECT
  public:
    HlManager();
    ~HlManager();

    Highlight *getHl(int n);
    int defaultHl();
    int nameFind(const char *name);
    
    int highlightFind(KWriteDoc *doc);
    int wildcardFind(const char *fileName);
    int mimeFind(KWriteDoc *doc);
    int findHl(Highlight *h) {return hlList.find(h);}
    
    void makeAttribs(Highlight *, Attribute *, int n);

    int defaultStyles();
    const char *defaultStyleName(int n);
    void getDefaults(ItemStyleList &, ItemFont &);
    void setDefaults(ItemStyleList &, ItemFont &);

    int highlights();
    const char *hlName(int n);
    void getHlDataList(HlDataList &);
    void setHlDataList(HlDataList &);
  signals:
    void changed();
  protected:
    QList<Highlight> hlList;
};

//--------


class StyleChanger : public QObject {
    Q_OBJECT
  public:
    StyleChanger(QWidget *parent, int x, int y);
    void setRef(ItemStyle *);
    void setEnabled(bool);
  protected slots:
    void changed();
  protected:
    ItemStyle *style;
    KColorButton *col;
    KColorButton *selCol;
    QCheckBox *bold;
    QCheckBox *italic;
};

class FontChanger : public QObject {
    Q_OBJECT
  public:
    FontChanger(QWidget *parent, int x, int y);
    void setRef(ItemFont *);
  protected slots:
    void familyChanged(const char *);
    void sizeChanged(int);
    void charsetChanged(const char *);
  protected:
    void displayCharsets();
    ItemFont *font;
    QComboBox *familyCombo;
    QComboBox *sizeCombo;
    QComboBox *charsetCombo;
};

//--------

class DefaultsDialog : public QDialog {
    Q_OBJECT
  public:
    DefaultsDialog(HlManager *, ItemStyleList *, ItemFont *, QWidget *parent);
  protected slots:
    void changed(int);
  protected:
    StyleChanger *styleChanger;
    ItemStyleList *itemStyleList;
};

class HighlightDialog : public QDialog {
    Q_OBJECT
  public:
    HighlightDialog(HlManager *, HlDataList *, int hlNumber, QWidget *parent);

  protected slots:
    void hlChanged(int);
    void itemChanged(int);
    void changed();
  protected:
    void writeback();
    virtual void done(int r);
    QComboBox *itemCombo, *hlCombo;
    QLineEdit *wildcards;
    QLineEdit *mimetypes;
    QCheckBox *styleDefault;
    QCheckBox *fontDefault;
    StyleChanger *styleChanger;
    FontChanger *fontChanger;

    HlDataList *hlDataList;
    HlData *hlData;
    ItemData *itemData;
};

#endif //_HIGHLIGHT_H_
