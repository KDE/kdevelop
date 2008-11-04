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

namespace KDevelop {



DocumentChangeSet::ChangeResult DocumentChangeSet::addChange(const DocumentChangePointer& change) {
    if(change->m_range.start.line != change->m_range.end.line)
        return ChangeResult("Multi-line ranges are not supported");
    
    m_changes[change->m_document].append(change);
    return ChangeResult(true);
}

DocumentChangeSet::ChangeResult DocumentChangeSet::applyAllChanges() {
    QMap<IndexedString, CodeRepresentation*> codeRepresentations;
    QMap<IndexedString, QString> newTexts;
    
    foreach(IndexedString file, m_changes.keys())
    {
        CodeRepresentation* repr = createCodeRepresentation(file);
        if(!repr)
            return ChangeResult(QString("Could not code for %1").arg(file.str()));
        
        codeRepresentations[file] = repr;
        
        QString text = repr->text();
        
        QStringList textLines = text.split('\n');
        
        QMultiMap<SimpleCursor, DocumentChangePointer> sortedChanges;
        
        foreach(DocumentChangePointer change, m_changes[file])
            sortedChanges.insert(change->m_range.end, change);
        
        //Remove duplicates
        DocumentChangePointer previous = DocumentChangePointer();
        for(QMultiMap<SimpleCursor, DocumentChangePointer>::iterator it =sortedChanges.begin(); it != sortedChanges.end(); ) {
            if(previous){
                if(previous->m_range.end > (*it)->m_range.start) {
                    //intersection
                    if(previous->m_range == (*it)->m_range && previous->m_oldText == (*it)->m_oldText && previous->m_newText == (*it)->m_newText) {
                        //duplicate, remove one
                        it = sortedChanges.erase(it);
                        continue;
                    }else{
                        qDeleteAll(codeRepresentations.values());
                        return ChangeResult(QString("Inconsistent change-request at %1").arg(file.str()));
                    }
                }
            }
            ++it;
        }
        
        
        QList<DocumentChangePointer> sortedChangesList = sortedChanges.values();
        for(int pos = sortedChangesList.size()-1; pos >= 0; --pos) {
            DocumentChange& change(*sortedChangesList[pos]);
            QString encountered;
            if(change.m_range.start < change.m_range.end &&
                change.m_range.end.line < textLines.size() &&
                change.m_range.start.line >= 0 &&
                change.m_range.start.column >= 0 &&
                change.m_range.start.column < textLines[change.m_range.start.line].length() &&
                change.m_range.end.column >= 0 && 
                change.m_range.end.column < textLines[change.m_range.end.line].length() && 
                change.m_range.start.line == change.m_range.end.line  && //We demand this, although it shoult be fixed
                (encountered = textLines[change.m_range.start.line].mid(change.m_range.start.column, change.m_range.end.column-change.m_range.start.column)) == change.m_oldText)
            {
                textLines[change.m_range.start.line].replace(change.m_range.start.column, change.m_range.end.column-change.m_range.start.column, change.m_newText);
            }else{
                qDeleteAll(codeRepresentations.values());
                return DocumentChangeSet::ChangeResult(QString("Inconsistent change in %1 at %2:%3 -> %4:%5 = %6(encountered %7) -> %8").arg(file.str()).arg(change.m_range.start.line).arg(change.m_range.start.column).arg(change.m_range.end.line).arg(change.m_range.end.column).arg(change.m_oldText).arg(encountered).arg(change.m_newText), sortedChangesList[pos]);
            }
        }
        
        newTexts[file] = textLines.join("\n");
    }
    
    QMap<IndexedString, QString> oldTexts;
    
    foreach(IndexedString file, m_changes.keys())
    {
        oldTexts[file] = codeRepresentations[file]->text();
        
        if(!codeRepresentations[file]->setText(newTexts[file])) {
            //Fail
            foreach(IndexedString revertFile, oldTexts.keys())
                codeRepresentations[revertFile]->setText(oldTexts[revertFile]);
            
            qDeleteAll(codeRepresentations.values());
            return DocumentChangeSet::ChangeResult(QString("Failed to set text on %1").arg(file.str()));
        }
    }
    
    
    qDeleteAll(codeRepresentations.values());
    return DocumentChangeSet::ChangeResult(true);
}


}