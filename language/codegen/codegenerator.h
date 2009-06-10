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

#include "language/editor/documentrange.h"
#include "../duchain/indexedstring.h"
#include "../duchain/topducontext.h"
#include "../duchain/duchain.h"

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
class KDEVPLATFORMLANGUAGE_EXPORT CodeGeneratorBase : public KJob
{
    Q_OBJECT

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
    
    /**
     * Retrieve the text edits to be performed as a result of this code generation.
     */
    DocumentChangeSet* textEdits() const;
    
    // Implementation from kJob
    virtual void start(void);


protected:

    /**
     * Generate text edits from duchain change set.
     * This generator now owns the changeset, and will delete it.
     *
     * You may call this method multiple times to edit different files.
     */
    void addChangeSet(DUChainChangeSet* duChainChange);
    
    /**
     * Accessor for KJob's KJob::setErrorText.
     */
    void setErrorText(const QString & error);

private:
    class CodeGeneratorPrivate * const d;
    
    void executeGenerator(void);
    bool displayChanges(void);
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
    CodeGenerator() {}
    
protected:
    
    /// Convenience definition of the TopAstNode that is contained by this AstContainer
    typedef typename AstContainer::TopAstNode TopAstNode;
    typedef AstChangeSet<TopAstNode> LanguageChangeSet;
    
    /**
     * Query an AST of a particular file
     */
    TopAstNode * ast(const IndexedString & file)
    {
        typename AstContainer::Ptr & container = m_AstContainers[file];
        if(container.isNull)
        {
            kDebug() << "Ast requested for: " << file.str();
            TopDUContext * context = DUChain::self()->chainForDocument(file);
            
            Q_ASSERT(context);
            m_AstContainers[file] = AstContainer::Ptr( context->ast().data() );
        }
        
        return container->topAstNode();
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
    
    /**
     * Generate text edits from duchain / ast change set.
     *
     * You may call this method multiple times to edit different files.
     */
    void addChangeSet(LanguageChangeSet * astChange);
    
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

#endif // KDEVELOP_CODEGENERATOR_H
