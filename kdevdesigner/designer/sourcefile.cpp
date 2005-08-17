/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "sourcefile.h"
#include <qfile.h>
#include <qtextstream.h>
//Added by qt3to4:
#include <Q3CString>
#include "designerappiface.h"
#include "sourceeditor.h"
#include "metadatabase.h"
#include "../interfaces/languageinterface.h"
#include <q3filedialog.h>
#include <qmessagebox.h>
#include "mainwindow.h"
#include "workspace.h"
#include <stdlib.h>

#include <kfiledialog.h>
#include <klocale.h>

SourceFile::SourceFile( const QString &fn, bool temp, Project *p )
    : filename( fn ), ed( 0 ), fileNameTemp( temp ),
      timeStamp( 0, p->makeAbsolute( fn ) ), pro( p ), pkg( FALSE )
      , accepted( TRUE )
{
    iface = 0;

    if ( !temp )
	accepted = checkFileName( TRUE );

    if (accepted) {
	load();
	pro->addSourceFile( this );
	MetaDataBase::addEntry( this );
    }

}

SourceFile::~SourceFile()
{
    if (iface)
	delete iface;
}

QString SourceFile::text() const
{
    return txt;
}

void SourceFile::setText( const QString &s )
{
    txt = s;
}

bool SourceFile::save( bool ignoreModified )
{
    if ( fileNameTemp )
	return saveAs();
    if ( !ignoreModified && !isModified() )
	return TRUE;
    if ( ed )
	ed->save();

    if ( QFile::exists( pro->makeAbsolute( filename ) ) ) {
	QString fn( pro->makeAbsolute( filename ) );
#if defined(Q_OS_WIN32)
	fn += ".bak";
#else
	fn += "~";
#endif
	QFile f( pro->makeAbsolute( filename ) );
	if ( f.open( QIODevice::ReadOnly ) ) {
	    QFile f2( fn );
	    if ( f2.open( QIODevice::WriteOnly | QIODevice::Translate ) ) {
		Q3CString data( f.size() );
		f.readBlock( data.data(), f.size() );
		f2.writeBlock( data );
	    }
	}
    }

    QFile f( pro->makeAbsolute( filename ) );
    if ( !f.open( QIODevice::WriteOnly | QIODevice::Translate ) )
	return saveAs();

    QTextStream ts( &f );
    ts << txt;
    timeStamp.update();
    setModified( FALSE );
    return TRUE;
}

bool SourceFile::saveAs( bool ignoreModified )
{
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    QString filter;
    if ( iface )
	filter = iface->fileFilterList().join("\n");

    QString old = filename;
    QString initFn = pro->makeAbsolute( filename );
    if ( ignoreModified ) {
	QString dir = QStringList::split( ':', project()->iFace()->customSetting( "QTSCRIPT_PACKAGES" ) ).first();
	initFn = QFileInfo( initFn ).fileName();
	initFn.prepend( dir + "/" );
    }
    QString fn = KFileDialog::getSaveFileName( initFn, filter );
    if ( fn.isEmpty() )
	return FALSE;
    fileNameTemp = FALSE;
    filename = pro->makeRelative( fn );
    if ( !checkFileName( TRUE ) ) {
	filename = old;
	return FALSE;
    }
    pro->setModified( TRUE );
    timeStamp.setFileName( pro->makeAbsolute( filename ) );
    if ( ed )
	ed->setCaption( i18n( "Edit %1" ).arg( filename ) );
    setModified( TRUE );
    if ( pro->isDummy() ) {
	QObject *o = ed->parent();
	while ( o && !o->isA( "MainWindow" ) )
	    o = o->parent();
	if ( o )
	    ((MainWindow *)o)->addRecentlyOpenedFile( fn );
    }
    return save( ignoreModified );
}

bool SourceFile::load()
{
    QFile f( pro->makeAbsolute( filename ) );
    if ( !f.open( QIODevice::ReadOnly ) )
	return FALSE;
    QTextStream ts( &f );
    txt = ts.read();
    timeStamp.update();
    return TRUE;
}

DesignerSourceFile *SourceFile::iFace()
{
    if ( !iface )
	iface = new DesignerSourceFileImpl( this );
    return iface;
}

void SourceFile::setEditor( SourceEditor *e )
{
    ed = e;
}

bool SourceFile::isModified() const
{
    if ( !ed )
	return FALSE;
    return ed->isModified();
}

static QMap<QString, int> *extensionCounter;
QString SourceFile::createUnnamedFileName( const QString &extension )
{
    if ( !extensionCounter )
	extensionCounter = new QMap<QString, int>;
    int count = -1;
    QMap<QString, int>::Iterator it;
    if ( ( it = extensionCounter->find( extension ) ) != extensionCounter->end() ) {
	count = *it;
	++count;
	extensionCounter->replace( extension, count );
    } else {
	count = 1;
	extensionCounter->insert( extension, count );
    }

    return "unnamed" + QString::number( count ) + "." + extension;
}

void SourceFile::setModified( bool m )
{
    if ( !ed )
	return;
    ed->setModified( m );
}

bool SourceFile::closeEvent()
{
    if ( !isModified() && fileNameTemp ) {
	pro->removeSourceFile( this );
	return TRUE;
    }

    if ( !isModified() )
	return TRUE;

    if ( ed )
	ed->save();

    switch ( QMessageBox::warning( MainWindow::self, i18n( "Save Code" ),
				   i18n( "Save changes to '%1'?" ).arg( filename ),
				   i18n( "&Yes" ), i18n( "&No" ), i18n( "&Cancel" ), 0, 2 ) ) {
    case 0: // save
	if ( !save() )
	    return FALSE;
	break;
    case 1: // don't save
	load();
	if ( ed )
	    ed->editorInterface()->setText( txt );
	if ( fileNameTemp ) {
	    pro->removeSourceFile( this );
	    return TRUE;
	}
	if ( MainWindow::self )
	    MainWindow::self->workspace()->update();
	break;
    case 2: // cancel
	return FALSE;
    default:
	break;
    }
    setModified( FALSE );
    return TRUE;
}

bool SourceFile::close()
{
    if ( !ed )
	return TRUE;
    return ed->close();
}

Project *SourceFile::project() const
{
    return pro;
}

void SourceFile::checkTimeStamp()
{
    if ( timeStamp.isUpToDate() )
	return;
    timeStamp.update();
    if ( QMessageBox::information( MainWindow::self, i18n( "Qt Designer" ),
				   i18n( "File '%1' has been changed outside Qt Designer.\n"
				       "Do you want to reload it?" ).arg( filename ),
				   i18n( "&Yes" ), i18n( "&No" ) ) == 0 ) {
	load();
	if ( ed )
	    ed->editorInterface()->setText( txt );
    }
}

bool SourceFile::checkFileName( bool allowBreak )
{
    SourceFile *sf = pro->findSourceFile( filename, this );
    if ( sf )
	QMessageBox::warning( MainWindow::self, i18n( "Invalid Filename" ),
			      i18n( "The project already contains a source file with \n"
				  "filename '%1'. Please choose a new filename." ).arg( filename ) );
    while ( sf ) {
	LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
	QString filter;
	if ( iface )
	    filter = iface->fileFilterList().join("\n");
	QString fn;
	while ( fn.isEmpty() ) {
	    fn = KFileDialog::getSaveFileName( pro->makeAbsolute( filename ), filter );
	    if ( allowBreak && fn.isEmpty() )
		return FALSE;
	}
	filename = pro->makeRelative( fn );
	sf = pro->findSourceFile( filename, this );
    }
    return TRUE;
}
