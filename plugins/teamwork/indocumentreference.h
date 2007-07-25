/***************************************************************************
Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef INDOCUMENTREFERENCE_H
#define INDOCUMENTREFERENCE_H

#include  <QString>
#include <string>
#include "network/sharedptr.h"
#include "sumsearch.h"

class InDocumentReference {
    int m_line;
    int m_col;
    string m_document;
    typedef SumReference<10> WeakReference;
    WeakReference m_position;
  public:

    ///This can be used to speed up the search if multiple positions should be found within one text(then this object should be created outside and reached to findInText.
    class TextSearchInstance {
      public:
      TextSearchInstance( const QString& txt );
      TextSearchInstance( const TextSearchInstance& rhs );
      TextSearchInstance();
      ~TextSearchInstance();
      operator bool() const;
      QString text() const;
      TextSearchInstance& operator = ( const TextSearchInstance& rhs );
      private:
        friend class InDocumentReference;
        class Private;
        SharedPtr<Private> m_data;
    };

    static QString createReference( const QString& file, int startLine, int startCol, int endLine, int endCol );
    static void parseReference( const QString& ref, QString& file, int& startLine, int& startCol, int& endLine, int& endCol );

    ///This creates a reference that refers to a position, but not intelligently.
    InDocumentReference( bool start = true, const QString& ref = "" );

    InDocumentReference( const QString& document, int line, int col = 0, const QString& text = "" );

    void useText( const QString& text );

    template <class ArchType>
    void serialize( ArchType& arch, uint /*version*/ ) {
      arch & m_line & m_col & m_document & m_position;
    }

    QString document() const;

    ///Finds the reference-position dynamically within the given text. If the search fails, puts -1 -1.
    void findInText( const TextSearchInstance& text, int& line, int& col ) const;

    bool isValid() const;

    bool isDynamic() const;

    operator bool() const;

    ///findInText(..) should be preferred, because it can find the correct position even if the text changed.
    int line() const;

    QString asText() const;

    int col() const;
};

class DocumentContextLines {
    int m_lineOffset;
    std::string m_lines;

  public:
    DocumentContextLines();

    DocumentContextLines( const InDocumentReference& beginRef, const InDocumentReference& endRef, const QString& text, int /*contextSize*/ = 5 );

    template <class Arch>
    void serialize( Arch& arch, uint /*version*/ ) {
      arch & m_lineOffset & m_lines;
    }

    operator bool() const ;

    QString text() const;

    int lineOffset() const;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
