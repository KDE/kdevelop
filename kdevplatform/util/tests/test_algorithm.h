/*
 *  SPDX-FileCopyrightText: 2024 Igor Kushnir <igorkuo@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KDEVPLATFORM_TEST_ALGORITHM_H
#define KDEVPLATFORM_TEST_ALGORITHM_H

#include <QObject>

/**
 * Tests namespace Algorithm
 */
class TestAlgorithm : public QObject
{
    Q_OBJECT
public:
    explicit TestAlgorithm(QObject* parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testUnite2String();
    void testUnite5Int();

    void testInsert();
};

#endif // KDEVPLATFORM_TEST_ALGORITHM_H
