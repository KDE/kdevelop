/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLANGTIDY_CHECKSETSELECTION_H
#define CLANGTIDY_CHECKSETSELECTION_H

// Qt
#include <QSharedDataPointer>

class QString;

namespace ClangTidy
{

class CheckSetSelectionPrivate;

class CheckSetSelection
{
public:
    CheckSetSelection();
    CheckSetSelection(const CheckSetSelection& other);
    ~CheckSetSelection();

public:
    CheckSetSelection& operator=(const CheckSetSelection& other);

public:
    void setId(const QString& id);
    QString id() const;

public:
    void setName(const QString& name);
    QString name() const;

public:
    QString selectionAsString() const;

    void setSelection(const QString& selection);

private:
    QSharedDataPointer<CheckSetSelectionPrivate> d;
};

}

#endif
