/*
 * Copyright 2015 Laszlo Kis-Adam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef PROBLEMSTORE_H
#define PROBLEMSTORE_H

#include <QObject>
#include <QList>
#include <shell/shellexport.h>
#include <serialization/indexedstring.h>
#include <interfaces/iproblem.h>

struct ProblemStorePrivate;

namespace KDevelop
{

class WatchedDocumentSet;
class ProblemStoreNode;

// Stores and handles problems
// Does no ordering, filtering, or grouping, those should be implemented in derived classes!
class KDEVPLATFORMSHELL_EXPORT ProblemStore : public QObject
{
    Q_OBJECT
public:
    ProblemStore(QObject *parent = nullptr);
    virtual ~ProblemStore();

    // Adds a problem
    virtual void addProblem(const IProblem::Ptr &problem);

    // Clears the current problems, and adds new ones from a list
    virtual void setProblems(const QVector<IProblem::Ptr> &problems);

    // Finds the specified node
    virtual const ProblemStoreNode* findNode(int row, ProblemStoreNode *parent = nullptr) const;

    // Returns the number of problems
    virtual int count(ProblemStoreNode *parent = nullptr) const;

    // Clears the problems
    virtual void clear();

    // Rebuild the problems list, if applicable. It does nothing in the base class.
    virtual void rebuild();

    // Specifies the severity filter
    virtual void setSeverity(int severity);

    // Retrives the severity filter settings
    int severity() const;

    // Retrieves the currently watched document set
    WatchedDocumentSet* documents() const;

    // Sets the scope filter
    void setScope(int scope);

    // Sets the grouping method
    virtual void setGrouping(int grouping);

    // Sets the currently shown document (in the editor, it's triggered by the IDE)
    void setCurrentDocument(const IndexedString &doc);

    // Retrives the path of the current document
    const KDevelop::IndexedString& currentDocument() const;

signals:
    // Emitted when the problems change
    void changed();

    // Emitted before the problemlist is rebuilt
    void beginRebuild();

    // Emitted once the problemlist has been rebuilt
    void endRebuild();

private slots:
    // Triggered when the watched document set changes. E.g.:document closed, new one added, etc
    virtual void onDocumentSetChanged();

protected:
    ProblemStoreNode* rootNode();

private:
    QScopedPointer<ProblemStorePrivate> d;
};

}

#endif

