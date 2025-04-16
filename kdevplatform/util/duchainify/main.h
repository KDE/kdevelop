/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_MAIN_H
#define KDEVPLATFORM_MAIN_H

#include <QObject>
#include <QAtomicInt>
#include <QUrl>

#include <language/duchain/topducontext.h>
#include <serialization/indexedstring.h>

class QCommandLineParser;

class Manager : public QObject
{
    Q_OBJECT

public:
    explicit Manager(QCommandLineParser* args);
    void addToBackgroundParser(const QString& path, KDevelop::TopDUContext::Features features);
    QSet<QUrl> waiting();

private:
    QSet<QUrl> m_waiting;
    uint m_total;
    bool m_exited = false;
    QCommandLineParser* m_args;
    QAtomicInt m_allFilesAdded;

public Q_SLOTS:
    // delay init into event loop so the DUChain can always shutdown gracefully
    void init();
    void updateReady(const KDevelop::IndexedString& url, const KDevelop::ReferencedTopDUContext& topContext);
    void finishIfDone();
    void dump(const KDevelop::ReferencedTopDUContext& topContext);
};

#endif
