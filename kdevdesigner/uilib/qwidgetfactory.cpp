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

#include "qwidgetfactory.h"
//Added by qt3to4:
#include <QPixmap>
#include <Q3StrList>
#include <Q3ActionGroup>
#include <Q3CString>
#include <QTextStream>
#include <QVBoxLayout>
#include <QGridLayout>
#include <Q3SqlCursor>
#include <QBoxLayout>
#include <Q3ValueList>
#include <QHBoxLayout>
#include <Q3PtrList>

#include "../interfaces/languageinterface.h"
#include "../interfaces/widgetinterface.h"

#include <qfeatures.h>
#include "database2.h"
#include <qdom.h>
#include <qdir.h>
#include <qlayout.h>
#include <qmetaobject.h>
#include <domtool.h>
#include <uib.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qobject.h>
#include <private/qpluginmanager_p.h>
#include <qmime.h>
#include <q3dragobject.h>

#ifndef QT_NO_SQL
#include <qsqlrecord.h>
#include <qsqldatabase.h>
#include <q3datatable.h>
#endif

// include all Qt widgets we support
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <q3groupbox.h>
#include <q3buttongroup.h>
#include <q3iconview.h>
#include <q3header.h>
#ifndef QT_NO_TABLE
#include <q3table.h>
#endif
#include <q3listbox.h>
#include <q3listview.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <q3multilineedit.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <q3wizard.h>
#include <qlcdnumber.h>
#include <q3progressbar.h>
#include <q3textview.h>
#include <q3textbrowser.h>
#include <qdial.h>
#include <qslider.h>
#include <q3frame.h>
#include <q3widgetstack.h>
#include <qtoolbox.h>
#include <q3textedit.h>
#include <qscrollbar.h>
#include <q3mainwindow.h>
#include <qsplitter.h>
#include <qaction.h>
#include <q3popupmenu.h>
#include <qmenubar.h>
#include <q3popupmenu.h>
#include <q3datetimeedit.h>
#include <q3vbox.h>
#include <q3hbox.h>
#include <q3grid.h>

#include <stdlib.h>

class QWidgetFactoryPrivate
{
public:
    Q3CString translationContext;
    Q3ListViewItem *lastItem;
    Q3Dict<bool> customWidgets;
};

static Q3PtrList<QWidgetFactory> widgetFactories;
static QPluginManager<LanguageInterface> *languageInterfaceManager = 0;
static QPluginManager<WidgetInterface> *widgetInterfaceManager = 0;

static QMap<QString, bool> *availableWidgetMap = 0;
static QStringList *availableWidgetList = 0;

QMap<QWidget*, QString> *qwf_forms = 0;
QString *qwf_language = 0;
bool qwf_execute_code = TRUE;
bool qwf_stays_on_top = FALSE;
QString qwf_currFileName = "";
QObject *qwf_form_object = 0;
QString *qwf_plugin_dir = 0;

static void setupPluginDir()
{
    if ( !qwf_plugin_dir )
	qwf_plugin_dir = new QString( "/designer" );
}

static void setupWidgetListAndMap()
{
    if ( availableWidgetMap )
	return;
    setupPluginDir();
    availableWidgetList = new QStringList;
    (*availableWidgetList) << "QPushButton" << "QToolButton" << "QCheckBox" << "QRadioButton"
			   << "QGroupBox" << "QButtonGroup" << "QIconView" << "QTable"
			   << "QListBox" << "QListView" << "QLineEdit" << "QSpinBox"
			   << "QMultiLineEdit" << "QLabel" << "TextLabel" << "PixmapLabel"
			   << "QLayoutWidget" << "QTabWidget" << "QComboBox"
			   << "QWidget" << "QDialog" << "QWizard" << "QLCDNumber";
    // put less stress on the compiler by limiting the template nesting depth
    (*availableWidgetList) << "QProgressBar" << "QTextView" << "QTextBrowser"
			   << "QDial" << "QSlider" << "QFrame" << "Line" << "QTextEdit"
			   << "QDateEdit" << "QTimeEdit" << "QDateTimeEdit" << "QScrollBar"
			   << "QPopupMenu" << "QWidgetStack" << "QMainWindow"
			   << "QDataTable" << "QDataBrowser" << "QDataView"
			   << "QVBox" << "QHBox" << "QGrid" << "QToolBox";

    if ( !widgetInterfaceManager )
	widgetInterfaceManager =
	    new QPluginManager<WidgetInterface>( IID_Widget, QApplication::libraryPaths(), *qwf_plugin_dir );

    QStringList l = widgetInterfaceManager->featureList();
    QStringList::Iterator it;
    for ( it = l.begin(); it != l.end(); ++it )
	(*availableWidgetList) << *it;

    availableWidgetMap = new QMap<QString, bool>;
    for ( it = availableWidgetList->begin(); it != availableWidgetList->end(); ++it )
	availableWidgetMap->insert( *it, TRUE );
}

static QImage loadImageData( const QString& format, ulong len, QByteArray data )
{
    QImage img;
    if ( format == "XPM.GZ" || format == "XBM.GZ" ) {
	if ( len < data.size() * 10 )
	    len = data.size() * 10;
	// qUncompress() expects the first 4 bytes to be the expected length of
	// the uncompressed data
	QByteArray dataTmp( data.size() + 4 );
	memcpy( dataTmp.data()+4, data.data(), data.size() );
	dataTmp[0] = ( len & 0xff000000 ) >> 24;
	dataTmp[1] = ( len & 0x00ff0000 ) >> 16;
	dataTmp[2] = ( len & 0x0000ff00 ) >> 8;
	dataTmp[3] = ( len & 0x000000ff );
	QByteArray baunzip = qUncompress( dataTmp );
	len = baunzip.size();
	img.loadFromData( (const uchar*)baunzip.data(), len, format.left(format.find('.')) );
    } else {
	img.loadFromData( (const uchar*)data.data(), data.size(), format );
    }
    return img;
}

static QSizePolicy::SizeType stringToSizeType( const QString& str )
{
    if ( str == "Fixed" ) {
	return QSizePolicy::Fixed;
    } else if ( str == "Minimum" ) {
	return QSizePolicy::Minimum;
    } else if ( str == "Maximum" ) {
	return QSizePolicy::Maximum;
    } else if ( str == "Preferred" ) {
	return QSizePolicy::Preferred;
    } else if ( str == "MinimumExpanding" ) {
	return QSizePolicy::MinimumExpanding;
    } else if ( str == "Expanding" ) {
	return QSizePolicy::Expanding;
    } else {
	return QSizePolicy::Ignored;
    }
}


/*!
  \class QWidgetFactory

  \brief The QWidgetFactory class provides for the dynamic creation of widgets
  from Qt Designer .ui files.

  This class basically offers two things:

  \list

  \i Dynamically creating widgets from \link designer-manual.book Qt
  Designer\endlink user interface description files.
  You can do this using the static function QWidgetFactory::create().
  This function also performs signal and slot connections, tab
  ordering, etc., as defined in the .ui file, and returns the
  top-level widget in the .ui file. After creating the widget you can
  use QObject::child() and QObject::queryList() to access child
  widgets of this returned widget.

  \i Adding additional widget factories to be able to create custom
  widgets. See createWidget() for details.

  \endlist

  This class is not included in the Qt library itself. To use it you
  must link against \c libqui.so (Unix) or \c qui.lib (Windows), which is
  built into \c INSTALL/lib if you built \e{Qt Designer} (\c INSTALL is
  the directory where Qt is installed ).

  See the "Creating Dynamic Dialogs from .ui Files" section of the \link
  designer-manual.book Qt Designer manual\endlink for an example. See
  also the \l{QWidgetPlugin} class and the \link plugins-howto.html
  Plugins documentation\endlink.
*/

/*! Constructs a QWidgetFactory. */

QWidgetFactory::QWidgetFactory()
    : d( new QWidgetFactoryPrivate() ), dbControls( 0 ),
      usePixmapCollection( FALSE ), defMargin( 11 ), defSpacing( 6 )
{
    widgetFactories.setAutoDelete( TRUE );
    d->customWidgets.setAutoDelete( TRUE );
}

/*! \fn QWidgetFactory::~QWidgetFactory()
    Destructor.
*/
QWidgetFactory::~QWidgetFactory()
{
    delete d;
}

/*!

    Loads the \e{Qt Designer} user interface description file \a uiFile
  and returns the top-level widget in that description. \a parent and
  \a name are passed to the constructor of the top-level widget.

  This function also performs signal and slot connections, tab
  ordering, etc., as described in the .ui file. In \e{Qt Designer} it
  is possible to add custom slots to a form and connect to them. If
  you want these connections to be made, you must create a class
  derived from QObject, which implements all these slots. Then pass an
  instance of the object as \a connector to this function. If you do
  this, the connections to the custom slots will be done using the \a
  connector as slot.

  If something fails, 0 is returned.

  The ownership of the returned widget is passed to the caller.
*/

QWidget *QWidgetFactory::create( const QString &uiFile, QObject *connector,
				 QWidget *parent, const char *name )
{
    setupPluginDir();
    QFile f( uiFile );
    bool failed = FALSE;
    if ( !f.open( QIODevice::ReadOnly ) )
	failed = TRUE;
    if ( failed && qApp->type() == QApplication::Tty ) {
	// for QSA: If we have no GUI, we have no form definition
	// files, but just the code. So try if only the code exists.
	f.setName( uiFile + ".qs" );
	failed = !f.open( QIODevice::ReadOnly );
    }
    if ( failed )
	return 0;

    qwf_currFileName = uiFile;
    QWidget *w = QWidgetFactory::create( &f, connector, parent, name );
    if ( !qwf_forms )
	qwf_forms = new QMap<QWidget*, QString>;
    qwf_forms->insert( w, uiFile );
    return w;
}

#undef slots

/*!  \overload
    Loads the user interface description from device \a dev.
 */

QWidget *QWidgetFactory::create( QIODevice *dev, QObject *connector, QWidget *parent, const char *name )
{
    setupPluginDir();
    QWidget *w = 0;
    QDomDocument doc;
    QString errMsg;
    int errLine;

    QWidgetFactory *widgetFactory = new QWidgetFactory;
    widgetFactory->toplevel = 0;

    // If we have no GUI, we only want to load the code
    if ( qApp->type() != QApplication::Tty ) {
	qlonglong start = dev->at();
	Q_UINT32 magic;
	QDataStream in( dev );
	in >> magic;
	if ( magic == UibMagic ) {
	    w = widgetFactory->createFromUibFile( in, connector, parent, name );
	} else {
	    in.unsetDevice();
	    dev->at( start );
	    if ( doc.setContent( dev, &errMsg, &errLine ) ) {
		w = widgetFactory->createFromUiFile( doc, connector, parent, name );
	    } else {
		// qDebug( QString("Parse error: ") + errMsg + QString(" in line %d"), errLine );
	    }
	}
	if ( !w ) {
	    delete widgetFactory;
	    return 0;
	}
    }

    if ( !languageInterfaceManager )
	languageInterfaceManager =
	    new QPluginManager<LanguageInterface>( IID_Language, QApplication::libraryPaths(), *qwf_plugin_dir );
    widgetFactory->loadExtraSource();

    if ( widgetFactory->toplevel ) {
#ifndef QT_NO_SQL
	QMap<QWidget*, SqlWidgetConnection>::Iterator cit = widgetFactory->sqlWidgetConnections.begin();
	for( ; cit != widgetFactory->sqlWidgetConnections.end(); ++cit ) {
	    if ( widgetFactory->noDatabaseWidgets.find( cit.key()->name() ) !=
		 widgetFactory->noDatabaseWidgets.end() )
		continue;
	    if ( cit.key()->inherits( "QDesignerDataBrowser2" ) )
		( (QDesignerDataBrowser2*)cit.key() )->initPreview( (*cit).conn, (*cit).table,
								    cit.key(), *(*cit).dbControls );
	    else if ( cit.key()->inherits( "QDesignerDataView2" ) )
		( (QDesignerDataView2*)cit.key() )->initPreview( (*cit).conn, (*cit).table,
								 cit.key(), *(*cit).dbControls );
	}

	for ( QMap<QString, QStringList>::Iterator it = widgetFactory->dbTables.begin();
	      it != widgetFactory->dbTables.end(); ++it ) {
	    Q3DataTable *table = (Q3DataTable*)widgetFactory->toplevel->child( it.key(), "QDataTable" );
	    if ( !table )
		continue;
	    if ( widgetFactory->noDatabaseWidgets.find( table->name() ) !=
		 widgetFactory->noDatabaseWidgets.end() )
		continue;
	    Q3ValueList<Field> fieldMap = *widgetFactory->fieldMaps.find( table );
	    QString conn = (*it)[ 0 ];
	    Q3SqlCursor* c = 0;
	    QSqlDatabase *db = 0;
	    if ( conn.isEmpty() || conn == "(default)" ) {
		db = QSqlDatabase::database();
		c = new Q3SqlCursor( (*it)[ 1 ] );
	    } else {
		db = QSqlDatabase::database( conn );
		c = new Q3SqlCursor( (*it)[ 1 ], TRUE, db );
	    }
	    if ( db ) {
		table->setSqlCursor( c, fieldMap.isEmpty(), TRUE );
		table->refresh( Q3DataTable::RefreshAll );
	    }
	}
#endif
    }

    for ( QMap<QString, QString>::Iterator it = widgetFactory->buddies.begin();
	  it != widgetFactory->buddies.end(); ++it ) {
	QLabel *label = (QLabel*)widgetFactory->toplevel->child( it.key(), "QLabel" );
	QWidget *buddy = (QWidget*)widgetFactory->toplevel->child( *it, "QWidget" );
	if ( label && buddy )
	    label->setBuddy( buddy );
    }

    delete widgetFactory;

    QApplication::sendPostedEvents();

    return w;
}

QWidget *QWidgetFactory::createFromUiFile( QDomDocument doc, QObject *connector,
	QWidget *parent, const char *name )
{
    DomTool::fixDocument( doc );

    uiFileVersion = doc.firstChild().toElement().attribute("version");
    QDomElement e = doc.firstChild().toElement().firstChild().toElement();

    QDomElement variables = e;
    while ( variables.tagName() != "variables" && !variables.isNull() )
	variables = variables.nextSibling().toElement();

    QDomElement slots = e;
    while ( slots.tagName() != "slots" && !slots.isNull() )
	slots = slots.nextSibling().toElement();

    QDomElement connections = e;
    while ( connections.tagName() != "connections" && !connections.isNull() )
	connections = connections.nextSibling().toElement();

    QDomElement imageCollection = e;
    while ( imageCollection.tagName() != "images" && !imageCollection.isNull() )
	imageCollection = imageCollection.nextSibling().toElement();

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

    QDomElement functions = e;
    while ( functions.tagName() != "functions" && !functions.isNull() )
	functions = functions.nextSibling().toElement();

    QDomElement widget;
    while ( !e.isNull() ) {
	if ( e.tagName() == "class" ) {
	    d->translationContext = e.firstChild().toText().data();
	} else if ( e.tagName() == "widget" ) {
	    widget = e;
	} else if ( e.tagName() == "pixmapinproject" ) {
	    usePixmapCollection = TRUE;
	} else if ( e.tagName() == "layoutdefaults" ) {
	    defSpacing = e.attribute( "spacing", QString::number( defSpacing ) ).toInt();
	    defMargin = e.attribute( "margin", QString::number( defMargin ) ).toInt();
	}
	e = e.nextSibling().toElement();
    }

    if ( !imageCollection.isNull() )
	loadImageCollection( imageCollection );

    createWidgetInternal( widget, parent, 0, widget.attribute("class", "QWidget") );
    QWidget *w = toplevel;
    if ( !w )
	return 0;

    if ( !actions.isNull() )
	loadActions( actions );
    if ( !toolbars.isNull() )
	loadToolBars( toolbars );
    if ( !menubar.isNull() )
	loadMenuBar( menubar );

    if ( !connections.isNull() )
	loadConnections( connections, connector );
    if ( w && name && qstrlen( name ) > 0 )
	w->setName( name );

    if ( !tabOrder.isNull() )
	loadTabOrder( tabOrder );

#if 0
    if ( !functions.isNull() ) // compatibiliy with early 3.0 betas
	loadFunctions( functions );
#endif

    return w;
}

void QWidgetFactory::unpackUInt16( QDataStream& in, Q_UINT16& n )
{
    Q_UINT8 half;
    in >> half;
    if ( half == 255 ) {
	in >> n;
    } else {
	n = half;
    }
}

void QWidgetFactory::unpackUInt32( QDataStream& in, Q_UINT32& n )
{
    Q_UINT16 half;
    in >> half;
    if ( half == 65535 ) {
	in >> n;
    } else {
	n = half;
    }
}

void QWidgetFactory::unpackByteArray( QDataStream& in, QByteArray& array )
{
    Q_UINT32 size;
    unpackUInt32( in, size );
    array.resize( size );
    in.readRawBytes( array.data(), size );
}

void QWidgetFactory::unpackCString( const UibStrTable& strings, QDataStream& in,
				    Q3CString& cstr )
{
    Q_UINT32 n;
    unpackUInt32( in, n );
    cstr = strings.asCString( n );
}

void QWidgetFactory::unpackString( const UibStrTable& strings, QDataStream& in,
				   QString& str )
{
    Q_UINT32 n;
    unpackUInt32( in, n );
    str = strings.asString( n );
}

void QWidgetFactory::unpackStringSplit( const UibStrTable& strings,
					QDataStream& in, QString& str )
{
    QString remainder;
    unpackString( strings, in, str );
    unpackString( strings, in, remainder );
    str += remainder;
}

void QWidgetFactory::unpackVariant( const UibStrTable& strings, QDataStream& in,
				    QVariant& value )
{
    QString imageName;
    Q_UINT32 number;
    Q_UINT16 count;
    Q_UINT16 x;
    Q_UINT16 y;
    Q_UINT16 width;
    Q_UINT16 height;
    Q_UINT8 bit;
    Q_UINT8 type;

    in >> type;

    switch ( type ) {
    case QVariant::String:
	unpackString( strings, in, value.asString() );
	break;
    case QVariant::Pixmap:
	unpackString( strings, in, imageName );
	if ( imageName.isEmpty() ) {
	    value.asPixmap() = QPixmap();
	} else {
	    value.asPixmap() = loadPixmap( imageName );
	}
	break;
    case QVariant::Image:
	unpackString( strings, in, imageName );
	if ( imageName.isEmpty() ) {
	    value.asImage() = QImage();
	} else {
	    value.asImage() = loadFromCollection( imageName );
	}
	break;
    case QCoreVariant::Icon:
	unpackString( strings, in, imageName );
	if ( imageName.isEmpty() ) {
	    value.asIconSet() = QIcon();
	} else {
	    value.asIconSet() = QIcon( loadPixmap(imageName) );
	}
	break;
    case QVariant::StringList:
	unpackUInt16( in, count );
	while ( count-- ) {
	    QString str;
	    unpackString( strings, in, str );
	    value.asStringList().append( str );
	}
	break;
    case QVariant::Rect:
	unpackUInt16( in, x );
	unpackUInt16( in, y );
	unpackUInt16( in, width );
	unpackUInt16( in, height );
	value = QRect( x, y, width, height );
	break;
    case QVariant::Size:
	unpackUInt16( in, width );
	unpackUInt16( in, height );
	value = QSize( width, height );
	break;
    case QVariant::Color:
	in >> value.asColor();
	break;
    case QVariant::Point:
	unpackUInt16( in, x );
	unpackUInt16( in, y );
	value = QPoint( x, y );
	break;
    case QVariant::Int:
	unpackUInt32( in, number );
	value = (int) number;
	break;
    case QVariant::Bool:
	in >> bit;
	value = QVariant( bit != 0, 0 );
	break;
    case QVariant::Double:
	in >> value.asDouble();
	break;
    case QVariant::CString:
	unpackCString( strings, in, value.asCString() );
	break;
    case QVariant::Cursor:
	in >> value.asCursor();
	break;
    case QVariant::Date:
	in >> value.asDate();
	break;
    case QVariant::Time:
	in >> value.asTime();
	break;
    case QVariant::DateTime:
	in >> value.asDateTime();
	break;
    default:
	in >> value;
    }
}

void QWidgetFactory::inputSpacer( const UibStrTable& strings, QDataStream& in,
				  QLayout *parent )
{
    Q3CString name;
    QVariant value;
    Q3CString comment;
    QSizePolicy::SizeType sizeType = QSizePolicy::Preferred;
    bool vertical = FALSE;
    int w = 0;
    int h = 0;
    Q_UINT16 column = 0;
    Q_UINT16 row = 0;
    Q_UINT16 colspan = 1;
    Q_UINT16 rowspan = 1;
    Q_UINT8 objectTag;

    in >> objectTag;
    while ( !in.atEnd() && objectTag != Object_End ) {
	switch ( objectTag ) {
	case Object_GridCell:
	    unpackUInt16( in, column );
	    unpackUInt16( in, row );
	    unpackUInt16( in, colspan );
	    unpackUInt16( in, rowspan );
	    break;
	case Object_VariantProperty:
	    unpackCString( strings, in, name );
	    unpackVariant( strings, in, value );

	    if ( name == "orientation" ) {
		vertical = ( value == "Vertical" );
	    } else if ( name == "sizeHint" ) {
		w = value.toSize().width();
		h = value.toSize().height();
	    } else if ( name == "sizeType" ) {
		sizeType = stringToSizeType( value.toString() );
	    }
	    break;
	default:
	    qFatal( "Corrupt" );
	}
	in >> objectTag;
    }

    if ( parent != 0 ) {
	QSpacerItem *spacer;
	if ( vertical ) {
	    spacer = new QSpacerItem( w, h, QSizePolicy::Minimum, sizeType );
	} else {
	    spacer = new QSpacerItem( w, h, sizeType, QSizePolicy::Minimum );
	}

	if ( parent->inherits("QGridLayout") ) {
	    ((QGridLayout *) parent)->addMultiCell( spacer, row,
		    row + rowspan - 1, column, column + colspan - 1,
		    vertical ? Qt::AlignHCenter : Qt::AlignVCenter );
	} else {
	    parent->addItem( spacer );
	}
    }
}

void QWidgetFactory::inputColumnOrRow( const UibStrTable& strings,
				       QDataStream& in, QObject *parent,
				       bool isRow )
{
    QString text;
    QPixmap pixmap;
    QString field;
    bool clickable = TRUE;
    bool resizable = TRUE;

    Q3CString name;
    QVariant value;
    Q3CString comment;
    QString str;
    Q_UINT8 objectTag;

    in >> objectTag;
    while ( !in.atEnd() && objectTag != Object_End ) {
	switch ( objectTag ) {
	case Object_TextProperty:
	    unpackCString( strings, in, name );
	    unpackCString( strings, in, value.asCString() );
	    unpackCString( strings, in, comment );
	    str = translate( value.asCString().data(), comment.data() );

	    if ( name == "field" ) {
		field = str;
	    } else if ( name == "text" ) {
		text = str;
	    }
	    break;
	case Object_VariantProperty:
	    unpackCString( strings, in, name );
	    unpackVariant( strings, in, value );

	    if ( name == "clickable" ) {
		clickable = value.toBool();
	    } else if ( name == "pixmap" ) {
		pixmap = value.asPixmap();
	    } else if ( name == "resizable" ) {
		resizable = value.toBool();
	    }
	    break;
	default:
	    qFatal( "Corrupt" );
	}
	in >> objectTag;
    }

    if ( parent != 0 ) {
	if ( parent->inherits("QListView") ) {
	    createListViewColumn( (Q3ListView *) parent, text, pixmap, clickable,
				  resizable );
#ifndef QT_NO_TABLE
	} else if ( parent->inherits("QTable") ) {
	    createTableColumnOrRow( (Q3Table *) parent, text, pixmap, field,
				    isRow );
#endif
	}
    }
}

void QWidgetFactory::inputItem( const UibStrTable& strings, QDataStream& in,
				QObject *parent, Q3ListViewItem *parentItem )
{
    QStringList texts;
    Q3ValueList<QPixmap> pixmaps;
    Q3CString name;
    QVariant value;
    Q3CString comment;
    Q_UINT8 objectTag;

    Q3ListView *listView = 0;
    if ( parent != 0 && parent->inherits("QListView") )
	parent = (Q3ListView *) parent;
    Q3ListViewItem *item = 0;
    if ( listView != 0 ) {
	if ( parentItem == 0 ) {
	    item = new Q3ListViewItem( listView, d->lastItem );
	} else {
	    item = new Q3ListViewItem( parentItem, d->lastItem );
	}
	d->lastItem = item;
    }

    in >> objectTag;
    while ( !in.atEnd() && objectTag != Object_End ) {
	switch ( objectTag ) {
	case Object_Item:
	    if ( listView != 0 )
		d->lastItem->setOpen( TRUE );
	    inputItem( strings, in, parent, item );
	    break;
	case Object_TextProperty:
	    unpackCString( strings, in, name );
	    unpackCString( strings, in, value.asCString() );
	    unpackCString( strings, in, comment );

	    if ( name == "text" )
		texts << translate( value.asCString().data(), comment.data() );
	    break;
	case Object_VariantProperty:
	    unpackCString( strings, in, name );
	    unpackVariant( strings, in, value );

	    if ( name == "pixmap" )
		pixmaps << value.asPixmap();
	    break;
	default:
	    qFatal( "Corrupt" );
	}
	in >> objectTag;
    }

    if ( listView != 0 ) {
	int i = 0;
	QStringList::ConstIterator t = texts.begin();
	while ( t != texts.end() ) {
	    item->setText( i, *t );
	    ++i;
	    ++t;
	}

	int j = 0;
	Q3ValueList<QPixmap>::ConstIterator p = pixmaps.begin();
	while ( p != pixmaps.end() ) {
	    item->setPixmap( j, *p );
	    ++j;
	    ++p;
	}
    } else {
	QString text = texts.last();
	QPixmap pixmap = pixmaps.last();

	if ( parent != 0 ) {
	    if ( parent->inherits("QComboBox") ||
		 parent->inherits("QListBox") ) {
		Q3ListBox *listBox = (Q3ListBox *) parent->qt_cast( "QListBox" );
		if ( listBox == 0 )
		    listBox = ((QComboBox *) parent)->listBox();

		if ( pixmap.isNull() ) {
		    (void) new Q3ListBoxText( listBox, text );
		} else {
		    (void) new Q3ListBoxPixmap( listBox, pixmap, text );
		}
    #ifndef QT_NO_ICONVIEW
	    } else if ( parent->inherits("QIconView") ) {
		(void) new Q3IconViewItem( (Q3IconView *) parent, text, pixmap );
    #endif
	    }
	}
    }
}

void QWidgetFactory::inputMenuItem( QObject **objects,
				    const UibStrTable& strings, QDataStream& in,
				    QMenuBar *menuBar )
{
    Q3CString name;
    Q3CString text;
    Q_UINT16 actionNo;
    Q_UINT8 objectTag;

    unpackCString( strings, in, name );
    unpackCString( strings, in, text );

    Q3PopupMenu *popupMenu = new Q3PopupMenu( menuBar->parentWidget(), name );

    in >> objectTag;
    while ( !in.atEnd() && objectTag != Object_End ) {
	switch ( objectTag ) {
	case Object_ActionRef:
	    unpackUInt16( in, actionNo );
	    ((QAction *) objects[actionNo])->addTo( popupMenu );
	    break;
	case Object_Separator:
	    popupMenu->insertSeparator();
	    break;
	default:
	    qFatal( "Corrupt" );
	}
	in >> objectTag;
    }
    menuBar->insertItem( translate(text.data()), popupMenu );
}

QObject *QWidgetFactory::inputObject( QObject **objects, int& numObjects,
				      const UibStrTable& strings,
				      QDataStream& in, QWidget *ancestorWidget,
				      QObject *parent, Q3CString className )
{
    QObject *obj = 0;
    QWidget *widget = 0;
    QLayout *layout = 0;
    QWidget *parentWidget = 0;
    QLayout *parentLayout = 0;

    bool isQObject = !className.isEmpty();
    if ( isQObject ) {
	if ( parent != 0 ) {
	    if ( parent->isWidgetType() ) {
		if ( parent->inherits("QMainWindow") ) {
		    parentWidget = ((Q3MainWindow *) parent)->centralWidget();
		} else {
		    parentWidget = (QWidget *) parent;
		}
	    } else if ( parent->inherits("QLayout") ) {
		parentLayout = (QLayout *) parent;
		parentWidget = ancestorWidget;
	    }
	}

	if ( className == "QAction" ) {
	    unpackCString( strings, in, className );
	    if ( className == "QActionGroup" ) {
		obj = new Q3ActionGroup( parent );
	    } else {
		obj = new QAction( parent );
	    }
	} else if ( className == "QLayout" ) {
	    unpackCString( strings, in, className );
	    LayoutType type = Grid;
	    if ( className == "QHBoxLayout" ) {
		type = HBox;
	    } else if ( className == "QVBoxLayout" ) {
		type = VBox;
	    }
	    if ( parentLayout != 0 && parentLayout->inherits("QGridLayout") ) {
		layout = createLayout( 0, 0, type );
	    } else {
		layout = createLayout( parentWidget, parentLayout, type );
	    }
	    obj = layout;
	} else if ( className == "QMenuBar" ) {
	    unpackCString( strings, in, className );
	    widget = ((Q3MainWindow *) parent)->menuBar();
	    obj = widget;
	} else if ( className == "QToolBar" ) {
	    Q_UINT8 dock;
	    in >> dock;
	    unpackCString( strings, in, className );
	    widget = new Q3ToolBar( QString::null, (Q3MainWindow *) parent,
				   (Qt::ToolBarDock) dock );
	    obj = widget;
	} else if ( className == "QWidget" ) {
	    unpackCString( strings, in, className );
	    widget = createWidget( className, parentWidget, 0 );
	    obj = widget;
	}

	if ( widget != 0 )
	    ancestorWidget = widget;
	d->lastItem = 0;
	objects[numObjects++] = obj;
    }

    Q3CString name;
    QVariant value;
    Q3CString comment;
    QString str;
    Q_UINT16 actionNo;
    int metAttribute = 0;
    Q_UINT16 column = 0;
    Q_UINT16 row = 0;
    Q_UINT16 colspan = 1;
    Q_UINT16 rowspan = 1;
    Q_UINT8 paletteTag;
    Q_UINT8 objectTag;

    in >> objectTag;
    while ( !in.atEnd() && objectTag != Object_End ) {
	switch ( objectTag ) {
	case Object_ActionRef:
	    unpackUInt16( in, actionNo );
	    ((QAction *) objects[actionNo])->addTo( (Q3ToolBar *) widget );
	    break;
	case Object_Attribute:
	    metAttribute = 2;
	    break;
	case Object_Column:
	    inputColumnOrRow( strings, in, obj, FALSE );
	    break;
	case Object_Event:
	    unpackCString( strings, in, name );
	    unpackVariant( strings, in, value );
	    // ### do something with value.asStringList()
	    break;
	case Object_FontProperty:
	    {
		QFont font;
		QString family;
		Q_UINT16 pointSize;
		Q_UINT8 fontFlags;

		unpackCString( strings, in, name );
		in >> fontFlags;

		if ( fontFlags & Font_Family ) {
		    unpackString( strings, in, family );
		    font.setFamily( family );
		}
		if ( fontFlags & Font_PointSize ) {
		    unpackUInt16( in, pointSize );
		    font.setPointSize( pointSize );
		}
		if ( fontFlags & Font_Bold )
		    font.setBold( TRUE );
		if ( fontFlags & Font_Italic )
		    font.setItalic( TRUE );
		if ( fontFlags & Font_Underline )
		    font.setUnderline( TRUE );
		if ( fontFlags & Font_StrikeOut )
		    font.setStrikeOut( TRUE );

		if ( obj != 0 )
		    setProperty( obj, name, font );
	    }
	    break;
	case Object_GridCell:
	    unpackUInt16( in, column );
	    unpackUInt16( in, row );
	    unpackUInt16( in, colspan );
	    unpackUInt16( in, rowspan );
	    break;
	case Object_Item:
	    inputItem( strings, in, obj );
	    break;
	case Object_MenuItem:
	    inputMenuItem( objects, strings, in, (QMenuBar *) widget );
	    break;
	case Object_PaletteProperty:
	    {
		QPalette palette;
		QColorGroup colorGroup;
		QColor color;
		int role = -1;

		unpackCString( strings, in, name );

		in >> paletteTag;
		while ( !in.atEnd() && paletteTag != Palette_End ) {
		    switch ( paletteTag ) {
		    case Palette_Active:
			palette.setActive( colorGroup );
			role = -1;
			break;
		    case Palette_Inactive:
			palette.setInactive( colorGroup );
			role = -1;
			break;
		    case Palette_Disabled:
			palette.setDisabled( colorGroup );
			role = -1;
			break;
		    case Palette_Color:
			role++;
			in >> color;
			colorGroup.setColor( (QColorGroup::ColorRole) role,
					     color );
			break;
		    case Palette_Pixmap:
			unpackVariant( strings, in, value );
			colorGroup.setBrush( (QColorGroup::ColorRole) role,
					     QBrush(color, value.asPixmap()) );
			break;
		    default:
			qFatal( "Corrupt" );
		    }
		    in >> paletteTag;
		}
		if ( obj != 0 )
		    setProperty( obj, name, palette );
	    }
	    break;
	case Object_Row:
	    inputColumnOrRow( strings, in, obj, TRUE );
	    break;
	case Object_Spacer:
	    inputSpacer( strings, in, layout );
	    break;
	case Object_Separator:
	    ((Q3ToolBar *) widget)->addSeparator();
	    break;
	case Object_SubAction:
	    inputObject( objects, numObjects, strings, in, parentWidget,
			 obj != 0 ? obj : parent, "QAction" );
	    break;
	case Object_SubLayout:
	    inputObject( objects, numObjects, strings, in, parentWidget, obj,
			 "QLayout" );
	    break;
	case Object_SubWidget:
	    inputObject( objects, numObjects, strings, in, parentWidget, obj,
			 "QWidget" );
	    break;
	case Object_TextProperty:
	    unpackCString( strings, in, name );
	    unpackCString( strings, in, value.asCString() );
	    unpackCString( strings, in, comment );
	    str = translate( value.asCString().data(), comment.data() );

	    if ( metAttribute > 0 ) {
		if ( name == "title" ) {
		    if ( parent != 0 ) {
			if ( parent->inherits("QTabWidget") ) {
			    ((QTabWidget *) parent)->insertTab( widget, str );
			} else if ( parent->inherits("QWizard") ) {
			    ((Q3Wizard *) parent)->addPage( widget, str );
			}
		    }
		}
	    } else {
		if ( obj != 0 )
		    setProperty( obj, name, str );
	    }
	    break;
	case Object_VariantProperty:
	    unpackCString( strings, in, name );
	    unpackVariant( strings, in, value );

	    if ( metAttribute > 0 ) {
		if ( name == "id" ) {
		    if ( parent != 0 && parent->inherits("QWidgetStack") )
			((Q3WidgetStack *) parent)->addWidget( widget, value.toInt() );
		}
	    } else {
		if ( obj != 0 )
		    setProperty( obj, name, value );
	    }
	    break;
	default:
	    qFatal( "Corrupt" );
	}
	in >> objectTag;
	metAttribute--;
    }

    if ( parentLayout != 0 ) {
	if ( widget != 0 ) {
	    if ( parentLayout->inherits("QGridLayout") ) {
		((QGridLayout *) parentLayout)->addMultiCellWidget(
			widget, row, row + rowspan - 1, column,
			column + colspan - 1 );
	    } else {
		((QBoxLayout *) parentLayout)->addWidget( widget );
	    }
	} else if ( layout != 0 ) {
	    if ( parentLayout->inherits("QGridLayout") ) {
		((QGridLayout *) parentLayout)->addMultiCellLayout(
			layout, row, row + rowspan - 1, column,
			column + colspan - 1 );
	    }
	}
    }
    return obj;
}

QWidget *QWidgetFactory::createFromUibFile( QDataStream& in,
	QObject * /* connector */ , QWidget *parent, const char *name )
{
#define END_OF_BLOCK() \
	( in.atEnd() || in.device()->at() >= nextBlock )

    Q_UINT8 lf;
    Q_UINT8 cr;
    in >> lf;
    in >> cr;
    if ( lf != '\n' || cr != '\r' ) {
	qWarning( "File corrupted" );
	return 0;
    }

    Q_UINT8 qdatastreamVersion;
    in >> qdatastreamVersion;
    if ( (int) qdatastreamVersion > in.version() ) {
	qWarning( "Incompatible version of Qt" );
	return 0;
    }
    in.setVersion( qdatastreamVersion );

    UibStrTable strings;
    QObject **objects = 0;
    int numObjects = 0;

    Q_UINT8 blockType;
    Q_UINT32 blockSize;

    in >> blockType;
    while ( !in.atEnd() && blockType != Block_End ) {
	unpackUInt32( in, blockSize );
	qlonglong nextBlock = in.device()->at() + blockSize;

	switch ( blockType ) {
	case Block_Actions:
	    inputObject( objects, numObjects, strings, in, toplevel, toplevel );
	    break;
	case Block_Buddies:
	    {
		Q_UINT16 labelNo;
		Q_UINT16 buddyNo;

		do {
		    unpackUInt16( in, labelNo );
		    unpackUInt16( in, buddyNo );
		    QLabel *label =
			(QLabel *) objects[labelNo]->qt_cast( "QLabel" );
		    if ( label != 0 )
			label->setBuddy( (QWidget *) objects[buddyNo] );
		} while ( !END_OF_BLOCK() );
	    }
	    break;
	case Block_Connections:
	    {
		QString language = "C++";
		Q_UINT16 senderNo = 0;
		QString signal = "clicked()";
		Q_UINT16 receiverNo = 0;
		QString slot = "accept()";
		Q_UINT8 connectionFlags;

		do {
		    in >> connectionFlags;
		    if ( connectionFlags & Connection_Language )
			unpackString( strings, in, language );
		    if ( connectionFlags & Connection_Sender )
			unpackUInt16( in, senderNo );
		    if ( connectionFlags & Connection_Signal )
			unpackStringSplit( strings, in, signal );
		    if ( connectionFlags & Connection_Receiver )
			unpackUInt16( in, receiverNo );
		    if ( connectionFlags & Connection_Slot )
			unpackStringSplit( strings, in, slot );
		    // ###
#if 0
		    qWarning( "connect( %p, %s, %p, %s )", objects[senderNo],
			      signal.latin1(), objects[receiverNo],
			      slot.latin1() );
#endif
		} while ( !END_OF_BLOCK() );
	    }
	    break;
	case Block_Functions:
	    // ###
	    qWarning( "Block_Functions not supported" );
	    in.device()->at( nextBlock );
	    break;
	case Block_Images:
	    {
		QString format;
		Q_UINT32 length;
		QByteArray data;
		Image image;

		do {
		    unpackString( strings, in, image.name );
		    unpackString( strings, in, format );
		    unpackUInt32( in, length );
		    unpackByteArray( in, data );
		    image.img = loadImageData( format, length, data );
		    images += image;
		} while ( !END_OF_BLOCK() );
	    }
	    break;
	case Block_Intro:
	    {
		Q_INT16 defaultMargin;
		Q_INT16 defaultSpacing;
		Q_UINT16 maxObjects;
		Q_UINT8 introFlags;

		in >> introFlags;
		in >> defaultMargin;
		in >> defaultSpacing;
		unpackUInt16( in, maxObjects );
		unpackCString( strings, in, d->translationContext );

		if ( introFlags & Intro_Pixmapinproject )
		    usePixmapCollection = TRUE;
		if ( defaultMargin != -32768 )
		    defMargin = defaultMargin;
		if ( defaultSpacing != -32768 )
		    defSpacing = defaultSpacing;
		objects = new QObject *[maxObjects];
	    }
	    break;
	case Block_Menubar:
	    inputObject( objects, numObjects, strings, in, toplevel, toplevel,
			 "QMenuBar" );
	    break;
	case Block_Slots:
	    {
		QString language;
		QString slot;

		do {
		    unpackString( strings, in, language );
		    unpackStringSplit( strings, in, slot );
		} while ( !END_OF_BLOCK() );
	    }
	    break;
	case Block_Strings:
	    strings.readBlock( in, blockSize );
	    break;
	case Block_Tabstops:
	    {
		Q_UINT16 beforeNo;
		Q_UINT16 afterNo;

		unpackUInt16( in, beforeNo );
		while ( !END_OF_BLOCK() ) {
		    unpackUInt16( in, afterNo );
		    toplevel->setTabOrder( (QWidget *) objects[beforeNo],
					   (QWidget *) objects[afterNo] );
		    beforeNo = afterNo;
		}
	    }
	    break;
	case Block_Toolbars:
	    do {
		inputObject( objects, numObjects, strings, in, toplevel,
			     toplevel, "QToolBar" );
	    } while ( !END_OF_BLOCK() );
	    break;
	case Block_Variables:
	    // ###
	    qWarning( "Block_Variables not supported" );
	    in.device()->at( nextBlock );
	    break;
	case Block_Widget:
	    toplevel = (QWidget *)
		inputObject( objects, numObjects, strings, in, toplevel, parent,
			     "QWidget" );
	    if ( toplevel != 0 )
		toplevel->setName( name );
	    break;
	default:
	    qWarning( "Version error" );
	    return 0;
	}
	in >> blockType;
    }
    delete[] objects;
    return toplevel;
}

/*! Installs a widget factory \a factory, which normally contains
  additional widgets that can then be created using a QWidgetFactory.
  See createWidget() for further details.
*/

void QWidgetFactory::addWidgetFactory( QWidgetFactory *factory )
{
    widgetFactories.append( factory );
}

/*!
    Creates a widget of the type \a className passing \a parent and \a
    name to its constructor.

    If \a className is a widget in the Qt library, it is directly
    created by this function. If the widget isn't in the Qt library,
    each of the installed widget plugins is asked, in turn, to create
    the widget. As soon as a plugin says it can create the widget it
    is asked to do so. It may occur that none of the plugins can
    create the widget, in which case each installed widget factory is
    asked to create the widget (see addWidgetFactory()). If the widget
    cannot be created by any of these means, 0 is returned.

    If you have a custom widget, and want it to be created using the
    widget factory, there are two approaches you can use:

    \list 1

    \i Write a widget plugin. This allows you to use the widget in
    \e{Qt Designer} and in this QWidgetFactory. See the widget plugin
    documentation for further details. (See the "Creating Custom
    Widgets with Plugins" section of the \link designer-manual.book Qt
    Designer manual\endlink for an example.

    \i Subclass QWidgetFactory. Then reimplement this function to
    create and return an instance of your custom widget if \a
    className equals the name of your widget, otherwise return 0. Then
    at the beginning of your program where you want to use the widget
    factory to create widgets do a:
    \code
    QWidgetFactory::addWidgetFactory( new MyWidgetFactory );
    \endcode
    where MyWidgetFactory is your QWidgetFactory subclass.

    \endlist
*/

QWidget *QWidgetFactory::createWidget( const QString &className, QWidget *parent,
				       const char *name ) const
{
    // create widgets we know
    if ( className == "QPushButton" ) {
	return new QPushButton( parent, name );
    } else if ( className == "QToolButton" ) {
	return new QToolButton( parent, name );
    } else if ( className == "QCheckBox" ) {
	return new QCheckBox( parent, name );
    } else if ( className == "QRadioButton" ) {
	return new QRadioButton( parent, name );
    } else if ( className == "QGroupBox" ) {
	return new Q3GroupBox( parent, name );
    } else if ( className == "QButtonGroup" ) {
	return new Q3ButtonGroup( parent, name );
    } else if ( className == "QIconView" ) {
#if !defined(QT_NO_ICONVIEW)
	return new Q3IconView( parent, name );
#endif
    } else if ( className == "QTable" ) {
#if !defined(QT_NO_TABLE)
	return new Q3Table( parent, name );
#endif
    } else if ( className == "QListBox" ) {
	return new Q3ListBox( parent, name );
    } else if ( className == "QListView" ) {
	return new Q3ListView( parent, name );
    } else if ( className == "QLineEdit" ) {
	return new QLineEdit( parent, name );
    } else if ( className == "QSpinBox" ) {
	return new QSpinBox( parent, name );
    } else if ( className == "QMultiLineEdit" ) {
	return new Q3MultiLineEdit( parent, name );
    } else if ( className == "QLabel" || className == "TextLabel" || className == "PixmapLabel" ) {
	return new QLabel( parent, name );
    } else if ( className == "QLayoutWidget" ) {
	return new QWidget( parent, name );
    } else if ( className == "QTabWidget" ) {
	return new QTabWidget( parent, name );
    } else if ( className == "QComboBox" ) {
	return new QComboBox( FALSE, parent, name );
    } else if ( className == "QWidget" ) {
	if ( !qwf_stays_on_top )
	    return new QWidget( parent, name );
	return new QWidget( parent, name, Qt::WStyle_StaysOnTop );
    } else if ( className == "QDialog" ) {
	if ( !qwf_stays_on_top )
	    return new QDialog( parent, name );
	return new QDialog( parent, name, FALSE, Qt::WStyle_StaysOnTop );
    } else if ( className == "QWizard" ) {
	return  new Q3Wizard( parent, name );
    } else if ( className == "QLCDNumber" ) {
	return new QLCDNumber( parent, name );
    } else if ( className == "QProgressBar" ) {
	return new Q3ProgressBar( parent, name );
    } else if ( className == "QTextView" ) {
	return new Q3TextView( parent, name );
    } else if ( className == "QTextBrowser" ) {
	return new Q3TextBrowser( parent, name );
    } else if ( className == "QDial" ) {
	return new QDial( parent, name );
    } else if ( className == "QSlider" ) {
	return new QSlider( parent, name );
    } else if ( className == "QFrame" ) {
	return new Q3Frame( parent, name );
    } else if ( className == "QSplitter" ) {
	return new QSplitter( parent, name );
    } else if ( className == "Line" ) {
	Q3Frame *f = new Q3Frame( parent, name );
	f->setFrameStyle( Q3Frame::HLine | Q3Frame::Sunken );
	return f;
    } else if ( className == "QTextEdit" ) {
	return new Q3TextEdit( parent, name );
    } else if ( className == "QDateEdit" ) {
	return new Q3DateEdit( parent, name );
    } else if ( className == "QTimeEdit" ) {
	return new Q3TimeEdit( parent, name );
    } else if ( className == "QDateTimeEdit" ) {
	return new Q3DateTimeEdit( parent, name );
    } else if ( className == "QScrollBar" ) {
	return new QScrollBar( parent, name );
    } else if ( className == "QPopupMenu" ) {
	return new Q3PopupMenu( parent, name );
    } else if ( className == "QWidgetStack" ) {
	return new Q3WidgetStack( parent, name );
    } else if ( className == "QToolBox" ) {
	return new QToolBox( parent, name );
    } else if ( className == "QVBox" ) {
	return new Q3VBox( parent, name );
    } else if ( className == "QHBox" ) {
	return new Q3HBox( parent, name );
    } else if ( className == "QGrid" ) {
	return new Q3Grid( 4, parent, name );
    } else if ( className == "QMainWindow" ) {
	Q3MainWindow *mw = 0;
	if ( !qwf_stays_on_top )
	    mw = new Q3MainWindow( parent, name );
	else
	    mw = new Q3MainWindow( parent, name, Qt::WType_TopLevel | Qt::WStyle_StaysOnTop );
	mw->setCentralWidget( new QWidget( mw, "qt_central_widget" ) );
	mw->centralWidget()->show();
	(void)mw->statusBar();
	return mw;

    }
#if !defined(QT_NO_SQL)
    else if ( className == "QDataTable" ) {
	return new Q3DataTable( parent, name );
    } else if ( className == "QDataBrowser" ) {
	return new QDesignerDataBrowser2( parent, name );
    } else if ( className == "QDataView" ) {
	return new QDesignerDataView2( parent, name );
    }
#endif

    setupPluginDir();
    // try to create it using the loaded widget plugins
    if ( !widgetInterfaceManager )
	widgetInterfaceManager =
	    new QPluginManager<WidgetInterface>( IID_Widget, QApplication::libraryPaths(),
						 *qwf_plugin_dir );

    QInterfacePtr<WidgetInterface> iface = 0;
    widgetInterfaceManager->queryInterface( className, &iface );
    if ( iface ) {
	QWidget *w = iface->create( className, parent, name );
	if ( w ) {
	    d->customWidgets.replace( className.latin1(), new bool(TRUE) );
	    return w;
	}
    }

    // hope we have a factory which can do it
    for ( QWidgetFactory* f = widgetFactories.first(); f; f = widgetFactories.next() ) {
	QWidget *w = f->createWidget( className, parent, name );
	if ( w )
	    return w;
    }

    // no success
    return 0;
}

/*! Returns the names of the widgets, which this facory can create. */

QStringList QWidgetFactory::widgets()
{
    setupWidgetListAndMap();
    return *availableWidgetList;
}

/*! Returns whether this widget factory can create the widget \a
  widget */

bool QWidgetFactory::supportsWidget( const QString &widget )
{
    setupWidgetListAndMap();
    return ( availableWidgetMap->find( widget ) != availableWidgetMap->end() );
}

QWidget *QWidgetFactory::createWidgetInternal( const QDomElement &e, QWidget *parent,
					       QLayout* layout, const QString &classNameArg )
{
    d->lastItem = 0;
    QDomElement n = e.firstChild().toElement();
    QWidget *w = 0; // the widget that got created
    QObject *obj = 0; // gets the properties

    QString className = classNameArg;

    int row = e.attribute( "row" ).toInt();
    int col = e.attribute( "column" ).toInt();
    int rowspan = e.attribute( "rowspan" ).toInt();
    int colspan = e.attribute( "colspan" ).toInt();
    if ( rowspan < 1 )
	rowspan = 1;
    if ( colspan < 1 )
	colspan = 1;

    bool isQLayoutWidget = FALSE;

    if ( !className.isEmpty() ) {
	if ( !layout && className  == "QLayoutWidget" ) {
	    className = "QWidget";
	    isQLayoutWidget = TRUE;
	}
	if ( layout && className == "QLayoutWidget" ) {
	    // hide layout widgets
	    w = parent;
	} else {
	    obj = QWidgetFactory::createWidget( className, parent, 0 );
	    if ( !obj )
		return 0;
	    w = (QWidget*)obj;
	    if ( !toplevel )
		toplevel = w;
	    if ( w->inherits( "QMainWindow" ) )
		w = ( (Q3MainWindow*)w )->centralWidget();
	    if ( layout ) {
		switch( layoutType( layout ) ) {
		case HBox:
		    ( (QHBoxLayout*)layout )->addWidget( w );
		    break;
		case VBox:
		    ( (QVBoxLayout*)layout )->addWidget( w );
		    break;
		case Grid:
		    ( (QGridLayout*)layout )->addMultiCellWidget( w, row, row + rowspan - 1,
								  col, col + colspan - 1 );
		    break;
		default:
		    break;
		}
	    }

	    layout = 0;
	}
    }
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
    QString parentClassName = parent ? parent->className() : 0;
    bool isPlugin = parent ? !!d->customWidgets.find( parent->className() ) : FALSE;
    if ( isPlugin )
	qWarning( "####### loading custom container widgets without page support not implemented!" );
    // ### TODO loading for custom container widgets without pages
#endif
    
    int idx = 0;
    while ( !n.isNull() ) {
	if ( n.tagName() == "spacer" ) {
	    createSpacer( n, layout );
	} else if ( n.tagName() == "widget" ) {
	    QMap< QString, QString> *oldDbControls = dbControls;
	    createWidgetInternal( n, w, layout, n.attribute( "class", "QWidget" ) );
	    dbControls = oldDbControls;
	} else if ( n.tagName() == "hbox" ) {
	    QLayout *parentLayout = layout;
	    if ( layout && layout->inherits( "QGridLayout" ) )
		layout = createLayout( 0, 0, QWidgetFactory::HBox, isQLayoutWidget );
	    else
		layout = createLayout( w, layout, QWidgetFactory::HBox, isQLayoutWidget );
	    obj = layout;
	    n = n.firstChild().toElement();
	    if ( parentLayout && parentLayout->inherits( "QGridLayout" ) )
		( (QGridLayout*)parentLayout )->addMultiCellLayout( layout, row,
				    row + rowspan - 1, col, col + colspan - 1 );
	    continue;
	} else if ( n.tagName() == "grid" ) {
	    QLayout *parentLayout = layout;
	    if ( layout && layout->inherits( "QGridLayout" ) )
		layout = createLayout( 0, 0, QWidgetFactory::Grid, isQLayoutWidget );
	    else
		layout = createLayout( w, layout, QWidgetFactory::Grid, isQLayoutWidget );
	    obj = layout;
	    n = n.firstChild().toElement();
	    if ( parentLayout && parentLayout->inherits( "QGridLayout" ) )
		( (QGridLayout*)parentLayout )->addMultiCellLayout( layout, row,
				    row + rowspan - 1, col, col + colspan - 1 );
	    continue;
	} else if ( n.tagName() == "vbox" ) {
	    QLayout *parentLayout = layout;
	    if ( layout && layout->inherits( "QGridLayout" ) )
		layout = createLayout( 0, 0, QWidgetFactory::VBox, isQLayoutWidget );
	    else
		layout = createLayout( w, layout, QWidgetFactory::VBox, isQLayoutWidget );
	    obj = layout;
	    n = n.firstChild().toElement();
	    if ( parentLayout && parentLayout->inherits( "QGridLayout" ) )
		( (QGridLayout*)parentLayout )->addMultiCellLayout( layout, row,
				    row + rowspan - 1, col, col + colspan - 1 );
	    continue;
	} else if ( n.tagName() == "property" && obj ) {
	    setProperty( obj, n.attribute( "name" ), n.firstChild().toElement() );
	} else if ( n.tagName() == "attribute" && w ) {
	    QString attrib = n.attribute( "name" );
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( parent ) {
		if ( parent->inherits( "QTabWidget" ) ) {
		    if ( attrib == "title" )
			( (QTabWidget*)parent )->insertTab( w, translate( v.toString() ) );
		} else if ( parent->inherits( "QWidgetStack" ) ) {
		    if ( attrib == "id" )
			( (Q3WidgetStack*)parent )->addWidget( w, v.toInt() );
		} else if ( parent->inherits( "QToolBox" ) ) {
		    if ( attrib == "label" )
			( (QToolBox*)parent )->addItem( w, v.toString() );
		} else if ( parent->inherits( "QWizard" ) ) {
		    if ( attrib == "title" )
			( (Q3Wizard*)parent )->addPage( w, translate( v.toString() ) );
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
		} else if ( isPlugin ) {
		    if ( attrib == "label" ) {
			WidgetInterface *iface = 0;
			widgetInterfaceManager->queryInterface( parentClassName, &iface );
			if ( iface ) {
			    QWidgetContainerInterfacePrivate *iface2 = 0;
			    iface->queryInterface( IID_QWidgetContainer,
						   (QUnknownInterface**)&iface2 );
			    if ( iface2 ) {
				iface2->insertPage( parentClassName,
						    (QWidget*)parent, translate( v.toString() ), -1, w );
				iface2->release();
			    }
			    iface->release();
			}
		    }
#endif
		}
	    }
	} else if ( n.tagName() == "item" ) {
	    createItem( n, w );
	} else if ( n.tagName() == "column" || n.tagName() == "row" ) {
	    createColumn( n, w );
	}

	n = n.nextSibling().toElement();
	idx++;
    }

    return w;
}

QLayout *QWidgetFactory::createLayout( QWidget *widget, QLayout* layout,
				       LayoutType type, bool isQLayoutWidget )
{
    int spacing = defSpacing;
    int margin = defMargin;

    if ( layout || !widget || isQLayoutWidget )
	margin = 0;

    if ( !layout && widget && widget->inherits( "QTabWidget" ) )
	widget = ((QTabWidget*)widget)->currentPage();

    if ( !layout && widget && widget->inherits( "QWizard" ) )
	widget = ((Q3Wizard*)widget)->currentPage();

    if ( !layout && widget && widget->inherits( "QWidgetStack" ) )
	widget = ((Q3WidgetStack*)widget)->visibleWidget();

    if ( !layout && widget && widget->inherits( "QToolBox" ) )
	widget = ((QToolBox*)widget)->currentItem();

    QLayout *l = 0;
    int align = 0;
    if ( !layout && widget && widget->inherits( "QGroupBox" ) ) {
	Q3GroupBox *gb = (Q3GroupBox*)widget;
	gb->setColumnLayout( 0, Qt::Vertical );
	layout = gb->layout();
	layout->setMargin( 0 );
	layout->setSpacing( 0 );
	align = Qt::AlignTop;
    }
    if ( layout ) {
	switch ( type ) {
        case HBox:
	    l = new QHBoxLayout( layout );
	    break;
	case VBox:
	    l = new QVBoxLayout( layout );
	    break;
	case Grid:
	    l = new QGridLayout( layout );
	    break;
	default:
	    return 0;
	}
    } else {
	switch ( type ) {
	case HBox:
	    l = new QHBoxLayout( widget );
	    break;
	case VBox:
	    l = new QVBoxLayout( widget );
	    break;
	case Grid:
	    l = new QGridLayout( widget );
	    break;
	default:
	    return 0;
	}
    }
    l->setAlignment( align );
    l->setMargin( margin );
    l->setSpacing( spacing );
    return l;
}

QWidgetFactory::LayoutType QWidgetFactory::layoutType( QLayout *layout ) const
{
    if ( layout->inherits( "QHBoxLayout" ) )
	return HBox;
    else if ( layout->inherits( "QVBoxLayout" ) )
	return VBox;
    else if ( layout->inherits( "QGridLayout" ) )
	return Grid;
    return NoLayout;
}

void QWidgetFactory::setProperty( QObject* obj, const QString &prop,
				  QVariant value )
{
    int offset = obj->metaObject()->findProperty( prop, TRUE );

    if ( offset != -1 ) {
	if ( prop == "geometry" && obj == toplevel ) {
	    toplevel->resize( value.toRect().size() );
	} else if ( prop == "accel" ) {
	    obj->setProperty( prop, value.toKeySequence() );
	} else {
	    if ( value.type() == QVariant::String ||
		 value.type() == QVariant::CString ) {
		const QMetaProperty *metaProp =
			obj->metaObject()->property( offset, TRUE );
		if ( metaProp != 0 && metaProp->isEnumType() ) {
		    if ( metaProp->isSetType() ) {
			Q3StrList flagsCStr;
			QStringList flagsStr =
			    QStringList::split( '|', value.asString() );
			QStringList::ConstIterator f = flagsStr.begin();
			while ( f != flagsStr.end() ) {
			    flagsCStr.append( *f );
			    ++f;
			}
			value = QVariant( metaProp->keysToValue(flagsCStr) );
		    } else {
			Q3CString key = value.toCString();
			value = QVariant( metaProp->keyToValue(key) );
		    }
		}
	    }
	    obj->setProperty( prop, value );
	}
    } else {
	if ( obj->isWidgetType() ) {
	    if ( prop == "toolTip" ) {
		if ( !value.toString().isEmpty() )
		    QToolTip::add( (QWidget*)obj, translate( value.toString() ) );
	    } else if ( prop == "whatsThis" ) {
		if ( !value.toString().isEmpty() )
		    Q3WhatsThis::add( (QWidget*)obj, translate( value.toString() ) );
	    } else if ( prop == "buddy" ) {
		buddies.insert( obj->name(), value.toCString() );
	    } else if ( prop == "buttonGroupId" ) {
		if ( obj->inherits( "QButton" ) && obj->parent()->inherits( "QButtonGroup" ) )
		    ( (Q3ButtonGroup*)obj->parent() )->insert( (Q3Button*)obj, value.toInt() );
#ifndef QT_NO_SQL
	    } else if ( prop == "database" && !obj->inherits( "QDataView" )
		 && !obj->inherits( "QDataBrowser" ) ) {
		const QStringList& lst = value.asStringList();
		if ( lst.count() > 2 ) {
		    if ( dbControls )
			dbControls->insert( obj->name(), lst[ 2 ] );
		} else if ( lst.count() == 2 ) {
		    dbTables.insert( obj->name(), lst );
		}
	    } else if ( prop == "database" ) {
		const QStringList& lst = value.asStringList();
		if ( lst.count() == 2 && obj->inherits( "QWidget" ) ) {
		    SqlWidgetConnection conn( lst[ 0 ], lst[ 1 ] );
		    sqlWidgetConnections.insert( (QWidget*)obj, conn );
		    dbControls = conn.dbControls;
		}
#endif
	    } else if ( prop == "frameworkCode" ) {
		if ( value.isValid() && !value.toBool() )
		    noDatabaseWidgets << obj->name();
	    }
	}
    }
}

void QWidgetFactory::setProperty( QObject* widget, const QString &prop, const QDomElement &e )
{
    QString comment;
    QVariant value( DomTool::elementToVariant( e, QVariant(), comment ) );

    if ( e.tagName() == "string" ) {
	value = translate( value.asString(), comment );
    } else if ( e.tagName() == "pixmap" ) {
	QPixmap pix = loadPixmap( value.toString() );
	if ( !pix.isNull() )
	    value = pix;
    } else if ( e.tagName() == "iconset" ) {
	QPixmap pix = loadPixmap( value.toString() );
	if ( !pix.isNull() )
	    value = QIcon( pix );
    } else if ( e.tagName() == "image" ) {
	value = loadFromCollection( value.toString() );
    } else if ( e.tagName() == "palette" ) {
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
	value = p;
    }
    setProperty( widget, prop, value );
}

void QWidgetFactory::createSpacer( const QDomElement &e, QLayout *layout )
{
    QDomElement n = e.firstChild().toElement();
    int row = e.attribute( "row" ).toInt();
    int col = e.attribute( "column" ).toInt();
    int rowspan = e.attribute( "rowspan" ).toInt();
    int colspan = e.attribute( "colspan" ).toInt();

    Qt::Orientation orient = Qt::Horizontal;
    int w = 0, h = 0;
    QSizePolicy::SizeType sizeType = QSizePolicy::Preferred;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString prop = n.attribute( "name" );
	    if ( prop == "orientation" ) {
		if ( n.firstChild().firstChild().toText().data() == "Horizontal" )
		    orient = Qt::Horizontal;
		else
		    orient = Qt::Vertical;
	    } else if ( prop == "sizeType" ) {
		sizeType = stringToSizeType( n.firstChild().firstChild().toText().data() );
	    } else if ( prop == "sizeHint" ) {
		w = n.firstChild().firstChild().firstChild().toText().data().toInt();
		h = n.firstChild().firstChild().nextSibling().firstChild().toText().data().toInt();
	    }
	}
	n = n.nextSibling().toElement();
    }

    if ( rowspan < 1 )
	rowspan = 1;
    if ( colspan < 1 )
	colspan = 1;
    QSpacerItem *item = new QSpacerItem( w, h, orient == Qt::Horizontal ? sizeType : QSizePolicy::Minimum,
					 orient == Qt::Vertical ? sizeType : QSizePolicy::Minimum );
    if ( layout ) {
	if ( layout->inherits( "QBoxLayout" ) )
	    ( (QBoxLayout*)layout )->addItem( item );
	else
	    ( (QGridLayout*)layout )->addMultiCell( item, row, row + rowspan - 1, col, col + colspan - 1,
						    orient == Qt::Horizontal ? Qt::AlignVCenter : Qt::AlignHCenter );
    }
}

static QImage loadImageData( QDomElement &n2 )
{
    QString format = n2.attribute( "format", "PNG" );
    QString hex = n2.firstChild().toText().data();
    int n = hex.length() / 2;
    QByteArray data( n );
    for ( int i = 0; i < n; i++ )
	data[i] = (char) hex.mid( 2 * i, 2 ).toUInt( 0, 16 );
    return loadImageData( format, n2.attribute("length").toULong(), data );
}

void QWidgetFactory::loadImageCollection( const QDomElement &e )
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

QImage QWidgetFactory::loadFromCollection( const QString &name )
{
    Q3ValueList<Image>::Iterator it = images.begin();
    for ( ; it != images.end(); ++it ) {
	if ( ( *it ).name == name )
	    return ( *it ).img;
    }
    return QImage();
}

QPixmap QWidgetFactory::loadPixmap( const QString& name )
{
    QPixmap pix;
    if ( usePixmapCollection ) {
	const QMimeSource *m = Q3MimeSourceFactory::defaultFactory()->data( name );
	if ( m )
	    Q3ImageDrag::decode( m, pix );
    } else {
	pix.convertFromImage( loadFromCollection(name) );
    }
    return pix;
}

QPixmap QWidgetFactory::loadPixmap( const QDomElement &e )
{
    return loadPixmap( e.firstChild().toText().data() );
}

QColorGroup QWidgetFactory::loadColorGroup( const QDomElement &e )
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

struct Connection
{
    QObject *sender, *receiver;
    Q3CString signal, slot;
    bool operator==( const Connection &c ) const {
	return sender == c.sender && receiver == c.receiver &&
	       signal == c.signal && slot == c.slot ;
    }

    Connection() : sender( 0 ), receiver( 0 ) { }
};

class NormalizeObject : public QObject
{
public:
    NormalizeObject() : QObject() {}
    static Q3CString normalizeSignalSlot( const char *signalSlot ) { return QObject::normalizeSignalSlot( signalSlot ); }
};

void QWidgetFactory::loadConnections( const QDomElement &e, QObject *connector )
{
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "connection" ) {
	    QString lang = n.attribute( "language", "C++" );
	    QDomElement n2 = n.firstChild().toElement();
	    Connection conn;
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "sender" ) {
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
			}
		    }
		    if ( !conn.sender )
			conn.sender = findAction( name );
		} else if ( n2.tagName() == "signal" ) {
		    conn.signal = n2.firstChild().toText().data();
		} else if ( n2.tagName() == "receiver" ) {
		    QString name = n2.firstChild().toText().data();
		    if ( name == "this" || qstrcmp( toplevel->name(), name ) == 0 ) {
			conn.receiver = toplevel;
		    } else {
			QObjectList *l = toplevel->queryList( 0, name, FALSE );
			if ( l ) {
			    if ( l->first() )
				conn.receiver = l->first();
			    delete l;
			}
		    }
		} else if ( n2.tagName() == "slot" ) {
		    conn.slot = n2.firstChild().toText().data();
		}
		n2 = n2.nextSibling().toElement();
	    }

	    conn.signal = NormalizeObject::normalizeSignalSlot( conn.signal );
	    conn.slot = NormalizeObject::normalizeSignalSlot( conn.slot );

	    if ( !conn.sender || !conn.receiver ) {
		n = n.nextSibling().toElement();
		continue;
	    }

	    QObject *sender = 0, *receiver = 0;
	    QObjectList *l = toplevel->queryList( 0, conn.sender->name(), FALSE );
	    if ( qstrcmp( conn.sender->name(), toplevel->name() ) == 0 ) {
		sender = toplevel;
	    } else {
		if ( !l || !l->first() ) {
		    delete l;
		    n = n.nextSibling().toElement();
		    continue;
		}
		sender = l->first();
		delete l;
	    }
	    if ( !sender )
		sender = findAction( conn.sender->name() );

	    if ( qstrcmp( conn.receiver->name(), toplevel->name() ) == 0 ) {
		receiver = toplevel;
	    } else {
		l = toplevel->queryList( 0, conn.receiver->name(), FALSE );
		if ( !l || !l->first() ) {
		    delete l;
		    n = n.nextSibling().toElement();
		    continue;
		}
		receiver = l->first();
		delete l;
	    }

	    QString s = "2""%1";
	    s = s.arg( conn.signal );
	    QString s2 = "1""%1";
	    s2 = s2.arg( conn.slot );

	    Q3StrList signalList = sender->metaObject()->signalNames( TRUE );
	    Q3StrList slotList = receiver->metaObject()->slotNames( TRUE );

	    // if this is a connection to a custom slot and we have a connector, try this as receiver
	    if ( slotList.find( conn.slot ) == -1 && receiver == toplevel && connector ) {
		slotList = connector->metaObject()->slotNames( TRUE );
		receiver = connector;
	    }

	    // avoid warnings
	    if ( signalList.find( conn.signal ) == -1 ||
		 slotList.find( conn.slot ) == -1 ) {
		n = n.nextSibling().toElement();
		continue;
	    }
	    QObject::connect( sender, s, receiver, s2 );
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::loadTabOrder( const QDomElement &e )
{
    QWidget *last = 0;
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "tabstop" ) {
	    QString name = n.firstChild().toText().data();
	    QObjectList *l = toplevel->queryList( 0, name, FALSE );
	    if ( l ) {
		if ( l->first() ) {
		    QWidget *w = (QWidget*)l->first();
		    if ( last )
			toplevel->setTabOrder( last, w );
		    last = w;
		}
		delete l;
	    }
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::createListViewColumn( Q3ListView *lv, const QString& txt,
					   const QPixmap& pix, bool clickable,
					   bool resizable )
{
    if ( pix.isNull() ) {
	lv->addColumn( txt );
    } else {
	lv->addColumn( pix, txt );
    }

    int i = lv->header()->count() - 1;
    if ( !pix.isNull() )
	lv->header()->setLabel( i, pix, txt );
    if ( !clickable )
	lv->header()->setClickEnabled( clickable, i );
    if ( !resizable )
	lv->header()->setResizeEnabled( resizable, i );
}

#ifndef QT_NO_TABLE
void QWidgetFactory::createTableColumnOrRow( Q3Table *table, const QString& txt,
					     const QPixmap& pix,
					     const QString& field, bool isRow )
{
#ifndef QT_NO_SQL
    bool isSql = table->inherits( "QDataTable" );
#endif
    if ( isRow )
	table->setNumRows( table->numRows() + 1 );
    else {
#ifndef QT_NO_SQL
	if ( !isSql )
#endif
	    table->setNumCols( table->numCols() + 1 );
    }

    Q3ValueList<Field> fieldMap;
    if ( fieldMaps.find( table ) != fieldMaps.end() ) {
	fieldMap = *fieldMaps.find( table );
	fieldMaps.remove( table );
    }

    int i = isRow ? table->numRows() - 1 : table->numCols() - 1;
    Q3Header *h = !isRow ? table->horizontalHeader() : table->verticalHeader();
    if ( !pix.isNull() ) {
#ifndef QT_NO_SQL
	if ( isSql )
	    ((Q3DataTable*)table)->addColumn( field, txt, -1, pix );
	else
#endif
	    h->setLabel( i, pix, txt );
    } else {
#ifndef QT_NO_SQL
	if ( isSql )
	    ((Q3DataTable*)table)->addColumn( field, txt );
	else
#endif
	    h->setLabel( i, txt );
    }
    if ( !isRow && !field.isEmpty() ) {
	fieldMap.append( Field( txt, pix, field ) );
	fieldMaps.insert( table, fieldMap );
    }

}
#endif

void QWidgetFactory::createColumn( const QDomElement &e, QWidget *widget )
{
    if ( widget->inherits( "QListView" ) && e.tagName() == "column" ) {
	Q3ListView *lv = (Q3ListView*)widget;
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	QString txt;
	bool clickable = TRUE, resizable = TRUE;
	while ( !n.isNull() ) {
	    if ( n.tagName() == "property" ) {
		QString attrib = n.attribute( "name" );
		QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
		if ( attrib == "text" )
		    txt = translate( v.toString() );
		else if ( attrib == "pixmap" )
		    pix = loadPixmap( n.firstChild().toElement().toElement() );
		else if ( attrib == "clickable" )
		    clickable = v.toBool();
		else if ( attrib == "resizable" || attrib == "resizeable" )
		    resizable = v.toBool();
	    }
	    n = n.nextSibling().toElement();
	}
	createListViewColumn( lv, txt, pix, clickable, resizable );
    }
#ifndef QT_NO_TABLE
    else if ( widget->inherits( "QTable" ) ) {
	Q3Table *table = (Q3Table*)widget;

	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	QString txt;
	QString field;

	while ( !n.isNull() ) {
	    if ( n.tagName() == "property" ) {
		QString attrib = n.attribute( "name" );
		QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
		if ( attrib == "text" )
		    txt = translate( v.toString() );
		else if ( attrib == "pixmap" ) {
		    if ( !n.firstChild().firstChild().toText().data().isEmpty() )
			pix = loadPixmap( n.firstChild().toElement().toElement() );
		} else if ( attrib == "field" )
		    field = translate( v.toString() );
	    }
	    n = n.nextSibling().toElement();
	}
	createTableColumnOrRow( table, txt, pix, field, e.tagName() == "row" );
    }
#endif
}

void QWidgetFactory::loadItem( const QDomElement &e, QPixmap &pix, QString &txt, bool &hasPixmap )
{
    QDomElement n = e;
    hasPixmap = FALSE;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute( "name" );
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" )
		txt = translate( v.toString() );
	    else if ( attrib == "pixmap" ) {
		pix = loadPixmap( n.firstChild().toElement() );
		hasPixmap = !pix.isNull();
	    }
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::createItem( const QDomElement &e, QWidget *widget, Q3ListViewItem *i )
{
    if ( widget->inherits( "QListBox" ) || widget->inherits( "QComboBox" ) ) {
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	loadItem( n, pix, txt, hasPixmap );
	Q3ListBox *lb = 0;
	if ( widget->inherits( "QListBox" ) )
	    lb = (Q3ListBox*)widget;
	else
	    lb = ( (QComboBox*)widget)->listBox();
	if ( hasPixmap ) {
	    new Q3ListBoxPixmap( lb, pix, txt );
	} else {
	    new Q3ListBoxText( lb, txt );
	}
#ifndef QT_NO_ICONVIEW
    } else if ( widget->inherits( "QIconView" ) ) {
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	loadItem( n, pix, txt, hasPixmap );

	Q3IconView *iv = (Q3IconView*)widget;
	new Q3IconViewItem( iv, txt, pix );
#endif
    } else if ( widget->inherits( "QListView" ) ) {
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	Q3ValueList<QPixmap> pixmaps;
	QStringList textes;
	Q3ListViewItem *item = 0;
	Q3ListView *lv = (Q3ListView*)widget;
	if ( i )
	    item = new Q3ListViewItem( i, d->lastItem );
	else
	    item = new Q3ListViewItem( lv, d->lastItem );
	while ( !n.isNull() ) {
	    if ( n.tagName() == "property" ) {
		QString attrib = n.attribute( "name" );
		QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
		if ( attrib == "text" )
		    textes << translate( v.toString() );
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
	d->lastItem = item;
    }
}



void QWidgetFactory::loadChildAction( QObject *parent, const QDomElement &e )
{
    QDomElement n = e;
    QAction *a = 0;
    bool hasMenuText = FALSE;
    if ( n.tagName() == "action" ) {
	a = new QAction( parent );
	QDomElement n2 = n.firstChild().toElement();
	
	while ( !n2.isNull() ) {
	    if ( n2.tagName() == "property" ) {
		QString prop(n2.attribute("name"));
		if (prop == "menuText")
		    hasMenuText = TRUE;
		setProperty( a, prop, n2.firstChild().toElement() );
	    }
	    n2 = n2.nextSibling().toElement();
	}
	if ( !parent->inherits( "QAction" ) )
	    actionList.append( a );
    } else if ( n.tagName() == "actiongroup" ) {
	a = new Q3ActionGroup( parent );
	QDomElement n2 = n.firstChild().toElement();
	while ( !n2.isNull() ) {
	    if ( n2.tagName() == "property" ) {
		QString prop(n2.attribute("name"));
		if (prop == "menuText")
		    hasMenuText = TRUE;
		setProperty( a, prop, n2.firstChild().toElement() );
	    } else if ( n2.tagName() == "action" ||
			n2.tagName() == "actiongroup" ) {
		loadChildAction( a, n2 );

	    }
	    n2 = n2.nextSibling().toElement();
	}
	if ( !parent->inherits( "QAction" ) )
	    actionList.append( a );
    }

    if (a && !hasMenuText && !a->text().isEmpty() && uiFileVersion < "3.3")
	a->setMenuText(a->text());
}

void QWidgetFactory::loadActions( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "action" ) {
	    loadChildAction( toplevel, n );
	} else if ( n.tagName() == "actiongroup" ) {
	    loadChildAction( toplevel, n );
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::loadToolBars( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    Q3MainWindow *mw = ( (Q3MainWindow*)toplevel );
    Q3ToolBar *tb = 0;
    while ( !n.isNull() ) {
	if ( n.tagName() == "toolbar" ) {
	    Qt::ToolBarDock dock = (Qt::ToolBarDock)n.attribute( "dock" ).toInt();
	    tb = new Q3ToolBar( QString::null, mw, dock );
	    tb->setLabel( n.attribute( "label" ) );
	    tb->setName( n.attribute( "name" ) );
	    QDomElement n2 = n.firstChild().toElement();
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "action" ) {
		    QAction *a = findAction( n2.attribute( "name" ) );
		    if ( a )
			a->addTo( tb );
		} else if ( n2.tagName() == "separator" ) {
		    tb->addSeparator();
		} else if ( n2.tagName() == "widget" ) {
		    (void)createWidgetInternal( n2, tb, 0, n2.attribute( "class", "QWidget" ) );
		} else if ( n2.tagName() == "property" ) {
		    setProperty( tb, n2.attribute( "name" ), n2.firstChild().toElement() );
		}
		n2 = n2.nextSibling().toElement();
	    }
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::loadMenuBar( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    Q3MainWindow *mw = ( (Q3MainWindow*)toplevel );
    QMenuBar *mb = mw->menuBar();
    while ( !n.isNull() ) {
	if ( n.tagName() == "item" ) {
	    Q3PopupMenu *popup = new Q3PopupMenu( mw );
	    loadPopupMenu( popup, n );
	    popup->setName( n.attribute( "name" ) );
	    mb->insertItem( translate( n.attribute( "text" ) ), popup );
	} else if ( n.tagName() == "property" ) {
	    setProperty( mb, n.attribute( "name" ), n.firstChild().toElement() );
	} else if ( n.tagName() == "separator" ) {
	    mb->insertSeparator();
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::loadPopupMenu( Q3PopupMenu *p, const QDomElement &e )
{
    Q3MainWindow *mw = ( (Q3MainWindow*)toplevel );
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "action" ) {
	    QAction *a = findAction( n.attribute( "name" ) );
	    QDomElement n2 = n.nextSibling().toElement();
	    if ( n2.tagName() == "item") { // load submenu
		Q3PopupMenu *popup = new Q3PopupMenu( mw );
		popup->setName( n2.attribute( "name" ) );
		if ( a ) {
		    p->setAccel( a->accel(), p->insertItem( a->iconSet(),
					     translate( n2.attribute( "text" ).utf8().data() ),
					     popup ) );
		} else {
		    p->insertItem( translate( n2.attribute( "text" ).utf8().data() ), popup );
		}
		loadPopupMenu( popup, n2 );
		n = n2;
	    } else {
		if ( a ) {
		    a->addTo( p );
		}
	    }
	    a = 0;
	} else if ( n.tagName() == "separator" ) {
	    p->insertSeparator();
	}
	n = n.nextSibling().toElement();
    }
}

// compatibility with early 3.0 betas
// ### remove for 4.0
void QWidgetFactory::loadFunctions( const QDomElement & )
{
}

QAction *QWidgetFactory::findAction( const QString &name )
{
    for ( QAction *a = actionList.first(); a; a = actionList.next() ) {
	if ( QString( a->name() ) == name )
	    return a;
	QAction *ac = (QAction*)a->child( name.latin1(), "QAction" );
	if ( ac )
	    return ac;
    }
    return 0;
}

/*!
    If you use a pixmap collection (which is the default for new
    projects) rather than saving the pixmaps within the .ui XML file,
    you must load the pixmap collection. QWidgetFactory looks in the
    default QMimeSourceFactory for the pixmaps. Either add it there
    manually, or call this function and specify the directory where
    the images can be found, as \a dir. This is normally the
    directory called \c images in the project's directory.
*/

void QWidgetFactory::loadImages( const QString &dir )
{
    QDir d( dir );
    QStringList l = d.entryList( QDir::Files );
    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it )
	Q3MimeSourceFactory::defaultFactory()->setPixmap( *it, QPixmap( d.path() + "/" + *it, "PNG" ) );

}

void QWidgetFactory::loadExtraSource()
{
    if ( !qwf_language || !languageInterfaceManager )
	return;
    QString lang = *qwf_language;
    LanguageInterface *iface = 0;
    languageInterfaceManager->queryInterface( lang, &iface );
    if ( !iface )
	return;
    QFile f( qwf_currFileName + iface->formCodeExtension() );
    if ( f.open( QIODevice::ReadOnly ) ) {
	QTextStream ts( &f );
	code = ts.read();
    }
}

QString QWidgetFactory::translate( const QString& sourceText, const QString& comment )
{
    return qApp->translate( d->translationContext, sourceText.utf8(), comment.utf8(),
			    QApplication::UnicodeUTF8 );
}

QString QWidgetFactory::translate( const char *sourceText, const char *comment )
{
    return qApp->translate( d->translationContext, sourceText, comment,
			    QApplication::UnicodeUTF8 );
}
