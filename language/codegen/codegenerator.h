/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVELOP_CODEGENERATOR_H
#define KDEVELOP_CODEGENERATOR_H

#include <kjob.h>

#include "../editor/documentrange.h"

namespace KDevelop
{

class DUContext;
class EditorChangeSet;
class AstChangeSet;
class DUChainChangeSet;

/**
 * \short Base class for code generators and refactorings
 *
 * CodeGenerator provides an api for a step-by-step process to
 * create and/or refactor code.
 *
 * \see Refactoring
 * \author Hamish Rodda <rodda@kde.org>
 */
class KDEVPLATFORMLANGUAGE_EXPORT CodeGenerator : public KJob
{
    Q_OBJECT

public:
    CodeGenerator();
    virtual ~CodeGenerator();

    enum State {
        Precondition,
        UserInput,
        Processing,
        Review,
        Finished
    };

    /**
     * Check whether the preconditions of this generation are met at the given \a context and
     * \a position.
     * \returns true if conditions are met and the generator can progress, otherwise false if
     *          the conditions are not met.
     */
    virtual bool checkPreconditions(DUContext* context, const DocumentRange& position) = 0;

    /**
     * Gather information required from the user for this generator.
     *
     * \returns true if all of the information is retrieved, otherwise false.
     */
    virtual bool gatherInformation() = 0;

    /**
     * Do final condition checking and perform the code generation.
     */
    virtual bool process() = 0;

    /**
     * Retrieve the text edits to be performed as a result of this code generation.
     */
    EditorChangeSet* textEdits() const;

protected:
    /**
     * Generate text edits from duchain / ast change set.
     *
     * You may call this method multiple times to edit different files.
     */
    void generateTextEdit(AstChangeSet* astChange);

    /**
     * Generate text edits from duchain / ast change set.
     *
     * You may call this method multiple times to edit different files.
     */
    void generateTextEdit(DUChainChangeSet* astChange);

private:
    class CodeGeneratorPrivate* const d;
};

}

#endif // KDEVELOP_CODEGENERATOR_H
