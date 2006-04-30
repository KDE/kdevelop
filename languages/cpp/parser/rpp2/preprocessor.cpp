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

#include <kurl.h>
#include <kdebug.h>

#include "pp-stream.h"
#include "pp-engine.h"

class PreprocessorPrivate
{
public:
    QString result;
    QHash<QString, pp_macro*> env;
    QStringList includePaths;
};

QHash<QString, QStringList> includedFiles;

Preprocessor::Preprocessor()
{
    d = new PreprocessorPrivate;
    includedFiles.clear();
}

Preprocessor::~Preprocessor()
{
    qDeleteAll(d->env);
    delete d;
}

QString Preprocessor::processFile(const QString& fileName)
{
    pp proc(this, d->env);

    return proc.processFile(fileName);
}

QString Preprocessor::processString(const QByteArray &bytearray)
{
    pp proc(this, d->env);

    return proc.processFile(bytearray);
}

void Preprocessor::addIncludePaths(const QStringList &includePaths)
{
    d->includePaths += includePaths;
}

QStringList Preprocessor::macroNames() const
{
    return d->env.keys();
}

QList<Preprocessor::MacroItem> Preprocessor::macros() const
{
    QList<MacroItem> items;

    QHashIterator<QString, pp_macro*> it = d->env;
    while (it.hasNext()) {
        it.next();

        MacroItem item;
        item.name = it.key();
        item.definition = it.value()->definition;
        item.parameters = it.value()->formals;
        item.isFunctionLike = it.value()->function_like;

#ifdef PP_WITH_MACRO_POSITION
        item.fileName = it.value()->file;
#endif
        items << item;
    }

    return items;
}

Stream * Preprocessor::sourceNeeded( QString & fileName, IncludeType type )
{
  Q_UNUSED(type)
  if (!QFile::exists(fileName)) {
    foreach (const QString& includePath, d->includePaths) {
      fileName = KUrl(KUrl::fromPath(includePath), fileName).path();
      if (QFile::exists(fileName))
        goto found;
    }

    return 0L;
  }

  found:
  QFile* f = new QFile(fileName);
  if (!f->open(QIODevice::ReadOnly)) {
    kWarning() << k_funcinfo << "Could not open successfully stat-ed file " << fileName << endl;
    delete f;
    return 0L;
  }

  // Hrm, hazardous?
  f->deleteLater();

  return new Stream(f);
}
