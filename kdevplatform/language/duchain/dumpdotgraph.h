/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_DUMPDOTGRAPH_H
#define KDEVPLATFORM_DUMPDOTGRAPH_H

#include <QtGlobal>
#include <QScopedPointer>
#include <language/languageexport.h>

class QString;

namespace KDevelop {
class TopDUContext;
class DUContext;
class DumpDotGraphPrivate;


/**
 * A helper-class for debugging, that nicely visualizes the whole structure of a du-context.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT DumpDotGraph
{
    Q_DISABLE_COPY(DumpDotGraph)

public:
    DumpDotGraph();
    ~DumpDotGraph();
    /**
     * The context, it's, and if it is not a top-context also all contexts importing it we be drawn.
     * Parent-contexts will not be respected, so if you want the whole structure, you will need to pass the top-context.
     * @param shortened if this is given sub-items like declarations, definitions, child-contexts, etc. will not be shown as separate nodes
     * */
    QString dotGraph(KDevelop::DUContext* context, bool shortened = false);

private:
    const QScopedPointer<class DumpDotGraphPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DumpDotGraph)
};
}

#endif // KDEVPLATFORM_DUMPDOTGRAPH_H
