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

#ifndef KDEVPLATFORM_CODEGENERATOR_H
#define KDEVPLATFORM_CODEGENERATOR_H

#include "language/editor/documentrange.h"
#include <serialization/indexedstring.h>
#include "../duchain/topducontext.h"
#include "../duchain/duchain.h"

#include "language/interfaces/iastcontainer.h"
#include "util/debug.h"

namespace KDevelop
{
template<class AstNode >
class AstChangeSet;

class DUContext;
class DUChainChangeSet;
class DocumentChangeSet;

/**
 * \short Base class for generic code generators
 *
 * CodeGeneratorBase provides an api for a step-by-step process to
 * create and/or refactor code.
 *
 * This class should be used as a superclass only when du-chain level
 * changes are made, since this level knows nothing about the
 * language-specific AST. For more complex changes that require knowledge
 * about the language AST, use CodeGenerator
 *
 * \see Refactoring
 * \see CodeGenerator
 * \author Hamish Rodda <rodda@kde.org>
 */
class KDEVPLATFORMLANGUAGE_EXPORT CodeGeneratorBase
{
public:
    CodeGeneratorBase();
    virtual ~CodeGeneratorBase();

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
     *          the conditions are not met. Use setErrorText to specify the nature of the Error.
     */
    virtual bool checkPreconditions(DUContext* context, const DocumentRange& position) = 0;

    /**
     * Gather information required from the user for this generator.
     *
     * \returns true if all of the information is retrieved, otherwise false, Use setErrorText
     * to specify the nature of the Error.
     */
    virtual bool gatherInformation() = 0;

    /**
     * Do final condition checking and perform the code generation.
     *
     * \returns true if code generation was successful, false otherwise. Use setErrorText to
     * specify the nature of the Error.
     */
    virtual bool process() = 0;

    const QString & errorText() const;

    // Implementation from kJob
    bool execute();

    /**
     * @brief Indicates that this generation should not expect interaction with the user/
     * Most probable scenarios are: Testing, and a generator that is being used by another one
     * @param context If not NULL, the custom context to use, instead of user selection
     * @param position If not NULL, the cursom range to use instead of user selection
     * @note If this function is called, then gather information will not be called.
     *       Derived classes should provide an alternative way of setting up the generator.
     */
    void autoGenerate(DUContext * context, const DocumentRange * range);

    /**
     * \return The Document Change set to add a single Change, it is more addicient than creating a local DocumentChangeSet and merging it
     */
    DocumentChangeSet & documentChangeSet();


protected:

    /**
     * Generate text edits from duchain change set.
     * This generator now owns the changeset, and will delete it.
     *
     * You may call this method multiple times to edit different files.
     */
    void addChangeSet(DUChainChangeSet* duChainChange);

    void addChangeSet(DocumentChangeSet & docChangeSet);

    /**
     * Accessor for KJob's KJob::setErrorText.
     */
    void setErrorText(const QString & error);

    /**
     * Inform the derived class if this generation is being performed without user interaction
     */
    bool autoGeneration() const;

    /**
     * Clean up all the change sets that this generator is in charge of
     */
    void clearChangeSets();

private:
    class CodeGeneratorPrivate * const d;

    bool displayChanges();
};

/**
 * \brief Base class for Ast aware code generators
 *
 * This class provides convenience for adding AstChangeSet, storing
 * the IAstContainer from the TopDUContext, and in general managing
 * Code generators that manipulate the AST
 *
 * \see CodeGeneratorBase
 * \author Ramón Zarazúa <killerfox512+kde@gmail.com>
 */
template <typename AstContainer>
class CodeGenerator : public CodeGeneratorBase
{
public:
    ~CodeGenerator()
    {
        clearChangeSets();
    }

protected:

    /// Convenience definition of the TopAstNode that is contained by this AstContainer
    typedef typename AstContainer::TopAstNode TopAstNode;
    typedef AstChangeSet<TopAstNode> LanguageChangeSet;

    /**
     * Query an AST of a particular file
     */
    TopAstNode * ast(const IndexedString & file)
    {
        return astContainer(file)->topAstNode();
    }

    TopAstNode * ast(const TopDUContext & context)
    {
        return astContainer(context)->topAstNode();
    }

    typename AstContainer::Ptr astContainer(const IndexedString & file)
    {
        if(!m_AstContainers.contains(file))
        {
            qCDebug(LANGUAGE) << "Ast requested for: " << file.str();

            TopDUContext * context = DUChain::self()->waitForUpdate(file, KDevelop::TopDUContext::AST).data();

            Q_ASSERT(context);
            m_AstContainers[file] = AstContainer::Ptr::template staticCast<IAstContainer>( context->ast() );
        }

        return m_AstContainers[file];
    }

    typename AstContainer::Ptr astContainer(const TopDUContext & context)
    {
        return astContainer(context.url());
    }

    /**
     * Generate text edits from duchain / ast change set.
     *
     * You may call this method multiple times to edit different files.
     */
    void addChangeSet(DUChainChangeSet * duChainChange)
    {
        CodeGeneratorBase::addChangeSet(duChainChange);
    }

    void addChangeSet(DocumentChangeSet & doc)
    {
        CodeGeneratorBase::addChangeSet(doc);
    }

    /**
     * Generate text edits from duchain / ast change set.
     *
     * You may call this method multiple times to edit different files.
     */
    void addChangeSet(LanguageChangeSet * astChange);

    void clearChangeSets()
    {
        CodeGeneratorBase::clearChangeSets();
    }

    /**
     * Inform the derived class if this generation is being performed without user interaction
     */
    bool autoGeneration() const
    {
        return CodeGeneratorBase::autoGeneration();
    }

    /**
     * Accessor for KJob's KJob::setErrorText.
     */
    void setErrorText(const QString & error)
    {
        CodeGeneratorBase::setErrorText(error);
    }

private:
    typedef QMap<IndexedString, typename AstContainer::Ptr> AstContainerMap;
    AstContainerMap m_AstContainers;
};

}

#endif // KDEVPLATFORM_CODEGENERATOR_H
