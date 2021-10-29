/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DUCHAINDUMPER_H
#define KDEVPLATFORM_DUCHAINDUMPER_H

#include <language/languageexport.h>

#include <QFlags>
#include <QScopedPointer>

class QTextStream;

namespace KDevelop {
class DUContext;
class DUChainDumperPrivate;

/**
 * @brief Debugging utility function to dump a DUContext including contained declarations.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainDumper
{
public:
    enum Feature {
        NoFeatures =        0,
        DumpContext =       1 << 0,
        DumpProblems =      1 << 1
    };
    Q_DECLARE_FLAGS(Features, Feature)

    explicit DUChainDumper(Features features = DumpContext);
    ~DUChainDumper();

    /**
     * Dump DUChain context to stdout
     *
     * NOTE: The DUChain must be readlocked when this is called.
     *
     * @param context The context to dump
     * @param allowedDepth How deep the dump will go into imported contexts, printing all the contents.
     */
    void dump(DUContext* context, int allowedDepth = 0);

    void dump(DUContext* context, int allowedDepth, QTextStream& out);

private:
    const QScopedPointer<class DUChainDumperPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DUChainDumper)
};
}
#endif // KDEVPLATFORM_DUCHAINDUMPER_H
