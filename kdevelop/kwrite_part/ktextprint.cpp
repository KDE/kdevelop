#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qfile.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>

#include <kapp.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include "ktextprint.h"
#include "ktextprint.moc"

struct MediaDef {
  const char *name;
  int width;
  int height;
};

static MediaDef mediaList[] = {
  {"A4"       ,  595,  842},
  {"B5"       ,  516,  729},
  {"Letter"   ,  612,  791},
  {"Legal"    ,  612, 1009},
  {"Executive",  541,  720},
  {"A0"       , 2384, 3370},
  {"A1"       , 1684, 2384},
  {"A2"       , 1191, 1684},
  {"A3"       ,  842, 1191},
  {"A5"       ,  420,  595},
  {"A6"       ,  297,  420},
  {"A7"       ,  210,  297},
  {"A8"       ,  148,  210},
  {"A9"       ,  105,  148},
  {"B0"       , 2920, 4127},
  {"B1"       , 2064, 2920},
  {"B10"      ,   91,  127},
  {"B2"       , 1460, 2064},
  {"B3"       , 1032, 1460},
  {"B4"       ,  729, 1032},
  {"B6"       ,  516,  729},
  {"B7"       ,  363,  516},
  {"B8"       ,  258,  363},
  {"B9"       ,  181,  258},
  {"C5E"      ,  127,  181},
  {"Comm10E"  ,  461,  648},
  {"DLE"      ,  297,  684},
  {"Folio"    ,  312,  624},
  {"Ledger"   ,  595,  935},
  {"Tabloid"  , 1224,  792},
  {"NPageSize",  792, 1224},
  {0L         ,    0,    0}};



KTextPrint::KTextPrint(KTextPrintConfigData &data, QIODevice *dev)
  : m_s(dev) {

  m_s.setEncoding(QTextStream::Latin1);

  m_paperWidth = data.paperWidth;
  m_paperHeight = data.paperHeight;
  m_mediaName = data.mediaName;
  m_colorMode = data.colorMode;
  m_orientation = data.orientation;
  m_title = data.title;
  m_currentLine = 0;
  m_numLines = data.numLines;
  m_currentCell = 0;
  m_currentCol = 0;
  m_numCols = data.numCols;
  m_currentRow = 0;
  m_numRows = data.numRows;
  m_currentPage = 0;
  m_countStep = data.countStep;
  m_flags = data.flags;
  m_tabWidth = 8;

  // init
  float w, h;

  if (m_orientation == Portrait) {
    w = m_paperWidth;
    h = m_paperHeight;
  } else {
    w = m_paperHeight;
    h = m_paperWidth;
  }

  m_pageX = w*0.025;
  m_pageY = h*0.025;
  m_pageW = w - 2.0*m_pageX;
  m_pageH = h - 2.0*m_pageY;

  // calc number of pages
  int docLines = data.docLines;
  m_numPages = (docLines - 1)/(m_numLines*m_numCols*m_numRows) + 1;

  // calc number of digits for line counter depending on lines of document
  m_countDigits = 1;
  while (docLines > 10) {
    docLines /= 10;
    m_countDigits++;
  }

  // start writing output
  m_s << "%!PS-Adobe-3.0\n";

  // orientation
  m_s << "%%Orientation: ";
  m_s << ((m_orientation == Portrait) ? "Portrait" : "Landscape");
  m_s << "\n";

  // bounding box
  m_s << "%%BoundingBox: " << (int) m_pageX << " " << (int) m_pageY <<
    " " << (int) m_pageW << " " << (int) m_pageH << "\n";

  // pages
  m_s << "%%Pages: " << m_numPages << "\n";

  // media
  if (m_mediaName != 0L) {
    m_s << "%%DocumentMedia: " << m_mediaName << " " << m_paperWidth <<
      " " << m_paperHeight << " 0 () ()\n";
  }

  // end comments
  m_s << "%%EndComments\n\n";

  m_s << "%%BeginProlog\n\n";

  // rectangle
  m_s << "% set clipping and draw rectangle\n";
  m_s << "% width height, R, -\n";
  writeProc("R",
    "  newpath\n"
    "  0 0 moveto\n"
    "  1 index 0 lineto\n"
    "  exch 1 index lineto\n"
    "  0 exch lineto\n"
    "  closepath clip stroke");

  // define font
  m_s << "% define font\n";
  m_s << "% newFont size font, defFont, -\n";
  writeProc("defFont",
    "  findfont exch scalefont def");

  // new line
  m_s << "% new line\n";
  m_s << "% -, n, -\n";
  writeProc("n",
    "  /yPos yPos fontH sub store\n"
    "  xMargin2 yPos moveto");

  // upright font
  m_s << "% draw in upright font\n";
  m_s << "% text, u, -\n";
  writeProc("u",
    "  fCourier setfont\n"
    "  show");

  // bold
  m_s << "% draw in bold upright font\n";
  m_s << "% text, U, -\n";
  writeProc("U",
    "  fCourier-Bold setfont\n"
    "  show");

  // italics
  m_s << "% draw in italic font\n";
  m_s << "% text, i, -\n";
  writeProc("i",
    "  fCourier-Oblique setfont\n"
    "  show");

  // bold+italics
  m_s << "% draw in bold italic font\n";
  m_s << "% text, I, -\n";
  writeProc("I",
    "  fCourier-BoldOblique setfont\n"
    "  show");

  // line numbers
  if (m_countStep > 0) {
    m_s << "% right aligned line number\n";
    m_s << "% text, z, -\n";
    writeProc("z",
      "  fCourier 0.5 scalefont setfont\n"
      "  dup stringwidth pop xMargin1 exch sub yPos moveto\n"
      "  show\n"
      "  xMargin2 yPos moveto");
  }

  // title
  if (m_flags & Title) {
    m_s << "% centered title\n";
    m_s << "% text, t, -\n";
    writeProc("t",
      "  gsave\n"
      "  titleX titleY translate\n"
      "  titleW titleH R\n"
      "  fTitle setfont\n"
      "  dup stringwidth pop\n"
      "  neg titleW add 0.5 mul\n"
      "  titleH titleFontH 0.6 mul sub 0.5 mul\n"
      "  moveto show\n"
      "  grestore");
  }

  // page numbers
  if (m_flags & PageNumbers) {
    m_s << "% right aligned page number\n";
    m_s << "% text, Z, -\n";
    writeProc("Z",
      "  fPageNumber setfont\n"
      "  dup stringwidth pop pageNumX exch sub pageNumY moveto\n"
      "  show\n");
  }
}

void KTextPrint::defineColor(int colorNum, int r, int g, int b) {

  if (m_colorMode == BlackWhite) return;
  m_s << "/c" << colorNum << " {\n  ";
  if (m_colorMode == Color) {
    m_s << r/255.0 << " " << g/255.0 << " " << b/255.0 << " setrgbcolor";
  } else {
    m_s << (r*0.299 + g*0.587 + b*0.114)/255.0 << " setgray";
  }
  m_s << "\n} bind def\n\n";
}

void KTextPrint::begin() {
  float titleFontH, titleH, titleGapY;
  float pageFontH, pageNumX, pageNumY;
  float fontH, countFontW;

  // title
  titleFontH = (m_pageW + m_pageH)/100.0;
  titleH = titleFontH*1.15;
  titleGapY = m_pageH*0.01;
  if (m_flags & Title) {
    m_pageH -= titleGapY + titleH;
  }

  // page numbers
  pageFontH = (m_pageW + m_pageH)/120.0;
  pageNumX = m_pageX + m_pageW;
  pageNumY = m_pageY;
  if (m_flags & PageNumbers) {
    m_pageH -= pageFontH;
    m_pageY += pageFontH;
  }

  // text cells
  m_gapX = m_pageW/m_numCols*0.025;
  m_gapY = m_pageH/m_numRows*0.025;
  m_cellW = (m_pageW + m_gapX)/m_numCols;
  m_cellH = (m_pageH + m_gapY)/m_numRows;


  // calc font height that desired number of lines fit into a cell
  fontH = (m_cellH - m_gapY)/(m_numLines + 0.5);
  // fontWidth = fontHeight*0.6 for courier
  countFontW = 0.5*(fontH*0.6);


  m_s << "%%EndProlog\n\n";

  m_s << "%%BeginSetup\n\n";

  m_s << "/fontH " << fontH << " def\n";
  m_s << "/fCourier fontH /Courier defFont\n";
  m_s << "/fCourier-Bold fontH /Courier-Bold defFont\n";
  m_s << "/fCourier-Oblique fontH /Courier-Oblique defFont\n";
  m_s << "/fCourier-BoldOblique fontH /Courier-BoldOblique defFont\n";
  if (m_countStep > 0) {
    // with linecounter
    m_s << "/xMargin1 " << countFontW*(m_countDigits + 0.5) << " def\n";
    m_s << "/xMargin2 " << countFontW*(m_countDigits + 1.2) << " def\n";
  } else {
    // without linecounter
    m_s << "/xMargin2 " << countFontW*0.4 << " def\n";
  }

  // tilte
  if (m_flags & Title) {
    m_s << "/titleFontH " << titleFontH << " def\n";
    m_s << "/fTitle titleFontH /Helvetica-Bold defFont\n";
    m_s << "/titleX " << m_pageX << " def\n";
    m_s << "/titleY " << m_pageY + m_pageH + titleGapY << " def\n";
    m_s << "/titleW " << m_pageW << " def\n";
    m_s << "/titleH " << titleH << " def\n";
  }

  // page numbers
  if (m_flags & PageNumbers) {
    m_s << "/fPageNumber " << pageFontH << " /Helvetica defFont\n";
    m_s << "/pageNumX " << pageNumX << " def\n";
    m_s << "/pageNumY " << pageNumY << " def\n";
  }

  m_s << "%%EndSetup\n\n";

  m_colorNum = 0;
  m_lastCol = -1;
  newPage();
  newCell();
  writeCount();
  m_x = 0;
}



void KTextPrint::print(const QChar *s, int len) {

  if (len <= 0) return;
  writeColor(m_colorNum);
  printInternal(s, len);

  switch (m_fontStyle) {
    case Bold:
      m_s << "U";
      break;
    case Italics :
      m_s << "i";
      break;
    case Bold + Italics:
      m_s << "I";
      break;
    default:
      m_s << "u";
  }
  m_s << " ";
}

void KTextPrint::newLine() {
  m_s << "n\n";

  m_currentLine++;
  if (m_currentLine >= m_numLines) {
    m_currentLine = 0;
    m_s << "grestore\n";

    m_currentCol++;
    if (m_currentCol >= m_numCols) {
      m_currentCol = 0;

      m_currentRow++;
      if (m_currentRow >= m_numRows) {
        m_currentRow = 0;
        m_s << "showpage\n";
        m_currentPage++;
        newPage();
      }
    }
    m_currentCell++;
    newCell();
  }

  writeCount();
  m_x = 0;
}

void KTextPrint::end() {
  m_s << "\n"
    "grestore\n"
    "showpage\n\n"
    "%%Trailer\n"
    "end\n"
    "%%EOF\n";
}

void KTextPrint::writeProc(const char *pName, const char *commands) {
  m_s << "/" << pName << " {\n" <<
    commands << "\n" <<
    "} bind def\n\n";
}

void KTextPrint::writeCount() {
  if (m_countStep > 0) {
    int l = m_currentCell*m_numLines + m_currentLine + 1;

    if (l % m_countStep == 0) {
      writeColor(0);
      m_s << "(" << l << ") z ";
    }
  }
}

void KTextPrint::writeColor(int colorNum) {
  if (m_colorMode != BlackWhite && colorNum != m_lastCol) {
    m_s << "c" << colorNum << " ";
    m_lastCol = colorNum;
  }
}

void KTextPrint::printInternal(const QChar *s, int len) {
  int z, spaces;
  char ch;

  m_s << "(";
  for (z = 0; z < len; z++) {
    ch = *s;
    if (ch == '\t') {
      spaces = m_tabWidth - (m_x % m_tabWidth);
      m_x += spaces;
      while (spaces > 0) {
        m_s << " ";
        spaces--;
      }
    } else {
      if (ch == '\\') m_s << "\\\\";
      else if (ch == '(') m_s << "\\(";
      else if (ch == ')') m_s << "\\)";
      else m_s << ch;
      m_x++;
    }
    s++;
  }
  m_s << ") ";
}

void KTextPrint::newCell() {
  float x, y, w, h;

  m_s << "gsave\n";

  x = m_pageX + m_cellW*m_currentCol;
  y = m_pageY + m_cellH*(m_numRows - 1 - m_currentRow);
  w = m_cellW - m_gapX;
  h = m_cellH - m_gapY;


  m_s << x << " " << y << " translate\n";
  m_s << w << " " << h << " R\n";
  m_s << "/yPos " << h << " def n\n";
}

void KTextPrint::newPage() {
  int p = m_currentPage + 1;

  m_s << "%%Page: (" << p << ") " << p << "\n";
  m_s << "%%BeginPageSetup\n";
  if (m_orientation == Landscape)
    m_s << m_paperWidth << " 0 translate 90 rotate\n";
  m_s << "%%EndPageSetup\n\n";

  if (m_flags & Title) {
    writeColor(0);
    m_x = 0;
    printInternal(m_title.unicode(), m_title.length());
    m_s << " t\n";
  }

  if (m_flags & PageNumbers) {
    writeColor(0);
    m_s << "(" << p << " / " << m_numPages << ") Z\n";
  }
}


// string list helper functions
static void addToStrList(QStringList &list, const QString &str) {
  if (str.isEmpty()) return;
  if (list.count() > 0) {
    if (list.first() == str) return;
    QStringList::Iterator it = list.find(str);
    if (*it != 0L) list.remove(it);
    if (list.count() >= 16) list.remove(list.fromLast());
  }
  list.prepend(str);
}

static void appendToStrList(QStringList &list, const QString &str) {
  if (str.isEmpty()) return;
  QStringList::Iterator it = list.find(str);
  if (*it != 0L) return;
  list.append(str);
}



KTextPrintConfig::KTextPrintConfig() {
  // Defaults for KTextPrint Configuration
  m_destination = 0;
  m_command = "lpr -P%p -#%c";
  m_copies = 1;
  m_fileName = "out.ps";
  m_media = A4;
  m_orientation = KTextPrint::Portrait;
  m_colorMode = KTextPrint::BlackWhite;
  m_numCols = 1;
  m_numRows = 1;
  m_numLines = 70;
  m_countStep = 5;
  m_flags = KTextPrint::Title | KTextPrint::PageNumbers;
}

void KTextPrintConfig::print(QWidget *parent, KConfig *config, bool dialog,
  const QString &title, int docLines, QObject *receiver, const char *member) {

  KTextPrintConfig textPrintConfig;
  bool accepted;

  textPrintConfig.readConfig(config);
  if (dialog || (textPrintConfig.m_destination == 0
    && textPrintConfig.m_printers.count() == 0)) {

    KTextPrintDialog *dlg;

    dlg = new KTextPrintDialog(parent, textPrintConfig);
    accepted = (dlg->exec() == QDialog::Accepted);
    if (accepted) dlg->getData(textPrintConfig);
    delete dlg;
  } else accepted = true;

  if (accepted) {
    KTextPrintConfigData data;

    // connect callback
    connect(&textPrintConfig, SIGNAL(doPrint(KTextPrint &)), receiver, member);
    // get data for KTextPrint
    textPrintConfig.getData(data);
    data.title = title;
    data.docLines = docLines;

    if (textPrintConfig.m_destination == 0) {
      // to printer

      // substitute command
      QString program(textPrintConfig.m_command);
      program.replace(QRegExp("%p"), textPrintConfig.m_printers.first());
      program.replace(QRegExp("%c"), QString::number(textPrintConfig.m_copies));
      // create pipe
      KPipe p(program);
      p.open();
      // ceate text printer
      KTextPrint printer(data, &p);
      // activate callback
      emit textPrintConfig.doPrint(printer);
      // close pipe and check status
      p.close();
      if (p.status() != IO_Ok) {
        KMessageBox::sorry(parent,
          i18n("Error executing printer program!"),
          i18n("Print"));
      }
    } else {
      // to file

      // create file
      QFile f(textPrintConfig.m_fileName);
      if (f.open(IO_WriteOnly)) {
        // ceate text printer
        KTextPrint printer(data, &f);
        // activate callback
        emit textPrintConfig.doPrint(printer);
        // close file and check status
        f.close();
        if (f.status() != IO_Ok) {
          KMessageBox::sorry(parent,
            i18n("Error writing file!"),
            i18n("Print"));
        }
      } else {
        KMessageBox::sorry(parent,
          i18n("Error opening file!"),
          i18n("Print"));
      }
    }
    textPrintConfig.writeConfig(config);
  }
}

void KTextPrintConfig::readConfig(KConfig *config) {
  int z;

  // read from KConfig
  config->setGroup("KTextPrint Options");

  m_destination = config->readNumEntry("Destination", m_destination);

  for (z = 0; z < 6; z++) {
    appendToStrList(m_printers, config->readEntry(QString("Printer%1").arg(z)));
  }

  m_command = config->readEntry("Command", m_command);
  m_copies = config->readNumEntry("Copies", m_copies);
  m_fileName = config->readEntry("FileName", m_fileName);
  m_colorMode = (KTextPrint::ColorMode) config->readNumEntry("ColorMode", (int) m_colorMode);
  m_orientation = (KTextPrint::Orientation) config->readNumEntry("Orientation", (int) m_orientation);
  m_media = (Media) config->readNumEntry("Media", (int) m_media);
  m_numCols = config->readNumEntry("Cols", m_numCols);
  m_numRows = config->readNumEntry("Rows", m_numRows);
  m_numLines = config->readNumEntry("Lines", m_numLines);
  m_countStep = config->readNumEntry("CountStep", m_countStep);
  m_flags = config->readNumEntry("Flags", m_flags);
}


void KTextPrintConfig::writeConfig(KConfig *config) {
  int z;

  config->setGroup("KTextPrint Options");

  config->writeEntry("Destination", m_destination);

  QStringList::Iterator it = m_printers.begin();
  z = 0;
  while (it != m_printers.end() && z < 6) {
    config->writeEntry(QString("Printer%1").arg(z), *it);
    it++;
    z++;
  }

  config->writeEntry("Command", m_command);
  config->writeEntry("Copies", m_copies);
  config->writeEntry("FileName", m_fileName);
  config->writeEntry("ColorMode", (int) m_colorMode);
  config->writeEntry("Orientation", (int) m_orientation);
  config->writeEntry("Media", (int) m_media);
  config->writeEntry("Cols", m_numCols);
  config->writeEntry("Rows", m_numRows);
  config->writeEntry("Lines", m_numLines);
  config->writeEntry("CountStep", m_countStep);
  config->writeEntry("Flags", m_flags);
}

void KTextPrintConfig::getData(KTextPrintConfigData &data) {
  MediaDef *media = &mediaList[m_media];

  data.paperWidth = media->width;
  data.paperHeight = media->height;
  data.mediaName = media->name;
  data.orientation = m_orientation;
  data.colorMode = m_colorMode;
  data.numCols = m_numCols;
  data.numRows = m_numRows;
  data.numLines = m_numLines;
  data.countStep = m_countStep;
  data.flags = m_flags;
}


KTextPrintPreview::KTextPrintPreview(QWidget *parent) : QWidget(parent) {

  m_orient = 0;
  m_rows = 1;
  m_cols = 1;
  m_title = false;

  setFixedSize(90, 90);
}

void KTextPrintPreview::setOrient(int orient) {
  m_orient = orient;
  update();
}

void KTextPrintPreview::setCols(int cols) {
  m_cols = cols;
  update();
}

void KTextPrintPreview::setRows(int rows) {
  m_rows = rows;
  update();
}

void KTextPrintPreview::setTitle(bool title) {
  m_title = title;
  update();
}

void KTextPrintPreview::setPageNumbers(bool pageNumbers) {
  m_pageNumbers = pageNumbers;
  update();
}

void KTextPrintPreview::paintEvent(QPaintEvent *) {
  int x, y, w, h, i, j, x1, y1, x2, y2;

  if (m_orient == 0) {
    // portrait
    x = width()/6;
    y = 0;
  } else {
    // landscape
    x = 0;
    y = height()/6;
  }
  w = width() - 2*x;
  h = height() - 2*y;

  QPainter paint;
  paint.begin(this);

  paint.fillRect(x, y, w, h, white);

  if (m_title) {
//    paint.drawLine(x + 1, y + 1, x + w - 1, y + 1);
    paint.drawRect(x + 1, y + 1, w - 2, 3);
    y += 5;
    h -= 5;
  }

  if (m_pageNumbers) {
    paint.drawLine(x + w - 4, y + h - 1, x + w - 2, y + h - 1);
    h -= 1;
  }

  paint.setPen(black);
  for (j = 0; j < m_rows; j++) {
    y1 = y + j*h/m_rows +1;
    y2 = y + (j + 1)*h/m_rows -1;
    for (i = 0; i < m_cols; i++) {
      x1 = x + i*w/m_cols +1;
      x2 = x + (i + 1)*w/m_cols -1;
      paint.drawRect(x1, y1, x2 - x1, y2 - y1);
    }
  }
  paint.end();
}

KTextPrintDialog::KTextPrintDialog(QWidget *parent, KTextPrintConfig &textPrintConfig)
  : KDialogBase(parent, 0L, true, i18n("Printer Setup"), Ok | Cancel, Ok) {

  QWidget *page = new QWidget(this);
  setMainWidget(page);

  QVBoxLayout *vl = new QVBoxLayout(page, 0, spacingHint());

  QGroupBox *group;

  group = setupDestination(page);
  vl->addWidget(group);

  group = setupOptions(page);
  vl->addWidget(group);

  // defaults
  appendToStrList(textPrintConfig.m_printers, QString::fromLatin1(getenv("PRINTER")));
  setData(textPrintConfig);
}

KTextPrint::ColorMode KTextPrintDialog::colorMode() {
  if (m_color->isChecked()) {
    return KTextPrint::Color;
  } else if (m_grayScale->isChecked()) {
    return KTextPrint::GrayScal;
  } else {
    return KTextPrint::BlackWhite;
  }
}

KTextPrint::Orientation KTextPrintDialog::orientation() {
  return (m_landscape->isChecked()) ? KTextPrint::Landscape
    : KTextPrint::Portrait;
}

void KTextPrintDialog::setData(KTextPrintConfig &config) {
  int dest;

  dest = config.m_destination;
  // to printer
  m_toPrinter->setChecked(dest == 0);
  m_printerName->insertStringList(config.m_printers);
  m_command->setText(config.m_command);
  m_copies->setValue(config.m_copies);

  // to file
  m_toFile->setChecked(dest != 0);
  m_fileName->setText(config.m_fileName);

  // options
  KTextPrint::ColorMode colorMode = config.m_colorMode;
  m_color->setChecked(colorMode == KTextPrint::Color);
  m_grayScale->setChecked(colorMode == KTextPrint::GrayScal);
  m_blackWhite->setChecked(colorMode == KTextPrint::BlackWhite);

  KTextPrint::Orientation orient = config.m_orientation;
  m_portrait->setChecked(orient == KTextPrint::Portrait);
  m_landscape->setChecked(orient == KTextPrint::Landscape);

  m_media->setCurrentItem(config.m_media);
  m_cols->setValue(config.m_numCols);
  m_rows->setValue(config.m_numRows);
  m_lines->setValue(config.m_numLines);
  m_countStep->setValue(config.m_countStep);
  m_title->setChecked(config.m_flags & KTextPrint::Title);
  m_pageNumbers->setChecked(config.m_flags & KTextPrint::PageNumbers);

  // set
  setDest(dest);
  m_preview->setOrient(orientation());
}

void KTextPrintDialog::getData(KTextPrintConfig &config) {

  config.m_destination = (m_toFile->isChecked()) ? 1 : 0;
  addToStrList(config.m_printers, m_printerName->currentText());
  config.m_command = m_command->text();
  config.m_copies = m_copies->value();
  config.m_fileName = m_fileName->text();
  config.m_colorMode = colorMode();
  config.m_orientation = orientation();
  config.m_media = (KTextPrintConfig::Media) m_media->currentItem();
  config.m_numCols = m_cols->value();
  config.m_numRows = m_rows->value();
  config.m_numLines = m_lines->value();
  config.m_countStep = m_countStep->value();
  config.m_flags = 0;
  if (m_title->isChecked()) config.m_flags |= KTextPrint::Title;
  if (m_pageNumbers->isChecked()) config.m_flags |= KTextPrint::PageNumbers;
}
/*
void KTextPrintDialog::getData(KTextPrint &printer) {

  // color
  printer.setColorMode(colorMode());
  // orientation
  printer.setOrientation(orientation());
  // media
  printer.setMedia((KTextPrint::Media) m_media->currentItem());
  // layout
  printer.setLayout(m_cols->value(), m_rows->value(), m_lines->value());
  // line counter
  printer.setLineCounter(m_countStep->value());
  // flags
  int flags = 0;
  if (m_title->isChecked()) flags |= KTextPrint::Title;
  if (m_pageNumbers->isChecked()) flags |= KTextPrint::PageNumbers;
  printer.setFlags(flags);
}
*/
void KTextPrintDialog::setDest(int d) {
  bool e = (d == 0);

  m_printerName->setEnabled(e);
  m_choose->setEnabled(e);
  m_command->setEnabled(e);
  m_copies->setEnabled(e);
  e = !e;
  m_fileName->setEnabled(e);
  m_browse->setEnabled(e);
}

void KTextPrintDialog::choosePrinter() {

}

void KTextPrintDialog::browseFile() {
  QString name;

  name = KFileDialog::getSaveFileName(m_fileName->text(), "*.ps", this);
  if (!name.isEmpty()) {
    m_fileName->setText(name);
  }
}


QGroupBox *KTextPrintDialog::setupDestination(QWidget *parent) {
  QBoxLayout *vl, *hl;
  QGridLayout *gl;
  QLabel *label;

  QGroupBox *group = new QGroupBox(i18n("Print Destination"), parent);

  vl = new QVBoxLayout(group, 8, 0);
  vl->addSpacing(8);

  // destination button group
  QButtonGroup *dest = new QButtonGroup(parent);
  dest->hide();
  connect(dest, SIGNAL(clicked(int)), this, SLOT(setDest(int)));

  // to printer
  m_toPrinter = new QRadioButton(i18n("Print To &Printer (%p)"), group);
  dest->insert(m_toPrinter, 0); // to printer has id 0
  vl->addWidget(m_toPrinter);


  m_printerName = new QComboBox(true, group);
  m_choose = new QPushButton(i18n("choose"), group);
  connect(m_choose, SIGNAL(clicked()), this, SLOT(choosePrinter()));


  m_command = new QLineEdit(group);
  m_copies = new QSpinBox(1, 0xffffff, 1, group);

  gl = new QGridLayout(vl, 3, 2, 5);
  gl->setColStretch(0, 3);
  gl->addWidget(m_printerName, 0, 0);
  gl->addWidget(m_choose, 0, 1);
  label = new QLabel(m_command, i18n("Printer Command"), group);
  gl->addWidget(label, 1, 0);
  label = new QLabel(m_command, i18n("Copies (%c)"), group);
  gl->addWidget(label, 1, 1);
  gl->addWidget(m_command, 2, 0);
  gl->addWidget(m_copies, 2, 1);
/*

  hl = new QHBoxLayout(vl, 5);
//  hl->addSpacing(19);
  hl->addWidget(m_printerName, 3);
  hl->addWidget(m_choose);

  m_command = new QLineEdit(group);

//  hl = new QHBoxLayout(vl, 5);
//  hl->addSpacing(19);
  QLabel *label = new QLabel(m_command, i18n("Printer Command"), group);
  vl->addWidget(label);
  vl->addWidget(m_command);

*/

  // to file
  m_toFile = new QRadioButton(i18n("Print To &File"), group);
  dest->insert(m_toFile, 1); // to file has id 1
  m_fileName = new QLineEdit(group);
  m_browse = new QPushButton(i18n("browse"), group);
  connect(m_browse, SIGNAL(clicked()), this, SLOT(browseFile()));

  vl->addWidget(m_toFile);
  hl = new QHBoxLayout(vl, 5);
//  hl->addSpacing(19);
  hl->addWidget(m_fileName, 3);
  hl->addWidget(m_browse);

//  vl->addStretch(10);

  return group;
}

QGroupBox *KTextPrintDialog::setupOptions(QWidget *parent) {
  QBoxLayout *topLayout, *vl, *hl;
  QButtonGroup *bg;
  QLabel *label;

  QGroupBox *group = new QGroupBox(i18n("Options"), parent);

  // preview object
  m_preview = new KTextPrintPreview(group);

  topLayout = new QVBoxLayout(group, 8, 0);
  topLayout->addSpacing(8);

  // --- first row ---
  hl = new QHBoxLayout(topLayout, 5);

  // color
  m_color = new QRadioButton(i18n("Color"), group);
  m_grayScale = new QRadioButton(i18n("GrayScale"), group);
  m_blackWhite = new QRadioButton(i18n("Black/White"), group);

  vl = new QVBoxLayout(hl);
//  hl->addLayout(vl);
//  vl->addSpacing(8);
  vl->addWidget(m_color);
  vl->addWidget(m_grayScale);
  vl->addWidget(m_blackWhite);

  // color button group
  bg = new QButtonGroup(parent);
  bg->hide();
  bg->insert(m_color, 0);
  bg->insert(m_grayScale, 1);
  bg->insert(m_blackWhite, 2);

  // orientation
  m_portrait = new QRadioButton(i18n("Portrait"), group);
  m_landscape = new QRadioButton(i18n("Landscape"), group);

  vl = new QVBoxLayout(hl);
//  hl->addLayout(vl);
//  vl->addSpacing(8);
  vl->addWidget(m_portrait);
  vl->addWidget(m_landscape);
  vl->addStretch(10);

  // orientation button group
  bg = new QButtonGroup(parent);
  bg->hide();
  bg->insert(m_portrait, 0);
  bg->insert(m_landscape, 1);
  connect(bg, SIGNAL(clicked(int)), m_preview, SLOT(setOrient(int)));

  //media
  m_media = new QComboBox(false, group);
  for (int z = 0; mediaList[z].name != 0L; z++) {
    m_media->insertItem(QString::fromLatin1(mediaList[z].name));
  }
  label = new QLabel(m_media, i18n("Media:"), group);

  vl = new QVBoxLayout(hl);
//  hl->addLayout(vl);
//  vl->addSpacing(8);
  vl->addWidget(label);
  vl->addWidget(m_media);
  vl->addStretch(10);

  topLayout->addSpacing(10);

  // --- second row ---
  hl = new QHBoxLayout(topLayout, 5);

  // preview
  hl->addWidget(m_preview);

  // cols/rows selectors
  m_cols = new QSpinBox(1, 6, 1, group);
  connect(m_cols, SIGNAL(valueChanged(int)), m_preview, SLOT(setCols(int)));
  m_rows = new QSpinBox(1, 6, 1, group);
  connect(m_rows, SIGNAL(valueChanged(int)), m_preview, SLOT(setRows(int)));
  m_lines = new QSpinBox(10, 1000, 5, group);

  vl = new QVBoxLayout(hl);
  label = new QLabel(m_cols, i18n("Colums:"), group);
  vl->addWidget(label);
  vl->addWidget(m_cols);
  label = new QLabel(m_rows, i18n("Rows:"), group);
  vl->addWidget(label);
  vl->addWidget(m_rows);
  label = new QLabel(m_lines, i18n("Text Lines:"), group);
  vl->addWidget(label);
  vl->addWidget(m_lines);
  vl->addStretch(10);

  // other options
  m_countStep = new QSpinBox(0, 1000, 1, group);
  m_title = new QCheckBox(i18n("Title"), group);
  connect(m_title, SIGNAL(toggled(bool)), m_preview, SLOT(setTitle(bool)));
  m_pageNumbers = new QCheckBox(i18n("Page Numbers"), group);
  connect(m_pageNumbers, SIGNAL(toggled(bool)), m_preview, SLOT(setPageNumbers(bool)));

  vl = new QVBoxLayout(hl);
  label = new QLabel(m_countStep, i18n("Line Counter:"), group);
  vl->addWidget(label);
  vl->addWidget(m_countStep);
  vl->addWidget(m_title);
  vl->addWidget(m_pageNumbers);
  vl->addStretch(10);

  topLayout->addStretch(10);

  return group;
}

/*
void KTextPrintDialog::accept() {
  hide();
  if (m_toPrinter->isChecked()) {
    // to printer
    QString program(m_command->text());
    program.replace(QRegExp("%p"), m_printerName->currentText());
    program.replace(QRegExp("%c"), m_copies->text());
    KPipe p(program);
    p.open();
    KTextPrint printer(&p);
    getData(printer);
    emit doPrint(printer);
    p.close();
    if (p.status() != IO_Ok) {
      KMessageBox::sorry(this,
        i18n("Error executing printer program!"),
        i18n("Print"));
    }
  } else {
    // to file
    QFile f(m_fileName->text());
    if (f.open(IO_WriteOnly)) {
      KTextPrint printer(&f);
      getData(printer);
      emit doPrint(printer);
      f.close();
      if (f.status() != IO_Ok) {
        KMessageBox::sorry(this,
          i18n("Error writing file!"),
          i18n("Print"));
      }
    } else {
      KMessageBox::sorry(this,
        i18n("Error opening file!"),
        i18n("Print"));
    }
  }
  writeConfig(kapp->config());
  KDialogBase::accept();
}

void KTextPrintDialog::readConfig(KConfig *config) {
  int dest, z;
  KTextPrint::ColorMode colMode;
  KTextPrint::Orientation orient;

  // read from KConfig
  config->setGroup("KTextPrint Options");

  dest = config->readNumEntry("Destination", m_toFile->isChecked());

  for (z = 0; z < 6; z++) {
    appendToStrList(m_printers, config->readEntry(QString("Printer%1").arg(z)));
  }

  m_command->setText(config->readEntry("Command", m_command->text()));
  m_copies->setValue(config->readNumEntry("Copies", m_copies->value()));
  m_fileName->setText(config->readEntry("FileName", m_fileName->text()));
  colMode = (KTextPrint::ColorMode) config->readNumEntry("ColorMode", (int) colorMode());
  orient = (KTextPrint::Orientation) config->readNumEntry("Orientation", (int) orientation());
  m_media->setCurrentItem(config->readNumEntry("Media", m_media->currentItem()));
  m_cols->setValue(config->readNumEntry("Cols", m_cols->value()));
  m_rows->setValue(config->readNumEntry("Rows", m_rows->value()));
  m_lines->setValue(config->readNumEntry("Lines", m_lines->value()));
  m_countStep->setValue(config->readNumEntry("CountStep", m_countStep->value()));
  m_title->setChecked(config->readNumEntry("Title", m_title->isChecked()));
  m_pageNumbers->setChecked(config->readNumEntry("PageNumbers", m_pageNumbers->isChecked()));

  // set defaults
  m_toPrinter->setChecked(dest == 0);
  m_toFile->setChecked(dest != 0);
  setDest(dest);

  m_printerName->insertStringList(m_printers);

  m_color->setChecked(colMode == KTextPrint::Color);
  m_grayScale->setChecked(colMode == KTextPrint::GrayScal);
  m_blackWhite->setChecked(colMode == KTextPrint::BlackWhite);

  m_portrait->setChecked(orient == KTextPrint::Portrait);
  m_landscape->setChecked(orient == KTextPrint::Landscape);

  m_preview->setOrient(orientation());
}

void KTextPrintDialog::writeConfig(KConfig *config) {
  int z;

  config->setGroup("KTextPrint Options");

  config->writeEntry("Destination", (m_toFile->isChecked()) ? 1 : 0);

  addToStrList(m_printers, m_printerName->currentText());
  QStringList::Iterator it = m_printers.begin();
  z = 0;
  while (it != m_printers.end() && z < 6) {
    config->writeEntry(QString("Printer%1").arg(z), *it);
    it++;
    z++;
  }

  config->writeEntry("Command", m_command->text());
  config->writeEntry("Copies", m_copies->value());
  config->writeEntry("FileName", m_fileName->text());
  config->writeEntry("ColorMode", (int) colorMode());
  config->writeEntry("Orientation", (int) orientation());
  config->writeEntry("Media", m_media->currentItem());
  config->writeEntry("Cols", m_cols->value());
  config->writeEntry("Rows", m_rows->value());
  config->writeEntry("Lines", m_lines->value());
  config->writeEntry("CountStep", m_countStep->value());
  config->writeEntry("Title", m_title->isChecked());
  config->writeEntry("PageNumbers", m_pageNumbers->isChecked());
}
*/



KPipe::KPipe(QString program) {
  int argn;
  const char *ascii;
  char **argp;
  char *arg;

  setType(IO_Sequential);

  program.simplifyWhiteSpace();
  argn = program.contains(' ') + 1;
  m_argv = new char *[argn + 1];
  m_argv[argn] = 0L;
  ascii = program.ascii();
  m_args = new char[strlen(ascii) +1];
  strcpy(m_args, ascii);

  argp = m_argv;
  arg = m_args;
  while (true) {
    *argp = arg;
    argp++;
    arg = strchr(arg, ' ');
    if (arg != 0L) {
      *arg = '\0';
      arg++;
    } else break;
  }
}

KPipe::~KPipe() {
  close();

  delete [] m_argv;
  delete [] m_args;
}

bool KPipe::open(int mode) {

  if (isOpen()) return false;

  setMode(mode);
  resetStatus();

  if ((mode & (IO_ReadOnly | IO_Append | IO_Truncate)) != 0) {
    setStatus(IO_OpenError);
    return false;
  }
  if (pipe(m_pipe) == -1) {
    setStatus(IO_OpenError);
    return false;
  }
  m_pid = fork();
  if (m_pid == -1) {
    setStatus(IO_OpenError);
    return false;
  }
  if (m_pid == 0) {
    // child
    ::close(m_pipe[1]);

    dup2(m_pipe[0], STDIN_FILENO);
    ::close(m_pipe[0]);
    execvp(m_args, m_argv);
    _exit(-1);
  }

  // parent
  ::close(m_pipe[0]);

//    sleep(2);

  // ignore broken pipe
  struct sigaction act;
  act.sa_handler = SIG_IGN;
  sigemptyset(&(act.sa_mask));
  sigaddset(&(act.sa_mask), SIGPIPE);
  act.sa_flags = 0;
  sigaction(SIGPIPE, &act, &m_oldAct);

  setState(IO_Open);
  return true;
}

void KPipe::close() {
  if (!isOpen()) return;

  ::close(m_pipe[1]);
  waitpid(m_pid, 0L, 0);

  // restore broken pipe handler
  sigaction(SIGPIPE, &m_oldAct, 0L);

  setState(0);
}

int KPipe::writeBlock(const char *data, uint len) {
  int n;

  if (status() != IO_Ok) return 0;

  n = write(m_pipe[1], data, len);
  if (n == -1) {
    setStatus(IO_WriteError);
    return 0;
  }
  return n;
}

