/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "pp-engine.h"

#include <QCoreApplication>
#include <QDebug>
#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include "preprocessor.h"
#include "pp-environment.h"
#include "pp-location.h"
#include "chartools.h"

using namespace rpp;

int main (int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  if (app.arguments().count() < 2)
    {
      qDebug() << "usage: pp file.cpp";
      return EXIT_FAILURE;
    }
  const QString filename = app.arguments().at(1);

  KDevelop::AutoTestShell::init();
  KDevelop::TestCore::initialize(KDevelop::Core::NoUi, "kdev-pp");

  Preprocessor p;

  QStringList ip;

  pp preprocess(&p);

  preprocess.processFile(QString("pp-configuration")); // ### put your macros here!

  QString result = QString::fromUtf8(stringFromContents(preprocess.processFile(filename)));

  QStringList resultLines = result.split('\n');
  for (int i = 0; i < resultLines.count(); ++i)
    qDebug() << i << ":" << resultLines[i];

  preprocess.environment()->locationTable()->dump();

  /*foreach (const Preprocessor::MacroItem& macro, p.macros())
    qDebug() << "Macro '" << macro.name << "' param (" << macro.parameters << ") def (" << macro.definition << ") isFnLike (" << macro.isFunctionLike << ") filename (" << macro.fileName << ")";*/

  return EXIT_SUCCESS;
}

