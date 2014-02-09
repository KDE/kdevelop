/* This file is part of KDevelop
Copyright 2007 Hamish Rodda <rodda@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_I_PROBLEM_H
#define KDEVPLATFORM_I_PROBLEM_H


#include <QtCore/QStack>

#include <ksharedptr.h>

#include "../editor/documentrange.h"
#include "../languageexport.h"

#include "duchainbase.h"
#include "indexedstring.h"
#include "indexedtopducontext.h"

namespace KDevelop
{
class IAssistant;
class Problem;

/**
 * Represents a problem only by its index within the top-context
 *
 * Fixme: share code with the other LocalIndexed* classes
 */
class KDEVPLATFORMLANGUAGE_EXPORT LocalIndexedProblem
{
  public:
    LocalIndexedProblem(const Problem* problem);
    LocalIndexedProblem(uint index = 0)
      : m_index(index)
    {}

    /**
     * \note Duchain must be read locked
     */
    Problem* data(const TopDUContext* top) const;

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

class KDEVPLATFORMLANGUAGE_EXPORT ProblemData : public DUChainBaseData
{
public:
    enum Source {
        Unknown        /**< Unknown problem */,
        Disk           /**< problem reading from disk */,
        Preprocessor   /**< problem during pre-processing */,
        Lexer          /**< problem while lexing the file */,
        Parser         /**< problem while parsing the file */,
        DUChainBuilder /**< problem while building the duchain */,
        SemanticAnalysis /**< problem during semantic analysis */,
        ToDo           /**< TODO item in a comment */
    };

    enum Severity {
        Error,
        Warning,
        Hint        //For implementation-helpers and such stuff. Is not highlighted if the user disables the "highlight semantic problems" option
    };

    ProblemData()
        : source(Unknown)
        , severity(Error)
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

    Source source;
    Severity severity;
    IndexedString url;
    IndexedString description;
    IndexedString explanation;

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
class KDEVPLATFORMLANGUAGE_EXPORT Problem : public DUChainBase, public KShared
{
public:
    using Ptr = KSharedPtr<Problem>;

    Problem();
    Problem(ProblemData& data);
    ~Problem();

    ProblemData::Source source() const;
    void setSource(ProblemData::Source source);

    /**
     * Returns a string version of the problem source
     */
    QString sourceString() const;

    void setContext(TopDUContext* context);
    TopDUContext* topContext() const override;
    KDevelop::IndexedString url() const override;

    /**
     * Location where this problem occurred
     * @warning Must only be called from the foreground
     * */
    DocumentRange finalLocation() const;
    void setFinalLocation(const DocumentRange& location);

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
     * => foo(0) is ambigous. This will give us a ProblemPointer pointing to 'foo(0)'.
     *
     * Additionally, @p diagnostics may return the two locations to the ambiguous overloads,
     * with descriptions such as 'test.cpp:1: candidate : ...'
     */
    QList<Ptr> diagnostics() const;
    void setDiagnostics(const QList<Ptr>& diagnostics);
    void addDiagnostic(const Ptr& diagnostic);
    void clearDiagnostics();

    /**
     * A brief description of the problem.
     */
    QString description() const;
    void setDescription(const QString& description);

    /**
     * A (detailed) explanation of why the problem occurred.
     */
    QString explanation() const;
    void setExplanation(const QString& explanation);

    /**
     * Get the severity of this problem.
     * This is used for example to decide for a highlighting color.
     *
     * @see setSeverity()
     */
    ProblemData::Severity severity() const;

    /**
     * Set the severity of this problem.
     */
    void setSeverity(ProblemData::Severity severity);

    /**
     * Returns a string representation of the severity.
     */
    QString severityString() const;

     /**
     * If this problem can be solved, this may return an assistant for the solution.
     */
    virtual KSharedPtr<IAssistant> solutionAssistant() const;

    enum {
        Identity = 15
    };

    /**
     * Returns a string representation of this problem, useful for debugging.
     */
    virtual QString toString() const;

private:
    void rebuildDynamicData(DUContext* parent, uint ownIndex) override;
    /**
     * Return a problem equivalent to this one for serialization in @p context.
     *
     * If this problem is not yet associated with a context, we set the context
     * of it to @p context. If this problem is associated with a different context,
     * the problem is cloned and a copy is returned which can be serialized.
     */
    Ptr prepareStorage(TopDUContext* context);

    Q_DISABLE_COPY(Problem);

    DUCHAIN_DECLARE_DATA(Problem)
    friend class TopDUContext;
    friend class TopDUContextDynamicData;
    friend class LocalIndexedProblem;

    //BEGIN dynamic data
    TopDUContext* m_topContext;
    mutable QList<Ptr> m_diagnostics;
    uint m_indexInTopContext;
    //END dynamic data
};

using ProblemPointer = KSharedPtr<Problem>;

}

Q_DECLARE_TYPEINFO(KDevelop::LocalIndexedProblem, Q_MOVABLE_TYPE);

KDEVPLATFORMLANGUAGE_EXPORT QDebug operator<<(QDebug s, const KDevelop::Problem& problem);
KDEVPLATFORMLANGUAGE_EXPORT QDebug operator<<(QDebug s, const KDevelop::ProblemPointer& problem);

#endif // KDEVPLATFORM_I_PROBLEM_H
