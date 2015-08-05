/*
 * KDevelop Problem Reporter
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef PROBLEMMODEL_H
#define PROBLEMMODEL_H

#include <shell/shellexport.h>
#include <QtCore/QAbstractItemModel>
#include <shell/problem.h>
#include <QUrl>

struct ProblemModelPrivate;

namespace KDevelop {
    class IDocument;

class ProblemStore;

// Provides a model interface for the problems so they can be shown
// Can support various features, see the FeatureCode enum!
class KDEVPLATFORMSHELL_EXPORT ProblemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    // List of supportable features
    enum FeatureCode
    {
        NoFeatures                 = 0, // No features :(
        CanDoFullUpdate            = 1, // Reload/Reparse problems
        CanShowImports             = 2, // Show problems from imported files. E.g.: Header files in C/C++
        ScopeFilter                = 4, // Filter problems by scope. E.g.: current document, open documents, etc
        SeverityFilter             = 8, // Filter problem by severity. E.g.: hint, warning, error, etc
        Grouping                   = 16,
        CanByPassScopeFilter       = 32 // Can bypass scope filter
    };

    Q_DECLARE_FLAGS(Features, FeatureCode)

    explicit ProblemModel(QObject *parent, ProblemStore *store = NULL);
    virtual ~ProblemModel();

    enum Columns {
        Error,
        Source,
        File,
        Line,
        Column,
        LastColumn
    };

    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex & index) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    IProblem::Ptr problemForIndex(const QModelIndex& index) const;

    // Adds a new problem to the model
    void addProblem(const IProblem::Ptr &problem);

    // Clears the problems, then adds a new set of them
    void setProblems(const QVector<IProblem::Ptr> &problems);

    // Clears the problems
    void clearProblems();

    // Retrieve the supported features
    Features features() const;

    // Set the supported features
    void setFeatures(Features features);

public slots:
    // Show imports
    virtual void setShowImports(bool){}

    // Sets the scope filter
    // Use int to be able to use QSignalMapper
    virtual void setScope(int scope);

    // Sets the severity filter
    // Use int to be able to use QSignalMapper
    virtual void setSeverity(int severity);

    void setGrouping(int grouping);

    // Force a full problem update.
    // E.g.: Reparse the source code
    // Obviously it doesn't make sense for run-time problem checkers.
    virtual void forceFullUpdate(){}

protected slots:
    // Triggered when problems change
    virtual void onProblemsChanged(){}

private slots:
    // Triggered when the current document changes
    virtual void setCurrentDocument(IDocument* doc);

    // Triggered before the problems are rebuilt
    void onBeginRebuild();

    // Triggered once the problems have been rebuilt
    void onEndRebuild();

protected:
    ProblemStore *store() const;

private:
    QScopedPointer<ProblemModelPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ProblemModel::Features)

}

#endif // PROBLEMMODEL_H
