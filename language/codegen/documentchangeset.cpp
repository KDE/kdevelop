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

namespace KDevelop {

struct DocumentChangeSetPrivate
{
    DocumentChangeSet::ReplacementPolicy replacePolicy;
    DocumentChangeSet::FormatPolicy formatPolicy;
    DocumentChangeSet::DUChainUpdateHandling updatePolicy;
    
    QMap< IndexedString, QList<DocumentChangePointer> > changes;
    QMap< IndexedString, IndexedString > tempFiles;
    
    DocumentChangeSet::ChangeResult replaceOldText(CodeRepresentation * repr, const QString & newText, const QList<DocumentChangePointer> & sortedChangesList);
    DocumentChangeSet::ChangeResult generateNewText(const KDevelop::IndexedString & file, QList< KDevelop::DocumentChangePointer > & sortedChanges, const KDevelop::CodeRepresentation* repr, QString& output);
    DocumentChangeSet::ChangeResult removeDuplicates(const IndexedString & file, QList<DocumentChangePointer> & filteredChanges);
    void formatChanges();
    void updateFiles();
    void addFileToProject(IndexedString file);
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
    return addChange(DocumentChangePointer(new DocumentChange(change)));
}

DocumentChangeSet::ChangeResult DocumentChangeSet::addChange(const DocumentChangePointer& change) {
    if(change->m_range.start.line != change->m_range.end.line)
        return ChangeResult("Multi-line ranges are not supported");
    
    if(d->tempFiles.contains(change->m_document))
        //Because the change is semantically constant, but not bitwise, correct the old file name for the temp one
        const_cast<DocumentChangePointer &>(change)->m_document = d->tempFiles[change->m_document];
    
    d->changes[change->m_document].append(change);
    return ChangeResult(true);
}

DocumentChangeSet & DocumentChangeSet::operator<<(DocumentChangeSet & rhs)
{
    for(QMap< IndexedString, QList<DocumentChangePointer> >::iterator it = rhs.d->changes.begin();
        it != rhs.d->changes.end(); ++it)
        d->changes[it.key()] += *it;
    rhs.clear();
    
    return *this;
}

void DocumentChangeSet::clear ( void )
{
    d->changes.clear();
    d->tempFiles.clear();
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


IndexedString DocumentChangeSet::tempNameForFile ( IndexedString file )
{
    if(d->tempFiles.contains(file))
        file = d->tempFiles[file];
    
    return file;
}


DocumentChangeSet::ChangeResult DocumentChangeSet::applyAllChanges() {
    QMap<IndexedString, CodeRepresentation*> codeRepresentations;
    QMap<IndexedString, QString> newTexts;
    QMap<IndexedString, QList<DocumentChangePointer> > filteredSortedChanges;

    foreach(const IndexedString &file, d->changes.keys())
    {
        CodeRepresentation* repr = createCodeRepresentation(file);
        if(!repr)
            return ChangeResult(QString("Could not code for %1").arg(file.str()));
        
        codeRepresentations[file] = repr;
        
        
        QList<DocumentChangePointer>& sortedChangesList(filteredSortedChanges[file]);
        {
            ChangeResult result(d->removeDuplicates(file, sortedChangesList));
            if(!result)
            {
                qDeleteAll(codeRepresentations);
                return result;
            }
        }

        {
            ChangeResult result(d->generateNewText(file, sortedChangesList, repr, newTexts[file]));
            if(!result)
            {
                qDeleteAll(codeRepresentations);
                return result;
            }
        }
    }
    
    QMap<IndexedString, QString> oldTexts;
    
    //! @todo apply correct formatting, and call it
    //d->autoformatChanges()
    
    //Apply the changes to the files
    foreach(const IndexedString &file, d->changes.keys())
    {
        oldTexts[file] = codeRepresentations[file]->text();
        
        DocumentChangeSet::ChangeResult result = d->replaceOldText(codeRepresentations[file], newTexts[file], filteredSortedChanges[file]);
        if(!result && d->replacePolicy == StopOnFailedChange)
        {
            //Revert all files 
            foreach(const IndexedString &revertFile, oldTexts.keys())
                codeRepresentations[revertFile]->setText(oldTexts[revertFile]);
            
            qDeleteAll(codeRepresentations);
            return result;
        }
    }
    
    qDeleteAll(codeRepresentations);
    ModificationRevisionSet::clearCache();

    d->updateFiles();
    
    return DocumentChangeSet::ChangeResult(true);
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
        QString warningString = QString("Could not replace text for file in disk: %1").arg(sortedChangesList.begin()->data()->m_document.str());
        if(replacePolicy == DocumentChangeSet::WarnOnFailedChange)
            kWarning() << warningString;
        
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
        if(changeIsValid(change, textLines)  && //We demand this, although it shoult be fixed
            ((encountered = textLines[change.m_range.start.line].mid(change.m_range.start.column, change.m_range.end.column-change.m_range.start.column)) == change.m_oldText || change.m_ignoreOldText))
        {
            ///Problem: This does not work if the other changes significantly alter the context @todo Use the changed context
            QString leftContext = QStringList(textLines.mid(0, change.m_range.start.line+1)).join("\n");
            leftContext.chop(textLines[change.m_range.start.line].length() - change.m_range.start.column);

            QString rightContext = QStringList(textLines.mid(change.m_range.end.line)).join("\n").mid(change.m_range.end.column);

            if(formatter && formatPolicy == DocumentChangeSet::AutoFormatChanges) {
                kDebug() << "formatting" << change.m_newText;
                change.m_newText = formatter->formatSource(change.m_newText, KMimeType::findByUrl(file.toUrl()), leftContext, rightContext);
                kDebug() << "result" << change.m_newText;
            }
            
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
    DocumentChangePointer previous = DocumentChangePointer();
    for(QMultiMap<SimpleCursor, DocumentChangePointer>::iterator it =sortedChanges.begin(); it != sortedChanges.end(); ) {
        if(previous && previous->m_range.end > (*it)->m_range.start) {
            //intersection
            if(previous->m_range == (*it)->m_range && previous->m_oldText == (*it)->m_oldText && previous->m_newText == (*it)->m_newText &&
                !previous->m_ignoreOldText && !(*it)->m_ignoreOldText) {
                //duplicate, remove one
                it = sortedChanges.erase(it);
                continue;
            }
            else
                return DocumentChangeSet::ChangeResult(QString("Inconsistent change-request at %1").arg(file.str()));
            
        }
        previous = *it;
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


}
