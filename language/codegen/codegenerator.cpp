/*
   Copyright 2008 Hamish Rodda <rodda@kde.org>
   Copyright 2009 Ramon Zarazua <killerfox512 gmail.com>

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

#include "codegenerator.h"

#include "documentchangeset.h"
#include "../duchain/topducontext.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include <qtimer.h>
#include "duchainchangeset.h"

namespace KDevelop
{

struct CodeGeneratorPrivate
{
    QMap<IndexedString, DUChainChangeSet *> duchainChanges;
    DocumentChangeSet documentChanges;
};

CodeGeneratorBase::CodeGeneratorBase()
    : d(new CodeGeneratorPrivate)
{
}

CodeGeneratorBase::~CodeGeneratorBase()
{
    delete d;
}

DocumentChangeSet* CodeGeneratorBase::textEdits() const
{
    return &d->documentChanges;
}

void CodeGeneratorBase::start()
{
    kDebug() << "Starting Code Generation Job";
    QTimer::singleShot(0, this, SLOT(executeGenerator()));
}

void CodeGeneratorBase::addChangeSet(DUChainChangeSet * duchainChange)
{
    IndexedString file = duchainChange->topDuContext().data()->url() ;
    
    QMap<IndexedString, DUChainChangeSet *>::iterator it = d->duchainChanges.find(file);
    
    //if we already have an entry for this file, merge it
    if(it !=d->duchainChanges.end())
    {
        **it << *duchainChange;
        delete duchainChange;
    }
    else
        d->duchainChanges.insert(file, duchainChange);
}

void CodeGeneratorBase::setErrorText(const QString & errorText)
{
    KJob::setErrorText(errorText);
}

void CodeGeneratorBase::executeGenerator()
{
    kDebug() << "Checking Preconditions for the codegenerator";
    
    //Shouldn't there be a method in iDocument to get a DocumentRange as well?
    DocumentRange range( ICore::self()->documentController()->activeDocument()->url().url(),
                         ICore::self()->documentController()->activeDocument()->textSelection());
    if(!checkPreconditions(0,range))
    {
        setErrorText("Error checking conditions to generate code: " + errorText());
        emitResult();
    }
    kDebug() << "Gathering user information for the codegenerator";
    if(!gatherInformation())
    {
        setErrorText("Error Ggathering user information: " + errorText());
        emitResult();
    }
    
    // Fetch the AST either from your cache, or recreate it
    // To recreate:
    //         Fetch text from the editor, with the same revision number as the duchain was 
    // created off (we have to save the revision number with the duchain, and not 
    // release it until the duchain is reparsed - I can help with this)
    //         Run through the preprocessor and parser
    //         Save the ast in the cache
    
    kDebug() << "Generating code";
    if(!process())
    {
        setErrorText("Error generating code: " + errorText());
        emitResult();
    }
    kDebug() << "Submitting to the user for review";
    
    if(!displayChanges())
        emitResult();
    
    //Apply Changes
    
    // Merge DUChainChangeSets
    // Apply DUChainChangeSets (to make the language independent modifications to the 
    // AST)
    // Present AST to language-specific refactoring code, if applicable
    //         Apply AstChangeSets
    // Create a DocumentChangeSet
    // Merge changes into document
    // If no errors were found, save
    // Reparse DUChain
    
    d->documentChanges.applyAllChanges();
    emitResult();
}

bool CodeGeneratorBase::displayChanges()
{
    //Create a window that shows changes to be made
    return true;
}

}
