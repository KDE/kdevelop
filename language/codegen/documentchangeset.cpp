/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "documentchangeset.h"

#include "coderepresentation.h"

#include <algorithm>

#include <QStringList>

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/idocumentcontroller.h>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/parsingenvironment.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/editor/modificationrevisionset.h>

#include <interfaces/isourceformattercontroller.h>
#include <interfaces/isourceformatter.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

#include <project/projectmodel.h>

namespace KDevelop {

typedef QList<DocumentChangePointer> ChangesList;
typedef QHash<IndexedString, ChangesList> ChangesHash;

struct DocumentChangeSetPrivate
{
    DocumentChangeSet::ReplacementPolicy replacePolicy;
    DocumentChangeSet::FormatPolicy formatPolicy;
    DocumentChangeSet::DUChainUpdateHandling updatePolicy;
    DocumentChangeSet::ActivationPolicy activationPolicy;

    ChangesHash changes;
    QHash<IndexedString, IndexedString> documentsRename;

    DocumentChangeSet::ChangeResult addChange(const DocumentChangePointer& change);
    DocumentChangeSet::ChangeResult replaceOldText(CodeRepresentation* repr, const QString& newText,
                                                   const ChangesList& sortedChangesList);
    DocumentChangeSet::ChangeResult generateNewText(const IndexedString& file,
                                                    ChangesList& sortedChanges,
                                                    const CodeRepresentation* repr,
                                                    QString& output);
    DocumentChangeSet::ChangeResult removeDuplicates(const IndexedString& file,
                                                     ChangesList& filteredChanges);
    void formatChanges();
    void updateFiles();
};

// Simple helpers to clear up code clutter
namespace
{
inline bool changeIsValid(const DocumentChange& change, const QStringList& textLines)
{
    return change.m_range.start <= change.m_range.end &&
           change.m_range.end.line < textLines.size() &&
           change.m_range.start.line >= 0 &&
           change.m_range.start.column >= 0 &&
           change.m_range.start.column <= textLines[change.m_range.start.line].length() &&
           change.m_range.end.column >= 0 &&
           change.m_range.end.column <= textLines[change.m_range.end.line].length();
}

inline bool duplicateChanges(const DocumentChangePointer& previous, const DocumentChangePointer& current)
{
    // Given the option of considering a duplicate two changes in the same range
    // but with different old texts to be ignored
    return previous->m_range == current->m_range &&
           previous->m_newText == current->m_newText &&
           (previous->m_oldText == current->m_oldText ||
           (previous->m_ignoreOldText && current->m_ignoreOldText));
}

inline QString rangeText(const SimpleRange& range, const QStringList& textLines)
{
    QStringList ret;
    ret.reserve(range.end.line - range.start.line + 1);
    for(int line = range.start.line; line <= range.end.line; ++line) {
        const QString lineText = textLines.at(line);
        int startColumn = 0;
        int endColumn = lineText.length();
        if (line == range.start.line) {
            startColumn = range.start.column;
        }
        if (line == range.end.line) {
            endColumn = range.end.column;
        }
        ret << lineText.mid(startColumn, endColumn - startColumn);
    }
    return ret.join("\n");
}

}

DocumentChangeSet::DocumentChangeSet()
: d(new DocumentChangeSetPrivate)
{
    d->replacePolicy = StopOnFailedChange;
    d->formatPolicy = AutoFormatChanges;
    d->updatePolicy = SimpleUpdate;
    d->activationPolicy = DoNotActivate;
}

DocumentChangeSet::DocumentChangeSet(const DocumentChangeSet& rhs)
: d(new DocumentChangeSetPrivate(*rhs.d))
{
}


DocumentChangeSet& DocumentChangeSet::operator=(const DocumentChangeSet& rhs)
{
    *d = *rhs.d;
    return *this;
}

DocumentChangeSet::~DocumentChangeSet()
{
    delete d;
}

DocumentChangeSet::ChangeResult DocumentChangeSet::addChange(const DocumentChange& change)
{
    return d->addChange(DocumentChangePointer(new DocumentChange(change)));
}

DocumentChangeSet::ChangeResult DocumentChangeSet::addChange(const DocumentChangePointer& change)
{
    return d->addChange(change);
}

DocumentChangeSet::ChangeResult DocumentChangeSet::addDocumentRenameChange(const IndexedString& oldFile,
                                                                           const IndexedString& newname)
{
    d->documentsRename.insert(oldFile, newname);
    return true;
}

DocumentChangeSet::ChangeResult DocumentChangeSetPrivate::addChange(const DocumentChangePointer& change)
{
    changes[change->m_document].append(change);
    return true;
}

void DocumentChangeSet::setReplacementPolicy(DocumentChangeSet::ReplacementPolicy policy)
{
    d->replacePolicy = policy;
}

void DocumentChangeSet::setFormatPolicy(DocumentChangeSet::FormatPolicy policy)
{
    d->formatPolicy = policy;
}

void DocumentChangeSet::setUpdateHandling(DocumentChangeSet::DUChainUpdateHandling policy)
{
    d->updatePolicy = policy;
}

void DocumentChangeSet::setActivationPolicy(DocumentChangeSet::ActivationPolicy policy)
{
    d->activationPolicy = policy;
}

DocumentChangeSet::ChangeResult DocumentChangeSet::applyAllChanges()
{
    KUrl oldActiveDoc;
    if (IDocument* activeDoc = ICore::self()->documentController()->activeDocument()) {
        oldActiveDoc = activeDoc->url();
    }

    // rename files
    QHash<IndexedString, IndexedString>::const_iterator it = d->documentsRename.constBegin();
    for(; it != d->documentsRename.constEnd(); ++it) {
        KUrl url = it.key().toUrl();
        IProject* p = ICore::self()->projectController()->findProjectForUrl(url);
        if(p) {
            QList<ProjectFileItem*> files = p->filesForPath(it.key());
            if(!files.isEmpty()) {
                ProjectBaseItem::RenameStatus renamed = files.first()->rename(it.value().str());
                if(renamed == ProjectBaseItem::RenameOk) {
                    const KUrl newUrl(url.upUrl(), it.value().str());
                    if (url == oldActiveDoc) {
                        oldActiveDoc = newUrl;
                    }
                    IndexedString idxNewDoc(newUrl);

                    // ensure changes operate on new file name
                    ChangesHash::iterator iter = d->changes.find(it.key());
                    if (iter != d->changes.end()) {
                        // copy changes
                        ChangesList value = iter.value();
                        // remove old entry
                        d->changes.erase(iter);
                        // adapt to new url
                        ChangesList::iterator itChange = value.begin();
                        ChangesList::iterator itEnd = value.end();
                        for(; itChange != itEnd; ++itChange) {
                            (*itChange)->m_document = idxNewDoc;
                        }
                        d->changes[idxNewDoc] = value;
                    }
                } else {
                    ///FIXME: share code with project manager for the error code string representation
                    return ChangeResult(i18n("Could not rename '%1' to '%2'", url.pathOrUrl(), it.value().str()));
                }
            } else {
                //TODO: do it outside the project management?
                kWarning() << "tried to rename file not tracked by project - not implemented";
            }
        } else {
            kWarning() << "tried to rename a file outside of a project - not implemented";
        }
    }

    QMap<IndexedString, CodeRepresentation::Ptr> codeRepresentations;
    QMap<IndexedString, QString> newTexts;
    ChangesHash filteredSortedChanges;
    ChangeResult result(true);

    QList<IndexedString> files(d->changes.keys());

    foreach(const IndexedString &file, files) {
        CodeRepresentation::Ptr repr = createCodeRepresentation(file);
        if(!repr) {
            return ChangeResult(QString("Could not create a Representation for %1").arg(file.str()));
        }

        codeRepresentations[file] = repr;

        QList<DocumentChangePointer>& sortedChangesList(filteredSortedChanges[file]);
        {
            result = d->removeDuplicates(file, sortedChangesList);
            if(!result)
                return result;
        }

        {
            result = d->generateNewText(file, sortedChangesList, repr.data(), newTexts[file]);
            if(!result)
                return result;
        }
    }

    QMap<IndexedString, QString> oldTexts;

    //Apply the changes to the files
    foreach(const IndexedString &file, files) {
        oldTexts[file] = codeRepresentations[file]->text();

        result = d->replaceOldText(codeRepresentations[file].data(), newTexts[file], filteredSortedChanges[file]);
        if(!result && d->replacePolicy == StopOnFailedChange) {
            //Revert all files
            foreach(const IndexedString &revertFile, oldTexts.keys()) {
                codeRepresentations[revertFile]->setText(oldTexts[revertFile]);
            }

            return result;
        }
    }

    d->updateFiles();

    if(d->activationPolicy == Activate) {
        foreach(const IndexedString& file, files) {
            ICore::self()->documentController()->openDocument(file.toUrl());
        }
    }

    // ensure the old document is still activated
    if (oldActiveDoc.isValid()) {
        ICore::self()->documentController()->openDocument(oldActiveDoc);
    }

    return result;
}

DocumentChangeSet::ChangeResult DocumentChangeSetPrivate::replaceOldText(CodeRepresentation* repr,
                                                                         const QString& newText,
                                                                         const ChangesList& sortedChangesList)
{
    DynamicCodeRepresentation* dynamic = dynamic_cast<DynamicCodeRepresentation*>(repr);
    if(dynamic) {
        auto transaction = dynamic->makeEditTransaction();
        //Replay the changes one by one

        for(int pos = sortedChangesList.size()-1; pos >= 0; --pos) {
            const DocumentChange& change(*sortedChangesList[pos]);
            if(!dynamic->replace(change.m_range.textRange(), change.m_oldText, change.m_newText, change.m_ignoreOldText))
            {
                QString warningString = QString("Inconsistent change in %1 at %2:%3 -> %4:%5 = %6(encountered \"%7\") -> \"%8\"")
                    .arg(change.m_document.str())
                    .arg(change.m_range.start.line)
                    .arg(change.m_range.start.column)
                    .arg(change.m_range.end.line)
                    .arg(change.m_range.end.column)
                    .arg(change.m_oldText)
                    .arg(dynamic->rangeText(change.m_range.textRange()))
                    .arg(change.m_newText);

                if(replacePolicy == DocumentChangeSet::WarnOnFailedChange) {
                    kWarning() << warningString;
                } else if(replacePolicy == DocumentChangeSet::StopOnFailedChange) {
                    return DocumentChangeSet::ChangeResult(warningString);
                }
                //If set to ignore failed changes just continue with the others
            }
        }
        return true;
    }

    //For files on disk
    if (!repr->setText(newText)) {
        QString warningString = QString("Could not replace text in the document: %1")
            .arg(sortedChangesList.begin()->data()->m_document.str());
        if(replacePolicy == DocumentChangeSet::WarnOnFailedChange) {
            kWarning() << warningString;
        }

        return DocumentChangeSet::ChangeResult(warningString);
    }

    return true;
}

DocumentChangeSet::ChangeResult DocumentChangeSetPrivate::generateNewText(const IndexedString & file,
                                                                          ChangesList& sortedChanges,
                                                                          const CodeRepresentation * repr,
                                                                          QString & output)
{

    ISourceFormatter* formatter = 0;
    if(ICore::self()) {
        formatter = ICore::self()->sourceFormatterController()->formatterForUrl(file.toUrl());
    }

    //Create the actual new modified file
    QStringList textLines = repr->text().split('\n');

    KUrl url = file.toUrl();

    KMimeType::Ptr mime = KMimeType::findByUrl(url);
    QVector<int> removedLines;

    for(int pos = sortedChanges.size()-1; pos >= 0; --pos) {
        DocumentChange& change(*sortedChanges[pos]);
        QString encountered;
        if(changeIsValid(change, textLines)  && //We demand this, although it should be fixed
            ((encountered = rangeText(change.m_range, textLines)) == change.m_oldText || change.m_ignoreOldText))
        {
            ///Problem: This does not work if the other changes significantly alter the context @todo Use the changed context
            QString leftContext = QStringList(textLines.mid(0, change.m_range.start.line+1)).join("\n");
            leftContext.chop(textLines[change.m_range.start.line].length() - change.m_range.start.column);

            QString rightContext = QStringList(textLines.mid(change.m_range.end.line)).join("\n").mid(change.m_range.end.column);

            if(formatter && (formatPolicy == DocumentChangeSet::AutoFormatChanges
                                || formatPolicy == DocumentChangeSet::AutoFormatChangesKeepIndentation))
            {
                QString oldNewText = change.m_newText;
                change.m_newText = formatter->formatSource(change.m_newText, url, mime, leftContext, rightContext);

                if(formatPolicy == DocumentChangeSet::AutoFormatChangesKeepIndentation) {
                    // Reproduce the previous indentation
                    QStringList oldLines = oldNewText.split('\n');
                    QStringList newLines = change.m_newText.split('\n');

                    if(oldLines.size() == newLines.size()) {
                        for(int line = 0; line < newLines.size(); ++line) {
                            // Keep the previous indentation
                            QString oldIndentation;
                            for (int a = 0; a < oldLines[line].size(); ++a) {
                                if (oldLines[line][a].isSpace()) {
                                    oldIndentation.append(oldLines[line][a]);
                                } else {
                                    break;
                                }
                            }

                            int newIndentationLength = 0;

                            for(int a = 0; a < newLines[line].size(); ++a) {
                                if(newLines[line][a].isSpace()) {
                                    newIndentationLength = a;
                                } else {
                                    break;
                                }
                            }

                            newLines[line].replace(0, newIndentationLength, oldIndentation);
                        }
                        change.m_newText = newLines.join("\n");
                    } else {
                        kDebug() << "Cannot keep the indentation because the line count has changed" << oldNewText;
                    }
                }
            }

            QString& line = textLines[change.m_range.start.line];
            if (change.m_range.start.line == change.m_range.end.line) {
                // simply replace existing line content
                line.replace(change.m_range.start.column,
                             change.m_range.end.column-change.m_range.start.column,
                             change.m_newText);
            } else {
                // replace first line contents
                line.replace(change.m_range.start.column, line.length() - change.m_range.start.column,
                             change.m_newText);
                // null other lines and remember for deletion
                for(int i = change.m_range.start.line + 1; i <= change.m_range.end.line; ++i) {
                    textLines[i].clear();
                    removedLines << i;
                }
            }
        }else{
            QString warningString = QString("Inconsistent change in %1 at %2:%3 -> %4:%5"
                                            " = \"%6\"(encountered \"%7\") -> \"%8\"")
                                            .arg(file.str())
                                            .arg(change.m_range.start.line)
                                            .arg(change.m_range.start.column)
                                            .arg(change.m_range.end.line)
                                            .arg(change.m_range.end.column)
                                            .arg(change.m_oldText)
                                            .arg(encountered)
                                            .arg(change.m_newText);

            if(replacePolicy == DocumentChangeSet::IgnoreFailedChange) {
                //Just don't do the replacement
            } else if(replacePolicy == DocumentChangeSet::WarnOnFailedChange) {
                kWarning() << warningString;
            } else {
                return DocumentChangeSet::ChangeResult(warningString, sortedChanges[pos]);
            }
        }
    }

    if (!removedLines.isEmpty()) {
        int offset = 0;
        qSort(removedLines);
        foreach(int l, removedLines) {
            textLines.removeAt(l - offset);
            ++offset;
        }
    }
    output = textLines.join("\n");
    return true;
}

//Removes all duplicate changes for a single file, and then returns (via filteredChanges) the filtered duplicates
DocumentChangeSet::ChangeResult DocumentChangeSetPrivate::removeDuplicates(const IndexedString& file,
                                                                           ChangesList& filteredChanges)
{
    typedef QMultiMap<SimpleCursor, DocumentChangePointer> ChangesMap;
    ChangesMap sortedChanges;

    foreach(const DocumentChangePointer &change, changes[file]) {
        sortedChanges.insert(change->m_range.end, change);
    }

    //Remove duplicates
    ChangesMap::iterator previous = sortedChanges.begin();
    for(ChangesMap::iterator it = ++sortedChanges.begin(); it != sortedChanges.end(); ) {
        if(( *previous ) && ( *previous )->m_range.end > (*it)->m_range.start) {
            //intersection
            if(duplicateChanges(( *previous ), *it)) {
                //duplicate, remove one
                it = sortedChanges.erase(it);
                continue;
            }

            //When two changes contain each other, and the container change is set to ignore old text, then it should be safe to
            //just ignore the contained change, and apply the bigger change
            else if((*it)->m_range.contains(( *previous )->m_range) && (*it)->m_ignoreOldText  ) {
                kDebug() << "Removing change: " << ( *previous )->m_oldText << "->" << ( *previous )->m_newText
                         << ", because it is contained by change: " << (*it)->m_oldText << "->" << (*it)->m_newText;
                sortedChanges.erase(previous);
            }
            //This case is for when both have the same end, either of them could be the containing range
            else if((*previous)->m_range.contains((*it)->m_range) && (*previous)->m_ignoreOldText  ) {
                kDebug() << "Removing change: " << (*it)->m_oldText << "->" << (*it)->m_newText
                         << ", because it is contained by change: " << ( *previous )->m_oldText
                         << "->" << ( *previous )->m_newText;
                it = sortedChanges.erase(it);
                continue;
            } else {
                return DocumentChangeSet::ChangeResult(
                       QString("Inconsistent change-request at %1; "
                               "intersecting changes: "
                               "\"%2\"->\"%3\"@%4:%5->%6:%7 & \"%8\"->\"%9\"@%10:%11->%12:%13 ")
                        .arg(file.str(), ( *previous )->m_oldText, ( *previous )->m_newText)
                        .arg(( *previous )->m_range.start.line)
                        .arg(( *previous )->m_range.start.column)
                        .arg(( *previous )->m_range.end.line)
                        .arg(( *previous )->m_range.end.column)
                        .arg((*it)->m_oldText, (*it)->m_newText)
                        .arg((*it)->m_range.start.line)
                        .arg((*it)->m_range.start.column)
                        .arg((*it)->m_range.end.line)
                        .arg((*it)->m_range.end.column));
            }

        }
        previous = it;
        ++it;
    }

    filteredChanges = sortedChanges.values();
    return true;
}

void DocumentChangeSetPrivate::updateFiles()
{
    ModificationRevisionSet::clearCache();
    foreach(const IndexedString& file, changes.keys()) {
        ModificationRevision::clearModificationCache(file);
    }

    if(updatePolicy != DocumentChangeSet::NoUpdate && ICore::self())
    {
        // The active document should be updated first, so that the user sees the results instantly
        if(IDocument* activeDoc = ICore::self()->documentController()->activeDocument()) {
            ICore::self()->languageController()->backgroundParser()->addDocument(IndexedString(activeDoc->url()));
        }

        // If there are currently open documents that now need an update, update them too
        foreach(const IndexedString& doc, ICore::self()->languageController()->backgroundParser()->managedDocuments()) {
            DUChainReadLocker lock(DUChain::lock());
            TopDUContext* top = DUChainUtils::standardContextForUrl(doc.toUrl(), true);
            if((top && top->parsingEnvironmentFile() && top->parsingEnvironmentFile()->needsUpdate()) || !top) {
                lock.unlock();
                ICore::self()->languageController()->backgroundParser()->addDocument(doc);
            }
        }

        // Eventually update _all_ affected files
        foreach(const IndexedString &file, changes.keys()) {
            if(!file.toUrl().isValid()) {
                kWarning() << "Trying to apply changes to an invalid document";
                continue;
            }

            ICore::self()->languageController()->backgroundParser()->addDocument(file);
        }
    }
}

}
