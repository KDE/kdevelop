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

#include <string>

// register callback for include hooks
static void includeFileHook(const std::string &, const std::string &, FILE *);
#define PP_HOOK_ON_FILE_INCLUDED(A, B, C) includeFileHook(A, B, C)
#include "pp.h"

#include <QtCore/QtCore>

class PreprocessorPrivate
{
public:
    QByteArray result;
    pp_environment env;
    QStringList includePaths;

    void initPP(pp &proc)
    {
        foreach(QString path, includePaths) {
            const QByteArray asc = path.toAscii();
            proc.push_include_path(std::string(asc.constData(), asc.length()));
        }
    }
};

QHash<QString, QStringList> includedFiles;

void includeFileHook(const std::string &fileName, const std::string &filePath, FILE *)
{
    includedFiles[QString::fromAscii(fileName.data(), int(fileName.size()))].append(QString::fromAscii(filePath.data(), int(filePath.size())));
}

Preprocessor::Preprocessor()
{
    d = new PreprocessorPrivate;
    includedFiles.clear();
}

Preprocessor::~Preprocessor()
{
    delete d;
}

void Preprocessor::processFile(const QString &fileName)
{
    pp proc(d->env);
    d->initPP(proc);

    d->result.reserve(d->result.size() + 20 * 1024);

    d->result += "# 1 \"" + fileName.toLatin1() + "\"\n"; // ### REMOVE ME
    proc.file(fileName.toLocal8Bit().constData(), std::back_inserter(d->result));
}

void Preprocessor::processString(const QByteArray &str)
{
    pp proc(d->env);
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

QStringList Preprocessor::macroNames() const
{
    QStringList macros;

    pp_environment::base_type::const_iterator it = d->env.begin();
    while (it != d->env.end()) {
        macros += QString::fromLatin1((*it).first->begin(), (*it).first->size());
        ++it;
    }

    return macros;
}

QList<Preprocessor::MacroItem> Preprocessor::macros() const
{
    QList<MacroItem> items;

    pp_environment::base_type::const_iterator it = d->env.begin();
    while (it != d->env.end()) {
        MacroItem item;
        item.name = QString::fromLatin1((*it).first->begin(), (*it).first->size());
        item.definition = QString::fromLatin1((*it).second.definition->begin(),
                                              (*it).second.definition->size());
        for (size_t i = 0; i < (*it).second.formals.size(); ++i) {
            item.parameters += QString::fromLatin1((*it).second.formals[i]->begin(),
                    (*it).second.formals[i]->size());
        }
        item.isFunctionLike = (*it).second.function_like;

#ifdef PP_WITH_MACRO_POSITION
        item.fileName = QString::fromLatin1((*it).second.file->begin(), (*it).second.file->size());
#endif
        items += item;

        ++it;
    }

    return items;
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

