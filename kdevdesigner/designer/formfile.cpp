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

#include "formfile.h"
#include "timestamp.h"
#include "project.h"
#include "formwindow.h"
#include "command.h"
#include "sourceeditor.h"
#include "mainwindow.h"
#include "../interfaces/languageinterface.h"
#include "resource.h"
#include "workspace.h"
#include <qmessagebox.h>
#include <qfile.h>
#include <qstatusbar.h>
#include "propertyeditor.h"
#include <qworkspace.h>
#include <stdlib.h>
#include "designerappiface.h"
#include <qapplication.h>

#include <kfiledialog.h>
#include <klocale.h>

static QString make_func_pretty( const QString &s )
{
    QString res = s;
    if ( res.find( ")" ) - res.find( "(" ) == 1 )
	return res;
    res.replace( "(", "( " );
    res.replace( ")", " )" );
    res.replace( "&", " &" );
    res.replace( "*", " *" );
    res.replace( ",", ", " );
    res.replace( ":", " : " );
    res = res.simplifyWhiteSpace();
    res.replace( " : : ", "::" );
    res.replace( ">>", "> >" );
    return res;
}

FormFile::FormFile( const QString &fn, bool temp, Project *p, const char *name )
    : filename( fn ), fileNameTemp( temp ), pro( p ), fw( 0 ), ed( 0 ),
      timeStamp( 0, fn + codeExtension() ), codeEdited( FALSE ), pkg( FALSE ),
      cm( FALSE ), codeFileStat( None )
{
    MetaDataBase::addEntry( this );
    fake = qstrcmp( name, "qt_fakewindow" ) == 0;
    //codeFileStat = FormFile::None;
    pro->addFormFile( this );
    loadCode();
    if ( !temp )
	checkFileName( FALSE );
}

FormFile::~FormFile()
{
    pro->removeFormFile( this );
    if ( formWindow() )
	formWindow()->setFormFile( 0 );
}

void FormFile::setFormWindow( FormWindow *f )
{
    if ( f == fw )
	return;
    if ( fw )
	fw->setFormFile( 0 );
    fw = f;
    if ( fw )
	fw->setFormFile( this );
    parseCode( cod, FALSE );
    QTimer::singleShot( 0, this, SLOT( notifyFormWindowChange() ) );
}

void FormFile::setEditor( SourceEditor *e )
{
    ed = e;
}

void FormFile::setFileName( const QString &fn )
{
    if ( fn == filename )
	return;
    if ( fn.isEmpty() ) {
	fileNameTemp = TRUE;
	if ( filename.find( "unnamed" ) != 0 )
	    filename = createUnnamedFileName();
	return;
    } else {
	fileNameTemp = FALSE;
    }
    filename = fn;
    timeStamp.setFileName( filename + codeExtension() );
    cod = "";
    loadCode();
}

void FormFile::setCode( const QString &c )
{
    cod = c;
}

FormWindow *FormFile::formWindow() const
{
    return fw;
}

SourceEditor *FormFile::editor() const
{
    return ed;
}

QString FormFile::fileName() const
{
    return filename;
}

QString FormFile::absFileName() const
{
    return pro->makeAbsolute( filename );
}

QString FormFile::codeFile() const
{
    QString codeExt = codeExtension();
    if ( codeExt.isEmpty() )
	return "";
    return filename + codeExt;
}

QString FormFile::code()
{
    return cod;
}

bool FormFile::save( bool withMsgBox, bool ignoreModified )
{
    if ( fileNameTemp )
	return saveAs();
    if ( !ignoreModified && !isModified() )
	return TRUE;
    if ( ed )
	ed->save();

    if ( formWindow() && isModified( WFormWindow ) ) {
	if ( withMsgBox ) {
	    if ( !formWindow()->checkCustomWidgets() )
		return FALSE;
	}

	if ( QFile::exists( pro->makeAbsolute( filename ) ) ) {
	    QString fn( pro->makeAbsolute( filename ) );
#if defined(Q_OS_WIN32)
	    fn += ".bak";
#else
	    fn += "~";
#endif
	    QFile f( pro->makeAbsolute( filename ) );
	    if ( f.open( IO_ReadOnly ) ) {
		QFile f2( fn );
		if ( f2.open( IO_WriteOnly | IO_Translate ) ) {
		    QCString data( f.size() );
		    f.readBlock( data.data(), f.size() );
		    f2.writeBlock( data );
		} else {
		    QMessageBox::warning( MainWindow::self, i18n( "Save" ),
                                  i18n( "The file %1 could not be saved" ).arg( codeFile() ) );
		}
	    }
	}
    }

    if ( isModified( WFormCode ) ) {
	if ( QFile::exists( pro->makeAbsolute( codeFile() ) ) ) {
	    QString fn( pro->makeAbsolute( codeFile() ) );
#if defined(Q_OS_WIN32)
	    fn += ".bak";
#else
	    fn += "~";
#endif
	    QFile f( pro->makeAbsolute( codeFile() ) );
	    if ( f.open( IO_ReadOnly ) ) {
		QFile f2( fn );
		if ( f2.open( IO_WriteOnly | IO_Translate) ) {
		    QCString data( f.size() );
		    f.readBlock( data.data(), f.size() );
		    f2.writeBlock( data );
		} else if ( qApp->type() != QApplication::Tty ) {
            QMessageBox::warning( MainWindow::self, i18n( "Save" ),
                                  i18n( "The file %1 could not be saved" ).arg( codeFile() ) );
		}
	    }
	}
    }

    if ( formWindow() ) {
	Resource resource( MainWindow::self );
	resource.setWidget( formWindow() );
	bool formCodeOnly = isModified( WFormCode ) && !isModified( WFormWindow );
	if ( !resource.save( pro->makeAbsolute( filename ), formCodeOnly ) ) {
	    if ( MainWindow::self )
		MainWindow::self->statusMessage( i18n( "Failed to save file '%1'.").arg( formCodeOnly ? codeFile(): filename ) );
	    if ( formCodeOnly )
		return FALSE;
	    return saveAs();
	}
	if ( MainWindow::self )
	    MainWindow::self->statusMessage( i18n( "'%1' saved.").
						    arg( formCodeOnly ? codeFile() : filename ));
    } else {
	if ( !Resource::saveFormCode(this, MetaDataBase::languageInterface(pro->language())) )
	    return FALSE;
    }
    timeStamp.update();
    setModified( FALSE );
    return TRUE;
}

bool FormFile::saveAs( bool ignoreModified )
{
    QString f = pro->makeAbsolute( fileName() );
    if ( fileNameTemp && formWindow() ) {
	f = QString( formWindow()->name() ).lower();
	f.replace( "::", "_" );
	f = pro->makeAbsolute( f + ".ui" );
    }
    bool saved = FALSE;
    if ( ignoreModified ) {
	QString dir = QStringList::split( ':', pro->iFace()->customSetting( "QTSCRIPT_PACKAGES" ) ).first();
	f = QFileInfo( f ).fileName();
	f.prepend( dir + "/" );
    }
    QString fn;
    while ( !saved ) {
	fn = KFileDialog::getSaveFileName( f,
					   i18n( "*.ui|Qt User-Interface Files" ) + "\n" +
					   i18n( "*|All Files" ), MainWindow::self, /*0,*/
					   i18n( "Save Form '%1' As ...").arg( formName() )/*,
					   MainWindow::self ? &MainWindow::self->lastSaveFilter : 0*/ );
	if ( fn.isEmpty() )
	    return FALSE;
	QFileInfo fi( fn );
	if ( fi.extension() != "ui" )
	    fn += ".ui";
	fileNameTemp = FALSE;
	filename = pro->makeRelative( fn );
	QFileInfo relfi( filename );
	if ( relfi.exists() ) {
	    if ( QMessageBox::warning( MainWindow::self, i18n( "File Already Exists" ),
		i18n( "The file already exists. Do you wish to overwrite it?" ),
		QMessageBox::Yes,
		QMessageBox::No ) == QMessageBox::Yes ) {
		saved = TRUE;
	    } else {
		filename = f;
	    }

	} else {
	    saved = TRUE;
	}
    }
    if ( !checkFileName( TRUE ) ) {
	filename = f;
	return FALSE;
    }
    pro->setModified( TRUE );
    timeStamp.setFileName( pro->makeAbsolute( codeFile() ) );
    if ( ed && formWindow() )
	ed->setCaption( i18n( "Edit %1" ).arg( formWindow()->name() ) );
    setModified( TRUE );
    if ( pro->isDummy() )
	fw->mainWindow()->addRecentlyOpenedFile( fn );
    return save( TRUE, ignoreModified );
}

bool FormFile::close()
{
    if ( editor() ) {
	editor()->save();
	editor()->close();
    }
    if ( formWindow() )
	return formWindow()->close();
    return TRUE;
}

bool FormFile::closeEvent()
{
    if ( !isModified() && fileNameTemp ) {
	pro->removeFormFile( this );
	return TRUE;
    }

    if ( !isModified() )
	return TRUE;

    if ( editor() )
	editor()->save();

    switch ( QMessageBox::warning( MainWindow::self, i18n( "Save Form" ),
				   i18n( "Save changes to form '%1'?" ).arg( filename ),
				   i18n( "&Yes" ), i18n( "&No" ), i18n( "&Cancel" ), 0, 2 ) ) {
    case 0: // save
	if ( !save() )
	    return FALSE;
    case 1: // don't save
	loadCode();
	if ( ed )
	    ed->editorInterface()->setText( cod );
	if ( fileNameTemp )
	    pro->removeFormFile( this );
	if ( MainWindow::self )
	    MainWindow::self->workspace()->update();
	break;
    case 2: // cancel
	return FALSE;
    default:
	break;
    }

    setModified( FALSE );
    if ( MainWindow::self )
	MainWindow::self->updateFunctionList();
    setCodeEdited( FALSE );
    return TRUE;
}

void FormFile::setModified( bool m, int who )
{
    if ( ( who & WFormWindow ) == WFormWindow )
	setFormWindowModified( m );
    if ( ( who & WFormCode ) == WFormCode )
	setCodeModified( m );
}

bool FormFile::isModified( int who )
{
    if ( who == WFormWindow )
	return isFormWindowModified();
    if ( who == WFormCode )
	return isCodeModified();
    return isCodeModified() || isFormWindowModified();
}

bool FormFile::isFormWindowModified() const
{
    if ( !formWindow()  || !formWindow()->commandHistory() )
	return FALSE;
    return formWindow()->commandHistory()->isModified();
}

bool FormFile::isCodeModified() const
{
    if ( !editor() )
	return cm;
    return editor()->isModified();
}

void FormFile::setFormWindowModified( bool m )
{
    bool b = isFormWindowModified();
    if ( m == b )
	return;
    if ( !formWindow() || !formWindow()->commandHistory() )
	return;
    formWindow()->commandHistory()->setModified( m );
    emit somethingChanged( this );
}

void FormFile::setCodeModified( bool m )
{
    bool b = isCodeModified();
    if ( m == b )
	return;
    emit somethingChanged( this );
    cm = m;
    if ( !editor() )
	return;
    editor()->setModified( m );
}

void FormFile::showFormWindow()
{
    if ( !MainWindow::self )
	return;
    if ( formWindow() ) {
	if ( ( formWindow()->hasFocus() ||
	      MainWindow::self->qWorkspace()->activeWindow() == formWindow() ) &&
	     MainWindow::self->propertyeditor()->formWindow() != formWindow() ) {
	    MainWindow::self->propertyeditor()->setWidget( formWindow()->currentWidget(), formWindow() );
	    MainWindow::self->objectHierarchy()->setFormWindow( formWindow(),
								formWindow()->currentWidget() );
	}
	formWindow()->setFocus();
	return;
    }
    MainWindow::self->openFormWindow( pro->makeAbsolute( filename ), TRUE, this );
}

bool FormFile::setupUihFile( bool askForUih )
{
    if ( !pro->isCpp() || !askForUih ) {
	if ( !hasFormCode() ) {
	    createFormCode();
	    setModified( TRUE );
	}
	codeFileStat = FormFile::Ok;
	return TRUE;
    }
    if ( codeFileStat != FormFile::Ok && !ed ) {
	if ( hasFormCode() ) {
	    int i = QMessageBox::information( MainWindow::self, i18n( "Using ui.h file" ),
	                                      i18n( "An \"ui.h\" file for this form already exists.\n"
					      "Do you want to use it or create a new one?" ),
	                                      i18n( "Use existing" ), i18n( "Create new" ),
					      i18n( "Cancel" ), 2, 2 );
	    if ( i == 2 )
		return FALSE;
	    if ( i == 1 )
		createFormCode();
	} else {
	    if ( QMessageBox::Yes != QMessageBox::information( MainWindow::self, i18n( "Creating ui.h file" ),
						 i18n( "Do you want to create an new \"ui.h\" file?" ),
						 QMessageBox::Yes, QMessageBox::No ) )
		return FALSE;
	    createFormCode();
	}
	setModified( TRUE );
    }
    codeFileStat = FormFile::Ok;
    return TRUE;
}

SourceEditor *FormFile::showEditor( bool askForUih )
{
    if ( !MainWindow::self )
	return 0;
    showFormWindow();
    if ( !setupUihFile( askForUih ) )
	return 0;
    return MainWindow::self->openSourceEditor();
}

static int ui_counter = 0;
QString FormFile::createUnnamedFileName()
{
    return QString( "unnamed" ) + QString::number( ++ui_counter ) + QString( ".ui" );
}

QString FormFile::codeExtension() const
{
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( iface )
	return iface->formCodeExtension();
    return "";
}

QString FormFile::codeComment() const
{
    return QString( "/****************************************************************************\n"
    "** ui.h extension file, included from the uic-generated form implementation.\n"
    "**\n"
    "** If you want to add, delete, or rename functions or slots, use\n"
    "** Qt Designer to update this file, preserving your code.\n"
    "**\n"
    "** You should not define a constructor or destructor in this file.\n"
    "** Instead, write your code in functions called init() and destroy().\n"
    "** These will automatically be called by the form's constructor and\n"
    "** destructor.\n"
    "*****************************************************************************/\n" );
}

bool FormFile::hasFormCode() const
{
    return !cod.isEmpty() && cod != QString( codeComment() );
}

int FormFile::codeFileState() const
{
    return hasFormCode() ? codeFileStat : None;
}

void FormFile::setCodeFileState( UihState s )
{
    codeFileStat = s;
}

void FormFile::createFormCode()
{
    if ( !formWindow() )
	return;
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( !iface )
	return;
    if ( pro->isCpp() )
	cod = codeComment();
    QValueList<MetaDataBase::Function> functionList = MetaDataBase::functionList( formWindow() );
    for ( QValueList<MetaDataBase::Function>::Iterator it = functionList.begin(); it != functionList.end(); ++it ) {
	cod += (!cod.isEmpty() ? "\n\n" : "") +
	       iface->createFunctionStart( formWindow()->name(), make_func_pretty((*it).function),
					   (*it).returnType.isEmpty() ?
					   QString( "void" ) :
					   (*it).returnType, (*it).access ) +
	       "\n" + iface->createEmptyFunction();
    }
    parseCode( cod, FALSE );
}

void FormFile::load()
{
    showFormWindow();
    code();
}

bool FormFile::loadCode()
{
    QFile f( pro->makeAbsolute( codeFile() ) );
    if ( !f.open( IO_ReadOnly ) ) {
	cod = "";
	setCodeFileState( FormFile::None );
	return FALSE;
    }
    QTextStream ts( &f );
    cod = ts.read();
    parseCode( cod, FALSE );
    if ( hasFormCode() && codeFileStat != FormFile::Ok )
	setCodeFileState( FormFile::Deleted );
    timeStamp.update();
    return TRUE;
}

bool FormFile::isCodeEdited() const
{
    return codeEdited;
}

void FormFile::setCodeEdited( bool b )
{
    codeEdited = b;
}

void FormFile::parseCode( const QString &txt, bool allowModify )
{
    if ( !formWindow() )
	return;
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( !iface )
	return;
    QValueList<LanguageInterface::Function> functions;
    QValueList<MetaDataBase::Function> newFunctions, oldFunctions;
    oldFunctions = MetaDataBase::functionList( formWindow() );
    iface->functions( txt, &functions );
    QMap<QString, QString> funcs;
    for ( QValueList<LanguageInterface::Function>::Iterator it = functions.begin();
	  it != functions.end(); ++it ) {
	bool found = FALSE;
	for ( QValueList<MetaDataBase::Function>::Iterator fit = oldFunctions.begin();
	      fit != oldFunctions.end(); ++fit ) {
	    QString f( (*fit).function );
	    if ( MetaDataBase::normalizeFunction( f ) ==
		 MetaDataBase::normalizeFunction( (*it).name ) ) {
		found = TRUE;
		MetaDataBase::Function function;
		function.function = make_func_pretty( (*it).name );
		function.specifier = (*fit).specifier;
		function.type = (*fit).type;
		if ( !pro->isCpp() )
		    function.access = (*it).access;
		else
		    function.access = (*fit).access;
		function.language = (*fit).language;
		function.returnType = (*it).returnType;
		newFunctions << function;
		funcs.insert( (*it).name, (*it).body );
		oldFunctions.remove( fit );
		break;
	    }
	}
	if ( !found ) {
	    MetaDataBase::Function function;
	    function.function = make_func_pretty( (*it).name );
	    function.specifier = "virtual";
	    function.access = "public";
	    function.language = pro->language();
	    function.returnType = (*it).returnType;
	    if ( function.returnType == "void" )
		function.type = "slot";
	    else
		function.type = "function";
	    if ( function.function == "init()" || function.function == "destroy()" ) {
		function.type = "function";
		function.access = "private";
		function.specifier = "non virtual";
	    }
	    newFunctions << function;
	    funcs.insert( (*it).name, (*it).body );
	    if ( allowModify )
		setFormWindowModified( TRUE );
	}
    }

    if ( allowModify && oldFunctions.count() > 0 )
	setFormWindowModified( TRUE );

    MetaDataBase::setFunctionList( formWindow(), newFunctions );
}

void FormFile::syncCode()
{
    if ( !editor() )
	return;
    parseCode( editor()->editorInterface()->text(), TRUE );
    cod = editor()->editorInterface()->text();
}

void FormFile::checkTimeStamp()
{
    if ( timeStamp.isUpToDate() )
	return;
    timeStamp.update();
    if ( codeEdited ) {
	if ( QMessageBox::information( MainWindow::self, i18n( "Qt Designer" ),
				       i18n( "File '%1' has been changed outside Qt Designer.\n"
					   "Do you want to reload it?" ).arg( timeStamp.fileName() ),
				       i18n( "&Yes" ), i18n( "&No" ) ) == 0 ) {
	    QFile f( timeStamp.fileName() );
	    if ( f.open( IO_ReadOnly ) ) {
		QTextStream ts( &f );
		editor()->editorInterface()->setText( ts.read() );
		editor()->save();
		if ( MainWindow::self )
		    MainWindow::self->functionsChanged();
	    }
	}
    } else {
	loadCode();
    }
}

bool FormFile::isUihFileUpToDate()
{
    if ( timeStamp.isUpToDate() )
	return TRUE;
    if ( !editor() ) {
	MainWindow::self->editSource();
	qDebug( "parse Code" );
	parseCode( editor()->editorInterface()->text(), TRUE );
    }
    checkTimeStamp();
    return FALSE;
}

void FormFile::addFunctionCode( MetaDataBase::Function function )
{
    if ( pro->isCpp() && !hasFormCode() && !codeEdited )
	return;
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( !iface )
	return;

    QValueList<LanguageInterface::Function> funcs;
    iface->functions( cod, &funcs );
    bool hasFunc = FALSE;
    for ( QValueList<LanguageInterface::Function>::Iterator it = funcs.begin();
	  it != funcs.end(); ++it ) {
	if ( MetaDataBase::normalizeFunction( (*it).name ) == MetaDataBase::normalizeFunction( function.function ) ) {
	    hasFunc = TRUE;
	    break;
	}
    }

    if ( !hasFunc ) {
	if ( !codeEdited && !timeStamp.isUpToDate() )
	    loadCode();
	MetaDataBase::MetaInfo mi = MetaDataBase::metaInfo( formWindow() );
	QString cn;
	if ( mi.classNameChanged )
	    cn = mi.className;
	if ( cn.isEmpty() )
	    cn = formWindow()->name();
	QString body = "\n\n" + iface->createFunctionStart( cn,
							    make_func_pretty( function.function ),
							    function.returnType.isEmpty() ?
							    QString( "void" ) :
							    function.returnType, function.access ) +
		       "\n" + iface->createEmptyFunction();
	cod += body;
	if ( codeEdited ) {
	    setModified( TRUE );
	    emit somethingChanged( this );
	}
    }
}

void FormFile::removeFunctionCode( MetaDataBase::Function function )
{
    if ( pro->isCpp() && !hasFormCode() && !codeEdited )
	return;

    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( !iface )
	return;

    checkTimeStamp();
    QString sourceCode = code();
    if ( sourceCode.isEmpty() )
	return;
    QValueList<LanguageInterface::Function> functions;
    iface->functions( sourceCode, &functions );
    QString fu = MetaDataBase::normalizeFunction( function.function );
    for ( QValueList<LanguageInterface::Function>::Iterator fit = functions.begin(); fit != functions.end(); ++fit ) {
	if ( MetaDataBase::normalizeFunction( (*fit).name ) == fu ) {
	    int line = 0;
	    int start = 0;
	    while ( line < (*fit).start - 1 ) {
		start = sourceCode.find( '\n', start );
		if ( start == -1 )
		    return;
		start++;
		line++;
	    }
	    if ( start == -1 )
	        return;
	    int end = start;
	    while ( line < (*fit).end + 1 ) {
		end = sourceCode.find( '\n', end );
		if ( end == -1 ) {
		    if ( line <= (*fit).end )
			end = sourceCode.length() - 1;
		    else
		        return;
		}
		end++;
		line++;
	    }
	    if ( end < start )
		return;
	    sourceCode.remove( start, end - start );
	    setCode( sourceCode );
	}
    }
}

void FormFile::functionNameChanged( const QString &oldName, const QString &newName )
{
    if ( !cod.isEmpty() ) {
	QString funcStart = QString( formWindow()->name() ) + QString( "::" );
	int i = cod.find( funcStart + oldName );
	if ( i != -1 ) {
	    cod.remove( i + funcStart.length(), oldName.length() );
	    cod.insert( i + funcStart.length(), newName );
	}
    }
}

void FormFile::functionRetTypeChanged( const QString &fuName, const QString &oldType, const QString &newType )
{
    if ( !cod.isEmpty() ) {
	QString oldFunct = oldType + " " + QString( formWindow()->name() ) + "::" + fuName;
	QString newFunct = newType + " " + QString( formWindow()->name() ) + "::" + fuName;

	int i = cod.find( oldFunct );
	if ( i != -1 ) {
	    cod.remove( i, oldFunct.length() );
	    cod.insert( i, newFunct );
	}
    }
}

QString FormFile::formName() const
{
    FormFile* that = (FormFile*) this;
    if ( formWindow() ) {
	that->cachedFormName = formWindow()->name();
	return cachedFormName;
    }
    if ( !cachedFormName.isNull() )
	return cachedFormName;
    QFile f( pro->makeAbsolute( filename ) );
    if ( f.open( IO_ReadOnly ) ) {
	QTextStream ts( &f );
	QString line;
	QString className;
	while ( !ts.eof() ) {
	    line = ts.readLine();
	    if ( !className.isEmpty() ) {
		int end = line.find( "</class>" );
		if ( end == -1 ) {
		    className += line;
		} else {
		    className += line.left( end );
		    break;
		}
		continue;
	    }
	    int start;
	    if ( ( start = line.find( "<class>" ) ) != -1 ) {
		int end = line.find( "</class>" );
		if ( end == -1 ) {
		    className = line.mid( start + 7 );
		} else {
		    className = line.mid( start + 7, end - ( start + 7 ) );
		    break;
		}
	    }
	}
	that->cachedFormName =  className;
    }
    if ( cachedFormName.isEmpty() )
	that->cachedFormName = filename;
    return cachedFormName;
}

void FormFile::formWindowChangedSomehow()
{
    emit somethingChanged( this );
}

bool FormFile::checkFileName( bool allowBreak )
{
    FormFile *ff = pro->findFormFile( filename, this );
    if ( ff )
	QMessageBox::warning( MainWindow::self, i18n( "Invalid Filename" ),
			      i18n( "The project already contains a form with a\n"
				  "filename of '%1'. Please choose a new filename." ).arg( filename ) );
    while ( ff ) {
	QString fn;
	while ( fn.isEmpty() ) {
	    fn = KFileDialog::getSaveFileName( pro->makeAbsolute( fileName() ),
					       i18n( "*.ui|Qt User-Interface Files" ) + "\n" +
					       i18n( "*|All Files" ), MainWindow::self,/* 0,*/
					       i18n( "Save Form '%1' As ...").
					       arg( formWindow()->name() )/*,
					       MainWindow::self ? &MainWindow::self->lastSaveFilter : 0 */);
	    if ( allowBreak && fn.isEmpty() )
		return FALSE;
	}
	filename = pro->makeRelative( fn );
	ff = pro->findFormFile( filename, this );
     }
    return TRUE;
}

void FormFile::addConnection( const QString &sender, const QString &signal,
			      const QString &receiver, const QString &slot )
{
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( iface )
	iface->addConnection( sender, signal, receiver, slot, &cod );
    if ( ed )
	ed->editorInterface()->setText( cod );
}

void FormFile::removeConnection( const QString &sender, const QString &signal,
				 const QString &receiver, const QString &slot )
{
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    if ( iface )
	iface->removeConnection( sender, signal, receiver, slot, &cod );
    if ( ed )
	ed->editorInterface()->setText( cod );
}

void FormFile::notifyFormWindowChange()
{
    if ( fw )
	pro->formOpened( fw );
}
