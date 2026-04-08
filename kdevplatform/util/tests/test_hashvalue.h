/*
 *  SPDX-FileCopyrightText: 2026 Jarmo Tiitto <jarmo.tiitto@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KDEVPLATFORM_TEST_HASHVALUE_H
#define KDEVPLATFORM_TEST_HASHVALUE_H

#include <QObject>

/**
 * Tests KDevelop::HashValue
 */
class TestHashValue : public QObject
{
    Q_OBJECT
public:
    explicit TestHashValue(QObject* parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testHashValues();
};

#endif // KDEVPLATFORM_TEST_ALGORITHM_H
