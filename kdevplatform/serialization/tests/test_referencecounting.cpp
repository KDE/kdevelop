/*
    SPDX-FileCopyrightText: 2026 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <serialization/referencecounting.h>

#include <QTest>

#include <thread>

using namespace KDevelop;

class TestReferenceCounting : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testEnabledRangeAcrossLibraryBoundary()
    {
        char storage[4] = {};
        QVERIFY(!shouldDoDUChainReferenceCounting(storage + 1));
        {
            const DUChainReferenceCountingEnabler enabler(storage + 1, 2);
            QVERIFY(!shouldDoDUChainReferenceCounting(storage));
            QVERIFY(shouldDoDUChainReferenceCounting(storage + 1));
            QVERIFY(shouldDoDUChainReferenceCounting(storage + 2));
            QVERIFY(!shouldDoDUChainReferenceCounting(storage + 3));
            {
                const DUChainReferenceCountingEnabler nestedEnabler(storage + 1, 2);
                QVERIFY(shouldDoDUChainReferenceCounting(storage + 1));
            }
            QVERIFY(shouldDoDUChainReferenceCounting(storage + 1));
        }
        QVERIFY(!shouldDoDUChainReferenceCounting(storage + 1));
    }

    void testThreadIsolation()
    {
        char storage = {};
        const DUChainReferenceCountingEnabler enabler(&storage, sizeof(storage));
        QVERIFY(shouldDoDUChainReferenceCounting(&storage));

        bool initiallyEnabled = true;
        bool enabledInThread = false;
        bool enabledAfterScope = true;
        std::thread worker([&] {
            initiallyEnabled = shouldDoDUChainReferenceCounting(&storage);
            {
                const DUChainReferenceCountingEnabler threadEnabler(&storage, sizeof(storage));
                enabledInThread = shouldDoDUChainReferenceCounting(&storage);
            }
            enabledAfterScope = shouldDoDUChainReferenceCounting(&storage);
        });
        worker.join();

        QVERIFY(!initiallyEnabled);
        QVERIFY(enabledInThread);
        QVERIFY(!enabledAfterScope);
        QVERIFY(shouldDoDUChainReferenceCounting(&storage));
    }
};

QTEST_GUILESS_MAIN(TestReferenceCounting)

#include "test_referencecounting.moc"
