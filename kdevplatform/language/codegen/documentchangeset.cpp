/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "documentchangeset.h"

#include "coderepresentation.h"
#include <debug.h>

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
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

#include <project/projectmodel.h>

#include <util/path.h>
#include <util/shellutils.h>

namespace KDevelop {
using ChangesList = QList<DocumentChangePointer>;
using ChangesHash = QHash<IndexedString, ChangesList>;

class DocumentChangeSetPrivate
{
public:
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
namespace {
inline bool changeIsValid(const DocumentChange& change, const QStringList& textLines)
{
    return change.m_range.start() <= change.m_range.end() &&
           change.m_range.end().line() < textLines.size() &&
           change.m_range.start().line() >= 0 &&
           change.m_range.start().column() >= 0 &&
           change.m_range.start().column() <= textLines[change.m_range.start().line()].length() &&
           change.m_range.end().column() >= 0 &&
           change.m_range.end().column() <= textLines[change.m_range.end().line()].length();
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

inline QString rangeText(const KTextEditor::Range& range, const QStringList& textLines)
{
    QStringList ret;
    ret.reserve(range.end().line() - range.start().line() + 1);
    for (int line = range.start().line(); line <= range.end().line(); ++line) {
        const QString lineText = textLines.at(line);
        int startColumn = 0;
        int endColumn = lineText.length();
        if (line == range.start().line()) {
            startColumn = range.start().column();
        }
        if (line == range.end().line()) {
            endColumn = range.end().column();
        }
        ret << lineText.mid(startColumn, endColumn - startColumn);
    }

    return ret.join(QLatin1Char('\n'));
}

// need to have it as otherwise the arguments can exceed the maximum of 10
static QString printRange(const KTextEditor::Range& r)
{
    return i18nc("text range line:column->line:column", "%1:%2->%3:%4",
                 r.start().line(), r.start().column(),
                 r.end().line(), r.end().column());
}
}

DocumentChangeSet::DocumentChangeSet()
    : d_ptr(new DocumentChangeSetPrivate)
{
    Q_D(DocumentChangeSet);

    d->replacePolicy = StopOnFailedChange;
    d->formatPolicy = AutoFormatChanges;
    d->updatePolicy = SimpleUpdate;
    d->activationPolicy = DoNotActivate;
}

DocumentChangeSet::DocumentChangeSet(const DocumentChangeSet& rhs)
    : d_ptr(new DocumentChangeSetPrivate(*rhs.d_ptr))
{
}

DocumentChangeSet& DocumentChangeSet::operator=(const DocumentChangeSet& rhs)
{
    *d_ptr = *rhs.d_ptr;
    return *this;
}

DocumentChangeSet::~DocumentChangeSet() = default;

DocumentChangeSet::ChangeResult DocumentChangeSet::addChange(const DocumentChange& change)
{
    Q_D(DocumentChangeSet);

    return d->addChange(DocumentChangePointer(new DocumentChange(change)));
}

DocumentChangeSet::ChangeResult DocumentChangeSet::addChange(const DocumentChangePointer& change)
{
    Q_D(DocumentChangeSet);

    return d->addChange(change);
}

DocumentChangeSet::ChangeResult DocumentChangeSet::addDocumentRenameChange(const IndexedString& oldFile,
                                                                           const IndexedString& newname)
{
    Q_D(DocumentChangeSet);

    d->documentsRename.insert(oldFile, newname);
    return DocumentChangeSet::ChangeResult::successfulResult();
}

DocumentChangeSet::ChangeResult DocumentChangeSetPrivate::addChange(const DocumentChangePointer& change)
{
    changes[change->m_document].append(change);
    return DocumentChangeSet::ChangeResult::successfulResult();
}

void DocumentChangeSet::setReplacementPolicy(DocumentChangeSet::ReplacementPolicy policy)
{
    Q_D(DocumentChangeSet);

    d->replacePolicy = policy;
}

void DocumentChangeSet::setFormatPolicy(DocumentChangeSet::FormatPolicy policy)
{
    Q_D(DocumentChangeSet);

    d->formatPolicy = policy;
}

void DocumentChangeSet::setUpdateHandling(DocumentChangeSet::DUChainUpdateHandling policy)
{
    Q_D(DocumentChangeSet);

    d->updatePolicy = policy;
}

void DocumentChangeSet::setActivationPolicy(DocumentChangeSet::ActivationPolicy policy)
{
    Q_D(DocumentChangeSet);

    d->activationPolicy = policy;
}

DocumentChangeSet::ChangeResult DocumentChangeSet::applyAllChanges()
{
    Q_D(DocumentChangeSet);

    QUrl oldActiveDoc;
    if (IDocument* activeDoc = ICore::self()->documentController()->activeDocument()) {
        oldActiveDoc = activeDoc->url();
    }

    QList<QUrl> allFiles;
    const auto changedFiles = QSet<KDevelop::IndexedString>(d->documentsRename.keyBegin(), d->documentsRename.keyEnd())
        + QSet<KDevelop::IndexedString>(d->changes.keyBegin(), d->changes.keyEnd());
    allFiles.reserve(changedFiles.size());
    for (const IndexedString& file : changedFiles) {
        allFiles << file.toUrl();
    }

    if (!KDevelop::ensureWritable(allFiles)) {
        return ChangeResult(QStringLiteral("some affected files are not writable"));
    }

    // rename files
    QHash<IndexedString, IndexedString>::const_iterator it = d->documentsRename.constBegin();
    for (; it != d->documentsRename.constEnd(); ++it) {
        QUrl url = it.key().toUrl();
        IProject* p = ICore::self()->projectController()->findProjectForUrl(url);
        if (p) {
            QList<ProjectFileItem*> files = p->filesForPath(it.key());
            if (!files.isEmpty()) {
                ProjectBaseItem::RenameStatus renamed = files.first()->rename(it.value().str());
                if (renamed == ProjectBaseItem::RenameOk) {
                    const QUrl newUrl = Path(Path(url).parent(), it.value().str()).toUrl();
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
                        for (auto& change : value) {
                            change->m_document = idxNewDoc;
                        }

                        d->changes[idxNewDoc] = value;
                    }
                } else {
                    ///FIXME: share code with project manager for the error code string representation
                    return ChangeResult(i18n("Could not rename '%1' to '%2'",
                                             url.toDisplayString(QUrl::PreferLocalFile), it.value().str()));
                }
            } else {
                //TODO: do it outside the project management?
                qCWarning(LANGUAGE) << "tried to rename file not tracked by project - not implemented";
            }
        } else {
            qCWarning(LANGUAGE) << "tried to rename a file outside of a project - not implemented";
        }
    }

    QMap<IndexedString, CodeRepresentation::Ptr> codeRepresentations;
    QMap<IndexedString, QString> newTexts;
    ChangesHash filteredSortedChanges;
    ChangeResult result = ChangeResult::successfulResult();

    const QList<IndexedString> files(d->changes.keys());

    for (const IndexedString& file : files) {
        CodeRepresentation::Ptr repr = createCodeRepresentation(file);
        if (!repr) {
            return ChangeResult(QStringLiteral("Could not create a Representation for %1").arg(file.str()));
        }

        codeRepresentations[file] = repr;

        QList<DocumentChangePointer>& sortedChangesList(filteredSortedChanges[file]);
        {
            result = d->removeDuplicates(file, sortedChangesList);
            if (!result)
                return result;
        }

        {
            result = d->generateNewText(file, sortedChangesList, repr.data(), newTexts[file]);
            if (!result)
                return result;
        }
    }

    QMap<IndexedString, QString> oldTexts;

    //Apply the changes to the files
    for (const IndexedString& file : files) {
        oldTexts[file] = codeRepresentations[file]->text();

        result = d->replaceOldText(codeRepresentations[file].data(), newTexts[file], filteredSortedChanges[file]);
        if (!result && d->replacePolicy == StopOnFailedChange) {
            //Revert all files
            for (auto it = oldTexts.constBegin(), end = oldTexts.constEnd(); it != end; ++it) {
                const IndexedString& revertFile = it.key();
                const QString& oldText = it.value();
                codeRepresentations[revertFile]->setText(oldText);
            }

            return result;
        }
    }

    d->updateFiles();

    if (d->activationPolicy == Activate) {
        for (const IndexedString& file : files) {
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
    auto* dynamic = dynamic_cast<DynamicCodeRepresentation*>(repr);
    if (dynamic) {
        auto transaction = dynamic->makeEditTransaction();
        //Replay the changes one by one

        for (int pos = sortedChangesList.size() - 1; pos >= 0; --pos) {
            const DocumentChange& change(*sortedChangesList[pos]);
            if (!dynamic->replace(change.m_range, change.m_oldText, change.m_newText, change.m_ignoreOldText)) {
                QString warningString = i18nc(
                    "Inconsistent change in <filename> between <range>, found <oldText> (encountered <foundText>) -> <newText>",
                    "Inconsistent change in %1 between %2, found %3 (encountered \"%4\") -> \"%5\"",
                    change.m_document.str(),
                    printRange(change.m_range),
                    change.m_oldText,
                    dynamic->rangeText(change.m_range),
                    change.m_newText);

                if (replacePolicy == DocumentChangeSet::WarnOnFailedChange) {
                    qCWarning(LANGUAGE) << warningString;
                } else if (replacePolicy == DocumentChangeSet::StopOnFailedChange) {
                    return DocumentChangeSet::ChangeResult(warningString);
                }
                //If set to ignore failed changes just continue with the others
            }
        }

        return DocumentChangeSet::ChangeResult::successfulResult();
    }

    //For files on disk
    if (!repr->setText(newText)) {
        QString warningString = i18n("Could not replace text in the document: %1",
                                     sortedChangesList.begin()->data()->m_document.str());
        if (replacePolicy == DocumentChangeSet::WarnOnFailedChange) {
            qCWarning(LANGUAGE) << warningString;
        }

        return DocumentChangeSet::ChangeResult(warningString);
    }

    return DocumentChangeSet::ChangeResult::successfulResult();
}

DocumentChangeSet::ChangeResult DocumentChangeSetPrivate::generateNewText(const IndexedString& file,
                                                                          ChangesList& sortedChanges,
                                                                          const CodeRepresentation* repr,
                                                                          QString& output)
{
    //Create the actual new modified file
    QStringList textLines = repr->text().split(QLatin1Char('\n'));

    ISourceFormatterController::FileFormatterPtr formatter;
    if (formatPolicy != DocumentChangeSet::NoAutoFormat) {
        formatter = ICore::self()->sourceFormatterController()->fileFormatter(file.toUrl());
    }

    QVector<int> removedLines;

    for (int pos = sortedChanges.size() - 1; pos >= 0; --pos) {
        DocumentChange& change(*sortedChanges[pos]);
        QString encountered;
        if (changeIsValid(change, textLines)  && //We demand this, although it should be fixed
            ((encountered = rangeText(change.m_range, textLines)) == change.m_oldText || change.m_ignoreOldText)) {
            ///Problem: This does not work if the other changes significantly alter the context @todo Use the changed context
            QString leftContext = QStringList(textLines.mid(0, change.m_range.start().line() + 1)).join(QLatin1Char(
                                                                                                            '\n'));
            leftContext.chop(textLines[change.m_range.start().line()].length() - change.m_range.start().column());

            QString rightContext = QStringList(textLines.mid(change.m_range.end().line())).join(QLatin1Char('\n')).mid(
                change.m_range.end().column());

            if (formatter) {
                QString oldNewText = change.m_newText;
                change.m_newText = formatter->format(change.m_newText, leftContext, rightContext);

                if (formatPolicy == DocumentChangeSet::AutoFormatChangesKeepIndentation) {
                    // Reproduce the previous indentation
                    const QStringList oldLines = oldNewText.split(QLatin1Char('\n'));
                    QStringList newLines = change.m_newText.split(QLatin1Char('\n'));

                    if (oldLines.size() == newLines.size()) {
                        for (int line = 0; line < newLines.size(); ++line) {
                            // Keep the previous indentation
                            QString oldIndentation;
                            for (const QChar a : oldLines[line]) {
                                if (a.isSpace()) {
                                    oldIndentation.append(a);
                                } else {
                                    break;
                                }
                            }

                            int newIndentationLength = 0;

                            for (int a = 0; a < newLines[line].size(); ++a) {
                                if (newLines[line][a].isSpace()) {
                                    newIndentationLength = a;
                                } else {
                                    break;
                                }
                            }

                            newLines[line].replace(0, newIndentationLength, oldIndentation);
                        }

                        change.m_newText = newLines.join(QLatin1Char('\n'));
                    } else {
                        qCDebug(LANGUAGE) << "Cannot keep the indentation because the line count has changed" <<
                            oldNewText;
                    }
                }
            }

            QString& line = textLines[change.m_range.start().line()];
            if (change.m_range.start().line() == change.m_range.end().line()) {
                // simply replace existing line content
                line.replace(change.m_range.start().column(),
                             change.m_range.end().column() - change.m_range.start().column(),
                             change.m_newText);
            } else {
                // replace first line contents
                line.replace(change.m_range.start().column(), line.length() - change.m_range.start().column(),
                             change.m_newText);
                // null other lines and remember for deletion
                const int firstLine = change.m_range.start().line() + 1;
                const int lastLine = change.m_range.end().line();
                removedLines.reserve(removedLines.size() + lastLine - firstLine + 1);
                for (int i = firstLine; i <= lastLine; ++i) {
                    textLines[i].clear();
                    removedLines << i;
                }
            }
        } else {
            QString warningString = i18nc("Inconsistent change in <document> at <range>"
                                          " = <oldText> (encountered <encountered>) -> <newText>",
                                          "Inconsistent change in %1 at %2"
                                          " = \"%3\"(encountered \"%4\") -> \"%5\"",
                                          file.str(),
                                          printRange(change.m_range),
                                          change.m_oldText,
                                          encountered,
                                          change.m_newText);

            if (replacePolicy == DocumentChangeSet::IgnoreFailedChange) {
                //Just don't do the replacement
            } else if (replacePolicy == DocumentChangeSet::WarnOnFailedChange) {
                qCWarning(LANGUAGE) << warningString;
            } else {
                return DocumentChangeSet::ChangeResult(warningString, sortedChanges[pos]);
            }
        }
    }

    if (!removedLines.isEmpty()) {
        int offset = 0;
        std::sort(removedLines.begin(), removedLines.end());
        for (int l : std::as_const(removedLines)) {
            textLines.removeAt(l - offset);
            ++offset;
        }
    }
    output = textLines.join(QLatin1Char('\n'));
    return DocumentChangeSet::ChangeResult::successfulResult();
}

//Removes all duplicate changes for a single file, and then returns (via filteredChanges) the filtered duplicates
DocumentChangeSet::ChangeResult DocumentChangeSetPrivate::removeDuplicates(const IndexedString& file,
                                                                           ChangesList& filteredChanges)
{
    using ChangesMap = QMultiMap<KTextEditor::Cursor, DocumentChangePointer>;
    ChangesMap sortedChanges;

    for (const DocumentChangePointer& change : std::as_const(changes[file])) {
        sortedChanges.insert(change->m_range.end(), change);
    }

    //Remove duplicates
    ChangesMap::iterator previous = sortedChanges.begin();
    for (ChangesMap::iterator it = ++sortedChanges.begin(); it != sortedChanges.end();) {
        if ((*previous) && (*previous)->m_range.end() > (*it)->m_range.start()) {
            //intersection
            if (duplicateChanges((*previous), *it)) {
                //duplicate, remove one
                it = sortedChanges.erase(it);
                continue;
            }
            //When two changes contain each other, and the container change is set to ignore old text, then it should be safe to
            //just ignore the contained change, and apply the bigger change
            else if ((*it)->m_range.contains((*previous)->m_range) && (*it)->m_ignoreOldText) {
                qCDebug(LANGUAGE) << "Removing change: " << (*previous)->m_oldText << "->" << (*previous)->m_newText
                                  << ", because it is contained by change: " << (*it)->m_oldText << "->" <<
                (*it)->m_newText;
                sortedChanges.erase(previous);
            }
            //This case is for when both have the same end, either of them could be the containing range
            else if ((*previous)->m_range.contains((*it)->m_range) && (*previous)->m_ignoreOldText) {
                qCDebug(LANGUAGE) << "Removing change: " << (*it)->m_oldText << "->" << (*it)->m_newText
                                  << ", because it is contained by change: " << (*previous)->m_oldText
                                  << "->" << (*previous)->m_newText;
                it = sortedChanges.erase(it);
                continue;
            } else {
                return DocumentChangeSet::ChangeResult(
                    i18nc("Inconsistent change-request at <document>:"
                          "intersecting changes: "
                          "<previous-oldText> -> <previous-newText> @<range> & "
                          "<new-oldText> -> <new-newText> @<range>",
                          "Inconsistent change-request at %1; "
                          "intersecting changes: "
                          "\"%2\"->\"%3\"@%4 & \"%5\"->\"%6\"@%7 ",
                          file.str(),
                          (*previous)->m_oldText,
                          (*previous)->m_newText,
                          printRange((*previous)->m_range),
                          (*it)->m_oldText,
                          (*it)->m_newText,
                          printRange((*it)->m_range)));
            }
        }
        previous = it;
        ++it;
    }

    filteredChanges = sortedChanges.values();
    return DocumentChangeSet::ChangeResult::successfulResult();
}

void DocumentChangeSetPrivate::updateFiles()
{
    ModificationRevisionSet::clearCache();
    const auto files = changes.keys();
    for (const IndexedString& file : files) {
        ModificationRevision::clearModificationCache(file);
    }

    if (updatePolicy != DocumentChangeSet::NoUpdate && ICore::self()) {
        // The active document should be updated first, so that the user sees the results instantly
        if (IDocument* activeDoc = ICore::self()->documentController()->activeDocument()) {
            ICore::self()->languageController()->backgroundParser()->addDocument(IndexedString(activeDoc->url()));
        }

        // If there are currently open documents that now need an update, update them too
        const auto documents = ICore::self()->languageController()->backgroundParser()->managedDocuments();
        for (const IndexedString& doc : documents) {
            DUChainReadLocker lock(DUChain::lock());
            TopDUContext* top = DUChainUtils::standardContextForUrl(doc.toUrl(), true);
            if ((top && top->parsingEnvironmentFile() && top->parsingEnvironmentFile()->needsUpdate()) || !top) {
                lock.unlock();
                ICore::self()->languageController()->backgroundParser()->addDocument(doc);
            }
        }

        // Eventually update _all_ affected files
        const auto files = changes.keys();
        for (const IndexedString& file : files) {
            if (!file.toUrl().isValid()) {
                qCWarning(LANGUAGE) << "Trying to apply changes to an invalid document";
                continue;
            }

            ICore::self()->languageController()->backgroundParser()->addDocument(file);
        }
    }
}
}
