/*****************************************************************
 
Copyright (c) 2000, Matthias Hoelzer-Kluepfel
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
******************************************************************/


#include <qfile.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qpushbutton.h>


#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <kaboutdata.h>
#include <ktextbrowser.h>
#include <kiconloader.h>


#include "ktip.h"


KTipDatabase::KTipDatabase(QString tipFile)
{
  if (tipFile.isEmpty())
    tipFile = QString(KGlobal::instance()->aboutData()->appName()) + "/tips";

  loadTips(tipFile);

  if (tips.count())
    current = kapp->random() % tips.count();
}


// if you change something here, please update the script
// preparetips, which depends on extracting exactly the same
// text as done here.
void KTipDatabase::loadTips(QString tipFile)
{
  QString fname;

  fname = locate("data", tipFile);

  if (fname.isEmpty())
    return ;

  tips.clear();

  QFile f(fname);
  if (f.open(IO_ReadOnly))
    {
      QTextStream ts(&f);

      QString line, tag, tip;
      bool inTip = false;
      while (!ts.eof())
        {
          line = ts.readLine();
          tag = line.stripWhiteSpace().lower();

          if (tag == "<html>")
            {
              inTip = true;
              tip = QString::null;
              continue;
            }

          if (inTip)
            {
              if (tag == "</html>")
                {
                  tips.append(tip);
                  inTip = false;
                }
              else
                tip.append(line).append("\n");
            }

        }

      f.close();
    }
}


void KTipDatabase::nextTip()
{
  if (tips.count() == 0)
    return ;
  current += 1;
  if (current >= (int) tips.count())
    current = 0;
}


void KTipDatabase::prevTip()
{
  if (tips.count() == 0)
    return ;
  current -= 1;
  if (current < 0)
    current = tips.count() - 1;
}


QString KTipDatabase::tip()
{
  return QString("<html>%1</html>").arg(i18n(tips[current]));
}



KTipDialog *KTipDialog::_instance = 0;


KTipDialog::KTipDialog(KTipDatabase *db, QWidget *parent, const char *name)
  : KDialog(parent, name)
{
  _database = db;

  setCaption(i18n("Tip of the day"));
  setIcon(KGlobal::iconLoader()->loadIcon("ktip", KIcon::Small));

  QVBoxLayout *vbox = new QVBoxLayout(this, marginHint(), spacingHint());

  QHBoxLayout *hbox = new QHBoxLayout(vbox, 0, 0);

  QLabel *bulb = new QLabel(this);
  bulb->setPixmap(locate("data", "kdeui/ktip-bulb.png"));
  hbox->addWidget(bulb);
 
  QLabel *titlePane = new QLabel(this);
  titlePane->setBackgroundPixmap(locate("data", "kdeui/ktip-background.png"));
  titlePane->setText(i18n("Did you know...?\n"));
  titlePane->setFont(QFont("helvetica", 20, QFont::Bold));
  titlePane->setAlignment(QLabel::AlignCenter);
  hbox->addWidget(titlePane,100);

  _tipText = new KTextBrowser(this);
  vbox->addWidget(_tipText);

  QFrame *f = new QFrame(this);
  f->setFrameStyle(QFrame::Sunken|QFrame::HLine);
  vbox->addWidget(f);  

  hbox = new QHBoxLayout(vbox);

  _tipOnStart = new QCheckBox(i18n("Show on start"), this);
  hbox->addWidget(_tipOnStart);
  hbox->addStretch();

  QPushButton *next = new QPushButton(i18n("&Next"), this);
  hbox->addWidget(next);

  QPushButton *ok = new QPushButton(i18n("&OK"), this);
  hbox->addWidget(ok);
  ok->setDefault(true);

  connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
  connect(next, SIGNAL(clicked()), this, SLOT(nextTip()));
  connect(_tipOnStart, SIGNAL(toggled(bool)), this, SLOT(showOnStart(bool)));
  
  KConfig *config = kapp->config();
  config->setGroup("TipOfDay");
  _tipOnStart->setChecked(config->readBoolEntry("RunOnStart", true));
}


void KTipDialog::showTip(QString tipFile, bool force)
{
  if (!force)
  {
    KConfig *config = kapp->config();
    config->setGroup("TipOfDay");
    if (!config->readBoolEntry("RunOnStart", true))
      return;
  }

  if (!_instance)
    _instance = new KTipDialog(new KTipDatabase(tipFile), kapp->mainWidget());

  _instance->nextTip();
  _instance->show();
  _instance->raise();
}


void KTipDialog::nextTip()
{
  _database->nextTip();
  _tipText->setText(_database->tip());
}


void KTipDialog::showOnStart(bool on)
{
  setShowOnStart(on);
}


void KTipDialog::setShowOnStart(bool on)
{
  KConfig *config = kapp->config();
  config->setGroup("TipOfDay");
  config->writeEntry("RunOnStart", on);
  config->sync();
}
