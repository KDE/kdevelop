/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <qtooltip.h>
#include <qlistbox.h>
#include <klocale.h>
#include <kiconloader.h>
#include "kdevlanguagesupport.h"
#include "classstore.h"
#include "parsedclass.h"
#include "classtooldlg.h"


/*------------------------------------ ClassToolDialog::ClassToolDialog()
 * ClassToolDialog()
 *   Constructor.
 *
 * Parameters:
 *   parent         Parent widget.
 *   name           The name of this widget.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ClassToolDialog::ClassToolDialog( ClassView *part )
  : QWidget( 0, "class tool dialog" ),
    classTree( part, this ),
    classLbl( this, "classLbl" ),
    classCombo( false, this, "classCombo" ),
    topLayout( this, 5 ),
    comboLayout( 1, 2 ),
    parentsBtn( this, "parentsBtn" ),
    childrenBtn( this, "childrenBtn" ),
    clientsBtn( this, "clientsBtn" ),
    suppliersBtn( this, "suppliersBtn" ),
    filler( this, "filler" ),
    methodsBtn( this, "methodsBtn" ),
    attributesBtn( this, "attributesBtn" ),
    exportCombo( false, this, "exportCombo" )
{
  currentOperation = ViewNone;
  comboExport = (PIExport)-1;
  currentClass = 0;
  m_part = part;
  m_store = NULL;

  setWidgetValues();
  readIcons();
  setCallbacks();
  setTooltips();

  connect( part, SIGNAL(setLanguageSupport(KDevLanguageSupport*)),
           this, SLOT(setLanguageSupport(KDevLanguageSupport*)) );
  connect( part, SIGNAL(setClassStore(ClassStore*)),
           this, SLOT(setClassStore(ClassStore*)) );

  m_part->registerClassToolDialog(this);
}


ClassToolDialog::~ClassToolDialog()
{
    m_part->unregisterClassToolDialog(this);
}


/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*---------------------------------- ClassToolDialog::setWidgetValues()
 * ClassToolDialog()
 *   Set all initial values of all widgets in the dialog.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassToolDialog::setWidgetValues()
{
  parentsBtn.setToggleButton(true);
  childrenBtn.setToggleButton(true);
  clientsBtn.setToggleButton(true);
  suppliersBtn.setToggleButton(true);
  methodsBtn.setToggleButton(true);
  attributesBtn.setToggleButton(true);

  classLbl.setText( i18n("Class:") );

  classCombo.setMinimumWidth( 260 );
  classCombo.setSizeLimit( 10 );

  exportCombo.setMinimumWidth( 100 );
  exportCombo.setSizeLimit( 10 );
  exportCombo.insertItem( i18n("All") );
  exportCombo.insertItem( "Public" );
  exportCombo.insertItem( "Protected" );
  exportCombo.insertItem( "Private" );

  classTree.setMinimumSize( 500, 400 );

  // Top layout
  topLayout.addLayout( &comboLayout );
  topLayout.addLayout( &btnLayout );

  // Combo layout
  comboLayout.addWidget( &classLbl, 0, 0, AlignLeft );
  comboLayout.addWidget( &classCombo, 0, 1, AlignLeft );

  // Button layout
  btnLayout.addWidget( &parentsBtn );
  btnLayout.addWidget( &childrenBtn );
  btnLayout.addWidget( &clientsBtn );
  btnLayout.addWidget( &suppliersBtn );
  btnLayout.addWidget( &filler );
  btnLayout.addWidget( &methodsBtn );
  btnLayout.addWidget( &attributesBtn );
  btnLayout.addWidget( &exportCombo );

  topLayout.addWidget( &classTree, 10);
}

/*----------------------------------------- ClassToolDialog::readIcons()
 * readIcons()
 *   Read and set the icons of all the buttons in the dialog.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassToolDialog::readIcons()
{
  parentsBtn.setPixmap( BarIcon("CTparents") );
  childrenBtn.setPixmap( BarIcon("CTchildren") );
  clientsBtn.setPixmap( BarIcon("CTclients"));
  suppliersBtn.setPixmap( BarIcon("CTsuppliers") );
  methodsBtn.setPixmap( BarIcon("CVpublic_meth"));
  attributesBtn.setPixmap( BarIcon("CVpublic_var") );
}

/*---------------------------------- ClassToolDialog::setTooltips()
 * setTooltips()
 *   Set tooltip strings.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassToolDialog::setTooltips()
{
  QToolTip::add( &parentsBtn, i18n("Show parents") );
  QToolTip::add( &childrenBtn, i18n("Show children") );
  QToolTip::add( &clientsBtn, i18n("Show clients") );
  QToolTip::add( &suppliersBtn, i18n("Show suppliers") );
  QToolTip::add( &methodsBtn, i18n("Show methods") );
  QToolTip::add( &attributesBtn, i18n("Show attributes") );
}

/*-------------------------------------- ClassToolDialog::setCallbacks()
 * setCallbacks()
 *   Set all signal<->slot mappings.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassToolDialog::setCallbacks()
{
  connect( &classCombo, SIGNAL(activated(const QString&)), SLOT(slotClassComboChoice(const QString&)));
  connect( &exportCombo, SIGNAL(activated(const QString&)), SLOT(slotExportComboChoice(const QString&)));
  connect( &parentsBtn, SIGNAL(clicked()), SLOT(viewParents()));
  connect( &childrenBtn, SIGNAL(clicked()), SLOT(viewChildren()));
  connect( &clientsBtn, SIGNAL(clicked()), SLOT(viewClients()));
  connect( &suppliersBtn, SIGNAL(clicked()), SLOT(viewSuppliers()));
  connect( &methodsBtn, SIGNAL(clicked()), SLOT(viewMethods()));
  connect( &attributesBtn, SIGNAL(clicked()), SLOT(viewAttributes()));
}


/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

void ClassToolDialog::setClassStore(ClassStore *store)
{
    m_store = store;
    classTree.setClassStore(store);

    if (!m_store)
        currentOperation = ViewNone;
    refresh();
}


void ClassToolDialog::setLanguageSupport(KDevLanguageSupport *ls)
{
    classTree.setLanguageSupport(ls);
    if (ls)
        connect(ls, SIGNAL(updateSourceInfo()), this, SLOT(refresh()));
}


void ClassToolDialog::setClassName(const QString &name)
{
    QListBox *lb = classCombo.listBox();

    for (int i=0; i < (int)lb->count(); ++i)
        if (lb->text(i) == name) {
            classCombo.setCurrentItem(i);
            break;
        }

    if (!name.isEmpty())
        currentClass = m_store->getClassByName(name);
}


/** View the parents of the current class. */
void ClassToolDialog::viewParents()
{
    currentOperation = ViewParents;
    refresh();
}


/** View the children of the current class. */
void ClassToolDialog::viewChildren()
{
    currentOperation = ViewChildren;
    refresh();
}


/** View all classes that has this class as an attribute. */
void ClassToolDialog::viewClients()
{
    currentOperation = ViewClients;
    refresh();
}


/** View all classes that this class has as attributes. */
void ClassToolDialog::viewSuppliers()
{
    currentOperation = ViewSuppliers;
    refresh();
}


/** View methods in this class and parents. */
void ClassToolDialog::viewMethods()
{
    currentOperation = ViewMethods;
    refresh();
}


/** View attributes in this class and parents. */
void ClassToolDialog::viewAttributes()
{
    currentOperation = ViewAttributes;
    refresh();
}


void ClassToolDialog::slotExportComboChoice(const QString &str)
{
    if( str == i18n("All") )
        comboExport = (PIExport)-1;
    else if( str == "Public" )
        comboExport = PIE_PUBLIC;
    else if( str == "Protected" )
        comboExport = PIE_PROTECTED;
    else if( str == "Private" )
        comboExport = PIE_PRIVATE;

    // Update the view if the choice affected the data.
    if (currentOperation == ViewMethods || currentOperation == ViewAttributes)
        buildTree();
}


void ClassToolDialog::slotClassComboChoice(const QString &str)
{
    setClassName(str);
}


void ClassToolDialog::refresh()
{
    // Clear the combo box and fill it with the new items.
    // Try to select the previously selected class
    
    QString oldName = classCombo.currentText();

    classCombo.clear();
    if (m_store) {
        QStrList *list = m_store->getSortedClassNameList();
        classCombo.insertStrList(list);
        delete list;
        setClassName(oldName);
    }

    // Rebuild the tree and caption/button state
    buildTree();
}


/** Change the caption depending on the current operation. */
void ClassToolDialog::updateCaptionAndButtons()
{
    QString caption;
    QToolButton *button;
    
    switch (currentOperation)
        {
        case ViewParents:
            button = &parentsBtn;
            caption = i18n("Parents");
            break;
        case ViewChildren:
            button = &childrenBtn;
            caption = i18n("Children");
            break;
        case ViewClients:
            button = &clientsBtn;
            caption = i18n("Clients");
            break;
        case ViewSuppliers:
            button = &suppliersBtn;
            caption = i18n("Suppliers");
            break;
        case ViewMethods:
            button = &methodsBtn;
            caption = i18n("%1 methods").arg(exportCombo.currentText());
            break;
        case ViewAttributes:
            button = &attributesBtn;
            caption = i18n("%1 attributes").arg(exportCombo.currentText());
            break;
        default:
            button = 0;
            caption = i18n("Class Tool Dialog");
            break;
        }

    parentsBtn.setOn(false);
    childrenBtn.setOn(false);
    clientsBtn.setOn(false);
    suppliersBtn.setOn(false);
    methodsBtn.setOn(false);
    attributesBtn.setOn(false);
    if (button) {
        button->setOn(true);
        setCaption(i18n("%1 of class %2").arg(caption).arg(currentClass->name));
    } else
        setCaption(caption);
}


void ClassToolDialog::buildTree()
{
    updateCaptionAndButtons();
    
    classTree.clear();
    
    switch (currentOperation)
        {
        case ViewParents:
            classTree.insertClassAndClasses(currentClass, &currentClass->parents);
            break;
        case ViewChildren:
            {
                QList<ParsedClass> *list = m_store->getClassesByParent(currentClass->name);
                classTree.insertClassAndClasses(currentClass, list);
                delete list;
            }
            break;
        case ViewClients:
            {
                QList<ParsedClass> *list = m_store->getClassClients(currentClass->name);
                classTree.insertClassAndClasses(currentClass, list);
                delete list;
            }
            break;
        case ViewSuppliers:
            {
                QList<ParsedClass> *list = m_store->getClassSuppliers( currentClass->name );
                classTree.insertClassAndClasses( currentClass, list );
                delete list;
            }
            break;
        case ViewMethods:
            classTree.insertAllClassMethods(currentClass, comboExport);
            break;
        case ViewAttributes:
            classTree.insertAllClassAttributes(currentClass, comboExport);
            break;
        default:
            break;
        }
}
#include "classtooldlg.moc"
