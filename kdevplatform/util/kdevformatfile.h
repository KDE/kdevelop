/*
    Copyright 2016 Anton Anikin <anton.anikin@htower.ru>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#pragma once

#include <QString>
#include <QTextStream>
#include <QVector>

namespace KDevelop {

struct KDevFormatLine
{
    QStringList wildcards;
    QString command;
};

class KDevFormatFile
{
public:
    KDevFormatFile(const QString& origFilePath, const QString& tempFilePath);

    bool find();
    bool read();
    bool apply();

private:
    bool executeCommand(QString command);

    /// This is logically a static constant, but since this class is instantiated
    /// once in non-test code, making it a non-static member is safe and efficient.
    const QString formatFileName;

    const QString m_origFilePath;
    const QString m_tempFilePath;

    QVector<KDevFormatLine> m_formatLines;
};

class AutoFlushingQTextStream : public QTextStream
{
public:
    AutoFlushingQTextStream(FILE* f, QIODevice::OpenMode o)
        : QTextStream(f, o)
    {
    }

    template<typename T>
    AutoFlushingQTextStream& operator <<(T&& s)
    {
        *(( QTextStream* ) this) << std::forward<T>(s);
        flush();
        return *this;
    }
};

inline AutoFlushingQTextStream& qStdOut()
{
    static AutoFlushingQTextStream s{stdout, QIODevice::WriteOnly | QIODevice::Text};
    return s;
}

}

Q_DECLARE_TYPEINFO(KDevelop::KDevFormatLine, Q_MOVABLE_TYPE);
