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

#ifndef _HIGHLIGHT_H_
#define _HIGHLIGHT_H_

#include <qlist.h>
#include <qdialog.h>

#include <kcolorbtn.h>

class QCheckBox;
class QComboBox;
class QLineEdit;

class TextLine;
class Attribute;

bool isInWord(QChar); //true for '_','0'-'9','A'-'Z','a'-'z'

class HlItem {
  public:
    HlItem(int attribute, int context);
    virtual bool startEnable(QChar) {return true;}
    virtual bool endEnable(QChar) {return true;}
    virtual const QChar *checkHgl(const QChar *) = 0;
    int attr;
    int ctx;
};

class HlItemWw : public HlItem {
  public:
    HlItemWw(int attribute, int context);
    virtual bool startEnable(QChar c) {return !isInWord(c);}
    virtual bool endEnable(QChar c) {return !isInWord(c);}
};


class HlCharDetect : public HlItem {
  public:
    HlCharDetect(int attribute, int context, QChar);
    virtual const QChar *checkHgl(const QChar *);
  protected:
    QChar sChar;
};

class Hl2CharDetect : public HlItem {
  public:
    Hl2CharDetect(int attribute, int context,  QChar ch1, QChar ch2);
    virtual const QChar *checkHgl(const QChar *);
  protected:
    QChar sChar1;
    QChar sChar2;
};

class HlStringDetect : public HlItem {
  public:
    HlStringDetect(int attribute, int context, const QString &);
    virtual ~HlStringDetect();
    virtual const QChar *checkHgl(const QChar *);
  protected:
    const QString str;
};

class HlRangeDetect : public HlItem {
  public:
    HlRangeDetect(int attribute, int context, QChar ch1, QChar ch2);
    virtual const QChar *checkHgl(const QChar *);
  protected:
    QChar sChar1;
    QChar sChar2;
};

/*
class KeywordData {
  public:
    KeywordData(const QString &);
    ~KeywordData();
    char *s;
    int len;
};
*/
class HlKeyword : public HlItemWw {
  public:
    HlKeyword(int attribute, int context);
    virtual ~HlKeyword();
    void addWord(const QString &);
    void addList(const char **);
    virtual const QChar *checkHgl(const QChar *);
  protected:
    QStringList words;
};

class HlInt : public HlItemWw {
  public:
    HlInt(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlFloat : public HlItemWw {
  public:
    HlFloat(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlCInt : public HlInt {
  public:
    HlCInt(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlCOct : public HlItemWw {
  public:
    HlCOct(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlCHex : public HlItemWw {
  public:
    HlCHex(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlCFloat : public HlFloat {
  public:
    HlCFloat(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlCSymbol : public HlItem {
  public:
    HlCSymbol(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlLineContinue : public HlItem {
  public:
    HlLineContinue(int attribute, int context);
    virtual bool endEnable(QChar c) {return c == '\0';}
    virtual const QChar *checkHgl(const QChar *);
};

class HlCStringChar : public HlItem {
  public:
    HlCStringChar(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlCChar : public HlItemWw {
  public:
    HlCChar(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlCPrep : public HlItem {
  public:
    HlCPrep(int attribute, int context);
    virtual bool startEnable(QChar c) {return c == '\0';}
    virtual const QChar *checkHgl(const QChar *);
};

class HlHtmlTag : public HlItem {
  public:
    HlHtmlTag(int attribute, int context);
    virtual bool startEnable(QChar c) {return c == '<';}
    virtual const QChar *checkHgl(const QChar *);
};

class HlHtmlValue : public HlItem {
  public:
    HlHtmlValue(int attribute, int context);
    virtual bool startEnable(QChar c) {return c == '=';}
    virtual const QChar *checkHgl(const QChar *);
};

class HlShellComment : public HlCharDetect {
  public:
    HlShellComment(int attribute, int context);
    virtual bool startEnable(QChar c) {return !isInWord(c);}
};

//modula 2 hex
class HlMHex : public HlItemWw {
  public:
    HlMHex(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};


//ada decimal
class HlAdaDec : public HlItemWw {
  public:
    HlAdaDec(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

//ada base n
class HlAdaBaseN : public HlItemWw {
  public:
    HlAdaBaseN(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

//ada float
class HlAdaFloat : public HlItemWw {
  public:
    HlAdaFloat(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

//ada character
class HlAdaChar : public HlItemWw {
  public:
    HlAdaChar(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlSatherClassname : public HlItemWw {
  public:
    HlSatherClassname(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlSatherIdent : public HlItemWw {
  public:
    HlSatherIdent(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlSatherDec : public HlItemWw {
  public:
    HlSatherDec(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlSatherBaseN : public HlItemWw {
  public:
    HlSatherBaseN(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlSatherFloat : public HlItemWw {
  public:
    HlSatherFloat(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlSatherChar : public HlItemWw {
  public:
    HlSatherChar(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlSatherString : public HlItemWw {
  public:
    HlSatherString(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlLatexTag : public HlItem {
  public:
    HlLatexTag(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlLatexChar : public HlItem {
  public:
    HlLatexChar(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
};

class HlLatexParam : public HlItem {
  public:
    HlLatexParam(int attribute, int context);
    virtual const QChar *checkHgl(const QChar *);
    virtual bool endEnable(QChar c) {return !isInWord(c);}
};

//--------


//Item Style: color, selected color, bold, italic
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
//    ItemFont(const QString & family, int size, const char *charset);
//    void setData(const ItemFont &);
    QString family;
    int size;
    QString charset;
};

//Item Properties: name, Item Style, Item Font
class ItemData : public ItemStyle, public ItemFont {
  public:
    ItemData(const char * name, int defStyleNum);
    ItemData(const char * name, int defStyleNum,
      const QColor&, const QColor&, bool bold, bool italic);

    const char * name;
    int defStyleNum;
    int defStyle; //boolean value
    int defFont;  //boolean value
};

typedef QList<ItemData> ItemDataList;

class HlData {
  public:
    HlData(const QString &wildcards, const QString &mimetypes);
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
    Highlight(const char * name);
    virtual ~Highlight();
    KConfig *getKConfig();
    QString getWildcards();
    QString getMimetypes();
    HlData *getData();
    void setData(HlData *);
    void getItemDataList(ItemDataList &);
    virtual void getItemDataList(ItemDataList &, KConfig *);
    virtual void setItemDataList(ItemDataList &, KConfig *);
    const char * name() {return iName;}
//    QString extensions();
//    QString mimetypes();
    void use();
    void release();
    virtual bool isInWord(QChar c) {return ::isInWord(c);}
    virtual int doHighlight(int ctxNum, TextLine *textLine);
    virtual QString getCommentStart() { return QString(""); };
    virtual QString getCommentEnd() { return QString(""); };
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void init();
    virtual void done();

    const char * iName;
    QString iWildcards;
    QString iMimetypes;
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
    GenHighlight(const char * name);

    virtual int doHighlight(int ctxNum, TextLine *);
  protected:
    virtual void makeContextList() = 0;
    virtual void init();
    virtual void done();

    static const int nContexts = 32;
    HlContext *contextList[nContexts];
};



class CHighlight : public GenHighlight {
  public:
    CHighlight(const char * name);
    virtual ~CHighlight();
    virtual QString getCommentStart() { return QString("/*"); };
    virtual QString getCommentEnd() { return QString("*/"); };
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
    virtual void setKeywords(HlKeyword *keyword, HlKeyword *dataType);
};

class CppHighlight : public CHighlight {
  public:
    CppHighlight(const char * name);
    virtual ~CppHighlight();
    virtual QString getCommentStart() { return QString("//"); };
    virtual QString getCommentEnd() { return QString(""); };
  protected:
    virtual void setKeywords(HlKeyword *keyword, HlKeyword *dataType);
};

class ObjcHighlight : public CHighlight {
  public:
    ObjcHighlight(const char * name);
    virtual ~ObjcHighlight();
    virtual QString getCommentStart() { return QString("//"); };
    virtual QString getCommentEnd() { return QString(""); };
  protected:
    virtual void makeContextList();
    virtual void setKeywords(HlKeyword *keyword, HlKeyword *dataType);
};

class IdlHighlight : public CHighlight {
  public:
    IdlHighlight(const char * name);
    virtual ~IdlHighlight();
    virtual QString getCommentStart() { return QString("//"); };
    virtual QString getCommentEnd() { return QString(""); };
  protected:
    virtual void setKeywords(HlKeyword *keyword, HlKeyword *dataType);
};

class JavaHighlight : public CHighlight {
  public:
    JavaHighlight(const char * name);
    virtual ~JavaHighlight();
    virtual QString getCommentStart() { return QString("//"); };
    virtual QString getCommentEnd() { return QString(""); };
  protected:
    virtual void setKeywords(HlKeyword *keyword, HlKeyword *dataType);
};

class HtmlHighlight : public GenHighlight {
  public:
    HtmlHighlight(const char * name);
    virtual ~HtmlHighlight();
    virtual QString getCommentStart() { return QString("<!--"); };
    virtual QString getCommentEnd() { return QString("-->"); };
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
};

class BashHighlight : public GenHighlight {
  public:
    BashHighlight(const char * name);
    virtual ~BashHighlight();
    virtual QString getCommentStart() { return QString("#"); };
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
};

class ModulaHighlight : public GenHighlight {
  public:
    ModulaHighlight(const char * name);
    virtual ~ModulaHighlight();
    virtual QString getCommentStart() { return QString("(*"); };
    virtual QString getCommentEnd() { return QString("*)"); };
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
};

class AdaHighlight : public GenHighlight {
  public:
    AdaHighlight(const char * name);
    virtual ~AdaHighlight();
    virtual QString getCommentStart() { return QString("--"); };
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
};

class PythonHighlight : public GenHighlight {
  public:
    PythonHighlight(const char * name);
    virtual ~PythonHighlight();
    virtual QString getCommentStart() { return QString("#"); };
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
};

class PerlHighlight : public Highlight {
  public:
    PerlHighlight(const char * name);

    virtual int doHighlight(int ctxNum, TextLine *);
    virtual QString getCommentStart() { return QString("#"); };
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void init();
    virtual void done();
    HlKeyword *keyword;
};

class SatherHighlight : public GenHighlight {
  public:
    SatherHighlight(const char * name);
    virtual ~SatherHighlight();
    virtual QString getCommentStart() { return QString("--"); };
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
};

class LatexHighlight : public GenHighlight {
  public:
    LatexHighlight(const char * name);
    virtual ~LatexHighlight();
    virtual QString getCommentStart() { return QString("%"); };
  protected:
    virtual void createItemData(ItemDataList &);
    virtual void makeContextList();
};

//class KWriteDoc;

class HlManager : public QObject {
    Q_OBJECT
  public:
    HlManager();
    ~HlManager();

    static HlManager *self();

    Highlight *getHl(int n);
    int defaultHl();
    int nameFind(const QString &name);

    int wildcardFind(const QString &fileName);
    int mimeFind(const QByteArray &contents, const QString &fname);
    int findHl(Highlight *h) {return hlList.find(h);}

    int makeAttribs(Highlight *, Attribute *, int maxAttribs);

    int defaultStyles();
    const char * defaultStyleName(int n);
    void getDefaults(ItemStyleList &, ItemFont &);
    void setDefaults(ItemStyleList &, ItemFont &);

    int highlights();
    const char * hlName(int n);
    void getHlDataList(HlDataList &);
    void setHlDataList(HlDataList &);
  signals:
    void changed();
  protected:
    QList<Highlight> hlList;

    static HlManager *s_pSelf;
};

//--------


class StyleChanger : public QWidget {
    Q_OBJECT
  public:
    StyleChanger(QWidget *parent );
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

class FontChanger : public QWidget {
    Q_OBJECT
  public:
    FontChanger(QWidget *parent );
    void setRef(ItemFont *);
  protected slots:
    void familyChanged(const QString &);
    void sizeChanged(int);
    void charsetChanged(const QString &);
  protected:
    void displayCharsets();
    ItemFont *font;
    QComboBox *familyCombo;
    QComboBox *sizeCombo;
    QComboBox *charsetCombo;
};

//--------

#include <kdialogbase.h>

class HighlightDialog : public KDialogBase
{
    Q_OBJECT
  public:
    HighlightDialog(HlManager *, ItemStyleList *, ItemFont *, HlDataList *, int hlNumber,
                    QWidget *parent=0, const char *name=0, bool modal=true);

  protected slots:
    void defaultChanged(int);

    void hlChanged(int);
    void itemChanged(int);
    void changed();
  protected:
    StyleChanger *defaultStyleChanger;
    ItemStyleList *defaultItemStyleList;
    FontChanger *defaultFontChanger;

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
