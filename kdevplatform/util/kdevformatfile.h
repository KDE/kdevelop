/*
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QString>
#include <QTextStream>
#include <QVector>
#include <QIODevice>

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
