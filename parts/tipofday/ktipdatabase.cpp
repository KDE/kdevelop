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


#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstddirs.h>


#include "ktipdatabase.h"


KTipDatabase::KTipDatabase(QString tipFile)
{
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

