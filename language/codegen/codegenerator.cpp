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
#include "duchainchangeset.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include <language/duchain/duchainlock.h>

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
    clearChangeSets();
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
    //executeGenerator();
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

void CodeGeneratorBase::addChangeSet(DocumentChangeSet & docChangeSet)
{
    d->documentChanges << docChangeSet;
}

DocumentChangeSet & CodeGeneratorBase::documentChangeSet()
{
    return d->documentChanges;
}


void CodeGeneratorBase::setErrorText(const QString & errorText)
{
    KJob::setErrorText(errorText);
}

void CodeGeneratorBase::clearChangeSets(void)
{
    kDebug() << "Cleaning up all the changesets registered by the generator";
    foreach(DUChainChangeSet * changeSet, d->duchainChanges)
        delete changeSet;
    d->duchainChanges.clear();
    
    d->documentChanges.clear();
}

void CodeGeneratorBase::executeGenerator()
{
    DUChainReadLocker lock(DUChain::self()->lock());
    kDebug() << "Checking Preconditions for the codegenerator";
    
    //Shouldn't there be a method in iDocument to get a DocumentRange as well?
    KUrl document = ICore::self()->documentController()->activeDocument()->url();
    DocumentRange range( document.url(), ICore::self()->documentController()->activeDocument()->textSelection());
    
    DUContext * selectionContext = DUChain::self()->chainForDocument(document)->findContextIncluding(range);
    
    if(!selectionContext)
    {
        setErrorText("Error finding context for selection range");
        emitResult();
        return;
    }
    
    if(!checkPreconditions(selectionContext,range))
    {
        setErrorText("Error checking conditions to generate code: " + errorText());
        emitResult();
        return;
    }
    kDebug() << "Gathering user information for the codegenerator";
    if(!gatherInformation())
    {
        setErrorText("Error Gathering user information: " + errorText());
        emitResult();
        return;
    }
    
    kDebug() << "Generating code";
    if(!process())
    {
        setErrorText("Error generating code: " + errorText());
        emitResult();
        return;
    }
    kDebug() << "Submitting to the user for review";
    
    if(!displayChanges())
    {
        emitResult();
        return;
    }
    
    if(DocumentChangeSet::ChangeResult result = d->documentChanges.applyAllChanges())
        setErrorText(result.m_failureReason);
    
    emitResult();
}

bool CodeGeneratorBase::displayChanges()
{
    //Create a window that shows changes to be made
    return true;
}

}

#include "codegenerator.moc"
