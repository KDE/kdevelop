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
#include <qstringlist.h>
#include <editor/modificationrevisionset.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/idocumentcontroller.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/parsingenvironment.h>
#include <interfaces/isourceformattercontroller.h>
#include <interfaces/isourceformatter.h>
#include <interfaces/iproject.h>
#include <KLocalizedString>
#include <algorithm>

namespace KDevelop {

struct DocumentChangeSetPrivate
{
    DocumentChangeSet::ReplacementPolicy replacePolicy;
    DocumentChangeSet::FormatPolicy formatPolicy;
    DocumentChangeSet::DUChainUpdateHandling updatePolicy;
    
    typedef QPair<IndexedString, InsertArtificialCodeRepresentationPointer> TempPair;
    QMap< IndexedString, TempPair > tempFiles;
    QMap< IndexedString, QList<DocumentChangePointer> > changes;
    QMap< IndexedString, IndexedString > tempToOriginal;
    
    DocumentChangeSet::ChangeResult addChange(DocumentChangePointer change);
    DocumentChangeSet::ChangeResult replaceOldText(CodeRepresentation * repr, const QString & newText, const QList<DocumentChangePointer> & sortedChangesList);
    DocumentChangeSet::ChangeResult generateNewText(const KDevelop::IndexedString & file, QList< KDevelop::DocumentChangePointer > & sortedChanges, const KDevelop::CodeRepresentation* repr, QString& output);
    DocumentChangeSet::ChangeResult removeDuplicates(const IndexedString & file, QList<DocumentChangePointer> & filteredChanges);
    void formatChanges();
    void updateFiles();
    void addFileToProject(IndexedString file);
    void addTempFile(IndexedString originalName, const QString & text);
    void adjustChangeToTemp(DocumentChangePointer newChange);
    QList<IndexedString> withoutOriginals();
    DocumentChangeSet::ChangeResult replaceOriginalsWithTemps();
};

//Simple helper to clear up code clutter
namespace
{
inline bool changeIsValid(const DocumentChange & change, const QStringList & textLines)
{
    return change.m_range.start <= change.m_range.end &&
           change.m_range.end.line < textLines.size() &&
           change.m_range.start.line >= 0 &&
           change.m_range.start.column >= 0 &&
           change.m_range.start.column <= textLines[change.m_range.start.line].length() &&
           change.m_range.end.column >= 0 && 
           change.m_range.end.column <= textLines[change.m_range.end.line].length() && 
           change.m_range.start.line == change.m_range.end.line;
}

inline bool duplicateChanges(DocumentChangePointer previous, DocumentChangePointer current)
{
    //Given the option of considering a duplicate two changes in the same range but with different old texts to be ignored
    return previous->m_range == current->m_range &&
           previous->m_newText == current->m_newText &&
           (previous->m_oldText == current->m_oldText ||
           (previous->m_ignoreOldText && current->m_ignoreOldText));
}
}

DocumentChangeSet::DocumentChangeSet() : d(new DocumentChangeSetPrivate)
{
    d->replacePolicy = StopOnFailedChange;
    d->formatPolicy = AutoFormatChanges;
    d->updatePolicy = SimpleUpdate;
}

DocumentChangeSet::DocumentChangeSet(const DocumentChangeSet & rhs) : d(new DocumentChangeSetPrivate(*rhs.d))
{
}


DocumentChangeSet& DocumentChangeSet::operator=(const KDevelop::DocumentChangeSet& rhs)
{
    *d = *rhs.d;
    return *this;
}

DocumentChangeSet::~DocumentChangeSet()
{
    delete d;
}

KDevelop::DocumentChangeSet::ChangeResult DocumentChangeSet::addChange(const KDevelop::DocumentChange& change) {
    return d->addChange(DocumentChangePointer(new DocumentChange(change)));
}

DocumentChangeSet::ChangeResult DocumentChangeSetPrivate::addChange(DocumentChangePointer change) {
    if(change->m_range.start.line != change->m_range.end.line)
        return DocumentChangeSet::ChangeResult("Multi-line ranges are not supported");
    
    if(tempFiles.contains(change->m_document))
        adjustChangeToTemp(change);
    
    changes[change->m_document].append(change);
    return true;
}

/*
 * Merges the changes from both DocumentChangeSets.
 * Merging changes from two changesets that have each applied temp changes is not supported
 * When temp files have been created in one change set, but not the other, all the changes from the
 * original file need to be adjusted, so they correspond to the temp version.
 */
DocumentChangeSet & DocumentChangeSet::operator<<(DocumentChangeSet & rhs)
{
#ifndef NDEBUG
    kDebug() << "Merging DocumentChangeSets, tempFiles.\nTemp Files in lhs:";
    for(QMap <KDevelop::IndexedString, KDevelop::DocumentChangeSetPrivate::TempPair >::iterator it = d->tempFiles.begin();
        it != d->tempFiles.end(); ++it)
        kDebug() << it.key().str() << " " << it->first.str();
    kDebug() << "Temp Files in rhs: ";
    for(QMap <KDevelop::IndexedString, KDevelop::DocumentChangeSetPrivate::TempPair >::iterator it = rhs.d->tempFiles.begin();
        it != rhs.d->tempFiles.end(); ++it)
        kDebug() << it.key().str() << " " << it->first.str();
#endif //NDEBUG
    
    //If a temp has been created on the rhs changeset, then the changes in this one must be adjusted
    //This is not an efficient method of adjusting them, however it is simpler to manage
    foreach(const IndexedString & file, rhs.d->tempFiles.keys())
        if(d->changes.contains(file))
        {
            foreach(const DocumentChangePointer & change, d->changes[file])
                rhs.d->addChange(change);
            d->changes.remove(file);
        }
    
    /// @todo Possibly check for duplicates, since it could create a lot of bloat when big changes are merged
    foreach(const QList<DocumentChangePointer> & changeList, rhs.d->changes.values())
        foreach(const DocumentChangePointer & change, changeList)
            d->addChange(change);

    /// @todo Fix for a possibility of two different temporaries created for the same fileName
    d->tempFiles.unite(rhs.d->tempFiles);
    Q_ASSERT(d->tempFiles.uniqueKeys().size() == d->tempFiles.size());
    d->tempToOriginal.unite(rhs.d->tempToOriginal);
    
    rhs.clear();
    
#ifndef NDEBUG
    kDebug() << "Merged Changes: ";
    for(QMap <KDevelop::IndexedString, QList <KDevelop::DocumentChangePointer > >::iterator it = d->changes.begin();
        it != d->changes.end(); ++it)
    {
        kDebug() << it.key().str();
        foreach(DocumentChangePointer p, *it)
            kDebug() << p->m_newText;
    }
#endif  //NDEBUG
        
    
    return *this;
}

void DocumentChangeSet::clear ()
{
    d->tempFiles.clear();
    d->changes.clear();
    d->tempToOriginal.clear();
}

void DocumentChangeSet::setReplacementPolicy ( DocumentChangeSet::ReplacementPolicy policy )
{
    d->replacePolicy = policy;
}

void DocumentChangeSet::setFormatPolicy ( DocumentChangeSet::FormatPolicy policy )
{
    d->formatPolicy = policy;
}

void DocumentChangeSet::setUpdateHandling ( DocumentChangeSet::DUChainUpdateHandling policy )
{
    d->updatePolicy = policy;
}


IndexedString DocumentChangeSet::tempNameForFile ( IndexedString file ) const
{
    if(d->tempFiles.contains(file))
        file = d->tempFiles[file].first;
    
    return file;
}

QList<QPair<IndexedString, IndexedString> > DocumentChangeSet::tempNamesForAll() const
{
    QList<QPair<IndexedString, IndexedString> > names;
    for(QMap< IndexedString, DocumentChangeSetPrivate::TempPair >::Iterator it = d->tempFiles.begin();
        it != d->tempFiles.end(); ++it)
        names << qMakePair(it.key(), it.value().first);
    
    return names;
}

DocumentChangeSet::ChangeResult DocumentChangeSet::applyToTemp(IndexedString fileName)
{
    //If there is a temporary version already, then use that one
    IndexedString tempFile = tempNameForFile(fileName);
    
    if(!d->changes.contains(tempFile))
        return ChangeResult(i18n("Trying to apply changes to a temporary that is not in this change set: %1", fileName.str()));
    
    CodeRepresentation::Ptr repr = createCodeRepresentation(tempFile);
    ChangeResult result(true);
    
    QList<DocumentChangePointer> sortedChanges;
    QString newText;
    {
        result = d->removeDuplicates(tempFile, sortedChanges);
        if(!result)
            return result;
        
        result = d->generateNewText(tempFile, sortedChanges, repr.data(), newText);
        if(!result)
            return result;
    }
    
    //If a previous temp did not exist, create one
    if(!d->tempToOriginal.contains(tempFile))
        d->addTempFile(fileName, newText);
    else
    {
        result = d->replaceOldText(repr.data(), newText, sortedChanges);
        //On success the changes' documents should be changed to the original to prevent double application
        if(result)
        {
            IndexedString original = d->tempToOriginal[tempFile];
            d->changes.erase(d->changes.find(tempFile));
            foreach(DocumentChangePointer p, sortedChanges)
            {
                p->m_document = original;
                d->changes[original] << p;
            }
        }
    }
    
    return result;
}


DocumentChangeSet::ChangeResult DocumentChangeSet::applyAllToTemp()
{
    QMap<IndexedString, CodeRepresentation::Ptr> codeRepresentations;
    QMap<IndexedString, QString> newTexts;
    QMap<IndexedString, QList<DocumentChangePointer> > filteredSortedChanges;
    
    QList<KDevelop::IndexedString > files(d->withoutOriginals());
    
    foreach(const IndexedString &file, files)
    {
        kDebug() << "Processing changes- for file: " << file.str();
        CodeRepresentation::Ptr repr = createCodeRepresentation(file);
        if(!repr)
            return ChangeResult(QString("Could not create a Representation for %1").arg(file.str()));
        
        codeRepresentations[file] = repr;
        
        QList<DocumentChangePointer>& sortedChangesList(filteredSortedChanges[file]);
        {
            ChangeResult result(d->removeDuplicates(file, sortedChangesList));
            if(!result)
                return result;
        }

        {
            ChangeResult result(d->generateNewText(file, sortedChangesList, repr.data(), newTexts[file]));
            if(!result)
                return result;
        }
    }
    
    QMap<IndexedString, QString> oldTexts;
    
    //! @todo apply correct formatting, and call it
    //d->autoformatChanges()
    
    //Apply the changes to the files
    foreach(const IndexedString & file, files)
    {
        if(!d->tempToOriginal.contains(file))
        {
            kDebug() << "Adding a new Temp file from original with text: " << newTexts[file];
            d->addTempFile(file, newTexts[file]);
        }
        else
        {
            oldTexts[file] = codeRepresentations[file]->text();
            kDebug() << "Applying to already created temp: " << file.str() << ". Old text: " << oldTexts[file];
            
            DocumentChangeSet::ChangeResult result = d->replaceOldText(codeRepresentations[file].data(), newTexts[file], filteredSortedChanges[file]);
            if(!result && d->replacePolicy == StopOnFailedChange)
            {
                //Revert all files 
                foreach(const IndexedString &revertFile, oldTexts.keys())
                    codeRepresentations[revertFile]->setText(oldTexts[revertFile]);
                
                return result;
            }
            
            //On success the changes' documents should be changed to the original to prevent double application
            d->changes.erase(d->changes.find(file));
            foreach(DocumentChangePointer p, filteredSortedChanges[file])
            {
                p->m_document = d->tempToOriginal[file];
                d->changes[p->m_document] << p;
            }
            
            kDebug() << "Applied new text: " << newTexts[file];
        }
    }
    
    ModificationRevisionSet::clearCache();

    d->updateFiles();
    
    return DocumentChangeSet::ChangeResult(true);
}

DocumentChangeSet::ChangeResult DocumentChangeSet::applyAllChanges() {
    QMap<IndexedString, CodeRepresentation::Ptr> codeRepresentations;
    QMap<IndexedString, QString> newTexts;
    QMap<IndexedString, QList<DocumentChangePointer> > filteredSortedChanges;
    ChangeResult result(true);
    
    QList<KDevelop::IndexedString > files(d->withoutOriginals());

    foreach(const IndexedString &file, files)
    {
        CodeRepresentation::Ptr repr = createCodeRepresentation(file);
        if(!repr)
            return ChangeResult(QString("Could not create a Representation for %1").arg(file.str()));
        
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
    
    //! @todo apply correct formatting, and call it
    //d->autoformatChanges()
    
    //Apply the changes to the files
    foreach(const IndexedString &file, files)
    {
        oldTexts[file] = codeRepresentations[file]->text();
        
        result = d->replaceOldText(codeRepresentations[file].data(), newTexts[file], filteredSortedChanges[file]);
        if(!result && d->replacePolicy == StopOnFailedChange)
        {
            //Revert all files 
            foreach(const IndexedString &revertFile, oldTexts.keys())
                codeRepresentations[revertFile]->setText(oldTexts[revertFile]);
            
            return result;
        }
    }
        
    result = d->replaceOriginalsWithTemps();
    
    ModificationRevisionSet::clearCache();

    d->updateFiles();
    
    return result;;
}

DocumentChangeSet::ChangeResult DocumentChangeSetPrivate::replaceOldText(CodeRepresentation * repr,
                                                                         const QString & newText,
                                                                         const QList<DocumentChangePointer> & sortedChangesList)
{
    DynamicCodeRepresentation* dynamic = dynamic_cast<DynamicCodeRepresentation*>(repr);
    if(dynamic) {
        dynamic->startEdit();
        //Replay the changes one by one
        
        for(int pos = sortedChangesList.size()-1; pos >= 0; --pos)
        {
            const DocumentChange& change(*sortedChangesList[pos]);
            if(!dynamic->replace(change.m_range.textRange(), change.m_oldText, change.m_newText, change.m_ignoreOldText))
            {
                QString warningString = QString("Inconsistent change in %1 at %2:%3 -> %4:%5 = %6(encountered \"%7\") -> \"%8\"")
                                                .arg(change.m_document.str()).arg(change.m_range.start.line).arg(change.m_range.start.column)
                                                .arg(change.m_range.end.line).arg(change.m_range.end.column).arg(change.m_oldText)
                                                .arg(dynamic->rangeText(change.m_range.textRange())).arg(change.m_newText);

                if(replacePolicy == DocumentChangeSet::WarnOnFailedChange)
                {
                    kWarning() << warningString;
                }
                else if(replacePolicy == DocumentChangeSet::StopOnFailedChange)
                {
                    dynamic->endEdit();
                    return DocumentChangeSet::ChangeResult(warningString);
                }
                //If set to ignore failed changes just continue with the others
            }
        }
        
        dynamic->endEdit();
        return true;
    }
    
    //For files on disk
    if (!repr->setText(newText))
    {
        QString warningString = QString("Could not replace text for file in disk, or artificial code: %1").arg(sortedChangesList.begin()->data()->m_document.str());
        if(replacePolicy == DocumentChangeSet::WarnOnFailedChange)
        {
            kWarning() << warningString;
        }
        
        return DocumentChangeSet::ChangeResult(warningString);
    }
    
    return true;
}

DocumentChangeSet::ChangeResult DocumentChangeSetPrivate::generateNewText(const IndexedString & file,
                                                                          QList<DocumentChangePointer> & sortedChanges,
                                                                          const CodeRepresentation * repr,
                                                                          QString & output)
{

    ISourceFormatter* formatter = 0;
    if(ICore::self())
        formatter = ICore::self()->sourceFormatterController()->formatterForUrl(file.toUrl());

    //Create the actual new modified file
    QStringList textLines = repr->text().split('\n');

    for(int pos = sortedChanges.size()-1; pos >= 0; --pos) {
        DocumentChange& change(*sortedChanges[pos]);
        QString encountered;
        if(changeIsValid(change, textLines)  && //We demand this, although it should be fixed
            ((encountered = textLines[change.m_range.start.line].mid(change.m_range.start.column, change.m_range.end.column-change.m_range.start.column)) == change.m_oldText || change.m_ignoreOldText))
        {
            ///Problem: This does not work if the other changes significantly alter the context @todo Use the changed context
            QString leftContext = QStringList(textLines.mid(0, change.m_range.start.line+1)).join("\n");
            leftContext.chop(textLines[change.m_range.start.line].length() - change.m_range.start.column);

            QString rightContext = QStringList(textLines.mid(change.m_range.end.line)).join("\n").mid(change.m_range.end.column);

            if(formatter && formatPolicy == DocumentChangeSet::AutoFormatChanges)
                change.m_newText = formatter->formatSource(change.m_newText, KMimeType::findByUrl(file.toUrl()), leftContext, rightContext);
            
            textLines[change.m_range.start.line].replace(change.m_range.start.column, change.m_range.end.column-change.m_range.start.column, change.m_newText);
        }else{
            QString warningString = QString("Inconsistent change in %1 at %2:%3 -> %4:%5 = \"%6\"(encountered \"%7\") -> \"%8\"")
                                            .arg(file.str()).arg(change.m_range.start.line).arg(change.m_range.start.column)
                                            .arg(change.m_range.end.line).arg(change.m_range.end.column).arg(change.m_oldText)
                                            .arg(encountered).arg(change.m_newText);
            
            if(replacePolicy == DocumentChangeSet::IgnoreFailedChange) {
                //Just don't do the replacement
            }else if(replacePolicy == DocumentChangeSet::WarnOnFailedChange)
                kWarning() << warningString;
            else
                return DocumentChangeSet::ChangeResult(warningString, sortedChanges[pos]);
                
        }
    }

    output = textLines.join("\n");
    return true;
}

//Removes all duplicate changes for a single file, and then returns (via filteredChanges) the filtered duplicates
DocumentChangeSet::ChangeResult DocumentChangeSetPrivate::removeDuplicates(const IndexedString & file,
                                                                           QList<DocumentChangePointer> & filteredChanges)
{
    QMultiMap<SimpleCursor, DocumentChangePointer> sortedChanges;
    
    foreach(const DocumentChangePointer &change, changes[file])
        sortedChanges.insert(change->m_range.end, change);
    
    //Remove duplicates
    QMultiMap<SimpleCursor, DocumentChangePointer>::iterator previous = sortedChanges.begin();
    for(QMultiMap<SimpleCursor, DocumentChangePointer>::iterator it = ++sortedChanges.begin(); it != sortedChanges.end(); ) {
        if(( *previous ) && ( *previous )->m_range.end > (*it)->m_range.start) {
            //intersection
            if(duplicateChanges(( *previous ), *it)) {
                //duplicate, remove one
                it = sortedChanges.erase(it);
                continue;
            }
            
            //When two changes contain each other, and the container change is set to ignore old text, then it should be safe to
            //just ignore the contained change, and apply the bigger change
            else if((*it)->m_range.contains(( *previous )->m_range) && (*it)->m_ignoreOldText  )
            {
                kDebug() << "Removing change: " << ( *previous )->m_oldText << "->" << ( *previous )->m_newText
                         << ", because it is contained by change: " << (*it)->m_oldText << "->" << (*it)->m_newText;
                sortedChanges.erase(previous);
            }
            //This case is for when both have the same end, either of them could be the containing range
            else if((*previous)->m_range.contains((*it)->m_range) && (*previous)->m_ignoreOldText  )
            {
                kDebug() << "Removing change: " << (*it)->m_oldText << "->" << (*it)->m_newText
                         << ", because it is contained by change: " << ( *previous )->m_oldText << "->" << ( *previous )->m_newText;
                it = sortedChanges.erase(it);
                continue;
            }
            else
                return DocumentChangeSet::ChangeResult(
                       QString("Inconsistent change-request at %1; intersecting changes: \"%2\"->\"%3\"@%4:%5->%6:%7 & \"%8\"->\"%9\"@%10:%11->%12:%13 ")
                       .arg(file.str(), ( *previous )->m_oldText, ( *previous )->m_newText).arg(( *previous )->m_range.start.line).arg(( *previous )->m_range.start.column)
                       .arg(( *previous )->m_range.end.line).arg(( *previous )->m_range.end.column).arg((*it)->m_oldText, (*it)->m_newText).arg((*it)->m_range.start.line)
                       .arg((*it)->m_range.start.column).arg((*it)->m_range.end.line).arg((*it)->m_range.end.column));
            
        }
        previous = it;
        ++it;
    }
    
    filteredChanges = sortedChanges.values();
    return true;
}

void DocumentChangeSetPrivate::updateFiles()
{
    if(updatePolicy != DocumentChangeSet::NoUpdate && ICore::self())
        foreach(const IndexedString &file, changes.keys())
        {
                ICore::self()->languageController()->backgroundParser()->addDocument(file.toUrl());
                foreach(KUrl doc, ICore::self()->languageController()->backgroundParser()->managedDocuments()) {
                    DUChainReadLocker lock(DUChain::lock());
                    TopDUContext* top = DUChainUtils::standardContextForUrl(doc);
                    if((top && top->parsingEnvironmentFile() && top->parsingEnvironmentFile()->needsUpdate()) || !top) {
                        lock.unlock();
                        ICore::self()->languageController()->backgroundParser()->addDocument(doc);
                    }
                }
        }
}

void DocumentChangeSetPrivate::formatChanges()
{
#if 0
    if(formatChanges() == DocumentChangeSet::AutoFormatChanges) {
        foreach(const IndexedString &file, d->changes.keys())
        {
            ISourceFormatter* formatter = ICore::self()->sourceFormatterController()->formatterForUrl(file.toUrl());
            if(!formatter)
                continue;
            
            filteredSortedChanges[file].clear(); //We create a new list of changes
            
            QStringList fileNewText = newTexts[file].split("\n");
            
            QList< SimpleRange > changes = changedRanges[file].keys();
            for(QList< SimpleRange >::const_iterator it = changes.end(); it != changes.begin(); ) {
                --it;
                
                QString leftContext = QStringList(fileNewText.mid(0, (*it).start.line+1)).join("\n");
                leftContext.chop((*it).start.column);

                QString text = fileNewText[(*it).start.line].mid((*it).start.column);
                
                if((*it).start.line == (*it).end.line)
                    text = text.left((*it).end.column - (*it).start.column);
                
                QString rightContext = QStringList(fileNewText.mid((*it).end.line)).join("\n").mid((*it).end.column);
                
                
                /*
                --it;
                int end = *it+1;
                int start = *it;
                
                while(it != changes.begin() && *(it-1) == start-1) {
                    --start;
                    --it;
                }
                
                QString leftContext = QStringList(fileNewText.mid(0, start)).join("\n");
                QString sourceToFormat = QStringList(fileNewText.mid(start, end-start)).join("\n");
                QString rightContext = QStringList(fileNewText.mid(end)).join("\n");
                QString formattedSource = formatter->formatSource(sourceToFormat, KMimeType::findByUrl(file.toUrl()), leftContext + "\n", "\n" + rightContext);
                
                fileNewText.erase(fileNewText.begin()+start, fileNewText.begin()+end);
                
                int insertAt = start;
                foreach(QString insertString, formattedSource.split("\n")) {
                    fileNewText.insert(insertAt, insertString);
                    ++insertAt;
                }
                
                //Create new changes that replace the whole lines at the same time
                filteredSortedChanges[file].prepend(DocumentChangePointer(new DocumentChange(file, SimpleRange(start, 0, end, 0), sourceToFormat+"\n", formattedSource+"\n")));
            */}
            
            newTexts[file] = fileNewText.join("\n");
        }
    }
#endif
}

void DocumentChangeSetPrivate::addFileToProject(IndexedString file)
{
    Q_UNUSED(file);
    #if 0
   //Pick the folder Item that should contain the new class
  IProject * p;
  QList<ProjectFolderItem*> folderList = p->foldersForUrl(newClassWizard.implementationUrl().upUrl());
  if(folderList.isEmpty())
    return;
  ProjectFolderItem* folder = folderList.first();
    
  //Add new files into the project
  if(!item)
    item=folder;
  ProjectFileItem* projectFile=p->buildSystemManager()->addFile(newClassWizard.implementationUrl(), folder);
  
  //Add them as targets
  if(item->target()) {
    p->buildSystemManager()->addFileToTarget(file, item->target());
    p->buildSystemManager()->addFileToTarget(header, item->target());
  } else if(item->project()->buildSystemManager() &&
            item->project()->buildSystemManager()->features() & IBuildSystemManager::Targets) {
    QList<KDevelop::ProjectTargetItem*> t=folder->targetList();
  
  for(QStandardItem* it=folder; it && t.isEmpty(); it=it->parent()) {
    KDevelop::ProjectBaseItem* bit=static_cast<KDevelop::ProjectBaseItem*>(it);
    t=bit->targetList();
  }
      
  if(t.count()==1) //Just choose this one
    p->buildSystemManager()->addFileToTarget(file, t.first());
  else {
    KDialog d;
    QWidget *w=new QWidget(&d);
    w->setLayout(new QVBoxLayout);
    w->layout()->addWidget(new QLabel("Choose one target to add the file or cancel if you do not want to do so."));
    QListWidget* targetsWidget=new QListWidget(w);
    targetsWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    foreach(ProjectTargetItem* it, t) {
      targetsWidget->addItem(it->text());
    }
    w->layout()->addWidget(targetsWidget);
        
    targetsWidget->setCurrentRow(0);
    d.setButtons( KDialog::Ok | KDialog::Cancel);
    d.enableButtonOk(true);
    d.setMainWidget(w);
      
    if(d.exec()==QDialog::Accepted) {
      if(targetsWidget->selectedItems().isEmpty())
        QMessageBox::warning(0, QString(), i18n("Did not select anything, not adding to a target."));
      else 
        p->buildSystemManager()->addFileToTarget(file, t[targetsWidget->currentRow()]);
    }
  }
}
#endif
}

void DocumentChangeSetPrivate::addTempFile(IndexedString originalName, const QString & text)
{
    QString name = originalName.str();
    KUrl url = CodeRepresentation::artificialUrl(name);
    
    unsigned int counter = 0;
    //Search for a unique new url for this file
    while(artificialCodeRepresentationExists(IndexedString(url)))
    {
        //Increment a number reference before the period
        name.insert(name.lastIndexOf("."), QString::number(counter++));
        url = CodeRepresentation::artificialUrl(name);
    }
    
    IndexedString tempFile(url);
    
    tempFiles[originalName] = qMakePair(tempFile, InsertArtificialCodeRepresentationPointer(new InsertArtificialCodeRepresentation(tempFile, text)));
    tempToOriginal[tempFile] = originalName;
    changes.erase(changes.find(tempFile));
}

void DocumentChangeSetPrivate::adjustChangeToTemp(DocumentChangePointer newChange)
{
    
    kDebug() << "Adjusting change to temp for change: " << newChange->m_newText << "With range: " << newChange->m_range.textRange();
    //Adjust the range of the new change according to temporaries already applied
    
    //Sort the changes to the original file
    QList<DocumentChangePointer> sortedChanges;
    removeDuplicates(newChange->m_document, sortedChanges );
    std::reverse(sortedChanges.begin(), sortedChanges.end());
    
    //foreach changed
    foreach(DocumentChangePointer originalChange, sortedChanges)
    {
        if(originalChange->m_range.textRange() < newChange->m_range.textRange() )
        {
            if(originalChange->m_range.end.line == newChange->m_range.start.line)
            {
                //Apply column offset
                int offset;
                if(originalChange->m_newText.count('\n'))
                    offset = originalChange->m_range.start.column - (originalChange->m_newText.lastIndexOf('\n') - originalChange->m_newText.size() + 1);
                else
                    offset = originalChange->m_newText.size() - (originalChange->m_range.end.column - originalChange->m_range.start.column);
                newChange->m_range.start.column += offset;
                newChange->m_range.end.column += offset;
            }
             
            //Get the line difference that the change poduced, and apply it to the new one
            int lineDifference = originalChange->m_newText.count('\n') - originalChange->m_range.textRange().numberOfLines();
            if(lineDifference)
            {
                newChange->m_range.start.line += lineDifference;
                newChange->m_range.end.line += lineDifference;
            }
        }
    }
    newChange->m_document = tempFiles[newChange->m_document].first;
    
    kDebug() << "New range: " << newChange->m_range.textRange();
}

// Return a list of the files that are logically unapplied changes
QList<IndexedString> DocumentChangeSetPrivate::withoutOriginals()
{
    QList<KDevelop::IndexedString > files(changes.keys());
    QList<KDevelop::IndexedString >::iterator it = files.begin();
    while(it != files.end())
    {
        if(tempFiles.contains(*it))
            it = files.erase(it);
        else
            ++it;
    }
    
    return files;
}

//Replace original files with temporaries
DocumentChangeSet::ChangeResult DocumentChangeSetPrivate::replaceOriginalsWithTemps()
{
    //These two are helpers for backtracking changes in case an error replacing text is encountered
    QMap<IndexedString, QString> oldText;
    QMap<IndexedString, CodeRepresentation::Ptr> originalRepresentations;
    
    QMap< IndexedString, TempPair >::iterator it = tempFiles.begin();
    
    while(it != tempFiles.end())
    {
        CodeRepresentation::Ptr original = originalRepresentations[it.key()] = createCodeRepresentation(it.key());
        oldText[it.key()] = original->text();
        CodeRepresentation::Ptr temp = createCodeRepresentation(it->first);
        
        if(!original->setText(temp->text()))
        {
            foreach(const IndexedString & file, originalRepresentations.keys())
                originalRepresentations[file]->setText(oldText[file]);
            
            return DocumentChangeSet::ChangeResult(QString("Error trying to write changes to file: %1").arg(it.key().str()));
        }
        ++it;
    }
    
    return true;
}

}
