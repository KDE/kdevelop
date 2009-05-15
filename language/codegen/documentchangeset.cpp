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

namespace KDevelop {


KDevelop::DocumentChangeSet::ChangeResult DocumentChangeSet::addChange(const KDevelop::DocumentChange& change) {
    return addChange(DocumentChangePointer(new DocumentChange(change)));
}

DocumentChangeSet::ChangeResult DocumentChangeSet::addChange(const DocumentChangePointer& change) {
    if(change->m_range.start.line != change->m_range.end.line)
        return ChangeResult("Multi-line ranges are not supported");
    
    m_changes[change->m_document].append(change);
    return ChangeResult(true);
}

DocumentChangeSet::ChangeResult DocumentChangeSet::applyAllChanges(KDevelop::DocumentChangeSet::ReplacementPolicy policy, KDevelop::DocumentChangeSet::FormatPolicy format, KDevelop::DocumentChangeSet::DUChainUpdateHandling scheduleUpdate) {
    QMap<IndexedString, CodeRepresentation*> codeRepresentations;
    QMap<IndexedString, QString> newTexts;
    QMap<IndexedString, QList<DocumentChangePointer> > filteredSortedChanges;

    foreach(const IndexedString &file, m_changes.keys())
    {
        CodeRepresentation* repr = createCodeRepresentation(file);
        if(!repr)
            return ChangeResult(QString("Could not code for %1").arg(file.str()));
        
        ISourceFormatter* formatter = ICore::self()->sourceFormatterController()->formatterForUrl(file.toUrl());
        
        codeRepresentations[file] = repr;
        
        QString text = repr->text();
        
        QStringList textLines = text.split('\n');
        
        QMultiMap<SimpleCursor, DocumentChangePointer> sortedChanges;
        
        foreach(const DocumentChangePointer &change, m_changes[file])
            sortedChanges.insert(change->m_range.end, change);
        
        //Remove duplicates
        DocumentChangePointer previous = DocumentChangePointer();
        for(QMultiMap<SimpleCursor, DocumentChangePointer>::iterator it =sortedChanges.begin(); it != sortedChanges.end(); ) {
            if(previous){
                if(previous->m_range.end > (*it)->m_range.start) {
                    //intersection
                    if(previous->m_range == (*it)->m_range && previous->m_oldText == (*it)->m_oldText && previous->m_newText == (*it)->m_newText &&
                       !previous->m_ignoreOldText && !(*it)->m_ignoreOldText) {
                        //duplicate, remove one
                        it = sortedChanges.erase(it);
                        continue;
                    }else{
                        qDeleteAll(codeRepresentations);
                        return ChangeResult(QString("Inconsistent change-request at %1").arg(file.str()));
                    }
                }
            }
            previous = *it;
            ++it;
        }

        QList<DocumentChangePointer>& sortedChangesList(filteredSortedChanges[file]);
        sortedChangesList = sortedChanges.values();

        for(int pos = sortedChangesList.size()-1; pos >= 0; --pos) {
            DocumentChange& change(*sortedChangesList[pos]);
            QString encountered;
            if(change.m_range.start <= change.m_range.end &&
                change.m_range.end.line < textLines.size() &&
                change.m_range.start.line >= 0 &&
                change.m_range.start.column >= 0 &&
                change.m_range.start.column <= textLines[change.m_range.start.line].length() &&
                change.m_range.end.column >= 0 && 
                change.m_range.end.column <= textLines[change.m_range.end.line].length() && 
                change.m_range.start.line == change.m_range.end.line  && //We demand this, although it shoult be fixed
                ((encountered = textLines[change.m_range.start.line].mid(change.m_range.start.column, change.m_range.end.column-change.m_range.start.column)) == change.m_oldText || change.m_ignoreOldText))
            {
                ///Problem: This does not work if the other changes significantly alter the context @todo Use the changed context
                QString leftContext = QStringList(textLines.mid(0, change.m_range.start.line+1)).join("\n");
                leftContext.chop(textLines[change.m_range.start.line].length() - change.m_range.start.column);

                QString rightContext = QStringList(textLines.mid(change.m_range.end.line)).join("\n").mid(change.m_range.end.column);

                if(formatter && format == AutoFormatChanges) {
                    kDebug() << "formatting" << change.m_newText;
                    change.m_newText = formatter->formatSource(change.m_newText, KMimeType::findByUrl(file.toUrl()), leftContext, rightContext);
                    kDebug() << "result" << change.m_newText;
                }
                
                textLines[change.m_range.start.line].replace(change.m_range.start.column, change.m_range.end.column-change.m_range.start.column, change.m_newText);
            }else{
                QString warningString = QString("Inconsistent change in %1 at %2:%3 -> %4:%5 = \"%6\"(encountered \"%7\") -> \"%8\"").arg(file.str()).arg(change.m_range.start.line).arg(change.m_range.start.column).arg(change.m_range.end.line).arg(change.m_range.end.column).arg(change.m_oldText).arg(encountered).arg(change.m_newText);
                
                if(policy == IgnoreFailedChange) {
                    //Just don't do the replacement
                }else if(policy == WarnOnFailedChange) {
                    kWarning() << warningString;
                }else{
                    qDeleteAll(codeRepresentations);
                    
                    return DocumentChangeSet::ChangeResult(warningString, sortedChangesList[pos]);
                }
            }
        }
        
        newTexts[file] = textLines.join("\n");
    }
    
    QMap<IndexedString, QString> oldTexts;
    
#if 0
    if(format == AutoFormatChanges) {
        foreach(const IndexedString &file, m_changes.keys())
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
    
    foreach(const IndexedString &file, m_changes.keys())
    {
        oldTexts[file] = codeRepresentations[file]->text();
        bool fail = false;
        
        DynamicCodeRepresentation* dynamic = dynamic_cast<DynamicCodeRepresentation*>(codeRepresentations[file]);
        if(dynamic) {
            dynamic->startEdit();
            //Replay the changes one by one
            QList<DocumentChangePointer>& sortedChangesList(filteredSortedChanges[file]);
            
            for(int pos = sortedChangesList.size()-1; pos >= 0; --pos) {
                DocumentChange& change(*sortedChangesList[pos]);
                if(!dynamic->replace(change.m_range.textRange(), change.m_oldText, change.m_newText, change.m_ignoreOldText)) {
                    QString warningString = QString("Inconsistent change in %1 at %2:%3 -> %4:%5 = %6(encountered \"%7\") -> \"%8\"").arg(file.str()).arg(change.m_range.start.line).arg(change.m_range.start.column).arg(change.m_range.end.line).arg(change.m_range.end.column).arg(change.m_oldText).arg(dynamic->rangeText(change.m_range.textRange())).arg(change.m_newText);
                    
                    if(policy == IgnoreFailedChange) {
                        //Just don't do the replacement
                    }else if(policy == WarnOnFailedChange) {
                        kWarning() << warningString;
                    }else{
                        dynamic->endEdit();
                        qDeleteAll(codeRepresentations);
                        return DocumentChangeSet::ChangeResult(warningString);
                    }
                }
            }
            
            dynamic->endEdit();
        }else{
            fail = !codeRepresentations[file]->setText(newTexts[file]);
        }
        
        if(fail) {
            //Fail
            foreach(const IndexedString &revertFile, oldTexts.keys())
                codeRepresentations[revertFile]->setText(oldTexts[revertFile]);
            
            qDeleteAll(codeRepresentations);
            return DocumentChangeSet::ChangeResult(QString("Failed to set text on %1").arg(file.str()));
        }
    }
    
    qDeleteAll(codeRepresentations);
    ModificationRevisionSet::clearCache();
    
    foreach(const IndexedString &file, m_changes.keys())
    {
        if(scheduleUpdate != NoUpdate) {
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

    return DocumentChangeSet::ChangeResult(true);
}


}
