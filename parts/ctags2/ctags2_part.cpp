/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qwhatsthis.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qregexp.h>

#include <klistview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kstringhandler.h>

#include <kdevgenericfactory.h>
#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevproject.h>
#include <kdevpartcontroller.h>
#include <kdevplugininfo.h>

#include "ctags2_widget.h"
#include "ctags2_part.h"
#include "tags.h"

namespace ctags
{
#include "readtags.h"
}

typedef KDevGenericFactory<CTags2Part> CTags2Factory;
static const KDevPluginInfo data("kdevctags2");
K_EXPORT_COMPONENT_FACTORY( libkdevctags2, CTags2Factory( data ) )

CTags2Part::CTags2Part(QObject *parent, const char *name, const QStringList& )
  : KDevPlugin(&data, parent, name ? name : "ctags2Part" )
{
	setInstance(CTags2Factory::instance());
	//setXMLFile("kdevpart_ctags2.rc");

	Tags::setTagsFile( project()->projectDirectory() + "/tags" );

	m_widget = new CTags2Widget(this);

	QWhatsThis::add(m_widget, i18n("<b>CTAGS</b><p>Result view for a tag lookup. Click a line to go to the corresponding place in the code."));
	m_widget->setCaption(i18n("CTAGS Lookup"));
	mainWindow()->embedOutputView( m_widget, i18n( "CTAGS" ), i18n( "CTAGS lookup results" ) );

    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
}


CTags2Part::~CTags2Part()
{
	if ( m_widget )
	{
		mainWindow()->removeView( m_widget );
	}
	delete m_widget;
}

bool CTags2Part::createTagsFile()
{
	KProcess proc;
	proc.setWorkingDirectory( project()->projectDirectory() );

	proc << "ctags";
	proc << "-R" << "--c++-types=+px" << "--excmd=pattern" << "--exclude=Makefile";

	bool success = proc.start(KProcess::Block);

	return success;
}

void CTags2Part::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (!context->hasType( Context::EditorContext ))
        return;

    const EditorContext *econtext = static_cast<const EditorContext*>(context);
    QString ident = econtext->currentWord();
    if (ident.isEmpty())
        return;

	if ( Tags::hasTag( ident ) )
	{
		m_contextString = ident;
	    QString squeezed = KStringHandler::csqueeze(ident, 30);

		int id = popup->insertItem( i18n("CTAGS Lookup: %1").arg(squeezed),
						this, SLOT(slotGotoTag()) );
		popup->setWhatsThis(id, i18n("<b>Go to ctags declaration</b><p>Searches in the tags database for a symbol "
			"under the cursor and opens a file that contains the symbol declaration."));
	}
}

void CTags2Part::slotGotoTag( )
{
	m_widget->displayHitsAndClear( Tags::getExactMatches( m_contextString ) );

	mainWindow()->raiseView( m_widget );
	m_widget->output_view->setFocus();
}

int CTags2Part::getFileLineFromStream( QTextStream & istream, QString const & pattern )
{

	QString reduced = pattern.mid( 2, pattern.length() -4 );
	QString escaped = QRegExp::escape( reduced );
	QString re_string( "^" + escaped + "$" );
/*
	kdDebug() << "pattern: " << pattern << endl;
	kdDebug() << "escaped: " << escaped << endl;
	kdDebug() << "re_string: " << re_string << endl;
*/
	QRegExp re( re_string );

	int n = 0;
	while ( !istream.atEnd() )
	{
		if ( re.exactMatch( istream.readLine() ) )
		{
			return n;
		}
		n++;
	}
	return -1;
}

int CTags2Part::getFileLineFromPattern( KURL const & url, QString const & pattern )
{
	// if the file is open - get the line from the editor buffer
	if ( KTextEditor::EditInterface * ei = dynamic_cast<KTextEditor::EditInterface*>( partController()->partForURL( url ) ) )
	{
		kdDebug() << "the file is open - get the line from the editor buffer" << endl;

		QString ibuffer = ei->text();
		QTextStream istream( &ibuffer, IO_ReadOnly );
		return getFileLineFromStream( istream, pattern );
	}
	else // else the file is not open - get the line from the file on disk
	{
		kdDebug() << "the file is not open - get the line from the file on disk" << endl;

		QFile file( url.path() );
		QString buffer;

		if ( file.open( IO_ReadOnly ) )
		{
			QTextStream istream( &file );
			return getFileLineFromStream( istream, pattern );
		}
	}
	return -1;
}

#include "ctags2_part.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
