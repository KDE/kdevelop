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

#include "coderepresentation.h"
#include <QtCore/qfile.h>
#include <KDE/KTextEditor/Document>
#include <language/duchain/indexedstring.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/icore.h>

namespace KDevelop {

class EditorCodeRepresentation : public DynamicCodeRepresentation {
  public:
  EditorCodeRepresentation(KTextEditor::Document* document) : m_document(document) {
  }
  QString line(int line) const {
    if(line < 0 || line >= m_document->lines())
      return QString();
    return m_document->line(line);
  }
  
  QString text() const {
    return m_document->text();
  }
  
  bool setText(QString text) {
    return m_document->setText(text);
  }
  
  void startEdit() {
      m_document->startEditing();
  }
  
  void endEdit() {
      m_document->endEditing();
  }
  
  bool replace(const KTextEditor::Range& range, QString oldText, QString newText, bool ignoreOldText) {
      QString old = m_document->text(range);
      if(oldText != old && !ignoreOldText) {
          return false;
      }
      
      return m_document->replaceText(range, newText);
  }
  
  virtual QString rangeText(KTextEditor::Range range) const {
      return m_document->text(range);
  }
  
  private:
    KTextEditor::Document* m_document;
};

class FileCodeRepresentation : public CodeRepresentation {
  public:
    FileCodeRepresentation(IndexedString document) : m_document(document) {
        QString localFile(document.toUrl().toLocalFile());
  
        QFile file( localFile );
        if ( file.open(QIODevice::ReadOnly) ) {
          data = file.readAll();
          lines = data.split('\n');
        }
    }
    
    QString line(int line) const {
    if(line < 0 || line >= lines.size())
      return QString();
      
      return QString::fromLocal8Bit(lines[line]);
    }
    QString text() const {
      return QString::fromLocal8Bit(data);
    }
    
    bool setText(QString text) {
      QString localFile(m_document.toUrl().toLocalFile());

      QFile file( localFile );
      if ( file.open(QIODevice::WriteOnly) ) {
          QByteArray data = text.toLocal8Bit();
          if(file.write(data) == data.size())
              return true;
      }
      return false;
    }
    
  private:
    //We use QByteArray, because the column-numbers are measured in utf-8
    IndexedString m_document;
    QList<QByteArray> lines;
    QByteArray data;
};

CodeRepresentation* createCodeRepresentation(IndexedString url) {
  IDocument* document = ICore::self()->documentController()->documentForUrl(url.toUrl());
  if(document && document->textDocument())
    return new EditorCodeRepresentation(document->textDocument());
  else
    return new FileCodeRepresentation(url);
}
}

