/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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

#include "actiondnd.h"
#include "command.h"
#ifndef QT_NO_SQL
#include "database.h"
#endif
#include "formfile.h"
#include "formwindow.h"
#include "mainwindow.h"
#include "menubareditor.h"
#include "metadatabase.h"
#include "pixmapcollection.h"
#include "popupmenueditor.h"
#include "project.h"
#include "resource.h"
#include "widgetfactory.h"

#include <domtool.h>
#include <widgetdatabase.h>

#include <qaccel.h>
#include <qapplication.h>
#include <qbuffer.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qdom.h>
#include <qfeatures.h>
#include <qfile.h>
#include <qheader.h>
#include <qiconview.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qmenudata.h>
#include <qmessagebox.h>
#include <qmetaobject.h>
#include <qobject.h>
#include <qobjectlist.h>
#include <qtabbar.h>
#ifndef QT_NO_TABLE
#include <qtable.h>
#include <qdatatable.h>
#endif
#include <qtabwidget.h>
#include <qtabwidget.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qwidget.h>
#include <qwidgetstack.h>
#include <qwizard.h>
#include <qworkspace.h>
#include <qworkspace.h>
#include <qsplitter.h>
#include <private/qucom_p.h>

#include <kiconloader.h>
#include <kfiledialog.h>
#include <klocale.h>

#include "kdevdesigner_part.h"

static QString makeIndent( int indent )
{
    QString s;
    s.fill( ' ', indent * 4 );
    return s;
}

static QString entitize( const QString &s, bool attribute = FALSE )
{
    QString s2 = s;
    s2 = s2.replace( "&", "&amp;" );
    s2 = s2.replace( ">", "&gt;" );
    s2 = s2.replace( "<", "&lt;" );
    if ( attribute ) {
	s2 = s2.replace( "\"", "&quot;" );
	s2 = s2.replace( "'", "&apos;" );
    }
    return s2;
}

#ifdef Q_WS_MACX
static struct {
    int key;
    const char* name;
} keyname[] = {
    { Qt::Key_Space,        QT_TRANSLATE_NOOP( "QAccel", "Space" ) },
    { Qt::Key_Escape,       QT_TRANSLATE_NOOP( "QAccel", "Esc" ) },
    { Qt::Key_Tab,          QT_TRANSLATE_NOOP( "QAccel", "Tab" ) },
    { Qt::Key_Backtab,      QT_TRANSLATE_NOOP( "QAccel", "Backtab" ) },
    { Qt::Key_Backspace,    QT_TRANSLATE_NOOP( "QAccel", "Backspace" ) },
    { Qt::Key_Return,       QT_TRANSLATE_NOOP( "QAccel", "Return" ) },
    { Qt::Key_Enter,        QT_TRANSLATE_NOOP( "QAccel", "Enter" ) },
    { Qt::Key_Insert,       QT_TRANSLATE_NOOP( "QAccel", "Ins" ) },
    { Qt::Key_Delete,       QT_TRANSLATE_NOOP( "QAccel", "Del" ) },
    { Qt::Key_Pause,        QT_TRANSLATE_NOOP( "QAccel", "Pause" ) },
    { Qt::Key_Print,        QT_TRANSLATE_NOOP( "QAccel", "Print" ) },
    { Qt::Key_SysReq,       QT_TRANSLATE_NOOP( "QAccel", "SysReq" ) },
    { Qt::Key_Home,         QT_TRANSLATE_NOOP( "QAccel", "Home" ) },
    { Qt::Key_End,          QT_TRANSLATE_NOOP( "QAccel", "End" ) },
    { Qt::Key_Left,         QT_TRANSLATE_NOOP( "QAccel", "Left" ) },
    { Qt::Key_Up,           QT_TRANSLATE_NOOP( "QAccel", "Up" ) },
    { Qt::Key_Right,        QT_TRANSLATE_NOOP( "QAccel", "Right" ) },
    { Qt::Key_Down,         QT_TRANSLATE_NOOP( "QAccel", "Down" ) },
    { Qt::Key_Prior,        QT_TRANSLATE_NOOP( "QAccel", "PgUp" ) },
    { Qt::Key_Next,         QT_TRANSLATE_NOOP( "QAccel", "PgDown" ) },
    { Qt::Key_CapsLock,     QT_TRANSLATE_NOOP( "QAccel", "CapsLock" ) },
    { Qt::Key_NumLock,      QT_TRANSLATE_NOOP( "QAccel", "NumLock" ) },
    { Qt::Key_ScrollLock,   QT_TRANSLATE_NOOP( "QAccel", "ScrollLock" ) },
    { Qt::Key_Menu,         QT_TRANSLATE_NOOP( "QAccel", "Menu" ) },
    { Qt::Key_Help,         QT_TRANSLATE_NOOP( "QAccel", "Help" ) },

    // Multimedia keys
    { Qt::Key_Back,         QT_TRANSLATE_NOOP( "QAccel", "Back" ) },
    { Qt::Key_Forward,      QT_TRANSLATE_NOOP( "QAccel", "Forward" ) },
    { Qt::Key_Stop,         QT_TRANSLATE_NOOP( "QAccel", "Stop" ) },
    { Qt::Key_Refresh,      QT_TRANSLATE_NOOP( "QAccel", "Refresh" ) },
    { Qt::Key_VolumeDown,   QT_TRANSLATE_NOOP( "QAccel", "Volume Down" ) },
    { Qt::Key_VolumeMute,   QT_TRANSLATE_NOOP( "QAccel", "Volume Mute" ) },
    { Qt::Key_VolumeUp,     QT_TRANSLATE_NOOP( "QAccel", "Volume Up" ) },
    { Qt::Key_BassBoost,    QT_TRANSLATE_NOOP( "QAccel", "Bass Boost" ) },
    { Qt::Key_BassUp,       QT_TRANSLATE_NOOP( "QAccel", "Bass Up" ) },
    { Qt::Key_BassDown,     QT_TRANSLATE_NOOP( "QAccel", "Bass Down" ) },
    { Qt::Key_TrebleUp,     QT_TRANSLATE_NOOP( "QAccel", "Treble Up" ) },
    { Qt::Key_TrebleDown,   QT_TRANSLATE_NOOP( "QAccel", "Treble Down" ) },
    { Qt::Key_MediaPlay,    QT_TRANSLATE_NOOP( "QAccel", "Media Play" ) },
    { Qt::Key_MediaStop,    QT_TRANSLATE_NOOP( "QAccel", "Media Stop" ) },
    { Qt::Key_MediaPrev,    QT_TRANSLATE_NOOP( "QAccel", "Media Previous" ) },
    { Qt::Key_MediaNext,    QT_TRANSLATE_NOOP( "QAccel", "Media Next" ) },
    { Qt::Key_MediaRecord,  QT_TRANSLATE_NOOP( "QAccel", "Media Record" ) },
    { Qt::Key_HomePage,     QT_TRANSLATE_NOOP( "QAccel", "Home" ) },
    { Qt::Key_Favorites,    QT_TRANSLATE_NOOP( "QAccel", "Favorites" ) },
    { Qt::Key_Search,       QT_TRANSLATE_NOOP( "QAccel", "Search" ) },
    { Qt::Key_Standby,      QT_TRANSLATE_NOOP( "QAccel", "Standby" ) },
    { Qt::Key_OpenUrl,      QT_TRANSLATE_NOOP( "QAccel", "Open URL" ) },
    { Qt::Key_LaunchMail,   QT_TRANSLATE_NOOP( "QAccel", "Launch Mail" ) },
    { Qt::Key_LaunchMedia,  QT_TRANSLATE_NOOP( "QAccel", "Launch Media" ) },
    { Qt::Key_Launch0,      QT_TRANSLATE_NOOP( "QAccel", "Launch (0)" ) },
    { Qt::Key_Launch1,      QT_TRANSLATE_NOOP( "QAccel", "Launch (1)" ) },
    { Qt::Key_Launch2,      QT_TRANSLATE_NOOP( "QAccel", "Launch (2)" ) },
    { Qt::Key_Launch3,      QT_TRANSLATE_NOOP( "QAccel", "Launch (3)" ) },
    { Qt::Key_Launch4,      QT_TRANSLATE_NOOP( "QAccel", "Launch (4)" ) },
    { Qt::Key_Launch5,      QT_TRANSLATE_NOOP( "QAccel", "Launch (5)" ) },
    { Qt::Key_Launch6,      QT_TRANSLATE_NOOP( "QAccel", "Launch (6)" ) },
    { Qt::Key_Launch7,      QT_TRANSLATE_NOOP( "QAccel", "Launch (7)" ) },
    { Qt::Key_Launch8,      QT_TRANSLATE_NOOP( "QAccel", "Launch (8)" ) },
    { Qt::Key_Launch9,      QT_TRANSLATE_NOOP( "QAccel", "Launch (9)" ) },
    { Qt::Key_LaunchA,      QT_TRANSLATE_NOOP( "QAccel", "Launch (A)" ) },
    { Qt::Key_LaunchB,      QT_TRANSLATE_NOOP( "QAccel", "Launch (B)" ) },
    { Qt::Key_LaunchC,      QT_TRANSLATE_NOOP( "QAccel", "Launch (C)" ) },
    { Qt::Key_LaunchD,      QT_TRANSLATE_NOOP( "QAccel", "Launch (D)" ) },
    { Qt::Key_LaunchE,      QT_TRANSLATE_NOOP( "QAccel", "Launch (E)" ) },
    { Qt::Key_LaunchF,      QT_TRANSLATE_NOOP( "QAccel", "Launch (F)" ) },

    // --------------------------------------------------------------
    // More consistent namings
    { Qt::Key_Print,        QT_TRANSLATE_NOOP( "QAccel", "Print Screen" ) },
    { Qt::Key_Prior,        QT_TRANSLATE_NOOP( "QAccel", "Page Up" ) },
    { Qt::Key_Next,         QT_TRANSLATE_NOOP( "QAccel", "Page Down" ) },
    { Qt::Key_CapsLock,     QT_TRANSLATE_NOOP( "QAccel", "Caps Lock" ) },
    { Qt::Key_NumLock,      QT_TRANSLATE_NOOP( "QAccel", "Num Lock" ) },
    { Qt::Key_NumLock,      QT_TRANSLATE_NOOP( "QAccel", "Number Lock" ) },
    { Qt::Key_ScrollLock,   QT_TRANSLATE_NOOP( "QAccel", "Scroll Lock" ) },
    { Qt::Key_Insert,       QT_TRANSLATE_NOOP( "QAccel", "Insert" ) },
    { Qt::Key_Delete,       QT_TRANSLATE_NOOP( "QAccel", "Delete" ) },
    { Qt::Key_Escape,       QT_TRANSLATE_NOOP( "QAccel", "Escape" ) },
    { Qt::Key_SysReq,       QT_TRANSLATE_NOOP( "QAccel", "System Request" ) },

    { 0, 0 }
};
#endif
static QString platformNeutralKeySequence(const QKeySequence &ks)
{
#ifndef Q_WS_MACX
    return QString(ks);
#else
    uint k;
    QString str;
    QString p;
    for (k = 0; k < ks.count(); ++k) {
	int keycombo = ks[k];
	int basekey = keycombo & ~(Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::META);
	if (keycombo & Qt::CTRL)
	    str += "Ctrl+";
	if (keycombo & Qt::ALT)
	    str += "Alt+";
	if (keycombo & Qt::META)
	    str += "Meta+";
	if (keycombo & Qt::SHIFT)
	    str += "Shift+";

	// begin copy and paste from QKeySequence :(
	if (basekey & Qt::UNICODE_ACCEL) {
	    // Note: This character should NOT be upper()'ed, since
	    // the encoded string should indicate EXACTLY what the
	    // key represents! Hence a 'Ctrl+Shift+c' is posible to
	    // represent, but is clearly impossible to trigger...
	    p = QChar(basekey & 0xffff);
	} else if ( basekey >= Qt::Key_F1 && basekey <= Qt::Key_F35 ) {
	    p = QAccel::tr( "F%1" ).arg(basekey - Qt::Key_F1 + 1);
	} else if ( basekey > Qt::Key_Space && basekey <= Qt::Key_AsciiTilde ) {
	    p.sprintf( "%c", basekey );
	} else {
	    int i = 0;
	    while (keyname[i].name) {
		if (basekey == keyname[i].key) {
		    p = QAccel::tr(keyname[i].name);
		    break;
		}
		++i;
	    }
	    // If we can't find the actual translatable keyname,
	    // fall back on the unicode representation of it...
	    // Or else characters like Key_aring may not get displayed
	    // ( Really depends on you locale )
	    if ( !keyname[i].name )
		// Note: This character should NOT be upper()'ed, see above!
		p = QChar(basekey & 0xffff);
	}
	// end copy...
	str += p + ", ";
    }
    str.truncate(str.length() - 2);
    return str;
#endif
}

static QString mkBool( bool b )
{
    return b? "true" : "false";
}

/*!
  \class Resource resource.h
  \brief Class for saving/loading, etc. forms

  This class is used for saving and loading forms, code generation,
  transferring data of widgets over the clipboard, etc..

*/


Resource::Resource()
{
    mainwindow = 0;
    formwindow = 0;
    toplevel = 0;
    copying = FALSE;
    pasting = FALSE;
    hadGeometry = FALSE;
    langIface = 0;
    hasFunctions = FALSE;
}

Resource::Resource( MainWindow* mw )
    : mainwindow( mw )
{
    formwindow = 0;
    toplevel = 0;
    copying = FALSE;
    pasting = FALSE;
    hadGeometry = FALSE;
    langIface = 0;
    hasFunctions = FALSE;
}

Resource::~Resource()
{
    if ( langIface )
	langIface->release();
}

void Resource::setWidget( FormWindow *w )
{
    formwindow = w;
    toplevel = w;
}

QWidget *Resource::widget() const
{
    return toplevel;
}

bool Resource::load( FormFile *ff, Project *defProject )
{
    if ( !ff || ff->absFileName().isEmpty() )
	return FALSE;
    currFileName = ff->absFileName();
    mainContainerSet = FALSE;

    QFile f( ff->absFileName() );
    f.open( IO_ReadOnly | IO_Translate );

    bool b = load( ff, &f, defProject );
    f.close();

    return b;
}

#undef signals
#undef slots

bool Resource::load( FormFile *ff, QIODevice* dev, Project *defProject )
{
    QDomDocument doc;
    QString errMsg;
    int errLine;
    if ( !doc.setContent( dev, &errMsg, &errLine ) ) {
	return FALSE;
    }

    DomTool::fixDocument( doc );

    QWidget *p = mainwindow ? mainwindow->qWorkspace() : 0;
    toplevel = formwindow = new FormWindow( ff, p, 0 );
    if ( defProject )
	formwindow->setProject( defProject );
    else if ( MainWindow::self )
	formwindow->setProject( MainWindow::self->currProject() );
    if ( mainwindow )
	formwindow->setMainWindow( mainwindow );
    MetaDataBase::addEntry( formwindow );

    if ( !langIface ) {
	QString lang = "Qt Script";
	if ( mainwindow )
	    lang = mainwindow->currProject()->language();
	langIface = MetaDataBase::languageInterface( lang );
	if ( langIface )
	    langIface->addRef();
    }

    uiFileVersion = doc.firstChild().toElement().attribute("version");
    QDomElement e = doc.firstChild().toElement().firstChild().toElement();

    QDomElement forwards = e;
    while ( forwards.tagName() != "forwards" && !forwards.isNull() )
	forwards = forwards.nextSibling().toElement();

    QDomElement includes = e;
    while ( includes.tagName() != "includes" && !includes.isNull() )
	includes = includes.nextSibling().toElement();

    QDomElement variables = e;
    while ( variables.tagName() != "variables" && !variables.isNull() )
	variables = variables.nextSibling().toElement();

    QDomElement signals = e;
    while ( signals.tagName() != "signals" && !signals.isNull() )
	signals = signals.nextSibling().toElement();

    QDomElement slots = e;
    while ( slots.tagName() != "slots" && !slots.isNull() )
	slots = slots.nextSibling().toElement();

    QDomElement functions = e;
    while ( functions.tagName() != "functions" && !functions.isNull() )
	functions = functions.nextSibling().toElement();

    QDomElement connections = e;
    while ( connections.tagName() != "connections" && !connections.isNull() )
	connections = connections.nextSibling().toElement();

    QDomElement imageCollection = e;
    images.clear();
    while ( imageCollection.tagName() != "images" && !imageCollection.isNull() )
	imageCollection = imageCollection.nextSibling().toElement();

    QDomElement customWidgets = e;
    while ( customWidgets.tagName() != "customwidgets" && !customWidgets.isNull() )
	customWidgets = customWidgets.nextSibling().toElement();

    QDomElement tabOrder = e;
    while ( tabOrder.tagName() != "tabstops" && !tabOrder.isNull() )
	tabOrder = tabOrder.nextSibling().toElement();

    QDomElement actions = e;
    while ( actions.tagName() != "actions" && !actions.isNull() )
	actions = actions.nextSibling().toElement();

    QDomElement toolbars = e;
    while ( toolbars.tagName() != "toolbars" && !toolbars.isNull() )
	toolbars = toolbars.nextSibling().toElement();

    QDomElement menubar = e;
    while ( menubar.tagName() != "menubar" && !menubar.isNull() )
	menubar = menubar.nextSibling().toElement();

    QDomElement widget;
    while ( !e.isNull() ) {
	if ( e.tagName() == "widget" ) {
	    widgets.clear();
	    widget = e;
	} else if ( e.tagName() == "include" ) { // compatibility with 2.x
	    MetaDataBase::Include inc;
	    inc.location = "global";
	    if ( e.attribute( "location" ) == "local" )
		inc.location = "local";
	    inc.implDecl = "in declaration";
	    if ( e.attribute( "impldecl" ) == "in implementation" )
		inc.implDecl = "in implementation";
	    inc.header = e.firstChild().toText().data();
	    if ( inc.header.right( 5 ) != ".ui.h" ) {
		metaIncludes.append( inc );
	    } else {
		if ( formwindow->formFile() )
		    formwindow->formFile()->setCodeFileState( FormFile::Ok );
	    }
	} else if ( e.tagName() == "comment" ) {
	    metaInfo.comment = e.firstChild().toText().data();
	} else if ( e.tagName() == "forward" ) { // compatibility with old betas
	    metaForwards << e.firstChild().toText().data();
	} else if ( e.tagName() == "variable" ) { // compatibility with old betas
	    MetaDataBase::Variable v;
	    v.varName = e.firstChild().toText().data();
	    v.varAccess = "protected";
	    metaVariables << v;
	} else if ( e.tagName() == "author" ) {
	    metaInfo.author = e.firstChild().toText().data();
	} else if ( e.tagName() == "class" ) {
	    metaInfo.className = e.firstChild().toText().data();
	} else if ( e.tagName() == "pixmapfunction" ) {
	    if ( formwindow ) {
		formwindow->setSavePixmapInline( FALSE );
		formwindow->setSavePixmapInProject( FALSE );
		formwindow->setPixmapLoaderFunction( e.firstChild().toText().data() );
	    }
	} else if ( e.tagName() == "pixmapinproject" ) {
	    if ( formwindow ) {
		formwindow->setSavePixmapInline( FALSE );
		formwindow->setSavePixmapInProject( TRUE );
	    }
	} else if ( e.tagName() == "exportmacro" ) {
	    exportMacro = e.firstChild().toText().data();
	} else if ( e.tagName() == "layoutdefaults" ) {
	    formwindow->setLayoutDefaultSpacing( e.attribute( "spacing", QString::number( formwindow->layoutDefaultSpacing() ) ).toInt() );
	    formwindow->setLayoutDefaultMargin( e.attribute( "margin", QString::number( formwindow->layoutDefaultMargin() ) ).toInt() );
	} else if ( e.tagName() == "layoutfunctions" ) {
	    formwindow->setSpacingFunction( e.attribute( "spacing" ) );
	    formwindow->setMarginFunction( e.attribute( "margin" ) );
	    if ( !formwindow->marginFunction().isEmpty() || !formwindow->spacingFunction().isEmpty() )
		formwindow->hasLayoutFunctions( TRUE );
	}

	e = e.nextSibling().toElement();
    }

    if ( !imageCollection.isNull() )
	loadImageCollection( imageCollection );
    if ( !customWidgets.isNull() )
	loadCustomWidgets( customWidgets, this );

#if defined (QT_NON_COMMERCIAL)
    bool previewMode = MainWindow::self ? MainWindow::self->isPreviewing() : FALSE;
    QWidget *w = (QWidget*)createObject( widget, !previewMode ? (QWidget*)formwindow : MainWindow::self );
    if ( !w )
	return FALSE;
    if ( previewMode )
	w->reparent( MainWindow::self, Qt::WType_TopLevel,  w->pos(), TRUE );
#else
    if ( !createObject( widget, formwindow) )
	return FALSE;
#endif

    if ( !forwards.isNull() ) {
	for ( QDomElement n = forwards.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() )
	    if ( n.tagName() == "forward" )
		metaForwards << n.firstChild().toText().data();
    }

    if ( !includes.isNull() ) {
	for ( QDomElement n = includes.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() )
	    if ( n.tagName() == "include" ) {
		if ( n.tagName() == "include" ) {
		    MetaDataBase::Include inc;
		    inc.location = "global";
		    if ( n.attribute( "location" ) == "local" )
			inc.location = "local";
		    inc.implDecl = "in declaration";
		    if ( n.attribute( "impldecl" ) == "in implementation" )
			inc.implDecl = "in implementation";
		    inc.header = n.firstChild().toText().data();
		    if ( inc.header.right( 5 ) != ".ui.h" ) {
			metaIncludes.append( inc );
		    } else {
			if ( formwindow->formFile() )
			    formwindow->formFile()->setCodeFileState( FormFile::Ok );
		    }
		}
	    }
    }

    if ( !variables.isNull() ) {
	for ( QDomElement n = variables.firstChild().toElement(); !n.isNull();
	      n = n.nextSibling().toElement() ) {
	    if ( n.tagName() == "variable" ) {
		MetaDataBase::Variable v;
		v.varName = n.firstChild().toText().data();
		v.varAccess = n.attribute( "access", "protected" );
		if ( v.varAccess.isEmpty() )
		    v.varAccess = "protected";
		metaVariables << v;
	    }
	}
    }
    if ( !signals.isNull() ) {
	for ( QDomElement n = signals.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() )
	    if ( n.tagName() == "signal" )
		metaSignals << n.firstChild().toText().data();
    }
    if ( !slots.isNull() ) {
	for ( QDomElement n = slots.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() )
	    if ( n.tagName() == "slot" ) {
		MetaDataBase::Function function;
		function.specifier = n.attribute( "specifier", "virtual" );
		if ( function.specifier.isEmpty() )
		    function.specifier = "virtual";
		function.access = n.attribute( "access", "public" );
		if ( function.access.isEmpty() )
		    function.access = "public";
		function.language = n.attribute( "language", "C++" );
		function.returnType = n.attribute( "returnType", "void" );
		if ( function.returnType.isEmpty() )
		    function.returnType = "void";
		function.type = "slot";
		function.function = n.firstChild().toText().data();
		if ( !MetaDataBase::hasFunction( formwindow, function.function, TRUE ) )
		    MetaDataBase::addFunction( formwindow, function.function, function.specifier,
					       function.access, "slot", function.language, function.returnType );
		else
		    MetaDataBase::changeFunctionAttributes( formwindow, function.function, function.function,
							    function.specifier, function.access,
							    "slot", function.language,
							    function.returnType );
	    }
    }

    if ( !functions.isNull() ) {
	for ( QDomElement n = functions.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( n.tagName() == "function" ) {
		MetaDataBase::Function function;
		function.specifier = n.attribute( "specifier", "virtual" );
		if ( function.specifier.isEmpty() )
		    function.specifier = "virtual";
		function.access = n.attribute( "access", "public" );
		if ( function.access.isEmpty() )
		    function.access = "public";
		function.type = n.attribute( "type", "function" );
		function.type = "function";
		function.language = n.attribute( "language", "C++" );
		function.returnType = n.attribute( "returnType", "void" );
		if ( function.returnType.isEmpty() )
		    function.returnType = "void";
		function.function = n.firstChild().toText().data();
		if ( !MetaDataBase::hasFunction( formwindow, function.function, TRUE ) )
		    MetaDataBase::addFunction( formwindow, function.function, function.specifier,
					       function.access, function.type, function.language,
					       function.returnType );
		else
		    MetaDataBase::changeFunctionAttributes( formwindow, function.function, function.function,
							    function.specifier, function.access,
							    function.type, function.language, function.returnType );
	    }
	}
    }

    if ( !actions.isNull() )
	loadActions( actions );
    if ( !toolbars.isNull() )
	loadToolBars( toolbars );
    if ( !menubar.isNull() )
	loadMenuBar( menubar );

    if ( !connections.isNull() )
	loadConnections( connections );

    if ( !tabOrder.isNull() )
	loadTabOrder( tabOrder );

    if ( formwindow ) {
	MetaDataBase::setIncludes( formwindow, metaIncludes );
	MetaDataBase::setForwards( formwindow, metaForwards );
	MetaDataBase::setVariables( formwindow, metaVariables );
	MetaDataBase::setSignalList( formwindow, metaSignals );
	metaInfo.classNameChanged = metaInfo.className != QString( formwindow->name() );
	MetaDataBase::setMetaInfo( formwindow, metaInfo );
	MetaDataBase::setExportMacro( formwindow->mainContainer(), exportMacro );
    }

    loadExtraSource( formwindow->formFile(), currFileName, langIface, hasFunctions );

    if ( mainwindow && formwindow )
	mainwindow->insertFormWindow( formwindow );

    if ( formwindow ) {
	formwindow->killAccels( formwindow );
	if ( formwindow->layout() )
	    formwindow->layout()->activate();
	if ( hadGeometry )
	    formwindow->resize( formwindow->size().expandedTo( formwindow->minimumSize().
							       expandedTo( formwindow->minimumSizeHint() ) ) );
	else
	    formwindow->resize( formwindow->size().expandedTo( formwindow->sizeHint() ) );
    }

    return TRUE;
}

static bool saveCode( const QString &filename, const QString &code )
{
    QFile f( filename );
    if ( f.open(IO_WriteOnly | IO_Translate) ) {
	QTextStream ts( &f );
	ts << code;
	return TRUE;
    }
    return FALSE;
}

bool Resource::save( const QString& filename, bool formCodeOnly )
{
    if ( !formwindow || filename.isEmpty() )
	return FALSE;
    if (!langIface) {
	QString lang = "Qt Script";
	if ( mainwindow )
	    lang = mainwindow->currProject()->language();
	langIface = MetaDataBase::languageInterface( lang );
	if ( langIface )
	    langIface->addRef();
    }
    if ( formCodeOnly && langIface ) {
	if ( saveFormCode(formwindow->formFile(), langIface) )
	    return TRUE;
	bool breakout = FALSE;
	FormFile *ff = formwindow->formFile();
	QString codeFile = ff->project()->makeAbsolute( ff->codeFile() );
	QString filter = langIface->fileFilterList().join("\n");
	while ( !breakout ) {
	    QString fn = KFileDialog::getSaveFileName( codeFile, filter );
	    breakout = fn.isEmpty();
	    if ( !breakout ) {
		if ( saveCode(fn, ff->code()) )
		    return TRUE;
	    }
	}
    }
    currFileName = filename;

    QFile f( filename );
    if ( !f.open( IO_WriteOnly | IO_Translate ) )
	return FALSE;
    bool b = save( &f );
    f.close();
    return b;
}

bool Resource::save( QIODevice* dev )
{
    if ( !formwindow )
	return FALSE;

    if ( !langIface ) {
	QString lang = "C++";
	if ( mainwindow )
	    lang = mainwindow->currProject()->language();
	langIface = MetaDataBase::languageInterface( lang );
	if ( langIface )
	    langIface->addRef();
    }

    QTextStream ts( dev );
    ts.setCodec( QTextCodec::codecForName( "UTF-8" ) );

    ts << "<!DOCTYPE UI><UI version=\"3.3\" stdsetdef=\"1\">" << endl;
    saveMetaInfoBefore( ts, 0 );
    saveObject( formwindow->mainContainer(), 0, ts, 0 );
    if ( ::qt_cast<QMainWindow*>(formwindow->mainContainer()) ) {
	saveMenuBar( (QMainWindow*)formwindow->mainContainer(), ts, 0 );
	saveToolBars( (QMainWindow*)formwindow->mainContainer(), ts, 0 );
    }
    if ( !MetaDataBase::customWidgets()->isEmpty() && !usedCustomWidgets.isEmpty() )
	saveCustomWidgets( ts, 0 );
    if ( ::qt_cast<QMainWindow*>(formwindow->mainContainer()) )
	saveActions( formwindow->actionList(), ts, 0 );
    if ( !images.isEmpty() )
	saveImageCollection( ts, 0 );
    if ( !MetaDataBase::connections( formwindow ).isEmpty() ||
	 !MetaDataBase::slotList( formwindow ).isEmpty() )
	saveConnections( ts, 0 );
    saveTabOrder( ts, 0 );
    saveMetaInfoAfter( ts, 0 );
    saveIncludeHints( ts, 0 );
    ts << "</UI>" << endl;
    bool ok = saveFormCode( formwindow->formFile(), langIface );
    images.clear();

    return ok;
}

QString Resource::copy()
{
    if ( !formwindow )
	return QString::null;

    copying = TRUE;
    QString s;
    QTextOStream ts( &s );

    ts << "<!DOCTYPE UI-SELECTION><UI-SELECTION>" << endl;
    QWidgetList widgets = formwindow->selectedWidgets();
    QWidgetList tmp( widgets );
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	QWidget *p = w->parentWidget();
	bool save = TRUE;
	while ( p ) {
	    if ( tmp.findRef( p ) != -1 ) {
		save = FALSE;
		break;
	    }
	    p = p->parentWidget();
	}
	if ( save )
	    saveObject( w, 0, ts, 0 );
    }
    if ( !MetaDataBase::customWidgets()->isEmpty() && !usedCustomWidgets.isEmpty() )
	saveCustomWidgets( ts, 0 );
    if ( !images.isEmpty() )
	saveImageCollection( ts, 0 );
    ts << "</UI-SELECTION>" << endl;

    return s;
}


void Resource::paste( const QString &cb, QWidget *parent )
{
    if ( !formwindow )
	return;
    mainContainerSet = TRUE;

    pasting = TRUE;
    QDomDocument doc;
    QString errMsg;
    int errLine;
    doc.setContent( cb, &errMsg, &errLine );

    QDomElement firstWidget = doc.firstChild().toElement().firstChild().toElement();

    QDomElement imageCollection = firstWidget;
    images.clear();
    while ( imageCollection.tagName() != "images" && !imageCollection.isNull() )
	imageCollection = imageCollection.nextSibling().toElement();

    QDomElement customWidgets = firstWidget;
    while ( customWidgets.tagName() != "customwidgets" && !customWidgets.isNull() )
	customWidgets = customWidgets.nextSibling().toElement();

    if ( !imageCollection.isNull() )
	loadImageCollection( imageCollection );
    if ( !customWidgets.isNull() )
	loadCustomWidgets( customWidgets, this );

    QWidgetList widgets;
    formwindow->clearSelection( FALSE );
    formwindow->setPropertyShowingBlocked( TRUE );
    formwindow->clearSelection( FALSE );
    while ( !firstWidget.isNull() ) {
	if ( firstWidget.tagName() == "widget" ) {
	    QWidget *w = (QWidget*)createObject( firstWidget, parent, 0 );
	    if ( !w )
		continue;
	    widgets.append( w );
	    int x = w->x() + formwindow->grid().x();
	    int y = w->y() + formwindow->grid().y();
	    if ( w->x() + w->width() > parent->width() )
		x = QMAX( 0, parent->width() - w->width() );
	    if ( w->y() + w->height() > parent->height() )
		y = QMAX( 0, parent->height() - w->height() );
	    if ( x != w->x() || y != w->y() )
		w->move( x, y );
	    formwindow->selectWidget( w );
	} else if ( firstWidget.tagName() == "spacer" ) {
	    QWidget *w = createSpacer( firstWidget, parent, 0, firstWidget.tagName() == "vspacer" ? Qt::Vertical : Qt::Horizontal );
	    if ( !w )
		continue;
	    widgets.append( w );
	    int x = w->x() + formwindow->grid().x();
	    int y = w->y() + formwindow->grid().y();
	    if ( w->x() + w->width() > parent->width() )
		x = QMAX( 0, parent->width() - w->width() );
	    if ( w->y() + w->height() > parent->height() )
		y = QMAX( 0, parent->height() - w->height() );
	    if ( x != w->x() || y != w->y() )
		w->move( x, y );
	    formwindow->selectWidget( w );
	}
	firstWidget = firstWidget.nextSibling().toElement();
    }
    formwindow->setPropertyShowingBlocked( FALSE );
    formwindow->emitShowProperties();

    PasteCommand *cmd = new PasteCommand( i18n( "Paste" ), formwindow, widgets );
    formwindow->commandHistory()->addCommand( cmd );
}

void Resource::saveObject( QObject *obj, QDesignerGridLayout* grid, QTextStream &ts, int indent )
{
    if ( obj && obj->isWidgetType() && ( (QWidget*)obj )->isHidden() )
	return;
    QString closeTag;
    const char* className = WidgetFactory::classNameOf( obj );
    int classID = WidgetDatabase::idFromClassName( className );
    bool isPlugin = WidgetDatabase::isCustomPluginWidget( classID );
    if ( obj->isWidgetType() ) {
	if ( obj->isA("CustomWidget") || isPlugin ) {
	    usedCustomWidgets << QString( className );
	    includeHints << WidgetDatabase::includeFile( classID );
	}

	if ( obj != formwindow && !formwindow->widgets()->find( (QWidget*)obj ) )
	    return; // we don't know anything about this thing

	QString attributes;
	if ( grid ) {
	    QDesignerGridLayout::Item item = grid->items[ (QWidget*)obj ];
	    attributes += QString(" row=\"") + QString::number(item.row) + "\"";
	    attributes += QString(" column=\"") + QString::number(item.column) + "\"";
	    if ( item.rowspan * item.colspan != 1 ) {
		attributes += QString(" rowspan=\"") + QString::number(item.rowspan) + "\"";
		attributes += QString(" colspan=\"") + QString::number(item.colspan) + "\"";
	    }
	}

	if ( qstrcmp( className, "Spacer" ) == 0 ) {
	    closeTag = makeIndent( indent ) + "</spacer>\n";
	    ts << makeIndent( indent ) << "<spacer" << attributes << ">" << endl;
	    ++indent;
	} else {
	    closeTag = makeIndent( indent ) + "</widget>\n";
	    ts << makeIndent( indent ) << "<widget class=\"" << className << "\"" << attributes << ">" << endl;
	    ++indent;
	}
	if ( WidgetFactory::hasItems(classID, obj) )
	    saveItems( obj, ts, indent );
	saveObjectProperties( obj, ts, indent );
    } else {
	// test for other objects we created. Nothing so far.
	return;
    }

    QDesignerWidgetStack* ws = 0;

    if ( ::qt_cast<QTabWidget*>(obj) ) {
	QTabWidget* tw = (QTabWidget*) obj;
	QObjectList* tmpl = tw->queryList( "QWidgetStack" );
	QWidgetStack *ws = (QWidgetStack*)tmpl->first();
	QTabBar *tb = ( (QDesignerTabWidget*)obj )->tabBar();
	for ( int i = 0; i < tb->count(); ++i ) {
	    QTab *t = tb->tabAt( i );
	    if ( !t )
		continue;
	    QWidget *w = ws->widget( t->identifier() );
	    if ( !w )
		continue;
	    if ( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf(w) ) == -1 )
		continue; // we don't know this widget
	    ts << makeIndent( indent ) << "<widget class=\"QWidget\">" << endl;
	    ++indent;
	    ts << makeIndent( indent ) << "<property name=\"name\">" << endl;
	    indent++;
	    ts << makeIndent( indent ) << "<cstring>" << entitize( w->name() ) << "</cstring>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</property>" << endl;

	    ts << makeIndent( indent ) << "<attribute name=\"title\">" << endl;
	    indent++;
	    ts << makeIndent( indent ) << "<string>" << entitize( t->text() ) << "</string>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</attribute>" << endl;
	    saveChildrenOf( w, ts, indent );
	    --indent;
	    ts << makeIndent( indent ) << "</widget>" << endl;
	}
	delete tmpl;
    } else if ( (ws = ::qt_cast<QDesignerWidgetStack*>(obj)) != 0 ) {
	for ( int i = 0; i < ws->count(); ++i ) {
	    QWidget *w = ws->page( i );
	    if ( !w )
		continue;
	    if ( WidgetDatabase::idFromClassName(WidgetFactory::classNameOf(w)) == -1 )
		continue; // we don't know this widget
	    ts << makeIndent( indent ) << "<widget class=\"QWidget\">" << endl;
	    ++indent;
	    ts << makeIndent( indent ) << "<property name=\"name\">" << endl;
	    indent++;
	    ts << makeIndent( indent ) << "<cstring>" << entitize( w->name() ) << "</cstring>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</property>" << endl;
	    ts << makeIndent( indent ) << "<attribute name=\"id\">" << endl;
	    indent++;
	    ts << makeIndent( indent ) << "<number>" << QString::number(i) << "</number>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</attribute>" << endl;
	    saveChildrenOf( w, ts, indent );
	    --indent;
	    ts << makeIndent( indent ) << "</widget>" << endl;
	}
    } else if ( ::qt_cast<QToolBox*>(obj) ) {
	QToolBox* tb = (QToolBox*)obj;
	for ( int i = 0; i < tb->count(); ++i ) {
	    QWidget *w = tb->item( i );
	    if ( !w )
		continue;
	    if ( WidgetDatabase::idFromClassName(WidgetFactory::classNameOf(w)) == -1 )
		continue; // we don't know this widget
	    ts << makeIndent( indent ) << "<widget class=\"QWidget\">" << endl;
	    ++indent;
	    ts << makeIndent( indent ) << "<property name=\"name\">" << endl;
	    indent++;
	    ts << makeIndent( indent ) << "<cstring>" << entitize( w->name() ) << "</cstring>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</property>" << endl;
	    ts << makeIndent( indent ) << "<property name=\"backgroundMode\">" << endl;
	    indent++;
	    saveEnumProperty( w, "backgroundMode", QVariant::Invalid, ts, indent );
	    indent--;
	    ts << makeIndent( indent ) << "</property>" << endl;
	    ts << makeIndent( indent ) << "<attribute name=\"label\">" << endl;
	    indent++;
	    ts << makeIndent( indent ) << "<string>" << entitize( tb->itemLabel( tb->indexOf(w) ) ) << "</string>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</attribute>" << endl;
	    saveChildrenOf( w, ts, indent );
	    --indent;
	    ts << makeIndent( indent ) << "</widget>" << endl;
	}
    } else if ( ::qt_cast<QWizard*>(obj) ) {
	QWizard* wiz = (QWizard*)obj;
	for ( int i = 0; i < wiz->pageCount(); ++i ) {
	    QWidget *w = wiz->page( i );
	    if ( !w )
		continue;
	    if ( WidgetDatabase::idFromClassName(WidgetFactory::classNameOf(w)) == -1 )
		continue; // we don't know this widget
	    ts << makeIndent( indent ) << "<widget class=\"QWidget\">" << endl;
	    ++indent;
	    ts << makeIndent( indent ) << "<property name=\"name\">" << endl;
	    indent++;
	    ts << makeIndent( indent ) << "<cstring>" << entitize( w->name() ) << "</cstring>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</property>" << endl;

	    ts << makeIndent( indent ) << "<attribute name=\"title\">" << endl;
	    indent++;
	    ts << makeIndent( indent ) << "<string>" << entitize( wiz->title( w ) ) << "</string>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</attribute>" << endl;
	    saveChildrenOf( w, ts, indent );
	    --indent;
	    ts << makeIndent( indent ) << "</widget>" << endl;
	}
    } else if ( ::qt_cast<QMainWindow*>(obj) ) {
	saveChildrenOf( ( (QMainWindow*)obj )->centralWidget(), ts, indent );
    } else {
	bool saved = FALSE;
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
	if ( isPlugin ) {
	    WidgetInterface *iface = 0;
	    widgetManager()->queryInterface( className, &iface );
	    if ( iface ) {
		QWidgetContainerInterfacePrivate *iface2 = 0;
		iface->queryInterface( IID_QWidgetContainer, (QUnknownInterface**)&iface2 );
		if ( iface2 ) {
		    if ( iface2->supportsPages( className ) )  {
			QWidgetList containers = iface2->pages( className, (QWidget*)obj );
			if ( !containers.isEmpty() ) {
			    saved = TRUE;
			    int i = 0;
			    for ( QWidget *w = containers.first(); w; w = containers.next(), ++i ) {
				if ( WidgetDatabase::
				     idFromClassName( WidgetFactory::classNameOf( w ) ) == -1 )
				    continue; // we don't know this widget
				ts << makeIndent( indent ) << "<widget class=\""
				   << WidgetFactory::classNameOf( w )
				   << "\">" << endl;
				++indent;
				ts << makeIndent( indent ) << "<property name=\"name\">" << endl;
				indent++;
				ts << makeIndent( indent ) << "<cstring>" << entitize( w->name() )
				   << "</cstring>" << endl;
				indent--;
				ts << makeIndent( indent ) << "</property>" << endl;
				ts << makeIndent( indent ) << "<attribute name=\"label\">" << endl;
				indent++;
				ts << makeIndent( indent ) << "<cstring>"
				   << entitize( iface2->pageLabel( className, (QWidget*)obj, i ) )
				   << "</cstring>" << endl;
				indent--;
				ts << makeIndent( indent ) << "</attribute>" << endl;
				saveChildrenOf( w, ts, indent );
				--indent;
				ts << makeIndent( indent ) << "</widget>" << endl;
			    }
			}
		    } else {
			saved = TRUE;
			QWidget *w = iface2->containerOfWidget( className, (QWidget*)obj );
			if ( obj != w ) {
			    ts << makeIndent( indent ) << "<widget class=\""
			       << WidgetFactory::classNameOf( w )
			       << "\">" << endl;
			    ++indent;
			    ts << makeIndent( indent ) << "<property name=\"name\">" << endl;
			    indent++;
			    ts << makeIndent( indent ) << "<cstring>" << entitize( w->name() )
			       << "</cstring>" << endl;
			    indent--;
			    ts << makeIndent( indent ) << "</property>" << endl;
			    saveChildrenOf( w, ts, indent );
			    --indent;
			    ts << makeIndent( indent ) << "</widget>" << endl;
			}
			// Create a custom widget and then store it in the database
			// so we can save the custom widgets.
			MetaDataBase::CustomWidget *cw = new MetaDataBase::CustomWidget;
			cw->className = className;
			cw->includeFile =  WidgetDatabase::includeFile( classID );
			QStrList lst = w->metaObject()->signalNames( TRUE );
			for ( QPtrListIterator<char> it(lst); it.current(); ++it )
			    cw->lstSignals.append(it.current());

			int i;
			int total = w->metaObject()->numProperties( TRUE );
			for ( i = 0; i < total; i++ ) {
			    const QMetaProperty *p = w->metaObject()->property( i, TRUE );
			    if ( p->designable(w) ) {
				MetaDataBase::Property prop;
				prop.property = p->name();
				QString pType = p->type();
				// *sigh* designer types are not normal types
				// Handle most cases, the ones it misses are
				// probably too difficult to deal with anyway...
				if ( pType.startsWith("Q") ) {
				    pType = pType.right( pType.length() - 1 );
				} else {
				    pType[0] = pType[0].upper();
				}
				prop.type = pType;
				cw->lstProperties.append( prop );
			    }
			}

			total = w->metaObject()->numSlots( TRUE );
			for ( i = 0; i < total; i++ ) {
			    const QMetaData *md = w->metaObject()->slot( i, TRUE );
			    MetaDataBase::Function funky;
			    // Find out if we have a return type.
			    if ( md->method->count > 0 ) {
				const QUParameter p = md->method->parameters[0];
				if ( p.inOut == QUParameter::InOut )
				    funky.returnType = p.type->desc();
			    }

			    funky.function = md->name;
			    funky.language = "C++";
			    switch ( md->access ) {
				case QMetaData::Public:
				    funky.access = "public";
				    break;
				case QMetaData::Protected:
				    funky.access = "protected";
				    break;
				case QMetaData::Private:
				    funky.access = "private";
				    break;
			    }
			    cw->lstSlots.append( funky );
			}
			MetaDataBase::addCustomWidget( cw );
		    }
		    iface2->release();
		}
		iface->release();
	    }
	}
#endif // QT_CONTAINER_CUSTOM_WIDGETS
	if ( !saved )
	    saveChildrenOf( obj, ts, indent );
    }

    indent--;
    ts << closeTag;
}

void Resource::saveItems( QObject *obj, QTextStream &ts, int indent )
{
    if ( ::qt_cast<QListBox*>(obj) || ::qt_cast<QComboBox*>(obj) ) {
	QListBox *lb = 0;
	if ( ::qt_cast<QListBox*>(obj) )
	    lb = (QListBox*)obj;
	else
	    lb = ( (QComboBox*)obj )->listBox();

	QListBoxItem *i = lb->firstItem();
	for ( ; i; i = i->next() ) {
	    ts << makeIndent( indent ) << "<item>" << endl;
	    indent++;
	    QStringList text;
	    text << i->text();
	    QPtrList<QPixmap> pixmaps;
	    if ( i->pixmap() )
		pixmaps.append( i->pixmap() );
	    saveItem( text, pixmaps, ts, indent );
	    indent--;
	    ts << makeIndent( indent ) << "</item>" << endl;
	}
    } else if ( ::qt_cast<QIconView*>(obj) ) {
	QIconView *iv = (QIconView*)obj;

	QIconViewItem *i = iv->firstItem();
	for ( ; i; i = i->nextItem() ) {
	    ts << makeIndent( indent ) << "<item>" << endl;
	    indent++;
	    QStringList text;
	    text << i->text();
	    QPtrList<QPixmap> pixmaps;
	    if ( i->pixmap() )
		pixmaps.append( i->pixmap() );
	    saveItem( text, pixmaps, ts, indent );
	    indent--;
	    ts << makeIndent( indent ) << "</item>" << endl;
	}
    } else if ( ::qt_cast<QListView*>(obj) ) {
	QListView *lv = (QListView*)obj;
	int i;
	for ( i = 0; i < lv->header()->count(); ++i ) {
	    ts << makeIndent( indent ) << "<column>" << endl;
	    indent++;
	    QStringList l;
	    l << lv->header()->label( i );
	    QPtrList<QPixmap> pix;
	    pix.setAutoDelete( TRUE );
	    if ( lv->header()->iconSet( i ) )
		pix.append( new QPixmap( lv->header()->iconSet( i )->pixmap() ) );
	    saveItem( l, pix, ts, indent );
	    ts << makeIndent( indent ) << "<property name=\"clickable\">" << endl;
	    indent++;
	    ts << makeIndent( indent ) << "<bool>" << mkBool( lv->header()->isClickEnabled( i ) )<< "</bool>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</property>" << endl;
	    ts << makeIndent( indent ) << "<property name=\"resizable\">" << endl;
	    indent++;
	    ts << makeIndent( indent ) << "<bool>" << mkBool( lv->header()->isResizeEnabled( i ) ) << "</bool>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</property>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</column>" << endl;
	}
	saveItem( lv->firstChild(), ts, indent - 1 );
    }
#if !defined (QT_NO_TABLE)
    else if ( ::qt_cast<QTable*>(obj) ) {
	QTable *table = (QTable*)obj;
	int i;
	QMap<QString, QString> columnFields = MetaDataBase::columnFields( table );
#  ifndef QT_NO_SQL
	bool isDataTable = ::qt_cast<QDataTable*>(table);
#  else
        bool isDataTable = false;
#  endif
	for ( i = 0; i < table->horizontalHeader()->count(); ++i ) {
	    if ( !table->horizontalHeader()->label( i ).isNull() &&
		 table->horizontalHeader()->label( i ).toInt() != i + 1 ||
		 table->horizontalHeader()->iconSet( i ) ||
		  isDataTable ) {
		ts << makeIndent( indent ) << "<column>" << endl;
		indent++;
		QStringList l;
		l << table->horizontalHeader()->label( i );
		QPtrList<QPixmap> pix;
		pix.setAutoDelete( TRUE );
		if ( table->horizontalHeader()->iconSet( i ) )
		    pix.append( new QPixmap( table->horizontalHeader()->iconSet( i )->pixmap() ) );
		saveItem( l, pix, ts, indent );
		if ( isDataTable && !columnFields.isEmpty() ) {
		    ts << makeIndent( indent ) << "<property name=\"field\">" << endl;
		    indent++;
		    ts << makeIndent( indent ) << "<string>" << entitize( *columnFields.find( l[ 0 ] ) ) << "</string>" << endl;
		    indent--;
		    ts << makeIndent( indent ) << "</property>" << endl;
		}
		indent--;
		ts << makeIndent( indent ) << "</column>" << endl;
	    }
	}
	for ( i = 0; i < table->verticalHeader()->count(); ++i ) {
	    if ( !table->verticalHeader()->label( i ).isNull() &&
		 table->verticalHeader()->label( i ).toInt() != i + 1 ||
		 table->verticalHeader()->iconSet( i ) ) {
		ts << makeIndent( indent ) << "<row>" << endl;
		indent++;
		QStringList l;
		l << table->verticalHeader()->label( i );
		QPtrList<QPixmap> pix;
		pix.setAutoDelete( TRUE );
		if ( table->verticalHeader()->iconSet( i ) )
		    pix.append( new QPixmap( table->verticalHeader()->iconSet( i )->pixmap() ) );
		saveItem( l, pix, ts, indent );
		indent--;
		ts << makeIndent( indent ) << "</row>" << endl;
	    }
	}
    }
#endif
}

void Resource::saveItem( QListViewItem *i, QTextStream &ts, int indent )
{
    QListView *lv = i->listView();
    while ( i ) {
	ts << makeIndent( indent ) << "<item>" << endl;
	indent++;

	QPtrList<QPixmap> pixmaps;
	QStringList textes;
	for ( int c = 0; c < lv->columns(); ++c ) {
	    pixmaps.append( i->pixmap( c ) );
	    textes << i->text( c );
	}
	saveItem( textes, pixmaps, ts, indent );

	if ( i->firstChild() )
	    saveItem( i->firstChild(), ts, indent );

	indent--;
	ts << makeIndent( indent ) << "</item>" << endl;
	i = i->nextSibling();
    }
}

void Resource::savePixmap( const QPixmap &p, QTextStream &ts, int indent, const QString &tagname )
{
    if ( p.isNull() ) {
	ts << makeIndent( indent ) << "<" << tagname << "></"  << tagname << ">" << endl;
	return;
    }

    if ( formwindow && formwindow->savePixmapInline() )
	ts << makeIndent( indent ) << "<" << tagname << ">" << saveInCollection( p ) << "</" << tagname << ">" << endl;
    else if ( formwindow && formwindow->savePixmapInProject() )
	ts << makeIndent( indent ) << "<" << tagname << ">" << MetaDataBase::pixmapKey( formwindow, p.serialNumber() )
	   << "</" << tagname << ">" << endl;
    else
	ts << makeIndent( indent ) << "<" << tagname << ">" << MetaDataBase::pixmapArgument( formwindow, p.serialNumber() )
	   << "</" << tagname << ">" << endl;
}

QPixmap Resource::loadPixmap( const QDomElement &e, const QString &/*tagname*/ )
{
    QString arg = e.firstChild().toText().data();

    if ( formwindow && formwindow->savePixmapInline() ) {
	QImage img = loadFromCollection( arg );
	QPixmap pix;
	pix.convertFromImage( img );
	MetaDataBase::setPixmapArgument( formwindow, pix.serialNumber(), arg );
	return pix;
    } else if ( formwindow && formwindow->savePixmapInProject() ) {
	QPixmap pix;
	if ( mainwindow && mainwindow->currProject() ) {
	    pix = mainwindow->currProject()->pixmapCollection()->pixmap( arg );
	} else {
	    pix = BarIcon( "designer_image.png", KDevDesignerPartFactory::instance() );
	    // we have to force the pixmap to get a new and unique serial number. Unfortunately detatch() doesn't do that
	    pix.convertFromImage( pix.convertToImage() );
	}

	MetaDataBase::setPixmapKey( formwindow, pix.serialNumber(), arg );
	return pix;
    }
    QPixmap pix = BarIcon( "designer_image.png", KDevDesignerPartFactory::instance() );
    // we have to force the pixmap to get a new and unique serial number. Unfortunately detatch() doesn't do that
    pix.convertFromImage( pix.convertToImage() );
    MetaDataBase::setPixmapArgument( formwindow, pix.serialNumber(), arg );
    return pix;
}

void Resource::saveItem( const QStringList &text,
			 const QPtrList<QPixmap> &pixmaps, QTextStream &ts,
			 int indent )
{
    QStringList::ConstIterator it = text.begin();
    for ( ; it != text.end(); ++it ) {
	ts << makeIndent( indent ) << "<property name=\"text\">" << endl;
	indent++;
	ts << makeIndent( indent ) << "<string>" << entitize( *it ) << "</string>" << endl;
	indent--;
	ts << makeIndent( indent ) << "</property>" << endl;
    }

    for ( int i = 0; i < (int)pixmaps.count(); ++i ) {
	QPixmap *p = ( (QPtrList<QPixmap>)pixmaps ).at( i );
	ts << makeIndent( indent ) << "<property name=\"pixmap\">" << endl;
	indent++;
	if ( p )
	    savePixmap( *p, ts, indent );
	else
	    savePixmap( QPixmap(), ts, indent );
	indent--;
	ts << makeIndent( indent ) << "</property>" << endl;
    }
}

void Resource::saveChildrenOf( QObject* obj, QTextStream &ts, int indent )
{
    const QObjectList *l = obj->children();
    if ( !l )
	return; // no children to save

    QString closeTag;
    // if the widget has a layout we pretend that all widget's childs are childs of the layout - makes the structure nicer
    QLayout *layout = 0;
    QDesignerGridLayout* grid = 0;
    if ( !::qt_cast<QSplitter*>(obj) &&
	 WidgetDatabase::isContainer( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( obj ) ) ) &&
	 obj->isWidgetType() &&
	 WidgetFactory::layoutType( (QWidget*)obj, layout ) != WidgetFactory::NoLayout ) {
	WidgetFactory::LayoutType lay = WidgetFactory::layoutType( (QWidget*)obj, layout );
	switch ( lay ) {
	case WidgetFactory::HBox:
	    closeTag = makeIndent( indent ) + "</hbox>";
	    ts << makeIndent( indent ) << "<hbox>" << endl;
	    ++indent;
	    break;
	case WidgetFactory::VBox:
	    closeTag = makeIndent( indent ) + "</vbox>";
	    ts << makeIndent( indent ) << "<vbox>" << endl;
	    ++indent;
	    break;
	case WidgetFactory::Grid:
	    closeTag = makeIndent( indent ) + "</grid>";
	    ts << makeIndent( indent ) << "<grid>" << endl;
	    ++indent;
	    grid = (QDesignerGridLayout*) layout;
	    break;
	default:
	    break;
	}

	// save properties of layout
	if ( lay != WidgetFactory::NoLayout )
	    saveObjectProperties( layout, ts, indent );

    }

    QObject *o = 0;
    for ( QPtrListIterator<QObject> it ( *l ); ( o = it.current() ); ++it )
	if ( !QString( o->name() ).startsWith( "qt_dead_widget_" ) )
	    saveObject( o, grid, ts, indent );
    if ( !closeTag.isEmpty() ) {
	indent--;
	ts << closeTag << endl;
    }
}

void Resource::saveObjectProperties( QObject *w, QTextStream &ts, int indent )
{
    QStringList saved;
    QStringList changed;
    changed = MetaDataBase::changedProperties( w );
    if ( w->isWidgetType() ) {
	if ( ::qt_cast<Spacer*>(w) ) {
	    if ( !changed.contains( "sizeHint" ) )
		changed << "sizeHint";
	    if ( !changed.contains( "geometry" ) )
		changed << "geometry";
 	} else {
	    QToolButton *tb = ::qt_cast<QToolButton*>(w);
	    if ( tb && !tb->iconSet().isNull() ) {
		changed << "iconSet";
	    }
	}
    } else if ( ::qt_cast<QLayout*>(w) ) {
	if ( MetaDataBase::spacing( WidgetFactory::containerOfWidget( WidgetFactory::layoutParent( (QLayout*)w ) ) ) > -1 )
	    changed << "spacing";
	if ( MetaDataBase::margin( WidgetFactory::containerOfWidget( WidgetFactory::layoutParent( (QLayout*)w ) ) ) > -1 )
	    changed << "margin";
	if ( MetaDataBase::resizeMode( WidgetFactory::containerOfWidget( WidgetFactory::layoutParent( (QLayout*)w ) ) ) != "Auto"
	     && !MetaDataBase::resizeMode( WidgetFactory::containerOfWidget( WidgetFactory::layoutParent( (QLayout*)w ) ) ).isEmpty() )
	    changed << "resizeMode";
    }

    if ( w == formwindow->mainContainer() ) {
	if ( changed.findIndex( "geometry" ) == -1 )
	    changed << "geometry";
	if ( changed.findIndex( "caption" ) == -1 )
	    changed << "caption";
    }

    if ( changed.isEmpty() )
	    return;

    bool inLayout = w != formwindow->mainContainer() && !copying && w->isWidgetType() && ( (QWidget*)w )->parentWidget() &&
		    WidgetFactory::layoutType( ( (QWidget*)w )->parentWidget() ) != WidgetFactory::NoLayout;

    QStrList lst = w->metaObject()->propertyNames( !::qt_cast<Spacer*>(w) );
    for ( QPtrListIterator<char> it( lst ); it.current(); ++it ) {
	if ( changed.find( QString::fromLatin1( it.current() ) ) == changed.end() )
	    continue;
	if ( saved.find( QString::fromLatin1( it.current() ) ) != saved.end() )
	    continue;
	saved << QString::fromLatin1( it.current() );
	const QMetaProperty* p = w->metaObject()->
				 property( w->metaObject()->findProperty( it.current(), TRUE ), TRUE );
	if ( !p || !p->stored( w ) || ( inLayout && qstrcmp( p->name(), "geometry" ) == 0 ) )
	    continue;
	if ( ::qt_cast<QLabel*>(w) && qstrcmp( p->name(), "pixmap" ) == 0 &&
	     ( !( (QLabel*)w )->pixmap() || ( (QLabel*)w )->pixmap()->isNull() ) )
	    continue;
	if ( ::qt_cast<MenuBarEditor*>(w) &&
	     ( qstrcmp( p->name(), "itemName" ) == 0 || qstrcmp( p->name(), "itemNumber" ) == 0 ||
	       qstrcmp( p->name(), "itemText" ) == 0 ) )
	    continue;
	if ( qstrcmp( p->name(), "name" ) == 0 )
	    knownNames << w->property( "name" ).toString();
	if ( !p->isSetType() && !p->isEnumType() && !w->property( p->name() ).isValid() )
	    continue;
	ts << makeIndent( indent ) << "<property";
	ts << " name=\"" << it.current() << "\"";
	if ( !p->stdSet() )
	    ts << " stdset=\"0\"";
	ts << ">" << endl;
	indent++;
	if ( strcmp( it.current(), "resizeMode" ) == 0 && ::qt_cast<QLayout*>(w) ) {
	    saveProperty( w, it.current(), "", QVariant::String, ts, indent );
	} else if ( p->isSetType() ) {
	    saveSetProperty( w, it.current(), QVariant::nameToType( p->type() ), ts, indent );
	} else if ( p->isEnumType() ) {
	    saveEnumProperty( w, it.current(), QVariant::nameToType( p->type() ), ts, indent );
	} else {
	    saveProperty( w, it.current(), w->property( p->name() ), QVariant::nameToType( p->type() ), ts, indent );
	}
	indent--;
	ts << makeIndent( indent ) << "</property>" << endl;
    }

    if ( w->isWidgetType() && MetaDataBase::fakeProperties( w ) ) {
	QMap<QString, QVariant>* fakeProperties = MetaDataBase::fakeProperties( w );
	for ( QMap<QString, QVariant>::Iterator fake = fakeProperties->begin();
	      fake != fakeProperties->end(); ++fake ) {
	    if ( MetaDataBase::isPropertyChanged( w, fake.key() ) ) {
		if ( w->inherits("CustomWidget") ) {
		    MetaDataBase::CustomWidget *cw = ( (CustomWidget*)w )->customWidget();
		    if ( cw && !cw->hasProperty( fake.key().latin1() ) && fake.key() != "toolTip" && fake.key() != "whatsThis" )
			continue;
		}

		ts << makeIndent( indent ) << "<property name=\"" << fake.key() << "\" stdset=\"0\">" << endl;
		indent++;
		saveProperty( w, fake.key(), *fake, (*fake).type(), ts, indent );
		indent--;
		ts << makeIndent( indent ) << "</property>" << endl;
	    }
	}
    }
}

void Resource::saveSetProperty( QObject *w, const QString &name, QVariant::Type, QTextStream &ts, int indent )
{
    const QMetaProperty *p = w->metaObject()->property( w->metaObject()->findProperty( name, TRUE ), TRUE );
    QStrList l( p->valueToKeys( w->property( name ).toInt() ) );
    QString v;
    for ( uint i = 0; i < l.count(); ++i ) {
	v += l.at( i );
	if ( i < l.count() - 1 )
	    v += "|";
    }
    ts << makeIndent( indent ) << "<set>" << v << "</set>" << endl;
}

void Resource::saveEnumProperty( QObject *w, const QString &name, QVariant::Type, QTextStream &ts, int indent )
{
    const QMetaProperty *p = w->metaObject()->property( w->metaObject()->findProperty( name, TRUE ), TRUE );
    ts << makeIndent( indent ) << "<enum>" << p->valueToKey( w->property( name ).toInt() ) << "</enum>" << endl;
}

void Resource::saveProperty( QObject *w, const QString &name, const QVariant &value, QVariant::Type t, QTextStream &ts, int indent )
{
    if ( name == "hAlign" || name =="vAlign" || name == "wordwrap" ||
	 name == "layoutMargin" || name =="layoutSpacing" )
	return;
    int num;
    uint unum;
    double dob;
    QString comment;
    if ( w && formwindow->widgets()->find( (QWidget*)w ) || formwindow->actionList().find( (QAction*)w ) )
	comment = MetaDataBase::propertyComment( w, name );
    switch ( t ) {
    case QVariant::String:
	if ( name == "resizeMode" ) {
	    QString resmod = MetaDataBase::resizeMode( WidgetFactory::containerOfWidget( WidgetFactory::layoutParent( (QLayout*)w ) ) );
	    if ( !resmod.isNull() && resmod != "Auto" ) {
		ts << makeIndent( indent ) << "<enum>";
		ts << resmod;
		ts << "</enum>" << endl;
	    }

	} else {
	    ts << makeIndent( indent ) << "<string>" << entitize( value.toString() ) << "</string>" << endl;
	    if ( !comment.isEmpty() )
		ts << makeIndent( indent ) << "<comment>" << entitize( comment ) << "</comment>" << endl;
	}
	break;
    case QVariant::CString:
	ts << makeIndent( indent ) << "<cstring>" << entitize( value.toCString() ).latin1() << "</cstring>" << endl;
	break;
    case QVariant::Bool:
	ts << makeIndent( indent ) << "<bool>" << mkBool( value.toBool() ) << "</bool>" << endl;
	break;
    case QVariant::Int:
	if ( ::qt_cast<QLayout*>(w) ) {
	    num = -1;
	    if ( name == "spacing" )
		num = MetaDataBase::spacing( WidgetFactory::containerOfWidget( WidgetFactory::layoutParent( (QLayout*)w ) ) );
	    else if ( name == "margin" )
		num = MetaDataBase::margin( WidgetFactory::containerOfWidget( WidgetFactory::layoutParent( (QLayout*)w ) ) );
	    if ( num != -1 )
		ts << makeIndent( indent ) << "<number>" << QString::number( num ) << "</number>" << endl;
	} else {
	    num = value.toInt();
	    ts << makeIndent( indent ) << "<number>" << QString::number( num ) << "</number>" << endl;
	}
	break;
    case QVariant::Double:
	dob = value.toDouble();
	ts << makeIndent( indent ) << "<number>" << QString::number( dob ) << "</number>" << endl;
	break;
    case QVariant::KeySequence:
	ts << makeIndent( indent ) << "<string>"
	   << entitize(platformNeutralKeySequence(value.toKeySequence())) << "</string>" << endl;
	break;
    case QVariant::UInt:
	unum = value.toUInt();
	ts << makeIndent( indent ) << "<number>" << QString::number( unum ) << "</number>" << endl;
	break;
    case QVariant::Rect: {
	QVariant v( value );
	ts << makeIndent( indent ) << "<rect>" << endl;
	indent++;
	ts << makeIndent( indent ) << "<x>" << QString::number( v.toRect().x() ) << "</x>" << endl;
	ts << makeIndent( indent ) << "<y>" << QString::number( v.toRect().y() ) << "</y>" << endl;
	ts << makeIndent( indent ) << "<width>" << QString::number( v.toRect().width() ) << "</width>" << endl;
	ts << makeIndent( indent ) << "<height>" << QString::number( v.toRect().height() ) << "</height>" << endl;
	indent--;
	ts << makeIndent( indent ) << "</rect>" << endl;
    } break;
    case QVariant::Point: {
	QVariant v( value );
	ts << makeIndent( indent ) << "<point>" << endl;
	indent++;
	ts << makeIndent( indent ) << "<x>" << QString::number( v.toPoint().x() ) << "</x>" << endl;
	ts << makeIndent( indent ) << "<y>" << QString::number( v.toPoint().y() ) << "</y>" << endl;
	indent--;
	ts << makeIndent( indent ) << "</point>" << endl;
    } break;
    case QVariant::Size: {
	QVariant v( value );
	ts << makeIndent( indent ) << "<size>" << endl;
	indent++;
	ts << makeIndent( indent ) << "<width>" << QString::number( v.toSize().width() ) << "</width>" << endl;
	ts << makeIndent( indent ) << "<height>" << QString::number( v.toSize().height() ) << "</height>" << endl;
	indent--;
	ts << makeIndent( indent ) << "</size>" << endl;
    } break;
    case QVariant::Color: {
	QVariant v( value );
	ts << makeIndent( indent ) << "<color>" << endl;
	indent++;
	saveColor( ts, indent, v.toColor() );
	indent--;
	ts << makeIndent( indent ) << "</color>" << endl;
    } break;
    case QVariant::Font: {
	QVariant v( value );
	ts << makeIndent( indent ) << "<font>" << endl;
	QFont f( qApp->font() );
	if ( w && w->isWidgetType() && ((QWidget*)w)->parentWidget() )
	    f = ((QWidget*)w)->parentWidget()->font();
	QFont f2( v.toFont() );
	indent++;
	if ( f.family() != f2.family() )
	    ts << makeIndent( indent ) << "<family>" << f2.family() << "</family>" << endl;
	if ( f.pointSize() != f2.pointSize() )
	    ts << makeIndent( indent ) << "<pointsize>" << QString::number( f2.pointSize() ) << "</pointsize>" << endl;
	if ( f.bold() != f2.bold() )
	    ts << makeIndent( indent ) << "<bold>" << QString::number( (int)f2.bold() ) << "</bold>" << endl;
	if ( f.italic() != f2.italic() )
	    ts << makeIndent( indent ) << "<italic>" << QString::number( (int)f2.italic() ) << "</italic>" << endl;
	if ( f.underline() != f2.underline() )
	    ts << makeIndent( indent ) << "<underline>" << QString::number( (int)f2.underline() ) << "</underline>" << endl;
	if ( f.strikeOut() != f2.strikeOut() )
	    ts << makeIndent( indent ) << "<strikeout>" << QString::number( (int)f2.strikeOut() ) << "</strikeout>" << endl;
	indent--;
	ts << makeIndent( indent ) << "</font>" << endl;
    } break;
    case QVariant::SizePolicy: {
	QSizePolicy sp( value.toSizePolicy() );
	ts << makeIndent( indent ) << "<sizepolicy>" << endl;
	indent++;
	ts << makeIndent( indent ) << "<hsizetype>" << (int)sp.horData() << "</hsizetype>" << endl;
	ts << makeIndent( indent ) << "<vsizetype>" << (int)sp.verData() << "</vsizetype>" << endl;
	ts << makeIndent( indent ) << "<horstretch>" << (int)sp.horStretch() << "</horstretch>" << endl;
	ts << makeIndent( indent ) << "<verstretch>" << (int)sp.verStretch() << "</verstretch>" << endl;
	indent--;
	ts << makeIndent( indent ) << "</sizepolicy>" << endl;
	break;
    }
    case QVariant::Pixmap:
	savePixmap( value.toPixmap(), ts, indent );
	break;
    case QVariant::IconSet:
	savePixmap( value.toIconSet().pixmap(), ts, indent, "iconset" );
	break;
    case QVariant::Image:
	ts << makeIndent( indent ) << "<image>" << saveInCollection( value.toImage() ) << "</image>" << endl;
    break;
    case QVariant::Palette: {
	QPalette p( value.toPalette() );
	ts << makeIndent( indent ) << "<palette>" << endl;
	indent++;

	ts << makeIndent( indent ) << "<active>" << endl;
	indent++;
	saveColorGroup( ts, indent, p.active() );
	indent--;
	ts << makeIndent( indent ) << "</active>" << endl;

	ts << makeIndent( indent ) << "<disabled>" << endl;
	indent++;
	saveColorGroup( ts, indent, p.disabled() );
	indent--;
	ts << makeIndent( indent ) << "</disabled>" << endl;

	ts << makeIndent( indent ) << "<inactive>" << endl;
	indent++;
	saveColorGroup( ts, indent, p.inactive() );
	indent--;
	ts << makeIndent( indent ) << "</inactive>" << endl;

	indent--;
	ts << makeIndent( indent ) << "</palette>" << endl;
    } break;
    case QVariant::Cursor:
	ts << makeIndent( indent ) << "<cursor>" << value.toCursor().shape() << "</cursor>" << endl;
	break;
    case QVariant::StringList: {
	QStringList lst = value.toStringList();
	uint i = 0;
	ts << makeIndent( indent ) << "<stringlist>" << endl;
	indent++;
	if ( !lst.isEmpty() ) {
	    for ( i = 0; i < lst.count(); ++i )
		ts << makeIndent( indent ) << "<string>" << entitize( lst[ i ] ) << "</string>" << endl;
	}
	indent--;
	ts << makeIndent( indent ) << "</stringlist>" << endl;
    } break;
    case QVariant::Date: {
	QDate d = value.toDate();
	ts << makeIndent( indent ) << "<date>" << endl;
	indent++;
	ts << makeIndent( indent ) << "<year>" << d.year() << "</year>"  << endl;
	ts << makeIndent( indent ) << "<month>" << d.month() << "</month>"  << endl;
	ts << makeIndent( indent ) << "<day>" << d.day() << "</day>"  << endl;
	indent--;
	ts << makeIndent( indent ) << "</date>" << endl;
	break;
    }
    case QVariant::Time: {
	QTime t = value.toTime();
	ts << makeIndent( indent ) << "<time>" << endl;
	indent++;
	ts << makeIndent( indent ) << "<hour>" << t.hour() << "</hour>"  << endl;
	ts << makeIndent( indent ) << "<minute>" << t.minute() << "</minute>"  << endl;
	ts << makeIndent( indent ) << "<second>" << t.second() << "</second>"  << endl;
	indent--;
	ts << makeIndent( indent ) << "</time>" << endl;
	break;
    }
    case QVariant::DateTime: {
	QDateTime dt = value.toDateTime();
	ts << makeIndent( indent ) << "<datetime>" << endl;
	indent++;
	ts << makeIndent( indent ) << "<year>" << dt.date().year() << "</year>"  << endl;
	ts << makeIndent( indent ) << "<month>" << dt.date().month() << "</month>"  << endl;
	ts << makeIndent( indent ) << "<day>" << dt.date().day() << "</day>"  << endl;
	ts << makeIndent( indent ) << "<hour>" << dt.time().hour() << "</hour>"  << endl;
	ts << makeIndent( indent ) << "<minute>" << dt.time().minute() << "</minute>"  << endl;
	ts << makeIndent( indent ) << "<second>" << dt.time().second() << "</second>"  << endl;
	indent--;
	ts << makeIndent( indent ) << "</datetime>" << endl;
	break;
    }
    default:
	qWarning( "saving the property %s of type %d not supported yet", name.latin1(), (int)t );
    }
}

void Resource::saveColorGroup( QTextStream &ts, int indent, const QColorGroup &cg )
{
    for( int r = 0 ; r < QColorGroup::NColorRoles ; r++ ) {
	ts << makeIndent( indent ) << "<color>" << endl;
	indent++;
	saveColor( ts, indent, cg.color( (QColorGroup::ColorRole)r ) );
	indent--;
	ts << makeIndent( indent ) << "</color>" << endl;
	QPixmap* pm = cg.brush( (QColorGroup::ColorRole)r ).pixmap();
	if ( pm && !pm->isNull() )
	    savePixmap( *pm, ts, indent );
    }
}

void Resource::saveColor( QTextStream &ts, int indent, const QColor &c )
{
    ts << makeIndent( indent ) << "<red>" << QString::number( c.red() ) << "</red>" << endl;
    ts << makeIndent( indent ) << "<green>" << QString::number( c.green() ) << "</green>" << endl;
    ts << makeIndent( indent ) << "<blue>" << QString::number( c.blue() ) << "</blue>" << endl;
}

QObject *Resource::createObject( const QDomElement &e, QWidget *parent, QLayout* layout )
{
    lastItem = 0;
    QDomElement n = e.firstChild().toElement();
    QWidget *w = 0; // the widget that got created
    QObject *obj = 0; // gets the properties

    int row = e.attribute( "row" ).toInt();
    int col = e.attribute( "column" ).toInt();
    int rowspan = e.attribute( "rowspan" ).toInt();
    int colspan = e.attribute( "colspan" ).toInt();
    if ( rowspan < 1 )
	rowspan = 1;
    if ( colspan < 1 )
	colspan = 1;

    QString className = e.attribute( "class", "QWidget" );
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
    QString parentClassName = WidgetFactory::classNameOf( parent );
    bool isPlugin =
	WidgetDatabase::isCustomPluginWidget( WidgetDatabase::idFromClassName( parentClassName ) );
    if ( isPlugin )
	qWarning( "####### loading custom container widgets without page support not implemented!" );
    // ### TODO loading for custom container widgets without pages
#endif
    if ( !className.isNull() ) {
	obj = WidgetFactory::create( WidgetDatabase::idFromClassName( className ), parent, 0, FALSE );
	if ( !obj ) {
	    QMessageBox::critical( MainWindow::self, i18n( "Loading File" ),
				   i18n( "Error loading %1.\n"
						   "The widget %2 could not be created." ).
				   arg( currFileName ).arg( className ) );
	    return 0;
	}
	if ( !mainContainerSet ) {
	    if ( formwindow )
		formwindow->setMainContainer( (QWidget*)obj );
	    mainContainerSet = TRUE;
	}
	w = (QWidget*)obj;
	if ( ::qt_cast<QMainWindow*>(w) )
	    w = ( (QMainWindow*)w )->centralWidget();
	if ( layout ) {
	    switch ( WidgetFactory::layoutType( layout ) ) {
	    case WidgetFactory::HBox:
		( (QHBoxLayout*)layout )->addWidget( w );
		break;
	    case WidgetFactory::VBox:
		( (QVBoxLayout*)layout )->addWidget( w );
		break;
	    case WidgetFactory::Grid:
		( (QDesignerGridLayout*)layout )->addMultiCellWidget( w, row, row + rowspan - 1,
								      col, col + colspan - 1 );
		break;
	    default:
		break;
	    }
	}

	if ( !toplevel )
	    toplevel = w;
	layout = 0;

	if ( w && formwindow ) {
	    if ( !parent ||
		 ( !::qt_cast<QTabWidget*>(parent) &&
		   !::qt_cast<QWidgetStack*>(parent) &&
		   !::qt_cast<QToolBox*>(parent) &&
		   !::qt_cast<QWizard*>(parent)
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
                   && !isPlugin
#endif
                     ) )
		formwindow->insertWidget( w, pasting );
	    else if ( parent &&
		      ( ::qt_cast<QTabWidget*>(parent) ||
			::qt_cast<QWidgetStack*>(parent) ||
			::qt_cast<QToolBox*>(parent) ||
			::qt_cast<QWizard*>(parent)
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
                        || isPlugin
#endif
                          ) )
		MetaDataBase::addEntry( w );
	}
    }

    QDomElement sizePolicyElement;
    QObject *sizePolicyObject = 0;

    while ( !n.isNull() ) {
	if ( n.tagName() == "spacer" ) {
	    createSpacer( n, w, layout, Qt::Horizontal );
	} else if ( n.tagName() == "widget" ) {
	    createObject( n, w, layout );
	} else if ( n.tagName() == "hbox" ) {
	    layout = WidgetFactory::createLayout( w, layout, WidgetFactory::HBox );
	    obj = layout;
	    n = n.firstChild().toElement();
	    continue;
	} else if ( n.tagName() == "grid" ) {
	    layout = WidgetFactory::createLayout( w, layout, WidgetFactory::Grid );
	    obj = layout;
	    n = n.firstChild().toElement();
	    continue;
	} else if ( n.tagName() == "vbox" ) {
	    layout = WidgetFactory::createLayout( w, layout, WidgetFactory::VBox );
	    obj = layout;
	    n = n.firstChild().toElement();
	    continue;
	} else if ( n.tagName() == "property" && obj ) {
	    if ( n.attribute( "name" ) == "sizePolicy" ) {
		// ### Evil hack ### Delay setting sizePolicy so it won't be overridden by other properties.
		sizePolicyElement = n;
		sizePolicyObject = obj;
	    } else {
		setObjectProperty( obj, n.attribute( "name" ), n.firstChild().toElement() );
	    }
	} else if ( n.tagName() == "attribute" && w ) {
	    QString attrib = n.attribute( "name" );
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( ::qt_cast<QTabWidget*>(parent) ) {
		if ( attrib == "title" )
		    ( (QTabWidget*)parent )->insertTab( w, v.toString() );
	    } else if ( ::qt_cast<QWidgetStack*>(parent) ) {
		if ( attrib == "id" )
		    ( (QDesignerWidgetStack*)parent )->insertPage( w, v.toInt() );
	    } else if ( ::qt_cast<QToolBox*>(parent) ) {
		if ( attrib == "label" )
		    ( (QToolBox*)parent )->addItem( w, v.toString() );
	    } else if ( ::qt_cast<QWizard*>(parent) ) {
		if ( attrib == "title" )
		    ( (QWizard*)parent )->addPage( w, v.toString() );
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
	    } else if ( isPlugin ) {
		if ( attrib == "label" ) {
		    WidgetInterface *iface = 0;
		    widgetManager()->queryInterface( parentClassName, &iface );
		    if ( iface ) {
			QWidgetContainerInterfacePrivate *iface2 = 0;
			iface->queryInterface( IID_QWidgetContainer, (QUnknownInterface**)&iface2 );
			if ( iface2 ) {
			    iface2->insertPage( parentClassName,
						(QWidget*)parent, v.toString(), -1, w );
			    iface2->release();
			}
			iface->release();
		    }
		}
#endif // QT_CONTAINER_CUSTOM_WIDGETS
	    }
	} else if ( n.tagName() == "item" ) {
	    createItem( n, w );
	} else if ( n.tagName() == "column" || n.tagName() =="row" ) {
	    createColumn( n, w );
	}

	n = n.nextSibling().toElement();
    }

    // ### Evil hack ### See description above.
    if ( !sizePolicyElement.isNull() ) {
	setObjectProperty( sizePolicyObject,
			   sizePolicyElement.attribute( "name" ),
			   sizePolicyElement.firstChild().toElement() );
    }

    if ( w->isWidgetType() )
	widgets.insert( w->name(), w );

    return w;
}

void Resource::createColumn( const QDomElement &e, QWidget *widget )
{
    if ( !widget )
	return;

    if ( ::qt_cast<QListView*>(widget) && e.tagName() == "column" ) {
	QListView *lv = (QListView*)widget;
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	bool clickable = TRUE, resizable = TRUE;
	while ( !n.isNull() ) {
	    if ( n.tagName() == "property" ) {
		QString attrib = n.attribute( "name" );
		QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
		if ( attrib == "text" )
		    txt = v.toString();
		else if ( attrib == "pixmap" ) {
		    pix = loadPixmap( n.firstChild().toElement().toElement() );
		    hasPixmap = !pix.isNull();
		} else if ( attrib == "clickable" )
		    clickable = v.toBool();
		else if ( attrib == "resizable" )
		    resizable = v.toBool();
	    }
	    n = n.nextSibling().toElement();
	}
	lv->addColumn( txt );
	int i = lv->header()->count() - 1;
	if ( hasPixmap ) {
	    lv->header()->setLabel( i, pix, txt );
	}
	if ( !clickable )
	    lv->header()->setClickEnabled( clickable, i );
	if ( !resizable )
	    lv->header()->setResizeEnabled( resizable, i );
    }
#ifndef QT_NO_TABLE
    else if ( ::qt_cast<QTable*>(widget) ) {
	QTable *table = (QTable*)widget;
	bool isRow;
	if ( ( isRow = e.tagName() == "row" ) )
	    table->setNumRows( table->numRows() + 1 );
	else
	    table->setNumCols( table->numCols() + 1 );

	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	QString field;
	QMap<QString, QString> fieldMap = MetaDataBase::columnFields( table );
	while ( !n.isNull() ) {
	    if ( n.tagName() == "property" ) {
		QString attrib = n.attribute( "name" );
		QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
		if ( attrib == "text" )
		    txt = v.toString();
		else if ( attrib == "pixmap" ) {
		    hasPixmap = !n.firstChild().firstChild().toText().data().isEmpty();
		    if ( hasPixmap )
			pix = loadPixmap( n.firstChild().toElement() );
		} else if ( attrib == "field" )
		    field = v.toString();
	    }
	    n = n.nextSibling().toElement();
	}

	int i = isRow ? table->numRows() - 1 : table->numCols() - 1;
	QHeader *h = !isRow ? table->horizontalHeader() : table->verticalHeader();
	if ( hasPixmap )
	    h->setLabel( i, pix, txt );
	else
	    h->setLabel( i, txt );
	if ( !isRow && !field.isEmpty() )
	    fieldMap.insert( txt, field );
	MetaDataBase::setColumnFields( table, fieldMap );
    }
#endif
}

void Resource::loadItem( const QDomElement &e, QPixmap &pix, QString &txt, bool &hasPixmap )
{
    QDomElement n = e;
    hasPixmap = FALSE;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute( "name" );
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" )
		txt = v.toString();
	    else if ( attrib == "pixmap" ) {
		pix = loadPixmap( n.firstChild().toElement() );
		hasPixmap = !pix.isNull();
	    }
	}
	n = n.nextSibling().toElement();
    }
}

void Resource::createItem( const QDomElement &e, QWidget *widget, QListViewItem *i )
{
    if ( !widget || !WidgetFactory::hasItems( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( widget ) ), widget ) )
	return;

    if ( ::qt_cast<QListBox*>(widget) || ::qt_cast<QComboBox*>(widget) ) {
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	loadItem( n, pix, txt, hasPixmap );
	QListBox *lb = 0;
	if ( ::qt_cast<QListBox*>(widget) )
	    lb = (QListBox*)widget;
	else
	    lb = ( (QComboBox*)widget)->listBox();
	if ( hasPixmap ) {
	    new QListBoxPixmap( lb, pix, txt );
	} else {
	    new QListBoxText( lb, txt );
	}
    } else if ( ::qt_cast<QIconView*>(widget) ) {
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	loadItem( n, pix, txt, hasPixmap );
	QIconView *iv = (QIconView*)widget;
	if ( hasPixmap )
	    new QIconViewItem( iv, txt, pix );
	else
	    new QIconViewItem( iv, txt );
    } else if ( ::qt_cast<QListView*>(widget) ) {
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	QValueList<QPixmap> pixmaps;
	QStringList textes;
	QListViewItem *item = 0;
	QListView *lv = (QListView*)widget;
	if ( i )
	    item = new QListViewItem( i, lastItem );
	else
	    item = new QListViewItem( lv, lastItem );
	while ( !n.isNull() ) {
	    if ( n.tagName() == "property" ) {
		QString attrib = n.attribute( "name" );
		QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
		if ( attrib == "text" )
		    textes << v.toString();
		else if ( attrib == "pixmap" ) {
		    QString s = v.toString();
		    if ( s.isEmpty() ) {
			pixmaps << QPixmap();
		    } else {
			pix = loadPixmap( n.firstChild().toElement() );
			pixmaps << pix;
		    }
		}
	    } else if ( n.tagName() == "item" ) {
		item->setOpen( TRUE );
		createItem( n, widget, item );
	    }

	    n = n.nextSibling().toElement();
	}

	for ( int i = 0; i < lv->columns(); ++i ) {
	    item->setText( i, textes[ i ] );
	    item->setPixmap( i, pixmaps[ i ] );
	}
	lastItem = item;
    }
}

QWidget *Resource::createSpacer( const QDomElement &e, QWidget *parent, QLayout *layout, Qt::Orientation o )
{
    QDomElement n = e.firstChild().toElement();
    int row = e.attribute( "row" ).toInt();
    int col = e.attribute( "column" ).toInt();
    int rowspan = e.attribute( "rowspan" ).toInt();
    int colspan = e.attribute( "colspan" ).toInt();
    if ( rowspan < 1 )
	rowspan = 1;
    if ( colspan < 1 )
	colspan = 1;

    Spacer *spacer = (Spacer*) WidgetFactory::create( WidgetDatabase::idFromClassName("Spacer"),
						      parent, "spacer", FALSE);
    spacer->setOrientation( o );
    spacer->setInteraciveMode( FALSE );
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" )
	    setObjectProperty( spacer, n.attribute( "name" ), n.firstChild().toElement() );
	n = n.nextSibling().toElement();
    }
    spacer->setInteraciveMode( TRUE );
    if ( formwindow )
	formwindow->insertWidget( spacer, pasting );
    if ( layout ) {
	if ( ::qt_cast<QBoxLayout*>(layout) )
	    ( (QBoxLayout*)layout )->addWidget( spacer, 0, spacer->alignment() );
	else
	    ( (QDesignerGridLayout*)layout )->addMultiCellWidget( spacer, row, row + rowspan - 1, col, col + colspan - 1,
								  spacer->alignment() );
    }
    return spacer;
}

/*!
  Attention: this function has to be in sync with Uic::setObjectProperty(). If you change one, change both.
*/
void Resource::setObjectProperty( QObject* obj, const QString &prop, const QDomElement &e )
{
    const QMetaProperty *p = obj->metaObject()->property( obj->metaObject()->findProperty( prop, TRUE ), TRUE );

    if ( !::qt_cast<QLayout*>(obj)  ) {// no layouts in metadatabase... (RS)
	if ( obj->inherits( "CustomWidget" ) ) {
	    MetaDataBase::CustomWidget *cw = ( (CustomWidget*)obj )->customWidget();
	    if ( cw && !cw->hasProperty( prop.latin1() ) && !p && prop != "toolTip" && prop != "whatsThis" )
		return;
	}
	MetaDataBase::setPropertyChanged( obj, prop, TRUE );
    }

    QVariant defVarient;
    if ( e.tagName() == "font" ) {
	QFont f( qApp->font() );
	if ( obj->isWidgetType() && ( (QWidget*)obj )->parentWidget() )
	    f = ( (QWidget*)obj )->parentWidget()->font();
	defVarient = QVariant( f );
    }

    QString comment;
    QVariant v( DomTool::elementToVariant( e, defVarient, comment ) );

    if ( !comment.isEmpty() ) {
	MetaDataBase::addEntry( obj );
	MetaDataBase::setPropertyComment( obj, prop, comment );
    }

    if ( e.tagName() == "pixmap" ) {
	QPixmap pix = loadPixmap( e );
	if ( pix.isNull() )
	    return;
	v = QVariant( pix );
    } else if ( e.tagName() == "iconset" ) {
	QPixmap pix = loadPixmap( e, "iconset" );
	if ( pix.isNull() )
	    return;
	v = QVariant( QIconSet( pix ) );
    } else if ( e.tagName() == "image" ) {
	v = QVariant( loadFromCollection( v.toString() ) );
    }

    if ( !p ) {
	MetaDataBase::setFakeProperty( obj, prop, v );
	if ( obj->isWidgetType() ) {
	    if ( prop == "database" && obj != toplevel ) {
		QStringList lst = MetaDataBase::fakeProperty( obj, "database" ).toStringList();
		if ( lst.count() > 2 )
		    dbControls.insert( obj->name(), lst[ 2 ] );
		else if ( lst.count() == 2 )
		    dbTables.insert( obj->name(), lst );
	    }
	    return;
	}
    }

    if ( e.tagName() == "palette" ) {
	QDomElement n = e.firstChild().toElement();
	QPalette p;
	while ( !n.isNull() ) {
	    QColorGroup cg;
	    if ( n.tagName() == "active" ) {
		cg = loadColorGroup( n );
		p.setActive( cg );
	    } else if ( n.tagName() == "inactive" ) {
		cg = loadColorGroup( n );
		p.setInactive( cg );
	    } else if ( n.tagName() == "disabled" ) {
		cg = loadColorGroup( n );
		p.setDisabled( cg );
	    }
	    n = n.nextSibling().toElement();
	}
	v = QPalette( p );
    } else if ( e.tagName() == "enum" && p && p->isEnumType() && prop != "resizeMode" ) {
	QString key( v.toString() );
	int vi = p->keyToValue( key );
	if ( p->valueToKey( vi ) != key )
	    return; // ignore invalid properties
	v = QVariant( vi );
    } else if ( e.tagName() == "set" && p && p->isSetType() ) {
	QString keys( v.toString() );
	QStringList lst = QStringList::split( '|', keys );
	QStrList l;
	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
	    l.append( *it );
	v = QVariant( p->keysToValue( l ) );
    }

    if ( prop == "caption" ) {
	QCString s1 = v.toCString();
	QString s2 = v.toString();
	if ( !s2.isEmpty() )
	    formwindow->setCaption( s2 );
	else if ( !s1.isEmpty() )
	    formwindow->setCaption( s1 );
    }
    if ( prop == "icon" ) {
	formwindow->setIcon( v.toPixmap() );
	QString pmk = MetaDataBase::pixmapKey( formwindow, v.toPixmap().serialNumber() );
	MetaDataBase::setPixmapKey( formwindow,
				    formwindow->icon()->serialNumber(), pmk );
    }

    if ( prop == "geometry" ) {
	if ( obj == toplevel ) {
	    hadGeometry = TRUE;
	    toplevel->resize( v.toRect().size() );
	    return;
	} else if ( obj == formwindow->mainContainer() ) {
	    hadGeometry = TRUE;
	    formwindow->resize( v.toRect().size() );
	    return;
	}
    }

    if ( ::qt_cast<QLayout*>(obj) ) {
	if ( prop == "spacing" ) {
	    MetaDataBase::setSpacing( WidgetFactory::containerOfWidget( WidgetFactory::layoutParent( (QLayout*)obj ) ), v.toInt() );
	    return;
	} else if ( prop == "margin" ) {
	    MetaDataBase::setMargin( WidgetFactory::containerOfWidget( WidgetFactory::layoutParent( (QLayout*)obj ) ), v.toInt() );
	    return;
	} else if ( e.tagName() == "enum" &&  prop == "resizeMode" ) {
	    MetaDataBase::setResizeMode( WidgetFactory::containerOfWidget( WidgetFactory::layoutParent( (QLayout*)obj ) ), v.toString() );
	    return;
	}
    }

    if ( prop == "name" ) {
	if ( pasting ) {
	    QString s = v.toString();
	    formwindow->unify( (QWidget*)obj, s, TRUE );
	    obj->setName( s );
	    return;
	} else if ( formwindow && obj == formwindow->mainContainer() ) {
	    formwindow->setName( v.toCString() );
	}
    }

    if ( prop == "sizePolicy" ) {
	QSizePolicy sp = v.toSizePolicy();
	sp.setHeightForWidth( ( (QWidget*)obj )->sizePolicy().hasHeightForWidth() );
    }

    if ( prop == "cursor" )
	MetaDataBase::setCursor( (QWidget*)obj, v.toCursor() );

    obj->setProperty( prop, v );
}


QString Resource::saveInCollection( const QImage &img )
{
    QString imgName = "none";
    QValueList<Image>::Iterator it = images.begin();
    for ( ; it != images.end(); ++it ) {
	if ( img == ( *it ).img ) {
	    imgName = ( *it ).name;
	    break;
	}
    }

    if ( imgName == "none" ) {
	Image i;
	imgName = "image" + QString::number( images.count() );
	i.name = imgName;
	i.img = img;
	images.append( i );
    }
    return imgName;
}

void Resource::saveImageData( const QImage &img, QTextStream &ts, int indent )
{
    QByteArray ba;
    QBuffer buf( ba );
    buf.open( IO_WriteOnly | IO_Translate );
    QString format;
    bool compress = FALSE;
    if (img.hasAlphaBuffer()) {
	format = "PNG";
    } else {
	format = img.depth() > 1 ? "XPM" : "XBM";
	compress = TRUE;
    }
    QImageIO iio( &buf, format );
    iio.setImage( img );
    iio.write();
    buf.close();
    QByteArray bazip = ba;
    int i = 0;
    if (compress) {
	bazip = qCompress( ba );
	format += ".GZ";
	// The first 4 bytes in qCompress() are the length of the unzipped
	// format. The XPM.GZ format does not use these.
	i = 4;
    }
    ulong len = bazip.size();
    ts << makeIndent( indent ) << "<data format=\"" + format + "\" length=\"" << ba.size() << "\">";
    static const char hexchars[] = "0123456789abcdef";
    for (; i < (int)len; ++i ) {
	uchar s = (uchar) bazip[i];
	ts << hexchars[s >> 4];
	ts << hexchars[s & 0x0f];
    }
    ts << "</data>" << endl;
}

void Resource::saveImageCollection( QTextStream &ts, int indent )
{
    ts << makeIndent( indent ) << "<images>" << endl;
    indent++;

    QValueList<Image>::Iterator it = images.begin();
    for ( ; it != images.end(); ++it ) {
	ts << makeIndent( indent ) << "<image name=\"" << (*it).name << "\">" << endl;
	indent++;
	saveImageData( (*it).img, ts, indent );
	indent--;
	ts << makeIndent( indent ) << "</image>" << endl;
    }

    indent--;
    ts << makeIndent( indent ) << "</images>" << endl;
}

static QImage loadImageData( QDomElement &n2 )
{
    QImage img;
    QString data = n2.firstChild().toText().data();
    const int lengthOffset = 4;
    int baSize = data.length() / 2 + lengthOffset;
    uchar *ba = new uchar[ baSize ];
    for ( int i = lengthOffset; i < baSize; ++i ) {
	char h = data[ 2 * (i-lengthOffset) ].latin1();
	char l = data[ 2 * (i-lengthOffset) + 1 ].latin1();
	uchar r = 0;
	if ( h <= '9' )
	    r += h - '0';
	else
	    r += h - 'a' + 10;
	r = r << 4;
	if ( l <= '9' )
	    r += l - '0';
	else
	    r += l - 'a' + 10;
	ba[ i ] = r;
    }
    QString format = n2.attribute( "format", "PNG" );
    if ( format == "XPM.GZ" || format == "XBM.GZ" ) {
	ulong len = n2.attribute( "length" ).toULong();
	if ( len < data.length() * 5 )
	    len = data.length() * 5;
	// qUncompress() expects the first 4 bytes to be the expected length of
	// the uncompressed data
	ba[0] = ( len & 0xff000000 ) >> 24;
	ba[1] = ( len & 0x00ff0000 ) >> 16;
	ba[2] = ( len & 0x0000ff00 ) >> 8;
	ba[3] = ( len & 0x000000ff );
	QByteArray baunzip = qUncompress( ba, baSize );
	img.loadFromData( (const uchar*)baunzip.data(), baunzip.size(), format.left(format.find('.')) );
    }  else {
	img.loadFromData( (const uchar*)ba+lengthOffset, baSize-lengthOffset, format );
    }
    delete [] ba;
    return img;
}

void Resource::loadImageCollection( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "image" ) {
	    Image img;
	    img.name =  n.attribute( "name" );
	    QDomElement n2 = n.firstChild().toElement();
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "data" )
		    img.img = loadImageData( n2 );
		n2 = n2.nextSibling().toElement();
	    }
	    images.append( img );
	    n = n.nextSibling().toElement();
	}
    }
}

QImage Resource::loadFromCollection( const QString &name )
{
    QValueList<Image>::Iterator it = images.begin();
    for ( ; it != images.end(); ++it ) {
	if ( ( *it ).name == name )
	    return ( *it ).img;
    }
    return QImage();
}

void Resource::saveConnections( QTextStream &ts, int indent )
{
    QValueList<MetaDataBase::Connection> connections = MetaDataBase::connections( formwindow );
    if ( connections.isEmpty() )
	return;
    ts << makeIndent( indent ) << "<connections>" << endl;
    indent++;
    QValueList<MetaDataBase::Connection>::Iterator it = connections.begin();
    for ( ; it != connections.end(); ++it ) {
	MetaDataBase::Connection conn = *it;
	if ( ( knownNames.findIndex( QString( conn.sender->name() ) ) == -1 &&
	       qstrcmp( conn.sender->name(), "this" ) != 0 ) ||
	     ( knownNames.findIndex( QString( conn.receiver->name() ) ) == -1 &&
	       qstrcmp( conn.receiver->name(), "this" ) != 0 ) )
	    continue;
	if ( formwindow->isMainContainer( (QWidget*)(*it).receiver ) &&
	     !MetaDataBase::hasSlot( formwindow, MetaDataBase::normalizeFunction( (*it).slot ).latin1() ) )
	    continue;

	if ( conn.sender->inherits( "CustomWidget" ) ) {
	    MetaDataBase::CustomWidget *cw = ( (CustomWidget*)conn.sender )->customWidget();
	    if ( cw && !cw->hasSignal( conn.signal ) )
		continue;
	}

	if ( conn.receiver->inherits( "CustomWidget" ) && !formwindow->isMainContainer( conn.receiver ) ) {
	    MetaDataBase::CustomWidget *cw = ( (CustomWidget*)conn.receiver )->customWidget();
	    if ( cw && !cw->hasSlot( MetaDataBase::normalizeFunction( conn.slot ).latin1() ) )
		continue;
	}

	ts << makeIndent( indent ) << "<connection>" << endl;
	indent++;
	ts << makeIndent( indent ) << "<sender>" << entitize( conn.sender->name() ) << "</sender>" << endl;
	ts << makeIndent( indent ) << "<signal>" << entitize( conn.signal ) << "</signal>" << endl;
	ts << makeIndent( indent ) << "<receiver>" << entitize( conn.receiver->name() ) << "</receiver>" << endl;
	ts << makeIndent( indent ) << "<slot>" << entitize( MetaDataBase::normalizeFunction( conn.slot ) ) << "</slot>" << endl;
	indent--;
	ts << makeIndent( indent ) << "</connection>" << endl;
    }

    QString lang = formwindow->project()->language();
    indent--;
    ts << makeIndent( indent ) << "</connections>" << endl;
}

void Resource::loadConnections( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "connection" ) {
	    QString lang = n.attribute( "language", "C++" );
	    QDomElement n2 = n.firstChild().toElement();
	    MetaDataBase::Connection conn;
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "sender" ) {
		    conn.sender = 0;
		    QString name = n2.firstChild().toText().data();
		    if ( name == "this" || qstrcmp( toplevel->name(), name ) == 0 ) {
			conn.sender = toplevel;
		    } else {
			if ( name == "this" )
			    name = toplevel->name();
			QObjectList *l = toplevel->queryList( 0, name, FALSE );
			if ( l ) {
			    if ( l->first() )
				conn.sender = l->first();
			    delete l;
			    l = 0;
			}
			if ( !conn.sender )
			    conn.sender = formwindow->findAction( name );
		    }
		} else if ( n2.tagName() == "signal" ) {
		    conn.signal = n2.firstChild().toText().data();
		} else if ( n2.tagName() == "receiver" ) {
		    conn.receiver = 0;
		    QString name = n2.firstChild().toText().data();
		    if ( name == "this" || qstrcmp( toplevel->name(), name ) == 0 ) {
			conn.receiver = toplevel;
		    } else {
			QObjectList *l = toplevel->queryList( 0, name, FALSE );
			if ( l ) {
			    if ( l->first() )
				conn.receiver = l->first();
			    delete l;
			    l = 0;
			}
			if ( !conn.receiver )
			    conn.receiver = formwindow->findAction( name );
		    }
		} else if ( n2.tagName() == "slot" ) {
		    conn.slot = n2.firstChild().toText().data();
		}
		n2 = n2.nextSibling().toElement();
	    }
	    if ( formwindow ) {
		if ( conn.sender == formwindow )
		    conn.sender = formwindow->mainContainer();
		if ( conn.receiver == formwindow )
		    conn.receiver = formwindow->mainContainer();
	    }
	    if ( conn.sender && conn.receiver ) {
		if ( lang == "C++" ) {
		    MetaDataBase::addConnection( formwindow ? formwindow : toplevel,
						 conn.sender, conn.signal, conn.receiver, conn.slot );
		}
	    }
	} else if ( n.tagName() == "slot" ) { // compatibility with 2.x
	    MetaDataBase::Function slot;
	    slot.specifier = n.attribute( "specifier", "virtual" );
	    if ( slot.specifier.isEmpty() )
		slot.specifier = "virtual";
	    slot.access = n.attribute( "access", "public" );
	    if ( slot.access.isEmpty() )
		slot.access = "public";
	    slot.language = n.attribute( "language", "C++" );
	    slot.returnType = n.attribute( "returnType", "void" );
	    if ( slot.returnType.isEmpty() )
		slot.returnType = "void";
	    slot.function = n.firstChild().toText().data();
	    if ( !MetaDataBase::hasFunction( formwindow, slot.function, TRUE ) )
		MetaDataBase::addFunction( formwindow, slot.function, slot.specifier,
				       slot.access, "slot", slot.language, slot.returnType );
	    else
		MetaDataBase::changeFunctionAttributes( formwindow, slot.function, slot.function,
							slot.specifier, slot.access,
							"slot", slot.language, slot.returnType );
	}
	n = n.nextSibling().toElement();
    }
}

void Resource::saveCustomWidgets( QTextStream &ts, int indent )
{
    ts << makeIndent( indent ) << "<customwidgets>" << endl;
    indent++;

    QPtrList<MetaDataBase::CustomWidget> *lst = MetaDataBase::customWidgets();
    for ( MetaDataBase::CustomWidget *w = lst->first(); w; w = lst->next() ) {
	if ( usedCustomWidgets.findIndex( w->className ) == -1 )
	    continue;
	ts << makeIndent( indent ) << "<customwidget>" << endl;
	indent++;
	ts << makeIndent( indent ) << "<class>" << w->className << "</class>" << endl;
	ts << makeIndent( indent ) << "<header location=\""
	   << ( w->includePolicy == MetaDataBase::CustomWidget::Local ? "local" : "global" )
	   << "\">" << w->includeFile << "</header>" << endl;
	ts << makeIndent( indent ) << "<sizehint>" << endl;
	indent++;
	ts << makeIndent( indent ) << "<width>" << w->sizeHint.width() << "</width>" << endl;
	ts << makeIndent( indent ) << "<height>" << w->sizeHint.height() << "</height>" << endl;
	indent--;
	ts << makeIndent( indent ) << "</sizehint>" << endl;
	ts << makeIndent( indent ) << "<container>" << (int)w->isContainer << "</container>" << endl;
	ts << makeIndent( indent ) << "<sizepolicy>" << endl;
	indent++;
	ts << makeIndent( indent ) << "<hordata>" << (int)w->sizePolicy.horData() << "</hordata>" << endl;
	ts << makeIndent( indent ) << "<verdata>" << (int)w->sizePolicy.verData() << "</verdata>" << endl;
	ts << makeIndent( indent ) << "<horstretch>" << (int)w->sizePolicy.horStretch() << "</horstretch>" << endl;
	ts << makeIndent( indent ) << "<verstretch>" << (int)w->sizePolicy.verStretch() << "</verstretch>" << endl;
	indent--;
	ts << makeIndent( indent ) << "</sizepolicy>" << endl;
	ts << makeIndent( indent ) << "<pixmap>" << saveInCollection( w->pixmap->convertToImage() ) << "</pixmap>" << endl;
	if ( !w->lstSignals.isEmpty() ) {
	    for ( QValueList<QCString>::Iterator it = w->lstSignals.begin(); it != w->lstSignals.end(); ++it )
		ts << makeIndent( indent ) << "<signal>" << entitize( *it ) << "</signal>" << endl;
	}
	if ( !w->lstSlots.isEmpty() ) {
	    for ( QValueList<MetaDataBase::Function>::Iterator it = w->lstSlots.begin(); it != w->lstSlots.end(); ++it )
		ts << makeIndent( indent ) << "<slot access=\"" << (*it).access << "\" specifier=\""
		   << (*it).specifier << "\">" << entitize( (*it).function ) << "</slot>" << endl;
	}
	if ( !w->lstProperties.isEmpty() ) {
	    for ( QValueList<MetaDataBase::Property>::Iterator it = w->lstProperties.begin(); it != w->lstProperties.end(); ++it )
		ts << makeIndent( indent ) << "<property type=\"" << (*it).type << "\">" << entitize( (*it).property ) << "</property>" << endl;
	}
	indent--;
	ts << makeIndent( indent ) << "</customwidget>" << endl;
    }


    indent--;
    ts << makeIndent( indent ) << "</customwidgets>" << endl;
}

void Resource::loadCustomWidgets( const QDomElement &e, Resource *r )
{
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "customwidget" ) {
	    QDomElement n2 = n.firstChild().toElement();
	    MetaDataBase::CustomWidget *w = new MetaDataBase::CustomWidget;
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "class" ) {
		    w->className = n2.firstChild().toText().data();
		} else if ( n2.tagName() == "header" ) {
		    w->includeFile = n2.firstChild().toText().data();
		    QString s = n2.attribute( "location" );
		    if ( s != "local" )
			w->includePolicy = MetaDataBase::CustomWidget::Global;
		    else
			w->includePolicy = MetaDataBase::CustomWidget::Local;
		} else if ( n2.tagName() == "sizehint" ) {
		    QDomElement n3 = n2.firstChild().toElement();
		    while ( !n3.isNull() ) {
			if ( n3.tagName() == "width" )
			    w->sizeHint.setWidth( n3.firstChild().toText().data().toInt() );
			else if ( n3.tagName() == "height" )
			    w->sizeHint.setHeight( n3.firstChild().toText().data().toInt() );
			n3 = n3.nextSibling().toElement();
		    }
		} else if ( n2.tagName() == "sizepolicy" ) {
		    QDomElement n3 = n2.firstChild().toElement();
		    while ( !n3.isNull() ) {
			if ( n3.tagName() == "hordata" )
			    w->sizePolicy.setHorData( (QSizePolicy::SizeType)n3.firstChild().toText().data().toInt() );
			else if ( n3.tagName() == "verdata" )
			    w->sizePolicy.setVerData( (QSizePolicy::SizeType)n3.firstChild().toText().data().toInt() );
			else if ( n3.tagName() == "horstretch" )
			    w->sizePolicy.setHorStretch( n3.firstChild().toText().data().toInt() );
			else if ( n3.tagName() == "verstretch" )
			    w->sizePolicy.setVerStretch( n3.firstChild().toText().data().toInt() );
			n3 = n3.nextSibling().toElement();
		    }
		} else if ( n2.tagName() == "pixmap" ) {
		    QPixmap pix;
		    if ( r ) {
			pix = r->loadPixmap( n2 );
		    } else {
			QDomElement n3 = n2.firstChild().toElement();
			QImage img;
			while ( !n3.isNull() ) {
			    if ( n3.tagName() == "data" ) {
				img = loadImageData( n3 );
			    }
			    n3 = n3.nextSibling().toElement();
			}
			pix.convertFromImage( img );
		    }
		    w->pixmap = new QPixmap( pix );
		} else if ( n2.tagName() == "signal" ) {
		    w->lstSignals.append( n2.firstChild().toText().data().latin1() );
		} else if ( n2.tagName() == "container" ) {
		    w->isContainer = (bool)n2.firstChild().toText().data().toInt();
		} else if ( n2.tagName() == "slot" ) {
		    MetaDataBase::Function function;
		    function.function = n2.firstChild().toText().data().latin1();
		    function.access = n2.attribute( "access" );
		    function.type = "slot";
		    w->lstSlots.append( function );
		} else if ( n2.tagName() == "property" ) {
		    MetaDataBase::Property property;
		    property.property = n2.firstChild().toText().data().latin1();
		    property.type = n2.attribute( "type" );
		    w->lstProperties.append( property );
		}
		n2 = n2.nextSibling().toElement();
	    }
	    MetaDataBase::addCustomWidget( w );
	}
	n = n.nextSibling().toElement();
    }
}

void Resource::saveTabOrder( QTextStream &ts, int indent )
{
    QWidgetList l = MetaDataBase::tabOrder( toplevel );
    if ( l.isEmpty() )
	return;

    ts << makeIndent( indent ) << "<tabstops>" << endl;
    indent++;

    for ( QWidget *w = l.first(); w; w = l.next() ) {
	if ( w->testWState( Qt::WState_ForceHide ) || knownNames.findIndex( w->name() ) == -1 )
	    continue;
	ts << makeIndent( indent ) << "<tabstop>" << w->name() << "</tabstop>" << endl;
    }

    indent--;
    ts << makeIndent( indent ) << "</tabstops>" << endl;
}

void Resource::loadTabOrder( const QDomElement &e )
{
    QWidget *last = 0;
    QDomElement n = e.firstChild().toElement();
    QWidgetList widgets;
    while ( !n.isNull() ) {
	if ( n.tagName() == "tabstop" ) {
	    QString name = n.firstChild().toText().data();
	    if ( name.isEmpty() )
		continue;
	    QObjectList *l = toplevel->queryList( 0, name, FALSE );
	    if ( l ) {
		if ( l->first() ) {
		    QWidget *w = (QWidget*)l->first();
		    widgets.append( w );
		    if ( last )
			toplevel->setTabOrder( last, w );
		    last = w;
		}
		delete l;
	    }
	}
	n = n.nextSibling().toElement();
    }

    if ( !widgets.isEmpty() )
	MetaDataBase::setTabOrder( toplevel, widgets );
}

void Resource::saveMetaInfoBefore( QTextStream &ts, int indent )
{
    MetaDataBase::MetaInfo info = MetaDataBase::metaInfo( formwindow );
    QString cn;
    if ( info.classNameChanged && !info.className.isEmpty() )
	cn = info.className;
    else
	cn = formwindow->name();
    ts << makeIndent( indent ) << "<class>" << entitize( cn ) << "</class>" << endl;
    if ( !info.comment.isEmpty() )
	ts << makeIndent( indent ) << "<comment>" << entitize( info.comment ) << "</comment>" << endl;
    if ( !info.author.isEmpty() )
	ts << makeIndent( indent ) << "<author>" << entitize( info.author ) << "</author>" << endl;
}

void Resource::saveMetaInfoAfter( QTextStream &ts, int indent )
{
    MetaDataBase::MetaInfo info = MetaDataBase::metaInfo( formwindow );
    if ( !langIface || formwindow->project()->isCpp() ) {
	QValueList<MetaDataBase::Include> includes = MetaDataBase::includes( formwindow );
	QString extensionInclude;
	bool needExtensionInclude = FALSE;
	if ( langIface &&
	     formwindow->formFile()->hasFormCode() &&
	     formwindow->formFile()->codeFileState() != FormFile::Deleted ) {
	    extensionInclude = QFileInfo( currFileName ).fileName() + langIface->formCodeExtension();
	    needExtensionInclude = TRUE;
	}
	if ( !includes.isEmpty() || needExtensionInclude ) {
	    ts << makeIndent( indent ) << "<includes>" << endl;
	    indent++;

	    for ( QValueList<MetaDataBase::Include>::Iterator it = includes.begin(); it != includes.end(); ++it ) {
		ts << makeIndent( indent ) << "<include location=\"" << (*it).location
		   << "\" impldecl=\"" << (*it).implDecl << "\">" << (*it).header << "</include>" << endl;
		if ( needExtensionInclude )
		    needExtensionInclude = (*it).header != extensionInclude;
	    }

	    if ( needExtensionInclude )
		ts << makeIndent( indent ) << "<include location=\"local\" impldecl=\"in implementation\">"
		   << extensionInclude << "</include>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</includes>" << endl;
	}

	QStringList forwards = MetaDataBase::forwards( formwindow );
	if ( !forwards.isEmpty() ) {
	    ts << makeIndent( indent ) << "<forwards>" << endl;
	    indent++;
	    for ( QStringList::Iterator it2 = forwards.begin(); it2 != forwards.end(); ++it2 )
		ts << makeIndent( indent ) << "<forward>" << entitize( *it2 ) << "</forward>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</forwards>" << endl;
	}
	QValueList<MetaDataBase::Variable> varLst = MetaDataBase::variables( formwindow );
	if ( !varLst.isEmpty() ) {
	    ts << makeIndent( indent ) << "<variables>" << endl;
	    indent++;

	    QValueList<MetaDataBase::Variable>::Iterator it = varLst.begin();
	    for ( ; it != varLst.end(); ++it ) {
		ts << makeIndent( indent ) << "<variable";
		if ( (*it).varAccess != "protected" )
		    ts << " access=\"" << (*it).varAccess << "\"";

		ts << ">" << entitize( (*it).varName ) << "</variable>" << endl;
	    }
	    indent--;
	    ts << makeIndent( indent ) << "</variables>" << endl;
	}
	QStringList sigs = MetaDataBase::signalList( formwindow );
	if ( !sigs.isEmpty() ) {
	    ts << makeIndent( indent ) << "<signals>" << endl;
	    indent++;
	    for ( QStringList::Iterator it3 = sigs.begin(); it3 != sigs.end(); ++it3 )
		ts << makeIndent( indent ) << "<signal>" << entitize( *it3 ) << "</signal>" << endl;
	    indent--;
	    ts << makeIndent( indent ) << "</signals>" << endl;
	}

	QValueList<MetaDataBase::Function> slotList = MetaDataBase::slotList( formwindow );
	if ( !slotList.isEmpty() ) {
	    ts << makeIndent( indent ) << "<slots>" << endl;
	    indent++;
	    QString lang = formwindow->project()->language();
	    QValueList<MetaDataBase::Function>::Iterator it = slotList.begin();
	    for ( ; it != slotList.end(); ++it ) {
		MetaDataBase::Function function = *it;
		ts << makeIndent( indent ) << "<slot";
		if ( function.access != "public" )
		    ts << " access=\"" << function.access << "\"";
		if ( function.specifier != "virtual" )
		    ts << " specifier=\"" << function.specifier << "\"";
		if ( function.language != "C++" )
		    ts << " language=\"" << function.language<< "\"";
		if ( function.returnType != "void" )
		    ts << " returnType=\"" << entitize( function.returnType ) << "\"";
		ts << ">" << entitize( function.function ) << "</slot>" << endl;
	    }
	    indent--;
	    ts << makeIndent( indent ) << "</slots>" << endl;
	}

	QValueList<MetaDataBase::Function> functionList = MetaDataBase::functionList( formwindow, TRUE );
	if ( !functionList.isEmpty() ) {
	    ts << makeIndent( indent ) << "<functions>" << endl;
	    indent++;
	    QString lang = formwindow->project()->language();
	    QValueList<MetaDataBase::Function>::Iterator it = functionList.begin();
	    for ( ; it != functionList.end(); ++it ) {
		MetaDataBase::Function function = *it;
		ts << makeIndent( indent ) << "<function";
		if ( function.access != "public" )
		    ts << " access=\"" << function.access << "\"";
		if ( function.specifier != "virtual" )
		    ts << " specifier=\"" << function.specifier << "\"";
		if ( function.language != "C++" )
		    ts << " language=\"" << function.language<< "\"";
		if ( function.returnType != "void" )
		    ts << " returnType=\"" << entitize( function.returnType ) << "\"";
		ts << ">" << entitize( function.function ) << "</function>" << endl;
	    }
	    indent--;
	    ts << makeIndent( indent ) << "</functions>" << endl;
	}
    }

    if ( formwindow && formwindow->savePixmapInline() )
	;
    else if ( formwindow && formwindow->savePixmapInProject() )
	ts << makeIndent( indent ) << "<pixmapinproject/>" << endl;
    else
	ts << makeIndent( indent ) << "<pixmapfunction>" << formwindow->pixmapLoaderFunction() << "</pixmapfunction>" << endl;
    if ( !( exportMacro = MetaDataBase::exportMacro( formwindow->mainContainer() ) ).isEmpty() )
	ts << makeIndent( indent ) << "<exportmacro>" << exportMacro << "</exportmacro>" << endl;
    if ( formwindow ) {
	ts << makeIndent( indent ) << "<layoutdefaults spacing=\"" << formwindow->layoutDefaultSpacing()
	   << "\" margin=\"" << formwindow->layoutDefaultMargin() << "\"/>" << endl;
	if ( formwindow->hasLayoutFunctions() ) {
	    QString s = "";
	    QString m = "";
	    if ( !formwindow->spacingFunction().isEmpty() )
		s = QString( " spacing=\"%1\"" ).arg( formwindow->spacingFunction() );
	    if ( !formwindow->marginFunction().isEmpty() )
		m = QString( " margin=\"%1\"" ).arg( formwindow->marginFunction() );
	    ts << makeIndent( indent ) << "<layoutfunctions" << s << m << "/>" << endl;
	}
    }
}

void Resource::saveIncludeHints( QTextStream &ts, int indent )
{
    if ( includeHints.isEmpty() )
	return;
    ts << makeIndent( indent ) << "<includehints>" << endl;
    indent++;
    for ( QStringList::Iterator it = includeHints.begin(); it != includeHints.end(); ++it )
	ts << makeIndent( indent ) << "<includehint>" << *it << "</includehint>" << endl;
    indent--;
    ts << makeIndent( indent ) << "</includehints>" << endl;
}

QColorGroup Resource::loadColorGroup( const QDomElement &e )
{
    QColorGroup cg;
    int r = -1;
    QDomElement n = e.firstChild().toElement();
    QColor col;
    while ( !n.isNull() ) {
	if ( n.tagName() == "color" ) {
	    r++;
	    cg.setColor( (QColorGroup::ColorRole)r, (col = DomTool::readColor( n ) ) );
	} else if ( n.tagName() == "pixmap" ) {
	    QPixmap pix = loadPixmap( n );
	    cg.setBrush( (QColorGroup::ColorRole)r, QBrush( col, pix ) );
	}
	n = n.nextSibling().toElement();
    }
    return cg;
}

void Resource::saveChildActions( QAction *a, QTextStream &ts, int indent )
{
    if ( !a->children() )
	return;
    QObjectListIt it( *a->children() );
    while ( it.current() ) {
	QObject *o = it.current();
	++it;
	if ( !::qt_cast<QAction*>(o) )
	    continue;
	QAction *ac = (QAction*)o;
	bool isGroup = ::qt_cast<QActionGroup*>(ac);
	if ( isGroup )
	    ts << makeIndent( indent ) << "<actiongroup>" << endl;
	else
	    ts << makeIndent( indent ) << "<action>" << endl;
	indent++;
	saveObjectProperties( ac, ts, indent );
	indent--;
	if ( isGroup ) {
	    indent++;
	    saveChildActions( ac, ts, indent );
	    indent--;
	}
	if ( isGroup )
	    ts << makeIndent( indent ) << "</actiongroup>" << endl;
	else
	    ts << makeIndent( indent ) << "</action>" << endl;
    }
}

void Resource::saveActions( const QPtrList<QAction> &actions, QTextStream &ts, int indent )
{
    if ( actions.isEmpty() )
	return;
    ts << makeIndent( indent ) << "<actions>" << endl;
    indent++;
    QPtrListIterator<QAction> it( actions );
    while ( it.current() ) {
	QAction *a = it.current();
	bool isGroup = ::qt_cast<QActionGroup*>(a);
	if ( isGroup )
	    ts << makeIndent( indent ) << "<actiongroup>" << endl;
	else
	    ts << makeIndent( indent ) << "<action>" << endl;
	indent++;
	saveObjectProperties( a, ts, indent );
	indent--;
	if ( isGroup ) {
	    indent++;
	    saveChildActions( a, ts, indent );
	    indent--;
	}
	if ( isGroup )
	    ts << makeIndent( indent ) << "</actiongroup>" << endl;
	else
	    ts << makeIndent( indent ) << "</action>" << endl;
	++it;
    }
    indent--;
    ts << makeIndent( indent ) << "</actions>" << endl;
}

void Resource::loadChildAction( QObject *parent, const QDomElement &e )
{
    QDomElement n = e;
    QAction *a = 0;
    if ( n.tagName() == "action" ) {
	a = new QDesignerAction( parent );
	MetaDataBase::addEntry( a );
	QDomElement n2 = n.firstChild().toElement();
	bool hasMenuText = FALSE;
	while ( !n2.isNull() ) {
	    if ( n2.tagName() == "property" ) {
		QDomElement n3(n2); // don't modify n2
		QString prop = n3.attribute( "name" );
		if (prop == "menuText")
		    hasMenuText = TRUE;
		QDomElement value(n3.firstChild().toElement());
		setObjectProperty( a, prop, value );
		if (!hasMenuText && uiFileVersion < "3.3" && prop == "text")
		    setObjectProperty( a, "menuText", value );
	    }
	    n2 = n2.nextSibling().toElement();
	}
	if ( !::qt_cast<QAction*>(parent) )
	    formwindow->actionList().append( a );
    } else if ( n.tagName() == "actiongroup" ) {
	a = new QDesignerActionGroup( parent );
	MetaDataBase::addEntry( a );
	QDomElement n2 = n.firstChild().toElement();
	bool hasMenuText = FALSE;
	while ( !n2.isNull() ) {
	    if ( n2.tagName() == "property" ) {
		QDomElement n3(n2); // don't modify n2
		QString prop = n3.attribute( "name" );
		if (prop == "menuText")
		    hasMenuText = TRUE;
		QDomElement value = n3.firstChild().toElement();
		setObjectProperty( a, prop, value );
		if (!hasMenuText && uiFileVersion < "3.3" && prop == "text")
		    setObjectProperty( a, "menuText", value );
	    } else if ( n2.tagName() == "action" ||
			n2.tagName() == "actiongroup" ) {
		loadChildAction( a, n2 );
	    }
	    n2 = n2.nextSibling().toElement();
	}
	if ( !::qt_cast<QAction*>(parent) )
	    formwindow->actionList().append( a );
    }
}

void Resource::loadActions( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "action" ) {
	    loadChildAction( formwindow, n );
	} else if ( n.tagName() == "actiongroup" ) {
	    loadChildAction( formwindow, n );
	}
	n = n.nextSibling().toElement();
    }
}

void Resource::saveToolBars( QMainWindow *mw, QTextStream &ts, int indent )
{
    ts << makeIndent( indent ) << "<toolbars>" << endl;
    indent++;

    QPtrList<QToolBar> tbList;
    for ( int i = 0; i <= (int)Qt::DockMinimized; ++i ) {
	tbList = mw->toolBars( (Qt::Dock)i );
	if ( tbList.isEmpty() )
	    continue;
	for ( QToolBar *tb = tbList.first(); tb; tb = tbList.next() ) {
	    if ( tb->isHidden() )
		continue;
	    ts << makeIndent( indent ) << "<toolbar dock=\"" << i << "\">" << endl;
	    indent++;
	    saveObjectProperties( tb, ts, indent );
	    QPtrList<QAction> actionList = ( (QDesignerToolBar*)tb )->insertedActions();
	    for ( QAction *a = actionList.first(); a; a = actionList.next() ) {
		if ( ::qt_cast<QSeparatorAction*>(a) ) {
		    ts << makeIndent( indent ) << "<separator/>" << endl;
		} else {
		    if ( ::qt_cast<QDesignerAction*>(a) && !( (QDesignerAction*)a )->supportsMenu() ) {
			QWidget *w = ( (QDesignerAction*)a )->widget();
			ts <<  makeIndent( indent ) << "<widget class=\""
			   << WidgetFactory::classNameOf( w ) << "\">" << endl;
			indent++;
			const char *className = WidgetFactory::classNameOf( w );
			if ( w->isA( "CustomWidget" ) )
			    usedCustomWidgets << QString( className );
			if ( WidgetFactory::hasItems( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) ), w ) )
			    saveItems( w, ts, indent );
			saveObjectProperties( w, ts, indent );
			indent--;
			ts << makeIndent( indent ) << "</widget>" << endl;
		    } else {
			ts << makeIndent( indent ) << "<action name=\"" << a->name() << "\"/>" << endl;
		    }
		}
	    }
	    indent--;
	    ts << makeIndent( indent ) << "</toolbar>" << endl;
	}
    }
    indent--;
    ts << makeIndent( indent ) << "</toolbars>" << endl;
}

void Resource::saveMenuBar( QMainWindow *mw, QTextStream &ts, int indent )
{
    MenuBarEditor *mb = (MenuBarEditor *)mw->child( 0, "MenuBarEditor" );
    if ( !mb )
	return;
    ts << makeIndent( indent ) << "<menubar>" << endl;
    indent++;
    MetaDataBase::setPropertyChanged( mb, "name", TRUE ); // FIXME: remove
    saveObjectProperties( mb, ts, indent );

    for ( int i = 0; i < (int)mb->count(); ++i ) {
	MenuBarEditorItem *m = mb->item( i );
	if ( !m )
	    continue;
	if ( m->isSeparator() ) {
	    ts << makeIndent( indent ) << "<separator/>" << endl;
	} else {
	    ts << makeIndent( indent ) << "<item text=\"" << entitize( m->menuText() )
	       << "\" name=\"" << entitize( m->menu()->name() ) << "\">" << endl;
	    indent++;
	    savePopupMenu( m->menu(), mw, ts, indent );
	    indent--;
	    ts << makeIndent( indent ) << "</item>" << endl;
	}
    }
    indent--;
    ts << makeIndent( indent ) << "</menubar>" << endl;
}

void Resource::savePopupMenu( PopupMenuEditor *pm, QMainWindow *mw, QTextStream &ts, int indent )
{
    for ( PopupMenuEditorItem *i = pm->items()->first(); i; i = pm->items()->next() ) {
	QAction *a = i->action();
	if ( ::qt_cast<QSeparatorAction*>(a) )
	    ts <<  makeIndent( indent ) << "<separator/>" << endl;
	else if ( ::qt_cast<QDesignerAction*>(a) )
	    ts <<  makeIndent( indent ) << "<action name=\"" << a->name() << "\"/>" << endl;
	else if  ( ::qt_cast<QDesignerActionGroup*>(a) )
	    ts <<  makeIndent( indent ) << "<actiongroup name=\"" << a->name() << "\"/>" << endl;
	PopupMenuEditor *s =  i->subMenu();
	if ( s && s->count() ) {
	    QString n = s->name();
	    ts << makeIndent( indent ) << "<item text=\"" << entitize( a->menuText() )
	       << "\" name=\"" << entitize( n )
	       << "\" accel=\"" << entitize( a->accel() )
	       << "\">" << endl;
	    indent++;
	    savePopupMenu( s, mw, ts, indent );
	    indent--;
	    ts << makeIndent( indent ) << "</item>" << endl;
	}
    }
}

void Resource::loadToolBars( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    QMainWindow *mw = ( (QMainWindow*)formwindow->mainContainer() );
    QDesignerToolBar *tb = 0;
    while ( !n.isNull() ) {
	if ( n.tagName() == "toolbar" ) {
	    Qt::Dock dock = (Qt::Dock)n.attribute( "dock" ).toInt();
	    tb = new QDesignerToolBar( mw, dock );
	    QDomElement n2 = n.firstChild().toElement();
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "action" ) {
		    QAction *a = formwindow->findAction( n2.attribute( "name" ) );
		    if ( a ) {
			a->addTo( tb );
			tb->addAction( a );
		    }
		} else if ( n2.tagName() == "separator" ) {
		    QAction *a = new QSeparatorAction( 0 );
		    a->addTo( tb );
		    tb->addAction( a );
		} else if ( n2.tagName() == "widget" ) {
		    QWidget *w = (QWidget*)createObject( n2, tb );
		    QDesignerAction *a = new QDesignerAction( w, tb );
		    a->addTo( tb );
		    tb->addAction( a );
		    tb->installEventFilters( w );
		} else if ( n2.tagName() == "property" ) {
		    setObjectProperty( tb, n2.attribute( "name" ), n2.firstChild().toElement() );
		}
		n2 = n2.nextSibling().toElement();
	    }
	}
	n = n.nextSibling().toElement();
    }
}

void Resource::loadMenuBar( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    QMainWindow *mw = (QMainWindow*)formwindow->mainContainer();
    MenuBarEditor *mb = new MenuBarEditor( formwindow, mw );
    MetaDataBase::addEntry( mb );
    while ( !n.isNull() ) {
	if ( n.tagName() == "item" ) {
	    PopupMenuEditor * popup = new PopupMenuEditor( formwindow, mw );
	    loadPopupMenu( popup, n );
	    popup->setName( n.attribute( "name" ) );
	    mb->insertItem( n.attribute( "text" ), popup );
	    MetaDataBase::addEntry( popup );
	} else if ( n.tagName() == "property" ) {
	    setObjectProperty( mb, n.attribute( "name" ), n.firstChild().toElement() );
	} else if ( n.tagName() == "separator" ) {
	    mb->insertSeparator();
	}
	n = n.nextSibling().toElement();
    }
}

void Resource::loadPopupMenu( PopupMenuEditor *p, const QDomElement &e )
{
    MetaDataBase::addEntry( p );
    QDomElement n = e.firstChild().toElement();
    QAction *a = 0;
    while ( !n.isNull() ) {
	if ( n.tagName() == "action" || n.tagName() == "actiongroup") {
	    a = formwindow->findAction( n.attribute( "name" ) );
	    if ( a )
		p->insert( a );
	}
	if ( n.tagName() == "item" ) {
	    PopupMenuEditorItem *i = p->at( p->find( a ) );
	    if ( i ) {
		QString name = n.attribute( "name" );
		formwindow->unify( i, name, TRUE );
		i->setName( name );
		MetaDataBase::addEntry( i );
		loadPopupMenu( i->subMenu(), n );
	    }
	} else if ( n.tagName() == "separator" ) {
	    a = new QSeparatorAction( 0 );
	    p->insert( a );
	}
	n = n.nextSibling().toElement();
    }
}

bool Resource::saveFormCode( FormFile *formfile, LanguageInterface * /*langIface*/ )
{
    QString lang = formfile->project()->language();
    if ( formfile->hasTempFileName() ||
	 formfile->code().isEmpty() ||
	 !formfile->hasFormCode() ||
	 !formfile->isModified(FormFile::WFormCode) )
	return TRUE;  // There is no code to be saved.
    return saveCode( formfile->project()->makeAbsolute(formfile->codeFile()),
		     formfile->code() );
}

void Resource::loadExtraSource( FormFile *formfile, const QString &currFileName,
				LanguageInterface *langIface, bool hasFunctions )
{
    QString lang = "Qt Script";
    if ( MainWindow::self )
	lang = MainWindow::self->currProject()->language();
    LanguageInterface *iface = langIface;
    if ( hasFunctions || !iface )
	return;
    QValueList<LanguageInterface::Function> functions;
    QStringList forwards;
    QStringList includesImpl;
    QStringList includesDecl;
    QStringList vars;
    QValueList<LanguageInterface::Connection> connections;

    iface->loadFormCode( formfile->formName(),
			 currFileName + iface->formCodeExtension(),
			 functions,
			 vars,
			 connections );

    QFile f( formfile->project()->makeAbsolute( formfile->codeFile() ) );
    QString code;
    if ( f.open( IO_ReadOnly ) ) {
	QTextStream ts( &f );
	code = ts.read();
    }
    formfile->setCode( code );

    if ( !MainWindow::self || !MainWindow::self->currProject()->isCpp() )
	MetaDataBase::setupConnections( formfile, connections );

    for ( QValueList<LanguageInterface::Function>::Iterator fit = functions.begin();
	  fit != functions.end(); ++fit ) {

	if ( MetaDataBase::hasFunction( formfile->formWindow() ?
					(QObject*)formfile->formWindow() :
					(QObject*)formfile,
					(*fit).name.latin1() ) ) {
	    QString access = (*fit).access;
	    if ( !MainWindow::self || !MainWindow::self->currProject()->isCpp() )
		MetaDataBase::changeFunction( formfile->formWindow() ?
					      (QObject*)formfile->formWindow() :
					      (QObject*)formfile,
					      (*fit).name,
					      (*fit).name,
					      QString::null );
	} else {
	    QString access = (*fit).access;
	    if ( access.isEmpty() )
		access = "protected";
	    QString type = "function";
	    if ( (*fit).returnType == "void" )
		type = "slot";
	    MetaDataBase::addFunction( formfile->formWindow() ?
				       (QObject*)formfile->formWindow() :
				       (QObject*)formfile,
				       (*fit).name.latin1(), "virtual", (*fit).access,
				       type, lang, (*fit).returnType );
	}
    }
}
