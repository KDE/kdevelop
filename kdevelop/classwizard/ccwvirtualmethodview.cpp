#include <qmessagebox.h>
#include <klocale.h>
#include "ccwvirtualmethodview.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*--------------------- CCWVirtualMethodView::CCWVirtualMethodView()
 * CCWVirtualMethodView()
 *   Constructor.
 *
 * Parameters:
 *   parent         Parent widget.
 *   name           The name of this widget.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CCWVirtualMethodView::CCWVirtualMethodView( QWidget *parent, const char *name )
  : QWidget( parent, name ),
    topLayout( this, 5 ),
    classComboLayout( 5, "vfClassComboLayout" ),
    classLbl( this, "classLbl" ),
    classCombo( false, this, "classCombo" ),
    listViewLayout( 5, "vfListViewLayout" ),
    availLayout( 5, "vfAvailLayout" ),
    availLbl( this, "availLbl" ),
    availLb( this, "availLb" ),
    implLayout( 5, "vfImplLayout" ),
    implLbl( this, "implLbl" ),
    implLb( this, "implLb" ),
    buttonLayout( 5, "vfButtonLayout" ),
    addBtn( this, "vfAddBtn" ),
    topFiller( this, "vfTopFiller" ),
    filler( this, "vfFiller" )
{
  store = NULL;

  setWidgetValues();
  setCallbacks();
  setTooltips();
}

/*--------------------- CCWVirtualMethodView::~CCWVirtualMethodView()
 * ~CCWVirtualMethodView()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CCWVirtualMethodView::~CCWVirtualMethodView()
{
}

/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------ CClassTreeHandler::setWidgetValues()
 * setWidgetValues()
 *   Set all initial values of all widgets in the view.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CCWVirtualMethodView::setWidgetValues()
{
  // Top layout
  topLayout.addLayout( &classComboLayout );
  topLayout.addLayout( &listViewLayout );

  // Listview layout
  listViewLayout.addLayout( &availLayout );
  listViewLayout.addLayout( &implLayout );
  listViewLayout.addLayout( &buttonLayout );

  // Classcombo
  classLbl.setMinimumSize( 110, 30 );
  classLbl.setText( i18n("Class:") );
  classCombo.setMinimumSize( 330, 30 );
  classCombo.setFixedHeight( 30 );
  classCombo.setAutoResize( FALSE );
  
  // Available scrollview
  availLbl.setText( i18n( "Available:" ) );
  availLbl.setMinimumSize( 210, 20 );
  availLbl.setFixedHeight( 20 );
  availLb.setMinimumSize( 210, 260 );

  // Overridden scrollview
  implLbl.setText( i18n( "Used:" ) );
  implLbl.setMinimumSize( 210, 20 );
  implLbl.setFixedHeight( 20 );
  implLb.setMinimumSize( 210, 260 );

  // Top filler
  topFiller.setFixedHeight( 20 );

  // Buttons
  addBtn.setText( i18n( "Add" ) );
  addBtn.setMinimumSize( 110, 30 );
  addBtn.setMaximumSize( 110, 30 );

  // Classcombo layout
  classComboLayout.addWidget( &classLbl );
  classComboLayout.addWidget( &classCombo );

  // Available layout
  availLayout.addWidget( &availLbl );
  availLayout.addWidget( &availLb );

  // Used layout
  implLayout.addWidget( &implLbl );
  implLayout.addWidget( &implLb );

  // Button layout
  buttonLayout.addWidget( &topFiller );
  buttonLayout.addWidget( &addBtn );
  buttonLayout.addWidget( &filler );
}

/*---------------------------------- CClassTreeHandler::setTooltips()
 * setTooltips()
 *   Set tooltip strings.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CCWVirtualMethodView::setTooltips()
{
}

/*--------------------------------- CClassTreeHandler::setCallbacks()
 * setCallbacks()
 *   Set all signal<->slot mappings.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CCWVirtualMethodView::setCallbacks()
{
  connect( &classCombo, SIGNAL(activated(int)), SLOT(slotClassComboChoice(int)));
  connect( &addBtn, SIGNAL(clicked()), SLOT(slotAddMethod()));
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- CClassTreeHandler::setStore()
 * setStore()
 *   Set the classtore.
 *
 * Parameters:
 *   aStore         The store.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CCWVirtualMethodView::setStore( CClassStore *aStore )
{
  assert( aStore != NULL );
  QStrList *list;

  store = aStore;
  
  // Fetch the list and update the combobox.
  list = store->getSortedClassNameList();
  classCombo.insertStrList( list );
  delete list;
}

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

void CCWVirtualMethodView::slotClassComboChoice(int idx)
{
  QList<CParsedMethod> availList;
  QList<CParsedMethod> usedList;
  CParsedMethod *aMethod;
  QString str;

  // Clear the listboxes and the dict.
  implLb.clear();
  availLb.clear();
  availDict.clear();

  // Fetch the virtual methods.
  store->getVirtualMethodsForClass( classCombo.text( idx ),
                                    &usedList, &availList );

  
  for( aMethod = usedList.first();
       aMethod != NULL;
       aMethod = usedList.next() )
  {
    implLb.inSort( aMethod->asString( str ) );
  }

  for( aMethod = availList.first();
       aMethod != NULL;
       aMethod = availList.next() )
  {
    // Create the string like <classname>::<methodname>
    aMethod->asString( str );
    str = aMethod->declaredInClass + "::" + str;

    // Add to the view.
    availLb.inSort( str );

    // Add to the internal dictionary.
    availDict.insert( str, aMethod );
  }
}

/** Executed when the user clicks on the add button. */
void CCWVirtualMethodView::slotAddMethod()
{
  CParsedMethod *aMethod;
  CParsedMethod *newMethod;

  if( availLb.currentItem() != -1 )
  {
    aMethod = availDict.find( availLb.text( availLb.currentItem() ) );

    if( aMethod != NULL )
    {
      newMethod = new CParsedMethod();
      newMethod->copy( aMethod );
      
      newMethod->out();

      emit addMethod( classCombo.currentText(), newMethod );

      // Add the method to the implemented listbox.
      implLb.inSort( availLb.text( availLb.currentItem() ) );
      
      // Remove the method from the available listbox.
      availLb.removeItem( availLb.currentItem() );

      QMessageBox::information( this, i18n( "Added method" ),
                                i18n( "The virtual method implementation has been added." ) );
    }
  }
}

