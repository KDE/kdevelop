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
#include <qregexp.h>
#include <qtextstream.h>

#include <kdebug.h>
#include <kparts/componentfactory.h>
#include <kservice.h>

#include "misc.h"

#include "kdevcompileroptions.h"


static KDevCompilerOptions *createCompilerOptions( const QString &name, QObject *parent )
{
	KService::Ptr service = KService::serviceByDesktopName( name );
	if ( !service )
	{
		kdDebug( 9020 ) << "Can't find service " << name;
		return 0;
	}
	
	
    KLibFactory *factory = KLibLoader::self()->factory(QFile::encodeName(service->library()));
    if (!factory) {
        QString errorMessage = KLibLoader::self()->lastErrorMessage();
        kdDebug(9020) << "There was an error loading the module " << service->name() << endl <<
	    "The diagnostics is:" << endl << errorMessage << endl;
        exit(1);
    }

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    QObject *obj = factory->create(parent, service->name().latin1(),
                                   "KDevCompilerOptions", args);

    if (!obj->inherits("KDevCompilerOptions")) {
        kdDebug(9020) << "Component does not inherit KDevCompilerOptions" << endl;
        return 0;
    }
    KDevCompilerOptions *dlg = (KDevCompilerOptions*) obj;

    return dlg;
	
/*
	QStringList args;
	QVariant prop = service->property( "X-KDevelop-Args" );
	if ( prop.isValid() )
		args = QStringList::split( " ", prop.toString() );

	return KParts::ComponentFactory
	       ::createInstanceFromService<KDevCompilerOptions>( service, parent,
	                                                         service->name().latin1(), args );*/
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
		res += ( str[ i ].isLetterOrNumber() || str[ i ] == '@' ) ? str[ i ] : QChar( '_' );

	kdDebug(9020) << k_funcinfo << "normalized '" << str << "' to '" << res << "'" << endl;
	return res;
}


void AutoProjectTool::parseMakefileam( const QString &fileName, QMap<QString, QString> *variables )
{
	QFile f( fileName );
	if ( !f.open( IO_ReadOnly ) )
		return ;
	QTextStream stream( &f );

	QRegExp re( "^(#kdevelop:[ \t]*)?([A-Za-z][@A-Za-z0-9_]*)[ \t]*:?=[ \t]*(.*)$" );

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

	QRegExp re( "^([A-Za-z][@A-Za-z0-9_]*)[ \t]*:?=[ \t]*(.*)$" );

	while ( !ins.atEnd() )
	{
		QString line;
		QString s = ins.readLine();
		if ( re.exactMatch( s ) )
		{
			QString lhs = re.cap( 1 );
			QString rhs = re.cap( 2 );
			QMap<QString, QString>::Iterator it = variables.find( lhs );

			if ( it != variables.end() )
			{
				// Skip continuation lines
				while ( !s.isEmpty() && s[ s.length() - 1 ] == '\\' && !ins.atEnd() )
					s = ins.readLine();
				if( !it.data().stripWhiteSpace().isEmpty() ) {
					QStringList variableList = QStringList::split(' ', it.data());
					s = it.key() + " = ";
					int l = s.length();
					for (uint i = 0; i < variableList.count(); i++) {
						l += variableList[i].length() + 1;
						if (l > 80)	{
							s += "\\\n\t";
							l = 8;
						}
						s += variableList[i];
						if( i != variableList.count() - 1 )
							s += ' ';
					}
				}
				else
				    s = QString::null;
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
	for ( it2 = variables.begin(); it2 != variables.end(); ++it2 ){
	    if( !it2.data().stripWhiteSpace().isEmpty() ) {
		QStringList variableList = QStringList::split(' ', it2.data());
		outs << it2.key() + " =";
		int l = it2.key().length() + 2;
		for (uint i = 0; i < variableList.count(); i++) {
			l += variableList[i].length() + 1;
			if (l > 80)	{
				outs << "\\\n\t" << variableList[i];
				l = 8 + variableList[i].length();
			} else
			{
				outs << ' ' << variableList[i];
			}
		}
		outs << endl;
            }
	}

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

	QRegExp re( "^([A-Za-z][@A-Za-z0-9_]*)[ \t]*:?=[ \t]*(.*)$" );

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


QStringList AutoProjectTool::configureinLoadMakefiles(QString configureinpath)
{
	QFile configurein(configureinpath);

	if ( !configurein.open( IO_ReadOnly ) )
	{
		kdDebug(9020) << k_funcinfo << " - couldn't open file: " << configureinpath << endl;
		return QStringList();
	}

	QTextStream stream( &configurein);
	QStringList list;

	QString ac_match("^AC_OUTPUT");

	QRegExp ac_regex(ac_match);

	while ( !stream.eof() ) {
		QString line = stream.readLine();
		if ( ac_regex.search(line) >= 0 ) {
			QRegExp open("\\(");
			QRegExp close("\\)");
			line = line.replace(ac_regex.search(line), ac_match.length() - 1, "");

			if (open.search(line) >= 0)
				line = line.replace(open.search(line), 1, "");

			if (close.search(line) >= 0)
				line = line.replace(close.search(line), 1, "");

			list = QStringList::split(" ", line);
			break;
		}
	}

	configurein.close();

	// make a new object on the heap
	return list;

}

void AutoProjectTool::configureinSaveMakefiles(QString configureinpath, QStringList makefiles)
{
	// read configure.in into buffer origfilecontent
	QFile configurein(configureinpath);

	configurein.open ( IO_ReadOnly );

	QTextStream instream( &configurein);

	QStringList origfilecontent;

	while ( !instream.eof() ) {
		QString line = instream.readLine();
		origfilecontent.push_back(line);
	}

	configurein.close();


	// put origfilecontent back into configure.in
	configurein.open ( IO_WriteOnly );
	QTextStream outstream( &configurein);

	QStringList::iterator it;
	for ( it = origfilecontent.begin(); it != origfilecontent.end(); it++ ) {
		QRegExp ac_regexp("^AC_OUTPUT");
		QString currline = (QString) (*it);

		if ( ac_regexp.search(currline) >= 0 ) {
			QString acline("AC_OUTPUT(");
			acline = acline.append(makefiles.join(" "));
			acline = acline.append(")");
			outstream << acline << "\n";
		}
		else
			outstream << currline << "\n";
	}

	configurein.close();
}

//kate: indent-mode csands; space-indent off; tab-width 4;
