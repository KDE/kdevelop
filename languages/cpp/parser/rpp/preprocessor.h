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

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <QtCore/qglobal.h>

class QByteArray;
class QString;
class QStringList;
class PreprocessorPrivate;

class Preprocessor
{
public:
    Preprocessor();
    ~Preprocessor();

    void processFile(const QString &fileName);
    void processString(const QByteArray &str);

    void addIncludePaths(const QStringList &includePaths);

    QByteArray result() const;

private:
    Q_DISABLE_COPY(Preprocessor)
    PreprocessorPrivate *d;
};

#endif

