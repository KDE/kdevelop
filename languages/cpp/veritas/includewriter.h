/*
* KDevelop xUnit integration
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#ifndef VERITAS_CPP_INCLUDEWRITER_H
#define VERITAS_CPP_INCLUDEWRITER_H

#include <KUrl>
#include <QTextStream>
#include <QDir>

namespace Veritas
{

/*! @unittest Veritas::Test::IncludeWriterTest */
class IncludeSerializer
{
public:
  void write(const QString& included, const QString& includer, QIODevice* target) {
      KUrl includedUrl(included);
      KUrl includerUrl(includer);
      target->open(QIODevice::Append);
      QTextStream str(target);
      if (includedUrl.upUrl() == includerUrl.upUrl()) {
          str << "#include \"" << includedUrl.fileName() << "\"\n";
      } else if (includedUrl.upUrl() == includerUrl.upUrl().upUrl()) {
          str << "#include \"../" << includedUrl.fileName() << "\"\n";
      } else if (includedUrl.upUrl().upUrl() == includerUrl.upUrl()) {
          str << "#include \"" << includedUrl.upUrl().fileName() << QDir::separator() << includedUrl.fileName() << "\"\n";
      } else {
          int common = 0;
          while ( common < included.count() && common < includer.count() ) {
              if (included[common] != includer[common]) {
                  break;
              }
              common++;
          }
          str << "#include \"" << included.mid(common) << "\"\n";
      }
      target->close();
  }
};

/*! @unittest Veritas::Test::IncludeWriterTest */
class IncludeGuardSerializer
{
public:
    void writeOpen(const QString& headerFile, QIODevice* target) {
        target->open(QIODevice::Append);
        QTextStream str(target);
        str << "\n#ifndef " << guard(headerFile)
            << "\n#define " << guard(headerFile) << "\n\n";
        target->close();
    }

    void writeClose(const QString& headerFile, QIODevice* target) {
        target->open(QIODevice::Append);
        QTextStream str(target);
        str << "\n#endif // " << guard(headerFile) << "\n";
        target->close();
    }

private:
    QString guard(const QString& headerFile) {
        KUrl url(headerFile);
        QString upUp = url.upUrl().upUrl().fileName().toUpper();
        QString up = url.upUrl().fileName().toUpper();
        QString fn = url.fileName().toUpper();
        fn = fn.replace(".", "_");
        return upUp + "_" + up + "_" + fn + "_INCLUDED";
    }
};

}

#endif VERITAS_CPP_INCLUDEWRITER_H
