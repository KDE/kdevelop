/***************************************************************************
copyright            : (C) 2006 by David Nolden
email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "indocumentreference.h"
#include "utils.h"

QString InDocumentReference::createReference( const QString& file, int startLine, int startCol, int endLine, int endCol ) {
	QString ret = file;
	if( startLine != -1 ) ret.append( QString( ":%1" ).arg( startLine ) );
	if( startCol != -1 ) ret.append( QString( ":%1" ).arg( startCol ) );
	if( endLine != -1 ) ret.append( QString( ":%1" ).arg( endLine ) );
	if( endCol != -1 ) ret.append( QString( ":%1" ).arg( endCol ) );
	return ret;
}

void InDocumentReference::parseReference( const QString& ref, QString& file, int& startLine, int& startCol, int& endLine, int& endCol ) {
	file = "";
	startLine = endLine = startCol = endCol = -1;
	if ( ref.indexOf( ':' ) != -1 ) {
		file = ref.left( ref.indexOf( ':' ) );

		QString tail = ref.mid( file.length()+1 );

		int refs[4] = {-1, -1, -1, -1};
		for( int a = 0; a < 4; a++ ) {
			if( tail.isEmpty() ) break;
			int nextPos = tail.indexOf( ':' );
			if( nextPos == -1 ) nextPos = tail.length();
			refs[a] = tail.left( nextPos ).toInt();
			tail = tail.mid( nextPos+1 );
			if( tail.isEmpty() ) break;
		}

		startLine = refs[0];
		startCol = refs[1];
		endLine = refs[2];
		endCol = refs[3];
	}
}

///This creates a reference that refers to a position, but not intelligently.
InDocumentReference::InDocumentReference( bool start, const QString& ref ) : m_line( -1 ), m_col( -1 ) {
	int d1, d2;
	QString doc;
	if( start )
		parseReference( ref, doc, m_line, m_col, d1, d2 );
	else
		parseReference( ref, doc, d1, d2, m_line, m_col );
	m_document = ~doc;
}

InDocumentReference::InDocumentReference( const QString& document, int line, int col, const QString& text ) {
	m_line = line;
	m_document = ~document;
	m_col = col;
	if ( !text.isEmpty() )
		useText( text );
}

void InDocumentReference::useText( const QString& text ) {
	if ( m_line == -1 )
		return ;
	int index = lineColToIndex( text, m_line, m_col == -1 ? 0 : m_col );
	if ( index != -1 ) {
		SumSearch<10> search( ~text );
		m_position = search.getReference( index );
	}
}

QString InDocumentReference::document() const {
	return ~m_document;
}

struct InDocumentReference::TextSearchInstance::Private : public Shared {
	SumSearch<10> search;
	QString text;

	Private( const QString& txt ) : search( ~txt ), text( txt ) {

	}
};


InDocumentReference::TextSearchInstance::TextSearchInstance( const TextSearchInstance& rhs ) {
	*this = rhs;
}

InDocumentReference::TextSearchInstance::TextSearchInstance( const QString& txt ) {
	m_data = new Private( txt );
}

InDocumentReference::TextSearchInstance::TextSearchInstance() {
}

QString InDocumentReference::TextSearchInstance::text() const {
	if( !m_data ) return "";
	else
		return m_data->text;
}

InDocumentReference::TextSearchInstance::~TextSearchInstance() {
}

InDocumentReference::TextSearchInstance& InDocumentReference::TextSearchInstance::operator = ( const TextSearchInstance& rhs ) {
	m_data = rhs.m_data;
	return *this;
}

InDocumentReference::TextSearchInstance::operator bool() const {
	return (bool)m_data;
}

///Finds the reference-position dynamically within the given text. If the search fails, puts -1 -1.
void InDocumentReference::findInText( const TextSearchInstance& text, int& line, int& col ) const {
	if ( !isValid() || !text ) {
		line = -1;
		col = -1;
		return ;
	}
	int pos = text.m_data->search.findReference( m_position );

	if ( pos == -1 ) {
		line = -1;
		col = -1;
		return ;
    /*
    ///Fall bock to returning the fixed line- and column-numbers
		line = m_line;
		if( m_col != -1 )
		col = m_col;
		else
		col = 0; */
	} else {
		indexToLineCol( pos, text.m_data->text, line, col );
	}
}

bool InDocumentReference::isValid() const {
	return ( m_line != -1 ) && !m_document.empty();
}

bool InDocumentReference::isDynamic() const {
	return m_position.isValid();
}

InDocumentReference::operator bool() const {
	return isValid();
}

///findInText(..) should be preferred, because it can find the correct position even if the text changed.
int InDocumentReference::line() const {
	return m_line;
}

QString InDocumentReference::asText() const {
	QString ret = QString( "%1" ).arg( m_line );
	if ( m_col != 0 && m_col != -1 )
		ret += QString( ":%1" ).arg( m_col );
	if ( m_position.isValid() )
		ret = "~" + ret;
	return ret;
}

int InDocumentReference::col() const {
	if ( m_col != -1 )
		return m_col;
	else
		return 0;
}

DocumentContextLines::DocumentContextLines() : m_lineOffset( 0 ) {}

DocumentContextLines::DocumentContextLines( const InDocumentReference& beginRef, const InDocumentReference& endRef, const QString& text, int /*contextSize*/ ) : m_lineOffset( 0 ) {
	int startLine = beginRef.line();
	int endLine = endRef.line();
	if ( startLine == -1 )
		return ;
	m_lineOffset = startLine - 5;
	if ( endLine == -1 )
		endLine = startLine;
	int end = endLine + 5;
	if ( m_lineOffset < 0 )
		m_lineOffset = 0;
	if ( end < m_lineOffset )
		end = m_lineOffset + 1;

	int i = lineColToIndex( text, m_lineOffset, 0 );
	if ( i == -1 ) {
		m_lineOffset = 0;
		return ;
	}
	int endI = lineColToIndex( text, end, 0 );
	if ( endI == -1 )
		endI = text.size();

	m_lines = ~text.mid( i, endI - i );
}

DocumentContextLines::operator bool() const {
	return !m_lines.empty();
}

QString DocumentContextLines::text() const {
	return ~m_lines;
}

int DocumentContextLines::lineOffset() const {
	return m_lineOffset;
}

