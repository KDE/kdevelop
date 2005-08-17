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

#include "widgetdatabase.h"
#include "../interfaces/widgetinterface.h"

#include "../designer/kdevdesigner_part.h"

#include <kiconloader.h>
#include <kdebug.h>

#include <qapplication.h>
//Added by qt3to4:
#include <QPixmap>
#define NO_STATIC_COLORS
#include <globaldefs.h>
#include <q3strlist.h>
#include <q3dict.h>
#include <qfile.h>
#include <qtextstream.h>
#include <q3cleanuphandler.h>
#include <qfeatures.h>

#include <stdlib.h>

#include <klocale.h>

const int dbsize = 300;
const int dbcustom = 200;
const int dbdictsize = 211;
static WidgetDatabaseRecord* db[ dbsize ];
static Q3Dict<int> *className2Id = 0;
static int dbcount  = 0;
static int dbcustomcount = 200;
static Q3StrList *wGroups;
static Q3StrList *invisibleGroups;
static bool whatsThisLoaded = FALSE;
static QPluginManager<WidgetInterface> *widgetPluginManager = 0;
static bool plugins_set_up = FALSE;
static bool was_in_setup = FALSE;

Q3CleanupHandler<QPluginManager<WidgetInterface> > cleanup_manager;

WidgetDatabaseRecord::WidgetDatabaseRecord()
{
    isForm = FALSE;
    isContainer = FALSE;
    icon = 0;
    nameCounter = 0;
    isCommon = FALSE;
    isPlugin = FALSE;
}

WidgetDatabaseRecord::~WidgetDatabaseRecord()
{
    delete icon;
}


/*!
  \class WidgetDatabase widgetdatabase.h
  \brief The WidgetDatabase class holds information about widgets

  The WidgetDatabase holds information about widgets like toolTip(),
  iconSet(), ... It works Id-based, so all access functions take the
  widget id as parameter. To get the id for a widget (classname), use
  idFromClassName().

  All access functions are static.  Having multiple widgetdatabases in
  one application doesn't make sense anyway and so you don't need more
  than an instance of the widgetdatabase.

  For creating widgets, layouts, etc. see WidgetFactory.
*/

/*!
  Creates widget database. Does nothing.
*/

WidgetDatabase::WidgetDatabase()
{
}

/*!  Sets up the widget database. If the static widgetdatabase already
  exists, the functions returns immediately.
*/

void WidgetDatabase::setupDataBase( int id )
{
    was_in_setup = TRUE;
#ifndef UIC
    Q_UNUSED( id )
    if ( dbcount )
	return;
#else
    if ( dbcount && id != -2 )
	return;
    if ( dbcount && !plugins_set_up ) {
	setupPlugins();
	return;
    }
    if ( dbcount && plugins_set_up)
	return;
#endif

    wGroups = new Q3StrList;
    invisibleGroups = new Q3StrList;
    invisibleGroups->append( "Forms" );
    invisibleGroups->append( "Temp" );
    className2Id = new Q3Dict<int>( dbdictsize );
    className2Id->setAutoDelete( TRUE );

    WidgetDatabaseRecord *r = 0;

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_pushbutton.png";
    r->name = "QPushButton";
    r->group = widgetGroup( "Buttons" );
    r->toolTip = "Push Button";
    r->isCommon = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_toolbutton.png";
    r->name = "QToolButton";
    r->group = widgetGroup( "Buttons" );
    r->toolTip = "Tool Button";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_radiobutton.png";
    r->name = "QRadioButton";
    r->group = widgetGroup( "Buttons" );
    r->toolTip = "Radio Button";
    r->isCommon = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_checkbox.png";
    r->name = "QCheckBox";
    r->group = widgetGroup( "Buttons" );
    r->toolTip = "Check Box";
    r->isCommon = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_groupbox.png";
    r->name = "QGroupBox";
    r->group = widgetGroup( "Containers" );
    r->toolTip = "Group Box";
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_buttongroup.png";
    r->name = "QButtonGroup";
    r->group = widgetGroup( "Containers" );
    r->toolTip = "Button Group";
    r->isContainer = TRUE;
    r->isCommon = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_frame.png";
    r->name = "QFrame";
    r->group = widgetGroup( "Containers" );
    r->toolTip = "Frame";
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_tabwidget.png";
    r->name = "QTabWidget";
    r->group = widgetGroup( "Containers" );
    r->toolTip = "Tabwidget";
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_widgetstack.png";
    r->name = "QWidgetStack";
    r->group = widgetGroup( "Containers" );
    r->toolTip = "Widget Stack";
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_toolbox.png";
    r->name = "QToolBox";
    r->group = widgetGroup( "Containers" );
    r->toolTip = "Tool Box";
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_listbox.png";
    r->name = "QListBox";
    r->group = widgetGroup( "Views" );
    r->toolTip = "List Box";
    r->isCommon = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_listview.png";
    r->name = "QListView";
    r->group = widgetGroup( "Views" );
    r->toolTip = "List View";

    append( r );

#if !defined(QT_NO_ICONVIEW) || defined(UIC)
    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_iconview.png";
    r->name = "QIconView";
    r->group = widgetGroup( "Views" );
    r->toolTip = "Icon View";

    append( r );
#endif

#if !defined(QT_NO_TABLE)
    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_table.png";
    r->name = "QTable";
    r->group = widgetGroup( "Views" );
    r->toolTip = "Table";

    append( r );
#endif

#if !defined(QT_NO_SQL)
    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_datatable.png";
    r->includeFile = "qdatatable.h";
    r->name = "QDataTable";
    r->group = widgetGroup( "Database" );
    r->toolTip = "Data Table";

    append( r );
#endif

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_lineedit.png";
    r->name = "QLineEdit";
    r->group = widgetGroup( "Input" );
    r->toolTip = "Line Edit";
    r->isCommon = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_spinbox.png";
    r->name = "QSpinBox";
    r->group = widgetGroup( "Input" );
    r->toolTip = "Spin Box";
    r->isCommon = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_dateedit.png";
    r->name = "QDateEdit";
    r->group = widgetGroup( "Input" );
    r->toolTip = "Date Edit";
    r->includeFile = "qdatetimeedit.h";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_timeedit.png";
    r->name = "QTimeEdit";
    r->group = widgetGroup( "Input" );
    r->toolTip = "Time Edit";
    r->includeFile = "qdatetimeedit.h";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_datetimeedit.png";
    r->name = "QDateTimeEdit";
    r->group = widgetGroup( "Input" );
    r->toolTip = "Date-Time Edit";
    r->includeFile = "qdatetimeedit.h";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_multilineedit.png";
    r->name = "QMultiLineEdit";
    r->group = widgetGroup( "Temp" );
    r->toolTip = "Multi Line Edit";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_richtextedit.png";
    r->name = "QTextEdit";
    r->group = widgetGroup( "Input" );
    r->toolTip = "Rich Text Edit";
    r->isCommon = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_combobox.png";
    r->name = "QComboBox";
    r->group = widgetGroup( "Input" );
    r->toolTip = "Combo Box";
    r->isCommon = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_slider.png";
    r->name = "QSlider";
    r->group = widgetGroup( "Input" );
    r->toolTip = "Slider";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_scrollbar.png";
    r->name = "QScrollBar";
    r->group = widgetGroup( "Input" );
    r->toolTip = "Scrollbar";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_dial.png";
    r->name = "QDial";
    r->group = widgetGroup( "Input" );
    r->toolTip = "Dial";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_label.png";
    r->name = "QLabel";
    r->group = widgetGroup( "Temp" );
    r->toolTip = "Label";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_label.png";
    r->name = "TextLabel";
    r->group = widgetGroup( "Display" );
    r->toolTip = "Text Label";
    r->whatsThis = "The Text Label provides a widget to display static text.";
    r->isCommon = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_pixlabel.png";
    r->name = "PixmapLabel";
    r->group = widgetGroup( "Display" );
    r->toolTip = "Pixmap Label";
    r->whatsThis = "The Pixmap Label provides a widget to display pixmaps.";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_lcdnumber.png";
    r->name = "QLCDNumber";
    r->group = widgetGroup( "Display" );
    r->toolTip = "LCD Number";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_line.png";
    r->name = "Line";
    r->group = widgetGroup( "Display" );
    r->toolTip = "Line";
    r->includeFile = "qframe.h";
    r->whatsThis = "The Line widget provides horizontal and vertical lines.";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_progress.png";
    r->name = "QProgressBar";
    r->group = widgetGroup( "Display" );
    r->toolTip = "Progress Bar";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_textview.png";
    r->name = "QTextView";
    r->group = widgetGroup( "Temp" );
    r->toolTip = "Text View";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_textbrowser.png";
    r->name = "QTextBrowser";
    r->group = widgetGroup( "Display" );
    r->toolTip = "Text Browser";

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_spacer.png";
    r->name = "Spacer";
    r->group = widgetGroup( "Temp" );
    r->toolTip = "Spacer";
    r->whatsThis = "The Spacer provides horizontal and vertical spacing to be able to manipulate the behaviour of layouts.";

    append( r );

    r = new WidgetDatabaseRecord;
    r->name = "QWidget";
    r->isForm = TRUE;
    r->group = widgetGroup( "Forms" );

    append( r );

    r = new WidgetDatabaseRecord;
    r->name = "QDialog";
    r->group = widgetGroup( "Forms" );
    r->isForm = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->name = "QWizard";
    r->group = widgetGroup( "Forms" );
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->name = "QDesignerWizard";
    r->group = widgetGroup( "Forms" );
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->name = "QLayoutWidget";
    r->group = widgetGroup( "Temp" );
    r->includeFile = "";
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->name = "QSplitter";
    r->group = widgetGroup( "Temp" );
    r->includeFile = "qsplitter.h";
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_tabwidget.png";
    r->name = "QDesignerTabWidget";
    r->group = widgetGroup( "Temp" );
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_tabwidget.png";
    r->name = "QDesignerWidget";
    r->group = widgetGroup( "Temp" );
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "designer_tabwidget.png";
    r->name = "QDesignerDialog";
    r->group = widgetGroup( "Temp" );
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "";
    r->name = "QMainWindow";
    r->includeFile = "qmainwindow.h";
    r->group = widgetGroup( "Temp" );
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "";
    r->name = "QDesignerAction";
    r->includeFile = "qaction.h";
    r->group = widgetGroup( "Temp" );
    r->isContainer = FALSE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "";
    r->name = "QDesignerActionGroup";
    r->includeFile = "qaction.h";
    r->group = widgetGroup( "Temp" );
    r->isContainer = FALSE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "";
    r->name = "QScrollView";
    r->includeFile = "qscrollview.h";
    r->group = widgetGroup( "Temp" );
    r->isContainer = TRUE;

    append( r );

#ifndef QT_NO_SQL
    r = new WidgetDatabaseRecord;
    r->iconSet = "";
    r->name = "QDataBrowser";
    r->includeFile = "qdatabrowser.h";
    r->group = widgetGroup( "Database" );
    r->toolTip = "Data Browser";
    r->iconSet = "designer_databrowser.png";
    r->isContainer = TRUE;

    append( r );

    r = new WidgetDatabaseRecord;
    r->iconSet = "";
    r->name = "QDataView";
    r->includeFile = "qdataview.h";
    r->group = widgetGroup( "Database" );
    r->toolTip = "Data View";
    r->iconSet = "designer_dataview.png";
    r->isContainer = TRUE;

    append( r );
#endif

#ifndef UIC
    setupPlugins();
#endif
}

void WidgetDatabase::setupPlugins()
{
    if ( plugins_set_up )
	return;
    plugins_set_up = TRUE;
    QStringList widgets = widgetManager()->featureList();
    for ( QStringList::Iterator it = widgets.begin(); it != widgets.end(); ++it ) {
	if ( hasWidget( *it ) )
	    continue;
	WidgetDatabaseRecord *r = new WidgetDatabaseRecord;
	WidgetInterface *iface = 0;
	widgetManager()->queryInterface( *it, &iface );
	if ( !iface )
	    continue;

#ifndef UIC
	QIcon icon = iface->iconSet( *it );
	if ( !icon.pixmap().isNull() )
	    r->icon = new QIcon( icon );
#endif
	QString grp = iface->group( *it );
	if ( grp.isEmpty() )
	    grp = "3rd party widgets";
	r->group = widgetGroup( grp );
	r->toolTip = iface->toolTip( *it );
	r->whatsThis = iface->whatsThis( *it );
	r->includeFile = iface->includeFile( *it );
	r->isContainer = iface->isContainer( *it );
	r->name = *it;
	r->isPlugin = TRUE;
	append( r );
	iface->release();
    }
}

/*!
  Returns the number of elements in the widget database.
*/

int WidgetDatabase::count()
{
    setupDataBase( -1 );
    return dbcount;
}

/*!
  Returns the id at which the ids of custom widgets start.
*/

int WidgetDatabase::startCustom()
{
    setupDataBase( -1 );
    return dbcustom;
}

/*!
  Returns the iconset which represents the class registered as \a id.
*/

QIcon WidgetDatabase::iconSet( int id )
{
    setupDataBase( id );
    WidgetDatabaseRecord *r = at( id );
    if ( !r )
	return QIcon();
#if !defined(UIC) && !defined(RESOURCE)
    if ( !r->icon ) {
	if ( r->iconSet.isEmpty() )
	    return QIcon();
	QPixmap pix = BarIcon( r->iconSet, KDevDesignerPartFactory::instance() );
	if ( pix.isNull() )
	    pix = QPixmap( r->iconSet );
	r->icon = new QIcon( pix );
    }
    return *r->icon;
#else
    return QIcon();
#endif
}

/*!
  Returns the classname of the widget which is registered as \a id.
*/

QString WidgetDatabase::className( int id )
{
    setupDataBase( id );
    WidgetDatabaseRecord *r = at( id );
    if ( !r )
	return QString::null;
    return r->name;
}

/*!
  Returns the group the widget registered as \a id belongs to.
*/

QString WidgetDatabase::group( int id )
{
    setupDataBase( id );
    WidgetDatabaseRecord *r = at( id );
    if ( !r )
	return QString::null;
    return r->group;
}

/*!
  Returns the tooltip text of the widget which is registered as \a id.
*/

QString WidgetDatabase::toolTip( int id )
{
    setupDataBase( id );
    WidgetDatabaseRecord *r = at( id );
    if ( !r )
	return QString::null;
    return r->toolTip;
}

/*!
  Returns the what's this? text of the widget which is registered as \a id.
*/

QString WidgetDatabase::whatsThis( int id )
{
    setupDataBase( id );
    WidgetDatabaseRecord *r = at( id );
    if ( !r )
	return QString::null;
    return r->whatsThis;
}

/*!
  Returns the include file if the widget which is registered as \a id.
*/

QString WidgetDatabase::includeFile( int id )
{
    setupDataBase( id );
    WidgetDatabaseRecord *r = at( id );
    if ( !r )
	return QString::null;
    if ( r->includeFile.isNull() )
	return r->name.lower() + ".h";
    return r->includeFile;
}

/*!  Returns whether the widget registered as \a id is a form.
*/
bool WidgetDatabase::isForm( int id )
{
    setupDataBase( id );
    WidgetDatabaseRecord *r = at( id );
    if ( !r )
	return FALSE;
    return r->isForm;
}

/*!  Returns whether the widget registered as \a id can have children.
*/

bool WidgetDatabase::isContainer( int id )
{
    setupDataBase( id );
    WidgetDatabaseRecord *r = at( id );
    if ( !r )
	return FALSE;
    return r->isContainer || r->isForm;
}

bool WidgetDatabase::isCommon( int id )
{
    setupDataBase( id );
    WidgetDatabaseRecord *r = at( id );
    if ( !r )
	return FALSE;
    return r->isCommon;
}

QString WidgetDatabase::createWidgetName( int id )
{
    setupDataBase( id );
    QString n = className( id );
    if ( n == "QLayoutWidget" )
	n = "Layout";
    if ( n[ 0 ] == 'Q' && n[ 1 ].lower() != n[ 1 ] )
	n = n.mid( 1 );
    int colonColon = n.findRev( "::" );
    if ( colonColon != -1 )
	n = n.mid( colonColon + 2 );

    WidgetDatabaseRecord *r = at( id );
    if ( !r )
	return n;
    n += QString::number( ++r->nameCounter );
    n[0] = n[0].lower();
    return n;
}

/*!  Returns the id for \a name or -1 if \a name is unknown.
 */
int WidgetDatabase::idFromClassName( const QString &name )
{
    setupDataBase( -1 );
    if ( name.isEmpty() )
	return 0;
    int *i = className2Id->find( name );
    if ( i )
	return *i;
    if ( name == "FormWindow" )
	return idFromClassName( "QLayoutWidget" );
#ifdef UIC
    setupDataBase( -2 );
    i = className2Id->find( name );
    if ( i )
	return *i;
#endif
    return -1;
}

bool WidgetDatabase::hasWidget( const QString &name )
{
    return className2Id->find( name ) != 0;
}

WidgetDatabaseRecord *WidgetDatabase::at( int index )
{
    if ( index < 0 )
	return 0;
    if ( index >= dbcustom && index < dbcustomcount )
	return db[ index ];
    if ( index < dbcount )
	return db[ index ];
    return 0;
}

void WidgetDatabase::insert( int index, WidgetDatabaseRecord *r )
{
    if ( index < 0 || index >= dbsize )
	return;
    db[ index ] = r;
    className2Id->insert( r->name, new int( index ) );
    if ( index < dbcustom )
	dbcount = QMAX( dbcount, index );
}

void WidgetDatabase::append( WidgetDatabaseRecord *r )
{
    if ( !was_in_setup )
	setupDataBase( -1 );
    insert( dbcount++, r );
}

QString WidgetDatabase::widgetGroup( const QString &g )
{
    if ( wGroups->find( g ) == -1 )
	wGroups->append( g );
    return g;
}

bool WidgetDatabase::isGroupEmpty( const QString &grp )
{
    WidgetDatabaseRecord *r = 0;
    for ( int i = 0; i < dbcount; ++i ) {
	if ( !( r = db[ i ] ) )
	    continue;
	if ( r->group == grp )
	    return FALSE;
    }
    return TRUE;
}

QString WidgetDatabase::widgetGroup( int i )
{
    setupDataBase( -1 );
    if ( i >= 0 && i < (int)wGroups->count() )
	return wGroups->at( i );
    return QString::null;
}

int WidgetDatabase::numWidgetGroups()
{
    setupDataBase( -1 );
    return wGroups->count();
}

bool WidgetDatabase::isGroupVisible( const QString &g )
{
    setupDataBase( -1 );
    return invisibleGroups->find( g ) == -1;
}

int WidgetDatabase::addCustomWidget( WidgetDatabaseRecord *r )
{
    insert( dbcustomcount++, r );
    return dbcustomcount - 1;
}

void WidgetDatabase::customWidgetClassNameChanged( const QString &oldName,
						   const QString &newName )
{
    int id = idFromClassName( oldName );
    if ( id == -1 )
	return;
    WidgetDatabaseRecord *r = db[ id ];
    r->name = newName;
    className2Id->remove( oldName );
    className2Id->insert( newName, new int( id ) );
}

bool WidgetDatabase::isCustomWidget( int id )
{
    if ( id >= dbcustom && id < dbcustomcount )
	return TRUE;
    return FALSE;
}

bool WidgetDatabase::isCustomPluginWidget( int id )
{
    setupDataBase( id );
    WidgetDatabaseRecord *r = at( id );
    if ( !r )
	return FALSE;
    return r->isPlugin;
}

bool WidgetDatabase::isWhatsThisLoaded()
{
    return whatsThisLoaded;
}

void WidgetDatabase::loadWhatsThis( const QString &docPath )
{
    QString whatsthisFile = docPath + "/whatsthis";
    QFile f( whatsthisFile );
    if ( !f.open( QIODevice::ReadOnly ) )
	return;
    QTextStream ts( &f );
    while ( !ts.atEnd() ) {
	QString s = ts.readLine();
	QStringList l = QStringList::split( " | ", s );
	int id = idFromClassName( l[ 1 ] );
	WidgetDatabaseRecord *r = at( id );
	if ( r )
	    r->whatsThis = l[ 0 ];
    }
    whatsThisLoaded = TRUE;
}


// ### Qt 3.1: make these publically accessible via QWidgetDatabase API
#if defined(UIC)
bool dbnounload = FALSE;
QStringList *dbpaths = 0;
#else
extern QString *qwf_plugin_dir;
#endif


QPluginManager<WidgetInterface> *widgetManager()
{
    if ( !widgetPluginManager ) {
	QString pluginDir = "/designer";
#if !defined(UIC)
	if ( qwf_plugin_dir )
	    pluginDir = *qwf_plugin_dir;
#endif
	widgetPluginManager = new QPluginManager<WidgetInterface>( IID_Widget, QApplication::libraryPaths(), pluginDir );
	cleanup_manager.add( &widgetPluginManager );
#if defined(UIC)
	if ( dbnounload )
	    widgetPluginManager->setAutoUnload( FALSE );
	if ( dbpaths ) {
	    QStringList::ConstIterator it = dbpaths->begin();
	    for ( ; it != dbpaths->end(); ++it )
		widgetPluginManager->addLibraryPath( *it );
	}
#endif
    }
    return widgetPluginManager;
}
