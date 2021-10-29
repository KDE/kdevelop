/*
    SPDX-FileCopyrightText: 2016 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BENCHDUCHAIN_H
#define BENCHDUCHAIN_H

#include <QObject>

class BenchDUChain : public QObject
{
    Q_OBJECT

public:
    BenchDUChain();
    ~BenchDUChain() override;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void benchDUChainBuilder();

private:
};

#endif // BENCHDUCHAIN_H
