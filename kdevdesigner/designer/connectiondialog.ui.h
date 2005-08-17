/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "mainwindow.h"
#include "formwindow.h"
#include "editfunctionsimpl.h"
//Added by qt3to4:
#include <QPixmap>
#include <Q3ValueList>
#include <Q3PtrList>

static const char *valid_connection[] = {
    "16 14 7 1",
    ". c none",
    "# c #00c000",
    "a c #008000",
    "b c #004000",
    "c c #000000",
    "d c #c0fcc0",
    "e c #400000",
    "...............d",
    "..............d#",
    ".............d#a",
    "............d#ab",
    "...........d#abc",
    "..da......d#abc.",
    ".d##a....d#abe..",
    "cba##a..d#abc...",
    ".cba##ad#abe....",
    "..eba###abe.....",
    "...cba#abc......",
    "....cbabc.......",
    ".....cec........",
    "................"
};

static const char * invalid_connection[] = {
    "17 18 7 1",
    "       c None",
    ".      c #000000",
    "+      c #C3C3C3",
    "@      c #FFFFFF",
    "#      c #C00000",
    "$      c #FFC0C0",
    "%      c #FF0000",
    "                 ",
    "       .         ",
    "      ...        ",
    "     .+..        ",
    "  ...+@+..       ",
    "..+@@@+#..       ",
    ".@@@@$#%#..      ",
    ".@@$$#%##..      ",
    " .$%%%##....     ",
    " .%%%##.  ..     ",
    "  .%#..    ..    ",
    "   ..      ..    ",
    "            ..   ",
    "            ..   ",
    "             ..  ",
    "             ..  ",
    "                 ",
    "                 "
};

static QPixmap *invalidConnection = 0;
static QPixmap *validConnection = 0;

void ConnectionDialog::init()
{
    connect( connectionsTable, SIGNAL( currentChanged( int, int ) ),
	     this, SLOT( updateEditSlotsButton() ) );
    connect( connectionsTable, SIGNAL( resorted() ),
	     this, SLOT( updateConnectionContainers() ) );
    buttonEditSlots->setEnabled( FALSE );

    if ( !invalidConnection ) {
	invalidConnection = new QPixmap( invalid_connection );
	validConnection = new QPixmap( valid_connection );
    }

    Q3ValueList<MetaDataBase::Connection> conns =
	MetaDataBase::connections( MainWindow::self->formWindow() );
    for ( Q3ValueList<MetaDataBase::Connection>::Iterator it = conns.begin();
	  it != conns.end(); ++it ) {
	ConnectionContainer *c = addConnection( (*it).sender, (*it).receiver,
						(*it).signal, (*it).slot );
	c->setModified( FALSE );
	updateConnectionState( c );
    }

    defaultSender = defaultReceiver = 0;
    connectionsTable->setCurrentCell( 0, 0 );
}

void ConnectionDialog::addConnection()
{
    addConnection( defaultSender, defaultReceiver, QString::null, QString::null );
    ensureConnectionVisible();
}

ConnectionContainer *ConnectionDialog::addConnection( QObject *sender, QObject *receiver,
						      const QString &signal,
						      const QString &slot )
{
    connectionsTable->insertRows( connectionsTable->numRows() );

    int row = connectionsTable->numRows() - 1;
    SenderItem *se;
    SignalItem *si;
    ReceiverItem *re;
    SlotItem *sl;
    connectionsTable->setItem( row, 0,
			       ( se = new SenderItem( connectionsTable,
						      MainWindow::self->formWindow() ) ) );
    connectionsTable->setItem( row, 1,
			       ( si = new SignalItem( connectionsTable,
						      MainWindow::self->formWindow() ) ) );
    connectionsTable->setItem( row, 2,
			       ( re = new ReceiverItem( connectionsTable,
							MainWindow::self->formWindow() ) ) );
    connectionsTable->setItem( row, 3,
			       ( sl = new SlotItem( connectionsTable,
						    MainWindow::self->formWindow() ) ) );

    si->setSender( se );
    re->setSender( se );
    sl->setSender( se );
    se->setSender( se );

    se->setSignal( si );
    re->setSignal( si );
    sl->setSignal( si );
    si->setSignal( si );

    se->setReceiver( re );
    si->setReceiver( re );
    sl->setReceiver( re );
    re->setReceiver( re );

    se->setSlot( sl );
    si->setSlot( sl );
    re->setSlot( sl );
    sl->setSlot( sl );

    connect( re, SIGNAL( currentReceiverChanged( QObject * ) ),
	     this, SLOT( updateEditSlotsButton() ) );

    ConnectionContainer *c = new ConnectionContainer( this, se, si, re, sl, row );
    connections.append( c );
    connect( c, SIGNAL( changed( ConnectionContainer * ) ),
	     this, SLOT( updateConnectionState( ConnectionContainer * ) ) );

    connectionsTable->setCurrentCell( row, 0 );

    connectionsTable->verticalHeader()->setLabel( row, *invalidConnection, QString::null );

    if ( sender )
	se->setSenderEx( sender );
    defaultSender = sender;
    if ( receiver )
	re->setReceiverEx( receiver );
    defaultReceiver = receiver;

    if ( !signal.isEmpty() && !slot.isEmpty() ) {
	si->setCurrentItem( signal );
	sl->signalChanged( signal );
	sl->setCurrentItem( slot );
    }

    c->setModified( TRUE );
    return c;
}

void ConnectionDialog::updateConnectionState( ConnectionContainer *c )
{
    c->repaint();
    if ( c->isValid() )
	connectionsTable->verticalHeader()->setLabel( c->row(),
						      *validConnection, QString::null );
    else
	connectionsTable->verticalHeader()->setLabel( c->row(),
						      *invalidConnection, QString::null );
}


void ConnectionDialog::okClicked()
{
    Q3ValueList<MetaDataBase::Connection> oldConnections =
	MetaDataBase::connections( MainWindow::self->formWindow() );
    Q3PtrList<Command> newConnectionCmds;
    Q3PtrList<Command> oldConnectionCmds;
    for ( ConnectionContainer *c = connections.first(); c; c = connections.next() ) {
	MetaDataBase::Connection conn;

        // find sender widget
	conn.sender = MainWindow::self->formWindow()->child( c->senderItem()->currentText() );
	if ( !conn.sender ) // if no sender widget, find sender action
	    conn.sender = MainWindow::self->formWindow()->findAction( c->senderItem()->currentText() );

	// find receiver widget
	conn.receiver = MainWindow::self->formWindow()->child( c->receiverItem()->currentText() );
	if ( !conn.receiver ) // if no receiver widget, find receiver action
	    conn.receiver = MainWindow::self->formWindow()->findAction( c->receiverItem()->currentText() );

	conn.signal = c->signalItem()->currentText();
	conn.slot = c->slotItem()->currentText();
	AddConnectionCommand *cmd = new AddConnectionCommand( i18n( "Add Signal/Slot "
								  "Connection" ),
							      MainWindow::self->formWindow(),
							      conn );
	newConnectionCmds.append( cmd );
    }


    Q3ValueList<MetaDataBase::Connection> conns =
	MetaDataBase::connections( MainWindow::self->formWindow() );
    for ( Q3ValueList<MetaDataBase::Connection>::Iterator it = conns.begin();
	  it != conns.end(); ++it ) {
	RemoveConnectionCommand *cmd = new RemoveConnectionCommand( i18n( "Remove Signal/Slot "
									"Connection" ),
								    MainWindow::self->
								    formWindow(), *it );
	oldConnectionCmds.append( cmd );
    }

    MacroCommand *cmd1 = new MacroCommand( i18n( "Add Signal/Slot Connections" ),
					   MainWindow::self->formWindow(),
					   newConnectionCmds );
    MacroCommand *cmd2 = new MacroCommand( i18n( "Remove Signal/Slot Connections" ),
					   MainWindow::self->formWindow(),
					   oldConnectionCmds );
    Q3PtrList<Command> cmds;
    cmds.append( cmd2 );
    cmds.append( cmd1 );
    MacroCommand *cmd = new MacroCommand( i18n( "Edit Signal/Slot Connections" ),
					  MainWindow::self->formWindow(),
					  cmds );
    MainWindow::self->formWindow()->commandHistory()->addCommand( cmd );
    cmd->execute();

    accept();
}


void ConnectionDialog::cancelClicked()
{
    reject();
}


void ConnectionDialog::deleteClicked()
{
    int cr = connectionsTable->currentRow();
    connections.remove( cr );
    connectionsTable->removeRow( cr );
    int i = 0;
    for ( ConnectionContainer *c = connections.first(); c; c = connections.next() )
	c->setRow( i++ );
}


void ConnectionDialog::editSlots()
{
    EditFunctions dlg( this, MainWindow::self->formWindow(), TRUE );
    dlg.exec();
    int currentCol = connectionsTable->currentColumn();
    connectionsTable->setCurrentCell( connectionsTable->currentRow(), 0 );
    for ( ConnectionContainer *c = connections.first(); c; c = connections.next() ) {
	if ( c->receiverItem()->currentText() !=
	     QString( MainWindow::self->formWindow()->mainContainer()->name() ) )
	    continue;
	c->slotItem()->customSlotsChanged();
    }
    connectionsTable->setCurrentCell( connectionsTable->currentRow(), currentCol );
}

void ConnectionDialog::setDefault( QObject *sender, QObject *receiver )
{
    defaultSender = sender;
    defaultReceiver = receiver;
}

void ConnectionDialog::updateEditSlotsButton()
{
    if ( connectionsTable->currentRow() < 0 ||
	 connectionsTable->currentRow() > (int)connections.count() - 1 )
	return;
    ConnectionContainer *c = connections.at( connectionsTable->currentRow() );
    if ( !c || !c->receiverItem() )
	return;
    buttonEditSlots->setEnabled( c->receiverItem()->currentText() ==
				 QString( MainWindow::self->formWindow()->
					  mainContainer()->name() ) );
}

void ConnectionDialog::updateConnectionContainers()
{
    Q3PtrList<ConnectionContainer> newContainers;
    for ( int i = 0; i < connectionsTable->numRows(); ++i ) {
	for ( ConnectionContainer *c = connections.first(); c; c = connections.next() ) {
	    if ( c->senderItem() == connectionsTable->item( i, 0 ) ) {
		newContainers.append( c );
		c->setRow( i );
                updateConnectionState(c);
		break;
	    }
	}
    }
    connections = newContainers;
    updateEditSlotsButton();
}

void ConnectionDialog::ensureConnectionVisible()
{
    connectionsTable->ensureCellVisible( connectionsTable->numRows() - 1, 0 );
}
