/*
  Copyright 2005 Harald Fernengel <harry@kdevelop.org>
  Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include "preprocessor.h"

#include <QFile>
#include <QTextStream>

#include <kurl.h>
#include <kdebug.h>

#include "pp-stream.h"
#include "pp-engine.h"
#include "pp-environment.h"

using namespace rpp;

Preprocessor::~Preprocessor()
{
}

QString Preprocessor::processString(const QString& string)
{
    pp proc(this);

    QString ret = proc.processFile(string, pp::Data);

    proc.environment()->cleanup();

    return ret;
}

Stream * Preprocessor::sourceNeeded( QString & fileName, IncludeType type, int sourceLine )
{
  Q_UNUSED(fileName)
  Q_UNUSED(type)
  Q_UNUSED(sourceLine)

  return 0;
}
