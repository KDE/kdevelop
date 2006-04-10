/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "makeitem.h"

#include <q3stylesheet.h>

#include <kdebug.h>
#include <klocale.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>
#include <QTextDocument>

MakeItem::MakeItem()
{
}

MakeItem::MakeItem( const QString& text )
	: m_text( text )
{
}

MakeItem::~MakeItem()
{
}

QString MakeItem::color( bool bright_bg )
{
	switch ( type() )
	{
	case Error:
		return QLatin1String(bright_bg ? "maroon" : "red");
	case Warning:
		return QLatin1String(bright_bg ? "#666" : "#999");
	case Diagnostic:
		return QLatin1String(bright_bg ? "black" : "white");
	default:
		return QLatin1String(bright_bg ? "navy" : "blue");
	}
}

QString MakeItem::icon()
{
	switch ( type() )
	{
	case Error:
	case Warning:
		return QLatin1String("<img src=\"error\"></img><nobr> </nobr>");
	case Diagnostic:
		return QLatin1String("<img src=\"warning\"></img><nobr> </nobr>");
	default:
		return QLatin1String("<img src=\"message\"></img><nobr> </nobr>");
	}
}

QString MakeItem::text( EOutputLevel )
{
	return Qt::escape( m_text );
}

QString MakeItem::formattedText( EOutputLevel level, bool bright_bg )
{
  QString txt = text(level);
	if (txt.isEmpty())
		return QLatin1String("<br>");
	if ( level == eFull )
	{
		return txt;
	}
	else
	{
		return QString::fromUtf8("<code>")
        .append( icon() ).append("<font color=\"").append( color( bright_bg) ).append("\">")
        .append( txt ).append("</font></code>");
	}
}

ErrorItem::ErrorItem( const QString& fn, int ln, const QString& tx, const QString& line, bool isWarning, const QString& compiler )
	: MakeItem( line )
	, fileName(fn)
	, m_error(tx)
	, m_isWarning(isWarning)
	, m_compiler(compiler)
	, m_cursor(new KTextEditor::Cursor(ln, 0))
	, m_doc(0L)
{}

ErrorItem::~ErrorItem()
{
	delete m_cursor;
}

bool ErrorItem::append( const QString& text )
{
	if ( !text.startsWith("   ") )
		return false;
	if ( text.startsWith("   ") && (m_compiler == "intel") )
		return false;
	m_text += text;
	m_error += text;
	m_error = m_error.simplified();
	m_text = m_text.simplified();
	return true;
}

void ErrorItem::setDocument(KTextEditor::Document* document)
{
	if (m_doc == document)
		return;

	m_doc = document;

	if ( KTextEditor::SmartInterface* smart = dynamic_cast<KTextEditor::SmartInterface*>(m_doc) ) {
		// try to get a KTextEditor::SmartCursor, so that we can retain position in
		// a document even when it is edited
		KTextEditor::Cursor* temp = m_cursor;
		m_cursor = smart->newSmartCursor(*temp);
		delete temp;

		// Go to the start of the text in the line
		static QRegExp startOfText( "[\\S]" );
		int newColumn = m_doc->line( cursor().line() ).find( startOfText );
		if (newColumn != -1)
			cursor().setColumn(newColumn);

	} else {
		KTextEditor::Cursor* temp = m_cursor;
		m_cursor = new KTextEditor::Cursor(*temp);
		delete temp;
	}
}

ExitStatusItem::ExitStatusItem( bool normalExit, int exitStatus )
	: m_normalExit( normalExit )
	, m_exitStatus( exitStatus )
{
//	kDebug() << "ExitStatusItem: normalExit=" << normalExit << "; exitStatus=" << exitStatus << endl;
	m_text = i18n("*** Compilation aborted ***");
	if ( m_normalExit )
		if (m_exitStatus )
			m_text = i18n("*** Exited with status: %1 ***", m_exitStatus );
        	else
			m_text = i18n("*** Success ***");
}

QString ExitStatusItem::text( EOutputLevel )
{
	return m_text;	
}

bool DirectoryItem::m_showDirectoryMessages = true;

QString EnteringDirectoryItem::text( EOutputLevel outputLevel )
{
	if ( outputLevel < eFull )
		return i18n("Entering directory %1", directory );
	return m_text;
}

QString ExitingDirectoryItem::text( EOutputLevel outputLevel )
{
	if ( outputLevel < eFull )
		return i18n("Leaving directory %1", directory );
	return m_text;
}

QString ActionItem::text( EOutputLevel outputLevel )
{
	if ( outputLevel < eFull )
	{
		if ( m_tool.isEmpty() )
			return QString(m_action).append(" <b>").append(m_file).append("</b>");
		return QString(m_action).append(" <b>").append(m_file).append("</b>").append(" (").append(m_tool).append(")");
	}
	return MakeItem::text( outputLevel );
}
