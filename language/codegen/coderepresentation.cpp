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
#include <editor/modificationrevision.h>
#include <ktexteditor/movinginterface.h>

namespace KDevelop {
    
static bool onDiskChangesForbidden = false;

QString CodeRepresentation::rangeText(KTextEditor::Range range) const
{
    Q_ASSERT(range.end().line() < lines());
    
    //Easier for single line ranges which should happen most of the time
    if(range.onSingleLine())
        return QString( line( range.start().line() ).mid( range.start().column(), range.columnWidth() ) );
    
    //Add up al the requested lines
    QString rangedText = line(range.start().line()).mid(range.start().column());
    
    for(int i = range.start().line() + 1; i <= range.end().line(); ++i)
        rangedText += '\n' + ((i == range.end().line()) ? line(i).left(range.end().column()) : line(i));
    
    return rangedText;
}

static void grepLine(const QString& identifier, const QString& lineText, int lineNumber, QVector<SimpleRange>& ret, bool surroundedByBoundary)
{
    if (identifier.isEmpty())
        return;

    int pos = 0;
    while(true)
    {
        pos = lineText.indexOf(identifier, pos);
        if(pos == -1)
            break;
        int start = pos;
        pos += identifier.length();
        int end = pos;
        
        if(!surroundedByBoundary || ( (end == lineText.length() || !lineText[end].isLetterOrNumber() || lineText[end] != '_')
                                        && (start-1 < 0 || !lineText[start-1].isLetterOrNumber() || lineText[start-1] != '_')) )
        {
            ret << SimpleRange(lineNumber, start, lineNumber, end);
        }
    }
    
}

class EditorCodeRepresentation : public DynamicCodeRepresentation {
  public:
  EditorCodeRepresentation(KTextEditor::Document* document) : m_document(document) {
      m_url = IndexedString(m_document->url());
  }
  
  virtual QVector< SimpleRange > grep ( const QString& identifier, bool surroundedByBoundary ) const {
      QVector< SimpleRange > ret;

      if (identifier.isEmpty())
        return ret;

      for(int line = 0; line < m_document->lines(); ++line)
        grepLine(identifier, m_document->line(line), line, ret, surroundedByBoundary);

      return ret;
  }
  
  QString line(int line) const {
        if(line < 0 || line >= m_document->lines())
            return QString();
        return m_document->line(line);
  }
  
  virtual int lines() const {
      return m_document->lines();
  }
  
  QString text() const {
    return m_document->text();
  }
  
  bool setText(QString text) {
    bool ret = m_document->setText(text);
    ModificationRevision::clearModificationCache(m_url);
    return ret;
  }
  
  bool fileExists(){
    return QFile(m_document->url().path()).exists();
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
      
      bool ret = m_document->replaceText(range, newText);
      ModificationRevision::clearModificationCache(m_url);
      
      return ret;
  }
  
  virtual QString rangeText(KTextEditor::Range range) const {
      return m_document->text(range);
  }
  
  private:
    KTextEditor::Document* m_document;
    IndexedString m_url;
};

class FileCodeRepresentation : public CodeRepresentation {
  public:
    FileCodeRepresentation(IndexedString document) : m_document(document) {
        QString localFile(document.toUrl().toLocalFile());
  
        QFile file( localFile );
        if ( file.open(QIODevice::ReadOnly) ) {
            data = QString::fromLocal8Bit(file.readAll());
            lineData = data.split('\n');
        }
        m_exists = file.exists();
    }
    
    QString line(int line) const {
        if(line < 0 || line >= lineData.size())
            return QString();
      
      return lineData.at(line);
    }
    
    virtual QVector< SimpleRange > grep ( const QString& identifier, bool surroundedByBoundary ) const {
        QVector< SimpleRange > ret;

        if (identifier.isEmpty())
            return ret;

        for(int line = 0; line < lineData.count(); ++line)
            grepLine(identifier, lineData.at(line), line, ret, surroundedByBoundary);

        return ret;
    }
    
    virtual int lines() const {
        return lineData.count();
    }
    
    QString text() const {
      return data;
    }
    
    bool setText(QString text) {
      Q_ASSERT(!onDiskChangesForbidden);
      QString localFile(m_document.toUrl().toLocalFile());

      QFile file( localFile );
      if ( file.open(QIODevice::WriteOnly) )
      {
          QByteArray data = text.toLocal8Bit();
          
          if(file.write(data) == data.size())
          {
              ModificationRevision::clearModificationCache(m_document);
              return true;
          }
      }
      return false;
    }
    
    bool fileExists(){
      return m_exists;
    }
    
  private:
    //We use QByteArray, because the column-numbers are measured in utf-8
    IndexedString m_document;
    bool m_exists;
    QStringList lineData;
    QString data;
};

class ArtificialStringData : public QSharedData {
    public:
    ArtificialStringData(QString data) {
        setData(data);
    }
    void setData(QString data) {
        m_data = data;
        m_lineData = m_data.split('\n');
    }
    QString data() const {
        return m_data;
    }
    const QStringList& lines() const {
        return m_lineData;
    }
    
    private:
    QString m_data;
    QStringList m_lineData;
};

class StringCodeRepresentation : public CodeRepresentation {
  public:
    StringCodeRepresentation(KSharedPtr<ArtificialStringData> _data) : data(_data) {
      Q_ASSERT(data);
    }
    
    QString line(int line) const {
        if(line < 0 || line >= data->lines().size())
            return QString();
      
      return data->lines().at(line);
    }
    
    virtual int lines() const {
        return data->lines().count();
    }
    
    QString text() const {
        return data->data();
    }
    
    bool setText(QString text) {
        data->setData(text);
        return true;
    }
    
    bool fileExists(){
        return false;
    }
    
    virtual QVector< SimpleRange > grep ( const QString& identifier, bool surroundedByBoundary ) const {
        QVector< SimpleRange > ret;

        if (identifier.isEmpty())
            return ret;

        for(int line = 0; line < data->lines().count(); ++line)
            grepLine(identifier, data->lines().at(line), line, ret, surroundedByBoundary);

        return ret;
    }
    
  private:
    KSharedPtr<ArtificialStringData> data;
};

static QHash<IndexedString, KSharedPtr<ArtificialStringData> > artificialStrings;

//Return the representation for the given URL if it exists, or an empty pointer otherwise
KSharedPtr<ArtificialStringData> representationForUrl(IndexedString url)
{
    if(artificialStrings.contains(url))
        return artificialStrings[url];
    else
    {
        IndexedString constructedUrl(CodeRepresentation::artificialUrl(url.str()));
        if(artificialStrings.contains(constructedUrl))
            return artificialStrings[constructedUrl];
        else
            return KSharedPtr<ArtificialStringData>();
    }
}

bool artificialCodeRepresentationExists(IndexedString url)
{
    return !representationForUrl(url).isNull();
}

CodeRepresentation::Ptr createCodeRepresentation(IndexedString url) {
    if(artificialCodeRepresentationExists(url))
        return CodeRepresentation::Ptr(new StringCodeRepresentation(representationForUrl(url)));

    IDocument* document = ICore::self()->documentController()->documentForUrl(url.toUrl());
    if(document && document->textDocument())
        return CodeRepresentation::Ptr(new EditorCodeRepresentation(document->textDocument()));
    else
        return CodeRepresentation::Ptr(new FileCodeRepresentation(url));
}

void CodeRepresentation::setDiskChangesForbidden(bool changesForbidden)
{
    onDiskChangesForbidden = changesForbidden;
}

KUrl CodeRepresentation::artificialUrl(const QString & name)
{
    KUrl url(name);
    url.setScheme("artificial");
    url.cleanPath();
    
    return url;
}

InsertArtificialCodeRepresentation::InsertArtificialCodeRepresentation(IndexedString file, QString text) : m_file(file) {
    if(m_file.toUrl().isRelative())
    {
        m_file = IndexedString(CodeRepresentation::artificialUrl(file.str()));
        
        int idx = 0;
        while(artificialStrings.contains(m_file))
        {
            ++idx;
            m_file = IndexedString(CodeRepresentation::artificialUrl(QString("%1_%2").arg(idx).arg(file.str())));
        }
    }
    
    Q_ASSERT(!artificialStrings.contains(m_file));

    artificialStrings.insert(m_file, KSharedPtr<ArtificialStringData>(new ArtificialStringData(text)));
}

IndexedString InsertArtificialCodeRepresentation::file()
{
    return m_file;
}

InsertArtificialCodeRepresentation::~InsertArtificialCodeRepresentation() {
    Q_ASSERT(artificialStrings.contains(m_file));
    artificialStrings.remove(m_file);
}

void InsertArtificialCodeRepresentation::setText(QString text) {
    Q_ASSERT(artificialStrings.contains(m_file));
    artificialStrings[m_file]->setData(text);
}

QString InsertArtificialCodeRepresentation::text() {
    Q_ASSERT(artificialStrings.contains(m_file));
    return artificialStrings[m_file]->data();
}

}

