/***************************************************************************
*   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <qdir.h>
#include <qfile.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qrect.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qwidget.h>

#include <kdebug.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>
#include <kservice.h>
#include <kwordwrap.h>

#include "misc.h"

#include "kdevcompileroptions.h"


static KDevCompilerOptions *createCompilerOptions( const QString &name, QObject *parent )
{
	KService::Ptr service = KService::serviceByName( name );
	if ( !service )
	{
		kdDebug( 9020 ) << "Can't find service " << name;
		return 0;
	}

	QStringList args;
	QVariant prop = service->property( "X-KDevelop-Args" );
	if ( prop.isValid() )
		args = QStringList::split( " ", prop.toString() );

	return KParts::ComponentFactory
	       ::createInstanceFromService<KDevCompilerOptions>( service, parent,
	                                                         service->name().latin1(), args );
}


QString AutoProjectTool::execFlagsDialog( const QString &compiler, const QString &flags, QWidget *parent )
{
	KDevCompilerOptions * plugin = createCompilerOptions( compiler, parent );

	if ( plugin )
	{
		QString newflags = plugin->exec( parent, flags );
		delete plugin;
		return newflags;
	}
	return QString::null;
}


QString AutoProjectTool::canonicalize( const QString &str )
{
	QString res;
	for ( uint i = 0; i < str.length(); ++i )
		res += str[ i ].isLetterOrNumber() ? str[ i ] : QChar( '_' );

	return res;
}


void AutoProjectTool::parseMakefileam( const QString &fileName, QMap<QString, QString> *variables )
{
	QFile f( fileName );
	if ( !f.open( IO_ReadOnly ) )
		return ;
	QTextStream stream( &f );

	QRegExp re( "^(#kdevelop:[ \t]*)?([A-Za-z][A-Za-z0-9_]*)[ \t]*:?=[ \t]*(.*)$" );

	while ( !stream.atEnd() )
	{
		QString line;
		QString s = stream.readLine();
		while ( !s.isEmpty() && s[ s.length() - 1 ] == '\\' && !stream.atEnd() )
		{
			// Read continuation lines
			line += s.left( s.length() - 1 );
			s = stream.readLine();
		}
		line += s;

		if ( re.exactMatch( line ) )
		{
			QString lhs = re.cap( 2 );
			// The need for stripWhitespace seems to be a Qt bug.
			QString rhs = re.cap( 3 ).stripWhiteSpace();
			variables->insert( lhs, rhs );
		}
	}

	f.close();
}


void AutoProjectTool::modifyMakefileam( const QString &fileName, QMap<QString, QString> variables )
{
	QFile fin( fileName );
	if ( !fin.open( IO_ReadOnly ) )
		return ;
	QTextStream ins( &fin );

	QFile fout( fileName + "#" );
	if ( !fout.open( IO_WriteOnly ) )
	{
		fin.close();
		return ;
	}
	QTextStream outs( &fout );

	QRegExp re( "^([A-Za-z][A-Za-z0-9_]*)[ \t]*:?=[ \t]*(.*)$" );

	while ( !ins.atEnd() )
	{
		QString line;
		QString s = ins.readLine();
		if ( re.exactMatch( s ) )
		{
			QString lhs = re.cap( 1 );
			QString rhs = re.cap( 2 );
			QMap<QString, QString>::Iterator it;
			
			for ( it = variables.begin(); it != variables.end(); ++it )
			{
				if ( lhs == it.key() )
					break;
			}
			if ( it != variables.end() )
			{
				// Skip continuation lines
				while ( !s.isEmpty() && s[ s.length() - 1 ] == '\\' && !ins.atEnd() )
					s = ins.readLine();
				s = it.key() + " = " + it.data();
				variables.remove( it );
			}
			else
			{
				while ( !s.isEmpty() && s[ s.length() - 1 ] == '\\' && !ins.atEnd() )
				{
					outs << s << endl;
					s = ins.readLine();
				}
			}
		}

		outs << s << endl;
	}

	// Write new variables out
	QMap<QString, QString>::Iterator it2;
	for ( it2 = variables.begin(); it2 != variables.end(); ++it2 )
		outs << it2.key() + " = " + it2.data() << endl;

	fin.close();
	fout.close();

	QDir().rename( fileName + "#", fileName );
}

void AutoProjectTool::removeFromMakefileam ( const QString &fileName, QMap <QString, QString> variables )
{
	QFile fin( fileName );
	if ( !fin.open( IO_ReadOnly ) )
		return ;
	QTextStream ins( &fin );

	QFile fout( fileName + "#" );
	if ( !fout.open( IO_WriteOnly ) )
	{
		fin.close();
		return ;
	}
	QTextStream outs( &fout );

	QRegExp re( "^([A-Za-z][A-Za-z0-9_]*)[ \t]*:?=[ \t]*(.*)$" );

	while ( !ins.atEnd() )
	{
		bool found = false;
		QString s = ins.readLine();
		
		if ( re.exactMatch( s ) )
		{
			QString lhs = re.cap( 1 );
			QString rhs = re.cap( 2 );
			QMap<QString, QString>::Iterator it;
			
			for ( it = variables.begin(); it != variables.end(); ++it )
			{
				if ( lhs == it.key() )
				{
					// Skip continuation lines
					while ( !s.isEmpty() && s[ s.length() - 1 ] == '\\' && !ins.atEnd() )
						s = ins.readLine();

					variables.remove ( it );
					
					found = true;
					
					break;
				}
			}
		}
		
		if ( !found )
			outs << s << endl;
	}
	
	fin.close();
	fout.close();
	
	QDir().rename ( fileName + "#", fileName );
}


/** Used by AddExisting*Dialog */
KImportIconView::KImportIconView ( const QString& strIntro, QWidget* parent, const char* name )
        : KFileDnDIconView ( parent, name )
{
    m_strIntro = strIntro;
    m_bDropped = false;

    setAcceptDrops ( true );

}


void KImportIconView::drawContents ( QPainter *p, int cx, int cy, int cw, int ch )
{
    if ( !m_bDropped)
    {
        QIconView::drawContents ( p, cx, cy, cw, ch );

        p->save();
        QFont font ( p->font() );
        font.setBold ( true );
        font.setFamily ( "Helvetica [Adobe]" );
        font.setPointSize ( 10 );
        p->setFont ( font );
        p->setPen ( QPen ( KGlobalSettings::highlightColor() ) );

        QRect rect = frameRect();
        QFontMetrics fm ( p->font() );
        rect.setLeft ( rect.left() + 30 );
        rect.setRight ( rect.right() - 30 );

        resizeContents ( contentsWidth(), contentsHeight() );

        // word-wrap the string
        KWordWrap* wordWrap1 = KWordWrap::formatText( fm, rect, AlignHCenter | WordBreak, m_strIntro );
        KWordWrap* wordWrap2 = KWordWrap::formatText( fm, rect, AlignHCenter | WordBreak, "Or just use the buttons!" );

        QRect introRect1 = wordWrap1->boundingRect();
		QRect introRect2 = wordWrap2->boundingRect();

        wordWrap1->drawText ( p, ( ( frameRect().right() - introRect1.right() ) / 2 ), ( ( frameRect().bottom() - introRect1.bottom() ) / 2 ) - 20, AlignHCenter | AlignVCenter );
        wordWrap2->drawText ( p, ( ( frameRect().right() - introRect2.right() ) / 2 ), ( ( frameRect().bottom() - introRect2.bottom() ) / 2 ) + introRect1.bottom(), AlignHCenter | AlignVCenter );

        p->restore();
    }
    else
    {
        QIconView::drawContents ( p, cx, cy, cw, ch );
    }
}

void KImportIconView::somethingDropped ( bool dropped )
{
    m_bDropped = dropped;
}
#include "misc.moc"
