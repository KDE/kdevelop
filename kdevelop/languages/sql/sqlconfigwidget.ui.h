/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qsqldatabase.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qdom.h>
#include <qstringlist.h>

#include <klocale.h>
#include <klineedit.h>
#include <kmessagebox.h>

#include <domutil.h>

#include "sqlsupport_part.h"

// displays a ComboBox on edit to let the user choose the plugin
class PluginTableItem: public QTableItem
{
public:
    PluginTableItem( QTable* table, const QString& text = QString::null ): QTableItem( table, QTableItem::OnTyping, text ) {}

    virtual QWidget* createEditor() const
    {
	QComboBox* combo = new QComboBox( true, table() );
	combo->insertStringList( QSqlDatabase::drivers() );
	if ( !text().isEmpty() )
	    combo->setCurrentText( text() );
	return combo;
    }

    virtual void setContentFromEditor( QWidget* w )
    {
	if ( w->inherits( "QComboBox" ) )
	    setText( ( (QComboBox*)w )->currentText() );
	else
	    QTableItem::setContentFromEditor( w );
    }
};

// uses a spinbox to let the user choose the port
class PortTableItem: public QTableItem
{
public:
    PortTableItem( QTable* table, const QString& text = QString::null ): QTableItem( table, QTableItem::OnTyping, text ) {}

    virtual QWidget* createEditor() const
    {
	QSpinBox* sb = new QSpinBox( -1, 65535, 1, table() );
	sb->setSpecialValueText( i18n("Default") );
	if ( !text().isEmpty() )
	    sb->setValue( text().toInt() );
	return sb;
    }

    virtual void setContentFromEditor( QWidget* w )
    {
	if ( w->inherits( "QSpinBox" ) )
	    setText( ( (QSpinBox*)w )->text() );
	else
	    QTableItem::setContentFromEditor( w );
    }
};

// uses a spinbox to let the user choose the port
class PasswordTableItem: public QTableItem
{
public:
    PasswordTableItem( QTable* table, const QString& text = QString::null ): QTableItem( table, QTableItem::OnTyping, text ) {}

    virtual QWidget* createEditor() const
    {
	KLineEdit* le = new KLineEdit( table() );
	le->setEchoMode( QLineEdit::Password );
	return le;
    }

    virtual void setContentFromEditor( QWidget* w )
    {
	if ( w->inherits( "KLineEdit" ) ) {
	    password = ( (KLineEdit*)w )->text();
	    setText( QString().fill( '*', password.length() ) );
	} else {
	    QTableItem::setContentFromEditor( w );
	}
    }

    virtual void setText(const QString& bogus)
    {
        QTableItem::setText(QString().fill('*', bogus.length()));
    }
    QString password;
};

static bool isEmptyRow( QTable* tbl, int row )
{
    for ( int i = 0; i < tbl->numCols(); i++ ) {
	if ( !tbl->text( row, i ).isEmpty() )
	    return false;
    }
    return true;
}

void SqlConfigWidget::init()
{
    int w = dbTable->fontMetrics().width( "W" );
    dbTable->verticalHeader()->hide();
    dbTable->setLeftMargin( 0 );
    dbTable->setColumnWidth( 3, w * 5 );
    updateButtons();
    valueChanged( 0, 0 );
    changed = false;
}

static void addRow( QTable* dbTable )
{
    dbTable->insertRows( dbTable->numRows() );
    dbTable->setItem( dbTable->numRows() - 1, 0, 
		      new PluginTableItem( dbTable ) );
    dbTable->setItem( dbTable->numRows() - 1, 3, 
		      new PortTableItem( dbTable ) );
    dbTable->setItem( dbTable->numRows() - 1, 5, 
		      new PasswordTableItem( dbTable ) );
}

void SqlConfigWidget::valueChanged( int, int )
{
    int lrow = dbTable->numRows() - 1;
    if ( lrow < 0 || !isEmptyRow( dbTable, lrow ) ) {
	addRow( dbTable );
    }
    changed = true;
}


void SqlConfigWidget::removeDb()
{
    if ( dbTable->currentRow() >= 0 )
	dbTable->removeRow( dbTable->currentRow() );
    valueChanged( 0, 0 );
    updateButtons();
}


void SqlConfigWidget::updateButtons()
{
    if ( dbTable->currentRow() < 0 ) {
	testBtn->setEnabled( false );
	removeBtn->setEnabled( false );
	return;
    }
    testBtn->setEnabled( !isEmptyRow( dbTable, dbTable->currentRow() ) );
    removeBtn->setEnabled( dbTable->currentRow() + 1 < dbTable->numRows() );
}


void SqlConfigWidget::testDb()
{
    static const QString cName( "SqlConfigWidgetTest" );
    int cr = dbTable->currentRow();
    if (cr < 0)
        return;

    QSqlDatabase* db = QSqlDatabase::addDatabase( dbTable->text( cr, 0 ), cName );
    db->setDatabaseName( dbTable->text( cr, 1 ) );
    db->setHostName( dbTable->text( cr, 2 ) );
    bool ok;
    int port = dbTable->text( cr, 3 ).toInt( &ok );
    if (ok && port >= 0)
        db->setPort( port );
    QString pass = ((PasswordTableItem*)dbTable->item( cr, 5 ))->password;

    if ( db->open( dbTable->text( cr, 4 ), pass ) ) {
        KMessageBox::information( this, i18n("Connection successful") );
        db->close();
    } else {
        KMessageBox::detailedSorry( this, i18n("Unable to connect to database server"),
                db->lastError().driverText() + "\n" +
                db->lastError().databaseText() );
    }

    db = 0;
    QSqlDatabase::removeDatabase( cName );
}


void SqlConfigWidget::accept()
{
    Q_ASSERT( doc );

    QDomElement dbElem = DomUtil::createElementByPath( *doc, "/kdevsqlsupport/servers" );
    DomUtil::makeEmpty( dbElem );

    for ( int i = 0; i < dbTable->numRows() - 1; i++ ) {
	QStringList db;
	db << dbTable->text( i, 0 ) << dbTable->text( i, 1 )
		<< dbTable->text( i, 2 ) << dbTable->text( i, 3 )
		<< dbTable->text( i, 4 )
		<< SQLSupportPart::cryptStr( ((PasswordTableItem*)dbTable->item( i, 5 ))->password );
	
	DomUtil::writeListEntry( *doc, 
				 "/kdevsqlsupport/servers/server" + QString::number( i ), 
				 "el", db );
    }
    if ( changed )
	emit newConfigSaved();
}


void SqlConfigWidget::setProjectDom( QDomDocument * doc )
{
    this->doc = doc;
}


void SqlConfigWidget::loadConfig()
{
    Q_ASSERT( doc );
    
    QStringList db;
    int i = 0;
    while ( true ) {
        QStringList db = DomUtil::readListEntry( *doc, "kdevsqlsupport/servers/server" + QString::number( i ), "el" );
        if ( db.isEmpty() )
            return;

        addRow( dbTable );
        int row = dbTable->numRows() - 2;
        for ( int ii = 0; ii < 6; ii++ )
            dbTable->setText( row, ii, db[ii] );
        ((PasswordTableItem*)dbTable->item( row, 5 ))->password = SQLSupportPart::cryptStr( db[5] );

        i++;
    }
    updateButtons();
    changed = false;
}
