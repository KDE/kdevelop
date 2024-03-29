/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROBLEM_H
#define KDEVPLATFORM_PROBLEM_H

#include <QExplicitlySharedDataPointer>

#include <language/languageexport.h>

#include "duchainbase.h"
#include <serialization/indexedstring.h>
#include "indexedtopducontext.h"
#include <interfaces/iproblem.h>
#include <interfaces/iassistant.h>

namespace KDevelop {
class IAssistant;
class Problem;

using ProblemPointer = QExplicitlySharedDataPointer<Problem>;

/**
 * Represents a problem only by its index within the top-context
 *
 * Fixme: share code with the other LocalIndexed* classes
 */
class KDEVPLATFORMLANGUAGE_EXPORT LocalIndexedProblem
{
public:
    LocalIndexedProblem(const ProblemPointer& problem, const TopDUContext* top);
    explicit LocalIndexedProblem(uint index = 0)
        : m_index(index)
    {}

    /**
     * \note Duchain must be read locked
     */
    ProblemPointer data(const TopDUContext* top) const;

    bool operator==(const LocalIndexedProblem& rhs) const
    {
        return m_index == rhs.m_index;
    }

    bool isValid() const
    {
        return m_index;
    }

    /**
     * Index of the Declaration within the top context
     */
    uint localIndex() const
    {
        return m_index;
    }

private:
    uint m_index;
};

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(ProblemData, diagnostics, LocalIndexedProblem)

class KDEVPLATFORMLANGUAGE_EXPORT ProblemData
    : public DUChainBaseData
{
public:
    ProblemData()
    {
        initializeAppendedLists();
    }

    ProblemData(const ProblemData& rhs)
        : DUChainBaseData(rhs)
        , source(rhs.source)
        , severity(rhs.severity)
        , url(rhs.url)
        , description(rhs.description)
        , explanation(rhs.explanation)
    {
        initializeAppendedLists();
        copyListsFrom(rhs);
    }

    ~ProblemData()
    {
        freeAppendedLists();
    }

    ProblemData& operator=(const ProblemData& rhs) = delete;

    IProblem::Source source = IProblem::Unknown;
    IProblem::Severity severity = IProblem::Error;
    IndexedString url;
    IndexedString description;
    IndexedString explanation;
    IProblem::FinalLocationMode finalLocationMode = IProblem::Range;

    START_APPENDED_LISTS_BASE(ProblemData, DUChainBaseData);
    APPENDED_LIST_FIRST(ProblemData, LocalIndexedProblem, diagnostics);
    END_APPENDED_LISTS(ProblemData, diagnostics);
};

/**
 * An object representing a problem in preprocessing, parsing, definition-use chain compilation, etc.
 *
 * You should always use ProblemPointer, because Problem may be subclassed.
 * The subclass would be lost while copying.
 *
 * @warning Access to problems must be serialized through DUChainLock.
 */
class KDEVPLATFORMLANGUAGE_EXPORT Problem
    : public DUChainBase
    , public IProblem
{
public:
    using Ptr = QExplicitlySharedDataPointer<Problem>;

    Problem();
    explicit Problem(ProblemData& data);
    ~Problem() override;

    Source source() const override;
    void setSource(IProblem::Source source) override;

    /**
     * Returns a string version of the problem source
     */
    QString sourceString() const override;

    TopDUContext* topContext() const override;
    KDevelop::IndexedString url() const override;

    /**
     * Location where this problem occurred
     * @warning Must only be called from the foreground
     * */
    DocumentRange finalLocation() const override;
    void setFinalLocation(const DocumentRange& location) override;

    FinalLocationMode finalLocationMode() const override;
    void setFinalLocationMode(FinalLocationMode mode) override;

    /**
     * Returns child diagnostics of this particular problem
     *
     * Example:
     * @code
     * void foo(unsigned int);
     * void foo(const char*);
     * int main() { foo(0); }
     * @endcode
     *
     * => foo(0) is ambiguous. This will give us a ProblemPointer pointing to 'foo(0)'.
     *
     * Additionally, @p diagnostics may return the two locations to the ambiguous overloads,
     * with descriptions such as 'test.cpp:1: candidate : ...'
     */
    void clearDiagnostics() override;

    QVector<IProblem::Ptr> diagnostics() const override;
    void setDiagnostics(const QVector<IProblem::Ptr>& diagnostics) override;
    void addDiagnostic(const IProblem::Ptr& diagnostic) override;

    /**
     * A brief description of the problem.
     */
    QString description() const override;
    void setDescription(const QString& description) override;

    /**
     * A (detailed) explanation of why the problem occurred.
     */
    QString explanation() const override;
    void setExplanation(const QString& explanation) override;

    /**
     * Get the severity of this problem.
     * This is used for example to decide for a highlighting color.
     *
     * @see setSeverity()
     */
    Severity severity() const override;

    /**
     * Set the severity of this problem.
     */
    void setSeverity(Severity severity) override;

    /**
     * Returns a string representation of the severity.
     */
    QString severityString() const override;

    /**
     * If this problem can be solved, this may return an assistant for the solution.
     */
    QExplicitlySharedDataPointer<IAssistant> solutionAssistant() const override;

    enum {
        Identity = 15
    };

    /**
     * Returns a string representation of this problem, useful for debugging.
     */
    virtual QString toString() const;

private:
    void rebuildDynamicData(DUContext* parent, uint ownIndex) override;

    Q_DISABLE_COPY(Problem)

    DUCHAIN_DECLARE_DATA(Problem)
    friend class TopDUContext;
    friend class TopDUContextDynamicData;
    friend class LocalIndexedProblem;

    //BEGIN dynamic data
    TopDUContextPointer m_topContext;
    mutable QList<Ptr> m_diagnostics;
    uint m_indexInTopContext = 0;
    //END dynamic data
};

class KDEVPLATFORMLANGUAGE_EXPORT StaticAssistantProblem
    : public KDevelop::Problem
{
public:
    KDevelop::IAssistant::Ptr solutionAssistant() const override
    {
        return m_solution;
    }
    void setSolutionAssistant(const KDevelop::IAssistant::Ptr& p)
    {
        m_solution = p;
    }

private:
    KDevelop::IAssistant::Ptr m_solution;
};
}

Q_DECLARE_TYPEINFO(KDevelop::LocalIndexedProblem, Q_MOVABLE_TYPE);

KDEVPLATFORMLANGUAGE_EXPORT QDebug operator<<(QDebug s, const KDevelop::Problem& problem);
KDEVPLATFORMLANGUAGE_EXPORT QDebug operator<<(QDebug s, const KDevelop::ProblemPointer& problem);

#endif // KDEVPLATFORM_PROBLEM_H
