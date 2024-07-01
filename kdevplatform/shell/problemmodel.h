/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PROBLEMMODEL_H
#define PROBLEMMODEL_H

#include "problemconstants.h"

#include <language/editor/documentrange.h>
#include <shell/problem.h>
#include <shell/shellexport.h>
#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>

#include <QAbstractItemModel>

class QUrl;

namespace KDevelop {
class IDocument;
class IndexedString;
class ProblemStore;
class ProblemModelPrivate;

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
        CanByPassScopeFilter       = 32, /// Can bypass scope filter
        ShowSource                 = 64  /// Show problem's source. Set if problems can have different sources.
    };

    Q_DECLARE_FLAGS(Features, FeatureCode)

    explicit ProblemModel(QObject *parent, ProblemStore *store = nullptr);
    ~ProblemModel() override;

    // NOTE: keep the enumerators of Columns in sync with the sizePolicy array in ProblemTreeView::resizeColumns().
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
    QVector<IProblem::Ptr> problems(const KDevelop::IndexedString& document) const;

    /**
     * Add new "placeholder" item (problem). The item will be displayed whenever the model is empty.
     *
     * The method should be used to notify user about some events. For example, analyzer plugin
     * can set placeholders at analysis state changes - started/finished without errors/etc.
     *
     * \param[in] text Sets problem description.
     * \param[in] location Sets problem final location.
     * \param[in] source Sets problem source string.
    */
    void setPlaceholderText(const QString& text,
                            const KDevelop::DocumentRange& location = KDevelop::DocumentRange::invalid(),
                            const QString& source = QString());

    /// Retrieve the supported features
    Features features() const;

    /// Retrieve 'show imports' filter setting
    bool showImports() const;

    /// Set the supported features
    void setFeatures(Features features);

    /// Tooltip for "Force Full Update" action in the Problems View when the model
    /// is active (correspondent tab is selected)
    QString fullUpdateTooltip() const;

    /// Set the "Force Full Update" action tooltip
    void setFullUpdateTooltip(const QString& tooltip);

Q_SIGNALS:
    /// Emitted when the stored problems are changed with addProblem(), setProblems() and
    /// clearProblems() methods. This signal emitted only when internal problems storage is
    /// really changed: for example, it is not emitted when we call clearProblems() method
    /// for empty model.
    void problemsChanged();

    /// Emitted when the "Force Full Update" action tooltip is changed with setFullUpdateTooltip().
    /// This signal emitted only when tooltip is really changed.
    void fullUpdateTooltipChanged();

public Q_SLOTS:
    /// Show imports
    void setShowImports(bool showImports);

    /// Sets the scope filter
    void setScope(ProblemScope scope);

    /// Sets the path filter for the DocumentsInPath scope
    void setPathForDocumentsInPathScope(const QString& path);

    /// Sets the severity filter
    void setSeverity(int severity);///old-style severity filtering

    void setSeverities(KDevelop::IProblem::Severities severities);///new-style severity filtering

    void setGrouping(int grouping);

    /**
     * Force a full problem update.
     * E.g.: Reparse the source code.
     * Obviously it doesn't make sense for run-time problem checkers.
     */
    virtual void forceFullUpdate(){}

protected Q_SLOTS:
    /// Triggered when problems change
    virtual void onProblemsChanged(){}

private Q_SLOTS:
    /// Triggered when the current document changes
    virtual void setCurrentDocument(IDocument* doc);

    virtual void closedDocument(IDocument* doc);

    void documentUrlChanged(IDocument* document, const QUrl& previousUrl);

    /// Triggered before the problems are rebuilt
    void onBeginRebuild();

    /// Triggered once the problems have been rebuilt
    void onEndRebuild();

protected:
    ProblemStore *store() const;

private:
    const QScopedPointer<class ProblemModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProblemModel)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ProblemModel::Features)

}

#endif // PROBLEMMODEL_H
