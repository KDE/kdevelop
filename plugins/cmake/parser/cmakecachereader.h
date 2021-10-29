/*
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKECACHEREADER_H
#define CMAKECACHEREADER_H

#include <QString>

#include "cmakecommonexport.h"

class KDEVCMAKECOMMON_EXPORT CacheLine
{
public:
    CacheLine() {}

    void readLine(const QString& line);
    bool isCorrect() const { return endName>=0 && equal>=0; }

    QString name() const;
    QString flag() const;
    QString type() const;
    QString value() const;

private:
    QString m_line;

    int endName = -1;
    int dash = -1;
    int colon = -1;
    int equal = -1;
};

#endif
