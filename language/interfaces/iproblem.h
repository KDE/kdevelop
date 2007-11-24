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

#include <languageexport.h>

#include <QStack>

#include <documentcursor.h>
#include <documentrange.h>

namespace KDevelop
{

/**
 * An object representing a problem in preprocessing, parsing, definition-use chain compilation, etc.
 */
class KDEVPLATFORMLANGUAGE_EXPORT Problem
{
public:
    Problem();
    Problem(const Problem& other);
    virtual ~Problem();

    enum Source {
        Unknown,
        Disk,
        Preprocessor,
        Lexer,
        Parser,
        DUChainBuilder
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

}

#endif // KDEV_PROBLEM_H
