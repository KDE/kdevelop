/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASTYLESTRINGITERATOR_H
#define ASTYLESTRINGITERATOR_H

#include <QString>
#include <QTextStream>

#include "astyle.h"

#include <string>

class AStyleStringIterator : public astyle::ASSourceIterator
{
public:
    explicit AStyleStringIterator(const QString &string);
    ~AStyleStringIterator() override;

    std::streamoff tellg() override;
    int getStreamLength() const override;
    bool hasMoreLines() const override;
    std::string nextLine(bool emptyLineWasDeleted = false) override;
    std::string peekNextLine() override;
    void peekReset() override;
    std::streamoff getPeekStart() const override;

private:
    QString m_content;
    QTextStream m_is;
    qint64 m_peekStart;
};

#endif // ASTYLESTRINGITERATOR_H
