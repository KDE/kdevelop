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

#ifndef KDEV_PROBLEM_H
#define KDEV_PROBLEM_H


#include <QStack>

#include <ksharedptr.h>

#include <editor/documentcursor.h>
#include <editor/documentrange.h>
#include "../languageexport.h"

namespace KDevelop
{

/**
 * An object representing a problem in preprocessing, parsing, definition-use chain compilation, etc.
 * 
 * You should always use ProblemPointer, because Problem may be subclassed.
 * The subclass would be lost while copying.
 * 
 * Warning: Access to problems must be serialized through DUChainLock.
 */
class KDEVPLATFORMLANGUAGE_EXPORT Problem : public KShared
{
public:
    Problem();
    Problem(const Problem& other);
    virtual ~Problem();

    enum Source {
        Unknown        /**< Unknown problem */,
        Disk           /**< problem reading from disk */,
        Preprocessor   /**< problem during pre-processing */,
        Lexer          /**< problem while lexing the file */,
        Parser         /**< problem while parsing the file */,
        DUChainBuilder /**< problem while building the duchain */
    };

    Source source() const;
    void setSource(Source source);
    
    /**
     * Returns the final location where the error occurred.
     */
    const DocumentRange& finalLocation() const;
    void setFinalLocation(const DocumentRange& location);

    /**
     * Returns the stack of locations via which the error occurred.
     */
    const QStack<DocumentCursor>& locationStack() const;
    void addLocation(const DocumentCursor& cursor);
    void setLocationStack(const QStack<DocumentCursor>& locationStack);
    void clearLocationStack();

    /**
     * A brief description of the problem.
     */
    const QString& description() const;
    void setDescription(const QString& description);

    /**
     * A (detailed) explanation of why the problem occurred.
     */
    const QString& explanation() const;
    void setExplanation(const QString& explanation);

    Problem& operator=(const Problem& rhs);

private:
    class Private;
    Private* d;
};

typedef KSharedPtr<Problem> ProblemPointer;

}

#endif // KDEV_PROBLEM_H
