// $Id$

#ifndef _KTEXTPRINT_H_
#define _KTEXTPRINT_H_

#include <sys/types.h> // pid_t
#include <signal.h> // struct sigaction

class QGroupBox;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;

class KConfig;

#include <qtextstream.h>

#include <kdialogbase.h>


struct KTextPrintConfigData;


class KTextPrint {
  public:
    enum ColorMode {Color, GrayScal, BlackWhite};
    enum Orientation {Portrait, Landscape};
    // font style
    static const int Bold = 1;
    static const int Italics = 2;
    // flags
    static const int Title = 1;
    static const int PageNumbers = 2;

    KTextPrint(KTextPrintConfigData &, QIODevice *dev);
//    ~KTextPrint()
/*    void setMedia(Media);
    void setMedia(int paperWidth, int paperHeight, const char *mediaName = 0L);
    void setOrientation(Orientation o) {m_orientation = o;}
    void setColorMode(ColorMode cm) {m_colorMode = cm;}
    void setTitle(const QString &title) {m_title = title;}
    void setLayout(int numCols, int numRows, int numLines);
    void setLineCounter(int countStep) {m_countStep = countStep;}
    void setFlags(int flags) {m_flags = flags;}*/
    void setTabWidth(int tw) {m_tabWidth = tw;}

//    void init(int docLines);
    void defineColor(int colorNum, int r, int g, int b);
    void begin();

    void setFontStyle(int fontStyle) {m_fontStyle = fontStyle;}
    void setColor(int colorNum) {m_colorNum = colorNum;}
    void print(const QChar *s, int len);
    void newLine();
    void end();

  protected:
    void writeProc(const char *pName, const char *commands);
    void writeCount();
    void writeColor(int colorNum);
    void printInternal(const QChar *s, int len);

    void newCell();
    void newPage();

//    QIODevice &m_dev;
    QTextStream m_s;

    int m_paperWidth;
    int m_paperHeight;
    const char *m_mediaName;
    Orientation m_orientation;
    ColorMode m_colorMode;

    QString m_title;

    float m_pageX;
    float m_pageY;
    float m_pageW;
    float m_pageH;

//    float m_titleH;
//    float m_titleGapY;

    float m_gapX;
    float m_gapY;
    float m_cellW;
    float m_cellH;

    int m_currentLine;
    int m_numLines;
    int m_currentCell;
    int m_currentCol;
    int m_numCols;
    int m_currentRow;
    int m_numRows;
    int m_currentPage;
    int m_numPages;

    int m_countDigits;
    int m_countStep;

    int m_flags;

    int m_tabWidth;
    int m_x;
    int m_fontStyle;
    int m_colorNum;
    int m_lastCol;
};

struct KTextPrintConfigData {
  int paperWidth;
  int paperHeight;
  const char *mediaName;
  KTextPrint::ColorMode colorMode;
  KTextPrint::Orientation orientation;
  QString title;
  int numLines;
  int numCols;
  int numRows;
  int countStep;
  int flags;
  int docLines;
};

class KTextPrintDialog;

class KTextPrintConfig : public QObject {
    Q_OBJECT
    friend KTextPrintDialog;
  public:
    enum Media {
      A4,
      B5,
      Letter,
      Legal,
      Executive,
      A0,
      A1,
      A2,
      A3,
      A5,
      A6,
      A7,
      A8,
      A9,
      B0,
      B1,
      B10,
      B2,
      B3,
      B4,
      B6,
      B7,
      B8,
      B9,
      C5E,
      Comm10E,
      DLE,
      Folio,
      Ledger,
      Tabloid,
      NPageSize};

    KTextPrintConfig();
    static void print(QWidget *parent, KConfig *, bool dialog,
      const QString &title, int docLines, QObject *receiver, const char *member);
    void readConfig(KConfig *);
    void writeConfig(KConfig *);
    void setData(KTextPrintConfigData &);
    void getData(KTextPrintConfigData &);
  signals:
    void doPrint(KTextPrint &);

  protected:
    int m_destination;
    QStringList m_printers;
    QString m_command;
    int m_copies;
    QString m_fileName;
    KTextPrint::ColorMode m_colorMode;
    KTextPrint::Orientation m_orientation;
    Media m_media;
    int m_numLines;
    int m_numCols;
    int m_numRows;
    int m_countStep;
    int m_flags;
};

class KTextPrintPreview : public QWidget {
    Q_OBJECT
  public:
    KTextPrintPreview(QWidget *parent);
  public slots:
    void setOrient(int);
    void setCols(int);
    void setRows(int);
    void setTitle(bool);
    void setPageNumbers(bool);
  protected:
    virtual void paintEvent(QPaintEvent *);

    int m_orient;
    int m_cols;
    int m_rows;
    bool m_title;
    bool m_pageNumbers;
};

class KTextPrintDialog : public KDialogBase {
    Q_OBJECT
  public:
    KTextPrintDialog(QWidget *parent, KTextPrintConfig &);
 //   void print();
    KTextPrint::ColorMode colorMode();
    KTextPrint::Orientation orientation();
    void setData(KTextPrintConfig &);
    void getData(KTextPrintConfig &);
//    void readConfig(KConfig *);
//    void writeConfig(KConfig *);
//  signals:
//    void doPrint(KTextPrint &);
  public slots:
    void setDest(int);
  protected slots:
    void choosePrinter();
    void browseFile();
  protected:
    QGroupBox *setupDestination(QWidget *parent);
    QGroupBox *setupOptions(QWidget *parent);
//    virtual void accept();

    // printer
    QRadioButton *m_toPrinter;
    QComboBox *m_printerName;
//    QStringList m_printers;
    QPushButton *m_choose;
    QLineEdit *m_command;
    QSpinBox *m_copies;
    // file
    QRadioButton *m_toFile;
    QLineEdit *m_fileName;
    QPushButton *m_browse;
    // color
    QRadioButton *m_color;
    QRadioButton *m_grayScale;
    QRadioButton *m_blackWhite;
    // orientation
    QRadioButton *m_portrait;
    QRadioButton *m_landscape;
    // media
    QComboBox *m_media;
    // layout
    KTextPrintPreview *m_preview;
    QSpinBox *m_cols;
    QSpinBox *m_rows;
    QSpinBox *m_lines;

    QSpinBox *m_countStep;
    QCheckBox *m_title;
    QCheckBox *m_pageNumbers;
};

class KPipe : public QIODevice {
  public:
    KPipe(QString program);
    ~KPipe();

    bool open() {return open(IO_WriteOnly);}
    virtual bool open(int mode);
    virtual void close();
    virtual void flush() {}

    virtual uint size() const {return 0;}

    virtual int readBlock(char *, uint) {return 0;}
    virtual int writeBlock(const char *data, uint len);

    virtual int getch() {return 0;}
    virtual int putch(int i) {char ch = i; return writeBlock(&ch, 1);}
    virtual int ungetch(int) {return 0;}

  protected:
    char **m_argv;
    char *m_args;
    int m_pipe[2];
    pid_t m_pid;
    struct sigaction m_oldAct;
};


#endif // _KTEXTPRINT_H_
