/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classtooldlg.h"

#include <qlistbox.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include "kdevlanguagesupport.h"
#include "classstore.h"
#include "parsedclass.h"
#include "classviewpart.h"


ClassToolDialog::ClassToolDialog( ClassViewPart *part )
    : QWidget(0, "class tool dialog")
{
    currentOperation = ViewNone;
    comboAccess = (PIAccess)-1;
    currentClass = 0;
    m_part = part;

    class_combo = new QComboBox(false, this);

    QPushButton *close_button = new KPushButton(KStdGuiItem::close(), this);

    parents_button = new QToolButton(this);
    parents_button->setPixmap( UserIcon("CTparents", KIcon::DefaultState, ClassViewFactory::instance()) );
    parents_button->setToggleButton(true);
    parents_button->setFixedSize(parents_button->sizeHint());
    QToolTip::add(parents_button, i18n("Show parents"));

    children_button = new QToolButton(this);
    children_button->setPixmap( UserIcon("CTchildren", KIcon::DefaultState, ClassViewFactory::instance()) );
    children_button->setToggleButton(true);
    children_button->setFixedSize(children_button->sizeHint());
    QToolTip::add(children_button, i18n("Show children"));

    clients_button = new QToolButton(this);
    clients_button->setPixmap( UserIcon("CTclients", KIcon::DefaultState, ClassViewFactory::instance()) );
    clients_button->setToggleButton(true);
    clients_button->setFixedSize(clients_button->sizeHint());
    QToolTip::add(clients_button, i18n("Show clients"));

    suppliers_button = new QToolButton(this);
    suppliers_button->setPixmap( UserIcon("CTsuppliers", KIcon::DefaultState, ClassViewFactory::instance()) );
    suppliers_button->setToggleButton(true);
    suppliers_button->setFixedSize(suppliers_button->sizeHint());
    QToolTip::add(suppliers_button, i18n("Show suppliers"));

    methods_button = new QToolButton(this);
    methods_button->setPixmap( UserIcon("CVpublic_meth", KIcon::DefaultState, ClassViewFactory::instance()) );
    methods_button->setToggleButton(true);
    methods_button->setFixedSize(methods_button->sizeHint());
    QToolTip::add(methods_button, i18n("Show methods"));

    attributes_button = new QToolButton(this);
    attributes_button->setPixmap( UserIcon("CVpublic_var", KIcon::DefaultState, ClassViewFactory::instance()) );
    attributes_button->setToggleButton(true);
    attributes_button->setFixedSize(attributes_button->sizeHint());
    QToolTip::add(attributes_button, i18n("Show attributes"));

    access_combo = new QComboBox(false, this, "access combo");
    access_combo->setMinimumWidth(100);
    access_combo->setSizeLimit(10);
    access_combo->insertItem(i18n("member access","All"));
    access_combo->insertItem(i18n("member access","Public"));
    access_combo->insertItem(i18n("member access","Protected"));
    access_combo->insertItem(i18n("member access","Private"));
    access_combo->insertItem(i18n("member access","Package"));

    class_tree = new ClassToolWidget(part, this);
    //    classTree->setMinimumSize( 500, 400 );

    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    QHBoxLayout *firstrowLayout = new QHBoxLayout();
    QHBoxLayout *secondrowLayout = new QHBoxLayout();
    layout->addLayout(firstrowLayout);
    layout->addLayout(secondrowLayout);

    firstrowLayout->addWidget(class_combo, 1);
    firstrowLayout->addWidget(close_button, 0);

    secondrowLayout->addWidget(parents_button);
    secondrowLayout->addWidget(children_button);
    secondrowLayout->addWidget(clients_button);
    secondrowLayout->addWidget(suppliers_button);
    secondrowLayout->addStretch();
    secondrowLayout->addWidget(methods_button);
    secondrowLayout->addWidget(attributes_button);
    secondrowLayout->addWidget(access_combo);

    layout->addWidget(class_tree, 10);

    connect( class_combo, SIGNAL(activated(const QString&)),
             this, SLOT(slotClassComboChoice(const QString&)) );
    connect( close_button, SIGNAL(clicked()),
             this, SLOT(slotClose()) );
    connect( access_combo, SIGNAL(activated(const QString&)),
             this, SLOT(slotAccessComboChoice(const QString&)) );
    connect( parents_button, SIGNAL(clicked()), SLOT(viewParents()));
    connect( children_button, SIGNAL(clicked()), SLOT(viewChildren()));
    connect( clients_button, SIGNAL(clicked()), SLOT(viewClients()));
    connect( suppliers_button, SIGNAL(clicked()), SLOT(viewSuppliers()));
    connect( methods_button, SIGNAL(clicked()), SLOT(viewMethods()));
    connect( attributes_button, SIGNAL(clicked()), SLOT(viewAttributes()));

    connect( part, SIGNAL(setLanguageSupport(KDevLanguageSupport*)),
             this, SLOT(setLanguageSupport(KDevLanguageSupport*)) );

    m_part->registerClassToolDialog(this);
}


ClassToolDialog::~ClassToolDialog()
{
    m_part->unregisterClassToolDialog(this);
}


void ClassToolDialog::setLanguageSupport(KDevLanguageSupport *ls)
{
    if (ls) {
        disconnect(ls, 0, this, 0);
        connect(ls, SIGNAL(updatedSourceInfo()), this, SLOT(refresh()));
    } else
        refresh();

    currentOperation = ViewNone;
}


void ClassToolDialog::setClassName(const QString &name)
{
    if ( class_combo->count() == 0 ) refresh();

    QListBox *lb = class_combo->listBox();

    for (int i=0; i < (int)lb->count(); ++i)
        if (lb->text(i) == name) {
            class_combo->setCurrentItem(i);
            break;
        }

    if (!name.isEmpty())
        currentClass = m_part->classStore()->getClassByName(name);
    else
        currentClass = 0;
}


void ClassToolDialog::viewNone()
{
    currentOperation = ViewNone;
    refresh();
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


void ClassToolDialog::slotAccessComboChoice(const QString &str)
{
    if( str == i18n("member access","All") )
        comboAccess = (PIAccess)-1;
    else if( str == i18n("member access","Public") )
        comboAccess = PIE_PUBLIC;
    else if( str == i18n("member access","Protected") )
        comboAccess = PIE_PROTECTED;
    else if( str == i18n("member access","Private") )
        comboAccess = PIE_PRIVATE;
    else if( str == i18n("member access","Package") )
        comboAccess = PIE_PACKAGE;

    // Update the view if the choice affected the data.
    if (currentOperation == ViewMethods || currentOperation == ViewAttributes)
        buildTree();
}


void ClassToolDialog::slotClose()
{
    delete this;
    //    QTimer::singleShot(0, this, SLOT(delayedClose()));
}


void ClassToolDialog::delayedClose()
{
    delete this;
}


void ClassToolDialog::slotClassComboChoice(const QString &str)
{
    setClassName(str);
    refresh();
}


void ClassToolDialog::refresh()
{
    // Clear the combo box and fill it with the new items.
    // Try to select the previously selected class

    QString oldName = class_combo->currentText();

    class_combo->clear();
    QStringList list = m_part->classStore()->getSortedClassNameList();
    class_combo->insertStringList(list);
    setClassName(oldName);

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
            button = parents_button;
            caption = i18n("Parents");
            break;
        case ViewChildren:
            button = children_button;
            caption = i18n("Children");
            break;
        case ViewClients:
            button = clients_button;
            caption = i18n("Clients");
            break;
        case ViewSuppliers:
            button = suppliers_button;
            caption = i18n("Suppliers");
            break;
        case ViewMethods:
            button = methods_button;
            caption = i18n("%1 Methods").arg(access_combo->currentText());
            break;
        case ViewAttributes:
            button = attributes_button;
            caption = i18n("%1 Attributes").arg(access_combo->currentText());
            break;
        default:
            button = 0;
            caption = i18n("Class Tool Dialog");
            break;
        }

    parents_button->setOn(false);
    children_button->setOn(false);
    clients_button->setOn(false);
    suppliers_button->setOn(false);
    methods_button->setOn(false);
    attributes_button->setOn(false);

    if (button) {
        button->setOn(true);
        setCaption(i18n("%1 of Class %2").arg(caption).arg(currentClass->name()));
    } else
        setCaption(caption);
}


void ClassToolDialog::buildTree()
{
    if (!currentClass)
        currentOperation = ViewNone;

    updateCaptionAndButtons();

    class_tree->clear();

    switch (currentOperation)
        {
        case ViewParents:
            class_tree->insertClassAndClasses(currentClass, currentClass->parents);
            break;
        case ViewChildren:
            {
                QValueList<ParsedClass*> list = m_part->classStore()->getClassesByParent(currentClass->name());
                class_tree->insertClassAndClasses(currentClass, list);
            }
            break;
        case ViewClients:
            {
                QValueList<ParsedClass*> list = m_part->classStore()->getClassClients(currentClass->name());
                class_tree->insertClassAndClasses(currentClass, list);
            }
            break;
        case ViewSuppliers:
            {
                QValueList<ParsedClass*> list = m_part->classStore()->getClassSuppliers(currentClass->name());
                class_tree->insertClassAndClasses(currentClass, list);
            }
            break;
        case ViewMethods:
            class_tree->insertAllClassMethods(currentClass, comboAccess);
            break;
        case ViewAttributes:
            class_tree->insertAllClassAttributes(currentClass, comboAccess);
            break;
        default:
            break;
        }
}

#include "classtooldlg.moc"
