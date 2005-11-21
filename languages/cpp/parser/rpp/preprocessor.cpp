/*
  Copyright 2005 Harald Fernengel <harry@kdevelop.org>

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
#include "pp.h"

#include <QtCore/QtCore>
#include <string>

class PreprocessorPrivate
{
public:
    QByteArray result;
    pp_environment env;
    QStringList includePaths;

    void initPP(pp<> &proc)
    {
        foreach(QString path, includePaths)
            proc.push_include_path(path.toStdString());
    }
};

Preprocessor::Preprocessor()
{
    d = new PreprocessorPrivate;
}

Preprocessor::~Preprocessor()
{
    delete d;
}

void Preprocessor::processFile(const QString &fileName)
{
    pp<> proc(d->env);
    d->initPP(proc);

    d->result.reserve(d->result.size() + 20 * 1024);

    proc.file(fileName.toLocal8Bit().constData(), std::back_inserter(d->result));
}

void Preprocessor::processString(const QByteArray &str)
{
    pp<> proc(d->env);
    d->initPP(proc);

    proc(str.begin(), str.end(), std::back_inserter(d->result));
}

QByteArray Preprocessor::result() const
{
    return d->result;
}

void Preprocessor::addIncludePaths(const QStringList &includePaths)
{
    d->includePaths += includePaths;
}

/*
int main()
{
    Preprocessor pp;

    QStringList paths;
    paths << "/usr/include";
    pp.addIncludePaths(paths);

    pp.processFile("pp-configuration");
    pp.processFile("/usr/include/stdio.h");

    qDebug() << pp.result();

    return 0;
}
*/

