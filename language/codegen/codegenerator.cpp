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

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include <language/duchain/duchainlock.h>

#include <qtimer.h>
#include "applychangeswidget.h"
#include "util/debug.h"

namespace KDevelop
{

class CodeGeneratorPrivate
{
public:

    CodeGeneratorPrivate() : autoGen(false), context(0) {}

    QMap<IndexedString, DUChainChangeSet *> duchainChanges;
    DocumentChangeSet documentChanges;

    bool autoGen;
    DUContext * context;
    DocumentRange range;
    QString error;
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

void CodeGeneratorBase::autoGenerate ( DUContext* context, const KDevelop::DocumentRange* range )
{
    d->autoGen = true;
    d->context = context;
    d->range = *range;
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

const QString & CodeGeneratorBase::errorText() const
{
    return d->error;
}

bool CodeGeneratorBase::autoGeneration() const
{
    return d->autoGen;
}

void CodeGeneratorBase::setErrorText(const QString & errorText)
{
    d->error = errorText;
}

void CodeGeneratorBase::clearChangeSets()
{
    qCDebug(LANGUAGE) << "Cleaning up all the changesets registered by the generator";
    foreach(DUChainChangeSet * changeSet, d->duchainChanges)
        delete changeSet;
    d->duchainChanges.clear();

    d->documentChanges = DocumentChangeSet();
}

bool CodeGeneratorBase::execute()
{
    qCDebug(LANGUAGE) << "Checking Preconditions for the codegenerator";

    //Shouldn't there be a method in iDocument to get a DocumentRange as well?

    QUrl document;
    if(!d->autoGen)
    {
        if( !ICore::self()->documentController()->activeDocument() )
        {
            setErrorText( i18n("Could not find an open document" ) );
            return false;
        }

        document = ICore::self()->documentController()->activeDocument()->url();

        if(d->range.isEmpty())
        {
            DUChainReadLocker lock(DUChain::lock());
            d->range = DocumentRange(document.url(), ICore::self()->documentController()->activeDocument()->textSelection());
        }
    }

    if(!d->context)
    {
        DUChainReadLocker lock(DUChain::lock());
        TopDUContext * documentChain = DUChain::self()->chainForDocument(document);
        if(!documentChain)
        {
            setErrorText(QString("Could not find chain for selected document: %1").arg(document.url()));
            return false;
        }
        d->context = documentChain->findContextIncluding(d->range);

        if(!d->context)
        {
            //Attempt to get the context again
            QList<TopDUContext *> contexts = DUChain::self()->chainsForDocument(document);
            foreach(TopDUContext * top, contexts)
            {
                qCDebug(LANGUAGE) << "Checking top context with range: " << top->range() << " for a context";
                if((d->context = top->findContextIncluding(d->range)))
                    break;
            }
        }
    }

    if(!d->context)
    {
        setErrorText(i18n("Error finding context for selection range"));
        return false;
    }

    if(!checkPreconditions(d->context,d->range))
    {
        setErrorText(i18n("Error checking conditions to generate code: %1",errorText()));
        return false;
    }

    if(!d->autoGen)
    {
        qCDebug(LANGUAGE) << "Gathering user information for the codegenerator";
        if(!gatherInformation())
        {
            setErrorText(i18n("Error Gathering user information: %1",errorText()));
            return false;
        }
    }

    qCDebug(LANGUAGE) << "Generating code";
    if(!process())
    {
        setErrorText(i18n("Error generating code: %1",errorText()));
        return false;
    }

    if(!d->autoGen)
    {
        qCDebug(LANGUAGE) << "Submitting to the user for review";
        return displayChanges();
    }

    //If it is autogenerated, it shouldn't need to apply changes, instead return them to client that my be another generator
    DocumentChangeSet::ChangeResult result(true);
    if(!d->autoGen && !(result = d->documentChanges.applyAllChanges()))
    {
        setErrorText(result.m_failureReason);
        return false;
    }

    return true;
}

bool CodeGeneratorBase::displayChanges()
{
    DocumentChangeSet::ChangeResult result = d->documentChanges.applyAllToTemp();
    if(!result)
    {
        setErrorText(result.m_failureReason);
        return false;
    }

    ApplyChangesWidget widget;
    //TODO: Find some good information to put
    widget.setInformation("Info?");

    QMap<IndexedString, IndexedString> temps = d->documentChanges.tempNamesForAll();
    for(QMap<IndexedString, IndexedString>::iterator it = temps.begin();
        it != temps.end(); ++it)
        widget.addDocuments(it.key() , it.value());

    if(widget.exec())
        return widget.applyAllChanges();
    else
        return false;
}

}
