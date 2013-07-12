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

#include "../editor/documentcursor.h"
#include "../editor/documentrange.h"
#include "../languageexport.h"
#include "../duchain/duchainbase.h"
#include <language/duchain/indexedstring.h>

namespace KDevelop
{
class IAssistant;

class ProblemData : public DUChainBaseData
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
    }

    Source source;
    KDevelop::IndexedString url;
//     QStack<DocumentCursor> locationStack;
    IndexedString description;
    IndexedString explanation;
    Severity severity;
};

/**
 * An object representing a problem in preprocessing, parsing, definition-use chain compilation, etc.
 * 
 * You should always use ProblemPointer, because Problem may be subclassed.
 * The subclass would be lost while copying.
 * 
 * Warning: Access to problems must be serialized through DUChainLock.
 */
class KDEVPLATFORMLANGUAGE_EXPORT Problem : public DUChainBase, public KShared
{
public:
    Problem();
    Problem(ProblemData& data);
    ~Problem();

    ProblemData::Source source() const;
    void setSource(ProblemData::Source source);

    /**
     * Returns a string version of the problem source
     */
    QString sourceString() const;

    virtual KDevelop::IndexedString url () const;

    /**
     * Returns the stack of locations via which the error occurred.
     * @todo Make the location-stack functional again. Currently these functions don't do anything.
     */
    QStack<DocumentCursor> locationStack() const;
    void addLocation(const DocumentCursor& cursor);
    void setLocationStack(const QStack<DocumentCursor>& locationStack);
    void clearLocationStack();

    /**
     * Location where this problem occurred
     * @warning Must only be called from the foreground
     * */
    DocumentRange finalLocation() const;
    void setFinalLocation(const DocumentRange & location);
    
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
     * If this problem can be solved, this may return an assistant for the solution.
     */
    virtual KSharedPtr<IAssistant> solutionAssistant() const;

    /**
     * Set an assistant for solving this problem
     */
    void setSolutionAssistant(KSharedPtr<IAssistant> assistant);
    
    enum {
        Identity = 15
    };
    
    /**
     * Returns a string representation of this problem, useful for debugging.
     */
    QString toString() const;

    private:
    Problem(const Problem& other);
    Problem& operator=(const Problem& rhs);
    KSharedPtr< KDevelop::IAssistant > m_solution;
    
    DUCHAIN_DECLARE_DATA(Problem)
};

typedef KSharedPtr<Problem> ProblemPointer;

}

KDEVPLATFORMLANGUAGE_EXPORT QDebug operator<<(QDebug s, const KDevelop::Problem& problem);
KDEVPLATFORMLANGUAGE_EXPORT QDebug operator<<(QDebug s, const KDevelop::ProblemPointer& problem);

#endif // KDEVPLATFORM_I_PROBLEM_H
