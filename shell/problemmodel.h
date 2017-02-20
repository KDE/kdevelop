/*
 * KDevelop Problem Reporter
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
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

/**
 * @brief Wraps a ProblemStore and adds the QAbstractItemModel interface, so the it can be used in a model/view architecture.
 *
 * By default ProblemModel instantiates a FilteredProblemStore, with the following features on:
 * \li ScopeFilter
 * \li SeverityFilter
 * \li Grouping
 * \li CanByPassScopeFilter
 *
 * Has to following columns:
 * \li Error
 * \li Source
 * \li File
 * \li Line
 * \li Column
 * \li LastColumn
 *
 * Possible ProblemModel features
 * \li NoFeatures
 * \li CanDoFullUpdate
 * \li CanShowImports
 * \li ScopeFilter
 * \li SeverityFilter
 * \li Grouping
 * \li CanByPassScopeFilter
 *
 * Scope, severity, grouping, imports can be set using the slots named after these features.
 *
 * Usage example:
 * @code
 * IProblem::Ptr problem(new DetectedProblem);
 * problem->setDescription(QStringLiteral("Problem"));
 * ProblemModel *model = new ProblemModel(nullptr);
 * model->addProblem(problem);
 * model->rowCount(); // returns 1
 * QModelIndex idx = model->index(0, 0);
 * model->data(index); // "Problem"
 * @endcode
 *
 */
class KDEVPLATFORMSHELL_EXPORT ProblemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /// List of supportable features
    enum FeatureCode
    {
        NoFeatures                 = 0,  /// No features :(
        CanDoFullUpdate            = 1,  /// Reload/Reparse problems
        CanShowImports             = 2,  /// Show problems from imported files. E.g.: Header files in C/C++
        ScopeFilter                = 4,  /// Filter problems by scope. E.g.: current document, open documents, etc
        SeverityFilter             = 8,  /// Filter problems by severity. E.g.: hint, warning, error, etc
        Grouping                   = 16, /// Can group problems
        CanByPassScopeFilter       = 32  /// Can bypass scope filter
    };

    Q_DECLARE_FLAGS(Features, FeatureCode)

    explicit ProblemModel(QObject *parent, ProblemStore *store = nullptr);
    ~ProblemModel() override;

    enum Columns {
        Error,
        Source,
        File,
        Line,
        Column,
        LastColumn
    };

    enum Roles {
        ProblemRole = Qt::UserRole + 1,
        SeverityRole
    };

    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    IProblem::Ptr problemForIndex(const QModelIndex& index) const;

    /// Adds a new problem to the model
    void addProblem(const IProblem::Ptr &problem);

    /// Clears the problems, then adds a new set of them
    void setProblems(const QVector<IProblem::Ptr> &problems);

    /// Clears the problems
    void clearProblems();

    /// Retrieve problems for selected document
    QVector<IProblem::Ptr> problems(const KDevelop::IndexedString& document);

    /// Retrieve the supported features
    Features features() const;

    /// Retrieve 'show imports' filter setting
    bool showImports();

    /// Set the supported features
    void setFeatures(Features features);

    /// Tooltip for "Force Full Update" action in the Problems View when the model
    /// is active (correspondent tab is selected)
    QString fullUpdateTooltip() const;

    /// Set the "Force Full Update" action tooltip
    void setFullUpdateTooltip(const QString& tooltip);

signals:
    /// Emitted when the stored problems are changed with addProblem(), setProblems() and
    /// clearProblems() methods. This signal emitted only when internal problems storage is
    /// really changed: for example, it is not emitted when we call clearProblems() method
    /// for empty model.
    void problemsChanged();

    /// Emitted when the "Force Full Update" action tooltip is changed with setFullUpdateTooltip().
    /// This signal emitted only when tooltip is really changed.
    void fullUpdateTooltipChanged();

public slots:
    /// Show imports
    void setShowImports(bool showImports);

    /// Sets the scope filter. Uses int to be able to use QSignalMapper
    void setScope(int scope);

    /// Sets the severity filter. Uses int to be able to use QSignalMapper
    void setSeverity(int severity);///old-style severity filtering

    void setSeverities(KDevelop::IProblem::Severities severities);///new-style severity filtering

    void setGrouping(int grouping);

    /**
     * Force a full problem update.
     * E.g.: Reparse the source code.
     * Obviously it doesn't make sense for run-time problem checkers.
     */
    virtual void forceFullUpdate(){}

protected slots:
    /// Triggered when problems change
    virtual void onProblemsChanged(){}

private slots:
    /// Triggered when the current document changes
    virtual void setCurrentDocument(IDocument* doc);

    virtual void closedDocument(IDocument* doc);

    /// Triggered before the problems are rebuilt
    void onBeginRebuild();

    /// Triggered once the problems have been rebuilt
    void onEndRebuild();

protected:
    ProblemStore *store() const;

private:
    QScopedPointer<ProblemModelPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ProblemModel::Features)

}

#endif // PROBLEMMODEL_H
