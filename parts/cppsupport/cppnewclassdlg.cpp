/***************************************************************************
*   Copyright (C) 1998 by Sandy Meier                                     *
*   smeier@rz.uni-potsdam.de                                              *
*   Copyright (C) 1999 by Benoit.Cerrina                                  *
*   Benoit.Cerrina@writeme.com                                            *
*   Copyright (C) 2002 by Bernd Gehrmann                                  *
*   bernd@kdevelop.org                                                    *
*   Copyright (C) 2003 by Eray Ozkural                                    *
*   <erayo@cs.bilkent.edu.tr>                                             *
*   Copyright (C) 2003 by Alexander Dymo                                  *
*   cloudtemple@mksat.net                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "cppnewclassdlg.h"

#include <qcheckbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qtextedit.h>
#include <qrect.h>
#include <qstyle.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <klineedit.h>
#include <kdeversion.h>

#include "kdevplugin.h"
#include "kdevproject.h"
#include "domutil.h"
#include "filetemplate.h"

#include "classstore.h"
#include "parsedmethod.h"

CppNewClassDialog::CppNewClassDialog(KDevPlugin *part, QWidget *parent, const char *name)
	: CppNewClassDialogBase(parent, name)
{
    headerModified = false;
    implementationModified = false;
    m_part = part;
    // read file template configuration
    //    KDevProject *project = part->project();
    QDomDocument &dom = *part->projectDom();
    interface_url = DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/interfaceURL");
    implementation_url = DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/implementationURL");
    interface_suffix = DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/interfacesuffix", ".h");
    implementation_suffix = DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/implementationsuffix", ".cpp");
    lowercase_filenames = DomUtil::readBoolEntry(dom, "/cppsupportpart/filetemplates/lowercasefilenames", true);
    m_parse = DomUtil::readEntry( *m_part->projectDom(), "/cppsupportpart/newclass/filenamesetting","none");
    //    name_handler_combo->setCurrentText(m_parse);
    baseclasses_view->setSorting(-1);
    constructors_view->setSorting(-1);

    accessMenu = new QPopupMenu(this);
    accessMenu->insertItem(i18n("Use as private"),
        this, SLOT(changeToPrivate()), 0, 1);
    accessMenu->insertItem(i18n("Use as protected"),
        this, SLOT(changeToProtected()), 0, 2);
    accessMenu->insertItem(i18n("Use as public"),
        this, SLOT(changeToPublic()), 0, 3);
    accessMenu->insertSeparator();
    accessMenu->insertItem(i18n("Unset"),
        this, SLOT(changeToInherited()), 0, 5);

    overMenu = new QPopupMenu(this);
    overMenu->insertItem(i18n("Extend base class functionality"),
        this, SLOT(extendFunctionality()), 0, 11);
    overMenu->insertItem(i18n("Replace base class method"),
        this, SLOT(replaceFunctionality()), 0, 12);

    comp = basename_edit->completionObject();
    setCompletion(m_part->classStore());
    setCompletion(m_part->ccClassStore());
    classname_edit->setFocus();
}


CppNewClassDialog::~CppNewClassDialog()
{
    delete comp;
}

void CppNewClassDialog::setCompletion(ClassStore *store)
{
  QValueList<ParsedClass*> classlist = store->getSortedClassList();
  QValueList<ParsedClass*>::iterator it;
  for ( it = classlist.begin(); it != classlist.end(); ++it )
    comp->addItem((*it)->name());
}

void CppNewClassDialog::nameHandlerChanged(const QString &text)
{
	DomUtil::writeEntry( *m_part->projectDom(), "/cppsupportpart/newclass/filenamesetting",text);
	m_parse = text;
	classNameChanged(classname_edit->text());
}

void CppNewClassDialog::classNameChanged(const QString &text)
{
    QString str = text;

    if (!headerModified) {
        QString header = str + interface_suffix;
        if (lowercase_filenames)
            header = header.lower();
        header_edit->setText(header);
    }
    if (!implementationModified) {
        QString implementation = str + implementation_suffix;
        if (lowercase_filenames)
            implementation = implementation.lower();
        implementation_edit->setText(implementation);
    }
}


void CppNewClassDialog::headerChanged()
{
	// Only if a change caused by the user himself
	if (header_edit->hasFocus())
		headerModified = true;
}


void CppNewClassDialog::implementationChanged()
{
	// Only if a change caused by the user himself
	if (implementation_edit->hasFocus())
		implementationModified = true;
}

void CppNewClassDialog::checkObjCInheritance(int val)
{
    childclass_box->setEnabled(!val);
    gtk_box->setEnabled(!val);
    qobject_box->setEnabled(!val);
    namespace_edit->setEnabled(!val);
    class_tabs->setTabEnabled(tab2, !val);
    if (val && (baseclasses_view->childCount() > 1))
        if (KMessageBox::warningContinueCancel(this,
        i18n("Objective C does not support multiple inheritance.\nOnly the first base class in the list will be taken into account."),
        i18n("Warning"), KStdGuiItem::cont(), "Check Objective C inheritance rules" ) == KMessageBox::Cancel)
        objc_box->setChecked(false);
}

void CppNewClassDialog::checkQWidgetInheritance(int val)
{
    if (val)
    {
        qobject_box->setEnabled(val);
	qobject_box->setChecked(val);
        objc_box->setEnabled(!val);
	gtk_box->setEnabled(!val);
    }
    else if (qobject_box->isChecked())
    {
        objc_box->setEnabled(false);
	gtk_box->setEnabled(false);
    }
    else
    {
        objc_box->setEnabled(!val);
	gtk_box->setEnabled(!val);
    }


    if (val)
    {
        if (baseclasses_view->childCount() == 0)
        {
            addBaseClass();
            basename_edit->setText("QWidget");
        }
/*        constructors_cpp_edit->append(classname_edit->text() + "::" + classname_edit->text() +
            "(QWidget *parent, const char *name):\n    QWidget(parent, name)\n{\n}\n");
        constructors_h_edit->append(classname_edit->text() + "(QWidget *parent, const char *name);\n");*/
    }

    if (val && (baseclasses_view->childCount() > 1))
        if (KMessageBox::warningContinueCancel(this,
            i18n("Multiple inheritance requires QObject derivative to be first and unique in base class list."),
            i18n("Warning"), KStdGuiItem::cont(), "Check QWidget inheritance rules" ) == KMessageBox::Cancel)
            childclass_box->setChecked(false);
}

void CppNewClassDialog::qobject_box_stateChanged(int val)
{
    if ( childclass_box->isChecked() )
        return;

    if (baseclasses_view->childCount() == 0)
    {
        addBaseClass();
        basename_edit->setText("QObject");
    }

    
    objc_box->setEnabled(!val);
    gtk_box->setEnabled(!val);
}

void CppNewClassDialog::gtk_box_stateChanged(int val)
{
    class_tabs->setTabEnabled(tab2, !val);
    childclass_box->setEnabled(!val);
    objc_box->setEnabled(!val);
    qobject_box->setEnabled(!val);
    namespace_edit->setEnabled(!val);

    basename_edit->setEnabled(!val);
    virtual_box->setEnabled(!val);
    public_button->setEnabled(!val);
    protected_button->setEnabled(!val);
    private_button->setEnabled(!val);
    addbaseclass_button->setEnabled(!val);
    rembaseclass_button->setEnabled(!val);
    upbaseclass_button->setEnabled(!val);
    downbaseclass_button->setEnabled(!val);
    baseclasses_view->setEnabled(!val);
}


void CppNewClassDialog::accept()
{
  ClassGenerator generator(*this);
  if (generator.generate())
    QDialog::accept();

}

void CppNewClassDialog::setStateOfInheritanceEditors(bool state, bool hideList)
{
    basename_edit->setEnabled(state);
    virtual_box->setEnabled(state);
    public_button->setEnabled(state);
    protected_button->setEnabled(state);
    private_button->setEnabled(state);
    if (state)
        baseclasses_view->setEnabled(state);
    else
        baseclasses_view->setEnabled(hideList ? state : true);
    rembaseclass_button->setEnabled(state);
    if (!state)
    {
        upbaseclass_button->setEnabled(state);
        downbaseclass_button->setEnabled(state);
    }
}

void CppNewClassDialog::addBaseClass()
{
    if (baseclasses_view->selectedItem())
        baseclasses_view->selectedItem()->setSelected(false);
    QListViewItem* it = new QListViewItem(baseclasses_view, baseclasses_view->lastItem(), QString::null, "public");
    setStateOfInheritanceEditors(true);
    public_button->setChecked(true);
    virtual_box->setChecked(false);
    basename_edit->setText(QString::null);
    basename_edit->setFocus();
    baseclasses_view->setSelected(it, true);
}

void CppNewClassDialog::remBaseClass()
{
    bool basename_focused = false;
    if (basename_edit->hasFocus())
    {
        basename_focused = true;
        basename_edit->clearFocus();
    }
    if (baseclasses_view->selectedItem())
    {
        QListViewItem *it = baseclasses_view->selectedItem();
        remClassFromAdv(it->text(0));
        baseclasses_view->selectedItem()->setSelected(false);
        if (it->itemBelow())
            baseclasses_view->setSelected(it->itemBelow(), true);
        else if (it->itemAbove())
            baseclasses_view->setSelected(it->itemAbove(), true);
        delete it;
        if (baseclasses_view->childCount() == 0)
            setStateOfInheritanceEditors(false);
    }
    if (basename_focused)
        basename_edit->setFocus();
}

void CppNewClassDialog::remBaseClassOnly()
{
    if (baseclasses_view->selectedItem())
    {
        QListViewItem *it = baseclasses_view->selectedItem();
        baseclasses_view->selectedItem()->setSelected(false);
        if (it->itemBelow())
            baseclasses_view->setSelected(it->itemBelow(), true);
        else if (it->itemAbove())
            baseclasses_view->setSelected(it->itemAbove(), true);
        delete it;
        if (baseclasses_view->childCount() == 0)
            setStateOfInheritanceEditors(false);
    }
}

void CppNewClassDialog::remClassFromAdv(QString text)
{
    QListViewItem *it = 0;
    if ((it = access_view->findItem(text, 0)))
        delete it;
    if ((it = methods_view->findItem(text, 0)))
        delete it;
    if ((it = constructors_view->findItem(text, 0)))
    {
        // TODO: changing constructors text in constructors_cpp_edit
        // and constructors_h_edit must be implemented

/*        int *para = new int(1);
        int *index = new int(1);
        if (constructors_cpp_edit->find(text + "(", true, false, true, para, index))
        {
            qWarning("%s( found", text.latin1());
            if (para) constructors_cpp_edit->removeParagraph(*para);
        }*/
        delete it;
    }
}

void CppNewClassDialog::currBaseNameChanged(const QString &text)
{
    if ( baseclasses_view->selectedItem() && (basename_edit->hasFocus()))
    {
        if (class_tabs->isTabEnabled(tab2))
        {
            //check for this class in the adv. inheritance lists
            //and delete if it exists
            remClassFromAdv(baseclasses_view->selectedItem()->text(0));
            //parse new base class
            parseClass(text, baseclasses_view->selectedItem()->text(1));
        }
        baseclasses_view->selectedItem()->setText(0, text);
        updateConstructorsOrder();
    }
}

void CppNewClassDialog::currBasePrivateSet()
{
    if ( baseclasses_view->selectedItem() )
    {
        setAccessForBase(baseclasses_view->selectedItem()->text(0), "private");
        baseclasses_view->selectedItem()->setText(1, (virtual_box->isChecked()?"virtual ":"") + QString("private"));
    }
}

void CppNewClassDialog::currBaseProtectedSet()
{
    if ( baseclasses_view->selectedItem() )
    {
        setAccessForBase(baseclasses_view->selectedItem()->text(0), "protected");
        baseclasses_view->selectedItem()->setText(1, (virtual_box->isChecked()?"virtual ":"") + QString("protected"));
    }
}

void CppNewClassDialog::currBasePublicSet()
{
    if ( baseclasses_view->selectedItem() )
    {
        setAccessForBase(baseclasses_view->selectedItem()->text(0), "public");
        baseclasses_view->selectedItem()->setText(1, (virtual_box->isChecked()?"virtual ":"") + QString("public"));
    }
}

void CppNewClassDialog::currBaseVirtualChanged(int val)
{
    if ( baseclasses_view->selectedItem() )
    {
        baseclasses_view->selectedItem()->setText(1, QString(val?"virtual ":"") +
         QString(private_button->isChecked()?"private":"") +
         QString(protected_button->isChecked()?"protected":"") +
         QString(public_button->isChecked()?"public":""));
    }
}

void CppNewClassDialog::currBaseSelected(QListViewItem *it)
{
    if (it == 0)
    {
        setStateOfInheritanceEditors(false, false);
        return;
    }
    setStateOfInheritanceEditors(true);
    basename_edit->setText(it->text(0));
    if (it->text(1).contains("private"))
        private_button->setChecked(true);
    else
        private_button->setChecked(false);
    if (it->text(1).contains("protected"))
        protected_button->setChecked(true);
    else
        protected_button->setChecked(false);
    if (it->text(1).contains("public"))
        public_button->setChecked(true);
    else
        public_button->setChecked(false);
    if (it->text(1).contains("virtual"))
        virtual_box->setChecked(true);
    else
        virtual_box->setChecked(false);
    checkUpButtonState();
    checkDownButtonState();
}

void CppNewClassDialog::upbaseclass_button_clicked()
{
    bool basename_focused = false;
    if (basename_edit->hasFocus())
    {
        basename_focused = true;
        basename_edit->clearFocus();
    }
    if (baseclasses_view->selectedItem())
    {
        QListViewItem *it = baseclasses_view->selectedItem();
        if (it->itemAbove())
        {
            QListViewItem *newit;
            if (it->itemAbove()->itemAbove())
                newit = new QListViewItem(baseclasses_view, it->itemAbove()->itemAbove(),
                    it->text(0), it->text(1));
            else
                newit = new QListViewItem(baseclasses_view, it->text(0), it->text(1));
            remBaseClassOnly();
            baseclasses_view->setSelected(newit, true);
            checkUpButtonState();
            updateConstructorsOrder();
        }
    }
    if (basename_focused)
        basename_edit->setFocus();
}

void CppNewClassDialog::downbaseclass_button_clicked()
{
    bool basename_focused = false;
    if (basename_edit->hasFocus())
    {
        basename_focused = true;
        basename_edit->clearFocus();
    }
    if (baseclasses_view->selectedItem())
    {
        QListViewItem *it = baseclasses_view->selectedItem();
        if (it->itemBelow())
        {
            QListViewItem *newit = new QListViewItem(baseclasses_view, it->itemBelow(),
                it->text(0), it->text(1));
            remBaseClassOnly();
            baseclasses_view->setSelected(newit, true);
            setStateOfInheritanceEditors(true);
            checkDownButtonState();
            updateConstructorsOrder();
        }
    }
    if (basename_focused)
        basename_edit->setFocus();
}

void CppNewClassDialog::updateConstructorsOrder()
{
    QListViewItemIterator it( baseclasses_view );
    QListViewItem *c_it;
    QListViewItem *fc_it = 0;

    while ( it.current() )
    {
        if ( (c_it = constructors_view->findItem(it.current()->text(0), 0)) )
        {
            c_it->moveItem(fc_it);
            fc_it = c_it;
        }
        ++it;
    }
}


void CppNewClassDialog::checkUpButtonState()
{
    if (baseclasses_view->selectedItem())
        upbaseclass_button->setEnabled(baseclasses_view->selectedItem()->itemAbove());
}

void CppNewClassDialog::checkDownButtonState()
{
    if (baseclasses_view->selectedItem())
        downbaseclass_button->setEnabled(baseclasses_view->selectedItem()->itemBelow());
}

void CppNewClassDialog::baseclasses_view_selectionChanged()
{
/*    if (baseclasses_view->selectedItem())
    {
        setStateOfInheritanceEditors(false, false);
    }*/
}

void CppNewClassDialog::changeToPrivate()
{
    if (access_view->selectedItem())
        access_view->selectedItem()->setText(2, "private");
}

void CppNewClassDialog::changeToProtected()
{
    if (access_view->selectedItem())
        access_view->selectedItem()->setText(2, "protected");
}

void CppNewClassDialog::changeToPublic()
{
    if (access_view->selectedItem())
        access_view->selectedItem()->setText(2, "public");
}

void CppNewClassDialog::changeToInherited()
{
    if (access_view->selectedItem())
        access_view->selectedItem()->setText(2, QString::null);
}

void CppNewClassDialog::newTabSelected(const QString& /*text*/)
{
/*    if (text == i18n("&Advanced Inheritance"))
        reloadAdvancedInheritance(true);*/
}

void CppNewClassDialog::newTabSelected(QWidget* /*w*/)
{
/*    if ( QString(w->name()) == QString("tab2"))
    {
        reloadAdvancedInheritance(false);
    }*/
}


void CppNewClassDialog::reloadAdvancedInheritance(bool /*clean*/)
{
/*    clearConstructorsList(clean);
    clearMethodsList(clean);
    clearUpgradeList(clean);

    QListViewItemIterator it( baseclasses_view );
    while ( it.current() )
    {
        if (! (it.current()->text(0).isNull()) )
        {
            parseClass(it.current()->text(0), it.current()->text(1));
        }
        ++it;
    }*/
}

void CppNewClassDialog::parseClass(QString clName, QString inheritance)
{
    ParsedClass * myClass = m_part->classStore()->getClassByName(clName);
    if (! myClass)
        myClass = m_part->ccClassStore()->getClassByName(clName);
    if (myClass)
    {
        PCheckListItem<ParsedClass*> *it = new PCheckListItem<ParsedClass*>(myClass, constructors_view, myClass->name());
        PListViewItem<ParsedClass*> *over = new PListViewItem<ParsedClass*>(myClass, methods_view, myClass->name());
        QListViewItem *over_methods = new QListViewItem(over, i18n("Methods"));
        QListViewItem *over_slots = new QListViewItem(over, i18n("Slots (Qt-specific)"));
        PListViewItem<ParsedClass*> *access = new PListViewItem<ParsedClass*>(myClass, access_view, myClass->name());
        QListViewItem *access_methods = new QListViewItem(access, i18n("Methods"));
        QListViewItem *access_slots = new QListViewItem(access, i18n("Slots (Qt-specific)"));
        QListViewItem *access_attrs = new QListViewItem(access, i18n("Attributes"));

        ParsedMethod *method = 0;
        myClass->methodIterator.toFirst();
        while ( (method = myClass->methodIterator.current()) != 0)
        {
            if (isConstructor(myClass->name(), method))
//            if (method->isConstructor())
            {
                addToConstructorsList(it, method);
            }
            else
            {
                addToMethodsList(over_methods, method);

                // display only public and protected methods of the base class
                if ( (!method->isDestructor()) && (!method->isPrivate()) )
                {
                    // see what modifier is given for the base class
                    QString inhModifier;
                    //protected inheritance gives protected methods
                    if (inheritance.contains("protected")) inhModifier = "protected";
                    //private inheritance gives private methods
                    else if (inheritance.contains("private")) inhModifier = "private";
                    //public inheritance gives protected and public methods
                    else if (inheritance.contains("public")) inhModifier = method->isPublic() ? "public" : "protected";
                    addToUpgradeList(access_methods, method, inhModifier );
                }
            }
            ++(myClass->methodIterator);
        }

        ParsedAttribute *attr = 0;
        myClass->attributeIterator.toFirst();
        while ( (attr = myClass->attributeIterator.current()) != 0 )
        {
            if (!attr->isPrivate())
            {
                QString inhModifier;
                //protected inheritance gives protected attributes
                if (inheritance.contains("protected")) inhModifier = "protected";
                //private inheritance gives private attributes
                else if (inheritance.contains("private")) inhModifier = "private";
                //public inheritance gives protected and public attributes
                else if (inheritance.contains("public")) inhModifier = attr->isPublic() ? "public" : "protected";
                addToUpgradeList(access_attrs, attr, inhModifier );
            }
            ++(myClass->attributeIterator);
        }

        ParsedMethod *slot = 0;
        myClass->slotIterator.toFirst();
        while ( (slot = myClass->slotIterator.current()) != 0 )
        {
            addToMethodsList(over_slots, slot);

            if (!slot->isPrivate())
            {
                QString inhModifier;
                //protected inheritance gives protected attributes
                if (inheritance.contains("protected")) inhModifier = "protected";
                //private inheritance gives private attributes
                else if (inheritance.contains("private")) inhModifier = "private";
                //public inheritance gives protected and public attributes
                else if (inheritance.contains("public")) inhModifier = slot->isPublic() ? "public" : "protected";
                addToUpgradeList(access_slots, slot, inhModifier );
            }
            ++(myClass->slotIterator);
        }

        parsedClasses << clName;
    }
}

bool CppNewClassDialog::isConstructor(QString className, ParsedMethod *method)
{
//  regexp:  myclass\\s*\\(\\s*(const)?\\s*myclass\\s*&[A-Za-z_0-9\\s]*\\) is for copy constructors
    if ( (className == method->name()) )
    {
        if ( method->asString().contains(QRegExp(className + "\\s*\\(\\s*(const)?\\s*" + className + "\\s*&[A-Za-z_0-9\\s]*\\)", true, false)) )
            return false;
        else
            return true;
    }
    else
        return false;
}

void CppNewClassDialog::addToConstructorsList(QCheckListItem *myClass, ParsedMethod *method)
{
    /*UNUSED! PCheckListItem<ParsedMethod*> *it = */ new PCheckListItem<ParsedMethod*>(method, myClass, method->asString(), QCheckListItem::RadioButton);
}

void CppNewClassDialog::addToMethodsList(QListViewItem *parent, ParsedMethod *method)
{
    if (!method->isDestructor())
    {
        PCheckListItem<ParsedMethod*> *it = new PCheckListItem<ParsedMethod*>(method, parent, method->asString(), QCheckListItem::CheckBox);
        it->setText(1, i18n("extend"));
    }
}

void CppNewClassDialog::addToUpgradeList(QListViewItem *parent, ParsedMethod *method, QString modifier)
{
    PListViewItem<ParsedMethod*> *it = new PListViewItem<ParsedMethod*>(method, parent, method->asString());
    it->setText(1, modifier);
}

void CppNewClassDialog::addToUpgradeList(QListViewItem *parent, ParsedAttribute *attr, QString modifier)
{
    PListViewItem<ParsedAttribute*> *it = new PListViewItem<ParsedAttribute*>(attr, parent, attr->asString());
    it->setText(1, modifier);
}

void CppNewClassDialog::clear_selection_button_clicked()
{
    QListViewItemIterator it( constructors_view );
    while ( it.current() )
    {
        PCheckListItem<ParsedMethod*> *curr;
        if ( (curr = dynamic_cast<PCheckListItem<ParsedMethod*>* >(it.current()) ) )
            curr->setOn(false);
        ++it;
    }
}

void CppNewClassDialog::clearConstructorsList(bool clean)
{
    if (clean)
        constructors_view->clear();
/*    else
    {
        QListViewItemIterator it( constructors_view );
        while ( it.current() )
        {
            if ( ! currBaseClasses.contains(it.current().text(0)) )
                delete it.current();
            ++it;
        }
    }*/
}

void CppNewClassDialog::clearMethodsList(bool clean)
{
    if (clean)
        methods_view->clear();
}

void CppNewClassDialog::clearUpgradeList(bool clean)
{
    if (clean)
        access_view->clear();
}

void CppNewClassDialog::setAccessForItem(QListViewItem *curr, QString newAccess, bool isPublic)
{
    if (newAccess == "public")
        curr->setText(1, isPublic ? "public" : "protected");
    else
        curr->setText(1, newAccess);
    if (!curr->text(2).isNull())
    {
        if ( (curr->text(2) == "private") && ((newAccess == "public") || (newAccess == "protected")) )
            curr->setText(2, QString::null);
        if ( (curr->text(2) == "protected") && ((newAccess == "public") && (isPublic)) )
            curr->setText(2, QString::null);
    }
}

void CppNewClassDialog::setAccessForBase(QString baseclass, QString newAccess)
{
    QListViewItem *base;

    if ( (base = access_view->findItem(baseclass, 0)) )
    {
        QListViewItemIterator it( base );
        while ( it.current() )
        {
            if ( !it.current()->text(1).isNull() )
            {
                PListViewItem<ParsedAttribute*> *curr;
                PListViewItem<ParsedMethod*> *curr_m;
                if ( (curr = dynamic_cast<PListViewItem<ParsedAttribute*>* >(it.current())) )
                    setAccessForItem(curr, newAccess, curr->item()->isPublic());
                else if ( (curr_m = dynamic_cast<PListViewItem<ParsedMethod*>* >(it.current())) )
                    setAccessForItem(curr_m, newAccess, curr_m->item()->isPublic());
            }
            ++it;
        }
    }
}


void CppNewClassDialog::access_view_mouseButtonPressed( int button, QListViewItem * item, const QPoint &p, int /*c*/ )
{
    if (item && ( (button == LeftButton) || (button == RightButton)) && (item->depth() > 1) )
    {
        accessMenu->setItemEnabled(1, true );
        accessMenu->setItemEnabled(2, true );
        accessMenu->setItemEnabled(3, true );
        if (item->text(1) == "protected")
        {
            accessMenu->setItemEnabled(1, false);
        }
        if (item->text(1) == "public")
        {
            accessMenu->setItemEnabled(1, false);
            accessMenu->setItemEnabled(2, false);
        }
        accessMenu->exec(p);

/*        accessMenu->setItemEnabled(1, item->text(1) == "private" ? false : true );
        accessMenu->setItemEnabled(2, item->text(1) == "protected" ? false : true );
        accessMenu->setItemEnabled(3, item->text(1) == "public" ? false : true );*/
    }
}


void CppNewClassDialog::methods_view_mouseButtonPressed(int button ,QListViewItem * item, const QPoint&p ,int /*c*/)
{
    if (item && ( button == RightButton ) && (item->depth() > 1) && (! item->text(1).isNull()) )
    {
        overMenu->exec(p);
    }
}

void CppNewClassDialog::extendFunctionality()
{
    if (methods_view->selectedItem())
        methods_view->selectedItem()->setText(1, i18n("extend"));
}

void CppNewClassDialog::replaceFunctionality()
{
    if (methods_view->selectedItem())
        methods_view->selectedItem()->setText(1, i18n("replace"));
}

void CppNewClassDialog::selectall_button_clicked()
{
    QListViewItemIterator it( constructors_view );
    while ( it.current() )
    {
	PCheckListItem<ParsedMethod*> *curr;
	if ( (curr = dynamic_cast<PCheckListItem<ParsedMethod*>* >(it.current()) ) )
	    curr->setOn(true);
	++it;
    }
}

void CppNewClassDialog::to_constructors_list_clicked()
{
    QString constructor_h = classname_edit->text();
    QString constructor_cpp = classname_edit->text() + "::" + classname_edit->text();
    constructor_h += "(";
    constructor_cpp += "(";
    QString params_h;
    QString params_cpp;
    QString base;
    int unnamed = 1;

    QListViewItemIterator it( constructors_view );
    while ( it.current() )
    {
        PCheckListItem<ParsedMethod*> *curr;
        if ( (curr = dynamic_cast<PCheckListItem<ParsedMethod*>* >(it.current())) )
        {
            if (curr->isOn() && curr->parent())
            {
                //fill the base classes list
                base += base.isEmpty() ? ": " : ", ";
                base += curr->parent()->text(0);
                params_h += params_h.isEmpty() ? "" : ", ";

                //fill arguments for both constructor and base class initializer
                //parser does not allow to process default values now ?!
                ParsedArgument *arg;
                QString cparams;
                QString bparams;
                for (arg = curr->item()->arguments.first(); arg; arg = curr->item()->arguments.next())
                {
                    bparams += bparams.isEmpty() ? "" : ", ";
                    cparams += cparams.isEmpty() ? "" : ", ";
                    cparams += arg->type() + " ";
                    if (arg->name().isNull())
                    {
                        cparams += QString("arg%1").arg(unnamed);
                        bparams += QString("arg%1").arg(unnamed++);
                    }
                    else
                    {
                        bparams += arg->name();
                        cparams += arg->name();
                    }
                }
                params_h += cparams;
                params_cpp = params_h;
                base += "(" + bparams + ")";
            }
        }
        ++it;
    }

    constructor_cpp += params_cpp + ")" + base + QString("\n{\n}\n\n\n");
    constructor_h += params_h + ");\n\n";

    constructors_h_edit->append(constructor_h);
    constructors_cpp_edit->append(constructor_cpp);
}

void CppNewClassDialog::updateClassStore()
{
}




bool CppNewClassDialog::ClassGenerator::validateInput()
{
  className = dlg.classname_edit->text().simplifyWhiteSpace();
  if (className.isEmpty()) {
    KMessageBox::error(&dlg, i18n("You must enter a classname."));
    return false;
  }

  header = dlg.header_edit->text().simplifyWhiteSpace();
  if (header.isEmpty()) {
    KMessageBox::error(&dlg, i18n("You must enter a name for the header file."));
    return false;
  }
  implementation = dlg.implementation_edit->text().simplifyWhiteSpace();
  if (implementation.isEmpty() ){
    KMessageBox::error(&dlg, i18n("You must enter a name for the implementation file."));
    return false;
  }

  // FIXME
  if (header.find('/') != -1 || implementation.find('/') != -1) {
    KMessageBox::error(&dlg, i18n("Generated files will always be added to the "
				  "active directory, so you must not give an "
				  "explicit subdirectory."));
    return false;
  }

  return true;
}


bool CppNewClassDialog::ClassGenerator::generate()
{
  if (!validateInput())
    return false;

  common_text();

  gen_implementation();

  gen_interface();

  return true;
}

void CppNewClassDialog::ClassGenerator::common_text()
{

  // common

  project = dlg.m_part->project();
  subDir = project->projectDirectory() + "/";
  if (!project->activeDirectory().isEmpty())
    subDir += project->activeDirectory() + "/";
  headerPath = subDir + header;
  implementationPath = subDir + implementation;

  if (QFileInfo(headerPath).exists() || QFileInfo(implementationPath).exists()) {
    KMessageBox::error(&dlg, i18n("KDevelop is not able to add classes "
				  "to existing header or implementation files."));
    return;
  }

  namespaceStr = dlg.namespace_edit->text();
  childClass = dlg.childclass_box->isChecked();
  objc = dlg.objc_box->isChecked();
  qobject = dlg.qobject_box->isChecked();
  gtk = dlg.gtk_box->isChecked();

  if ( (dlg.baseclasses_view->childCount() == 0) && childClass)
    /*UNUSED! QListViewItem* it = */ new QListViewItem(dlg.baseclasses_view, "QWidget", "public");
  if (objc && (dlg.baseclasses_view->childCount() == 0))
    /*UNUSED! QListViewItem* it = */ new QListViewItem(dlg.baseclasses_view, "NSObject", "public");

  doc = dlg.documentation_edit->text();

  if (!namespaceStr.isEmpty()) {
    namespaceBeg = "namespace " + namespaceStr + " {\n";
    namespaceEnd = "}\n";
  }

  //advanced constructor creation

  advConstructorsHeader = QString::null;
  advConstructorsSource = QString::null;
  if (!dlg.constructors_h_edit->text().isEmpty())
  {
      advConstructorsHeader = "    " + dlg.constructors_h_edit->text();
#if KDE_VERSION > 305
      advConstructorsHeader.replace("\n", "\n    ");
#else
#endif
  }
  if (!dlg.constructors_cpp_edit->text().isEmpty())
  {
      advConstructorsSource = dlg.constructors_cpp_edit->text();
  }

  //advanced method overriding

  advH_public = QString::null;
  advH_public_slots = QString::null;
  advH_protected = QString::null;
  advH_protected_slots = QString::null;
  advH_private = QString::null;
  advH_private_slots = QString::null;
  advCpp = QString::null;

  QListViewItemIterator it( dlg.methods_view );
  while ( it.current() )
  {
    PCheckListItem<ParsedMethod*> *curr;
    if ( (curr = dynamic_cast<PCheckListItem<ParsedMethod*>* >(it.current())) )
    {
      if (curr->isOn() && (curr->parent()) && (curr->parent()->parent()))
      {
        QString *adv_h = 0;
        if (curr->item()->isPrivate())
          adv_h = curr->item()->isSlot() ? &advH_private_slots : &advH_private;
        if (curr->item()->isProtected())
          adv_h = curr->item()->isSlot() ? &advH_protected_slots : &advH_protected;
        if (curr->item()->isPublic())
          adv_h = curr->item()->isSlot() ? &advH_public_slots : &advH_public;

        if (advCpp.isNull()) advCpp += "\n\n";

        genMethodDeclaration(curr->item(), className, adv_h, &advCpp,
            (curr->text(1) == i18n("extend")) ? true : false, curr->parent()->parent()->text(0));
      }
    }
    ++it;
  }

  //advanced access control and upgrading
  QListViewItemIterator ita( dlg.access_view );
  while ( ita.current() )
  {
    PListViewItem<ParsedAttribute*> *curr;
    PListViewItem<ParsedMethod*> *curr_m;
    if ( (curr = dynamic_cast<PListViewItem<ParsedAttribute*>* >(ita.current())) )
    {
        if ((!curr->text(2).isNull()) && (curr->parent()) && (curr->parent()->parent()) )
        {
            QString *adv_h = 0;
            if (curr->text(2) == "private") adv_h = &advH_private;
            if (curr->text(2) == "public") adv_h = &advH_public;
            if (curr->text(2) == "protected") adv_h = &advH_protected;

	    if ((*adv_h).isNull())
    	        *adv_h += "\n\n";

            *adv_h += QString("    using ") + curr->parent()->parent()->text(0) + "::"  + curr->item()->name() + ";\n";
        }
    }
    else if ( (curr_m = dynamic_cast<PListViewItem<ParsedMethod*>* >(ita.current())) )
    {
        if ((!curr_m->text(2).isNull())  && (curr_m->parent()) && (curr_m->parent()->parent()) )
        {
            QString *adv_h = 0;
            if (curr_m->text(2) == "private") adv_h = &advH_private;
            if (curr_m->text(2) == "public") adv_h = &advH_public;
            if (curr_m->text(2) == "protected") adv_h = &advH_protected;

	    if ((*adv_h).isNull())
    	        *adv_h += "\n\n";

            QString methodName = curr_m->item()->name();
            if (!methodName.contains(QRegExp("^[a-zA-z_]")))
                methodName = "operator" + methodName;
            *adv_h += "    using " + curr_m->parent()->parent()->text(0) + "::"  + methodName + ";\n";
        }
    }
    ++ita;
  }
}

void CppNewClassDialog::ClassGenerator::genMethodDeclaration(ParsedMethod *method,
    QString className, QString *adv_h, QString *adv_cpp, bool extend, QString baseClassName )
{
    if ((*adv_h).isNull())
        *adv_h += "\n\n";
    QString methodName = method->name();
    if (!methodName.contains(QRegExp("^[a-zA-z_]")))
	methodName = "operator" + methodName;
    *adv_h += "    " + (method->isVirtual() ? QString("virtual ") : QString(""))
	+ (method->isStatic() ? QString("static ") : QString(""))
	+ method->type() + " " + methodName + "(";
    *adv_cpp += method->type() + " " + className + "::" + methodName + "(";

    ParsedArgument *arg;
    QString bparams;
    QString cparams;
    int unnamed = 1;
    for (arg = method->arguments.first(); arg; arg = method->arguments.next())
    {
        bparams += bparams.isEmpty() ? "" : ", ";
        cparams += cparams.isEmpty() ? "" : ", ";
        cparams += arg->type() + " ";
        if (arg->name().isNull())
        {
            cparams += QString("arg%1").arg(unnamed);
            bparams += QString("arg%1").arg(unnamed++);
        }
        else
        {
            bparams += arg->name();
            cparams += arg->name();
        }
    }
    *adv_h += cparams + ")" + (method->isConst() ? " const" : "") + ";\n";
    *adv_cpp += cparams + ")" + (method->isConst() ? " const" : "") + "\n{\n";
    if (extend)
        *adv_cpp += ((method->type() == "void") ? "    " : "    return ") +
                    baseClassName + "::" + methodName + "(" + bparams + ");\n";
    *adv_cpp += "}\n\n";
}


void CppNewClassDialog::ClassGenerator::gen_implementation()
{

  // implementation

  QString classImpl;
  if (dlg.filetemplate_box->isChecked()) {
    QDomDocument dom = *dlg.m_part->projectDom();
    if(DomUtil::readBoolEntry(dom,"/cppsupportpart/filetemplates/choosefiles",false))
      classImpl = FileTemplate::read(dlg.m_part, DomUtil::readEntry(dom,"/cppsupportpart/filetemplates/implementationURL",""), FileTemplate::Custom);
    else
      classImpl = FileTemplate::read(dlg.m_part, "cpp");
  }
  if (objc) {
    classImpl += QString("\n"
			 "#include \"$HEADER$\"\n"
			 "@implementation $CLASSNAME$\n"
			 "@end\n");
  } else if (gtk)
  {
    classImpl += QString(
			 "#include \"$HEADER$\"\n"
			 "\n"
			 "\n"
            "$CLASSNAME$* $CLASSNAME$_new(void)\n"
            "{\n"
            "    $CLASSNAME$* self;\n"
            "    self = g_new($CLASSNAME$, 1);\n"
            "    if(NULL != self)\n"
            "    {\n"
            "        if(!$CLASSNAME$_init(self))\n"
            "        {\n"
            "            g_free(self);\n"
            "            self = NULL;\n"
            "        }\n"
            "    }\n"
            "    return self;\n"
            "}\n"
            "\n"
            "\n"
            "void $CLASSNAME$_delete($CLASSNAME$* self)\n"
            "{\n"
            "    g_return_if_fail(NULL != self);\n"
            "    $CLASSNAME$_end(self);\n"
            "    g_free(self);\n"
            "}\n"
            "\n"
            "\n"
            "gboolean $CLASSNAME$_init($CLASSNAME$* self)\n"
            "{\n"
            "    /* TODO: put init code here */\n"
            "\n"
            "    return TRUE;\n"
            "}\n"
            "\n"
            "\n"
            "void $CLASSNAME$_end($CLASSNAME$* self)\n"
            "{\n"
            "    /* TODO: put deinit code here */\n"
            "}\n"
            "\n");
  } else {
    classImpl += QString( /* "// $FILENAME\n" */
			 "#include \"$HEADER$\"\n"
			 "\n"
			 "\n")
      + namespaceBeg
      + ( advConstructorsSource.isNull() ? QString("$CLASSNAME$::$CLASSNAME$($ARGS$)\n"
		"$BASEINITIALIZER$"
		"{\n"
		"}\n") : advConstructorsSource )
      + QString("\n"
		"$CLASSNAME$::~$CLASSNAME$()\n"
		"{\n"
		"}\n")
      + advCpp
      + namespaceEnd;
  }

  QString relPath;
  for (int i = implementation.findRev('/'); i != -1; i = implementation.findRev('/', --i))
    relPath += "../";
  if (childClass)
    args = "QWidget *parent, const char *name, WFlags f";
  else if (qobject)
    args = "QObject *parent, const char *name";
  else
    args = "";
  QString baseInitializer;

  if (childClass && (dlg.baseclasses_view->childCount() == 0))
    baseInitializer = "  : QWidget(parent, name, f)";
  else if (qobject && (dlg.baseclasses_view->childCount() == 0))
    baseInitializer = "  : QObject(parent, name)";
  else if (dlg.baseclasses_view->childCount() != 0)
  {
    QListViewItemIterator it( dlg.baseclasses_view );
    baseInitializer += " : ";
    while ( it.current() )
    {
      if (!it.current()->text(0).isNull())
      {
        if (baseInitializer != " : ")
          baseInitializer += ", ";
        if (childClass && (baseInitializer == " : "))
          baseInitializer += it.current()->text(0) + "(parent, name, f)";
        else if (qobject && (baseInitializer == " : "))
          baseInitializer += it.current()->text(0) + "(parent, name)";
        else
          baseInitializer += it.current()->text(0) + "()";
      }
      ++it;
    }
    baseInitializer += "\n";
  }

  classImpl.replace(QRegExp("\\$HEADER\\$"), relPath+header);
  classImpl.replace(QRegExp("\\$BASEINITIALIZER\\$"), baseInitializer);
  classImpl.replace(QRegExp("\\$CLASSNAME\\$"), className);
  classImpl.replace(QRegExp("\\$ARGS\\$"), args);
  classImpl.replace(QRegExp("\\$FILENAME\\$"), implementation);

  QFile ifile(implementationPath);
  if (!ifile.open(IO_WriteOnly)) {
    KMessageBox::error(&dlg, "Cannot write to implementation file");
    return;
  }
  QTextStream istream(&ifile);
  istream << classImpl;
  ifile.close();

}


void CppNewClassDialog::ClassGenerator::gen_interface()
{
  // interface

  QString classIntf;
  if (dlg.filetemplate_box->isChecked()) {
    QDomDocument dom = *dlg.m_part->projectDom();
    if(DomUtil::readBoolEntry(dom,"/cppsupportpart/filetemplates/choosefiles",false))
      classIntf =
	FileTemplate::read(dlg.m_part, DomUtil::readEntry(dom,"/cppsupportpart/filetemplates/interfaceURL",""), FileTemplate::Custom);
    else
      classIntf  = FileTemplate::read(dlg.m_part, "h");
  }

  if (objc) {
    classIntf = QString("\n"
			"#ifndef _$HEADERGUARD$_\n"
			"#define _$HEADERGUARD$_\n"
			"\n"
			"$INCLUDEBASEHEADER$\n"
			"#include <Foundation/NSObject.h>\n"
			"\n"
			"\n"
			"/**\n"
			" * $DOC$\n"
			" * $AUTHOR$\n"
			" **/\n"
			"@interface $CLASSNAME$ : $BASECLASS$\n"
			"@end\n"
			"\n"
			"#endif\n"
			);
  }
  else if (gtk) {
    classIntf = QString("\n"
			"#ifndef $HEADERGUARD$\n"
			"#define $HEADERGUARD$\n"
			"\n"
            "#include <sys/types.h>\n"
            "#include <sys/stat.h>\n"
            "#include <unistd.h>\n"
            "#include <string.h>\n"
            "\n"
            "#include <gdk/gdk.h>\n"
            "#include <gtk/gtk.h>\n")
      + QString("/**\n"
		" * $DOC$\n"
		" * $AUTHOR$\n"
		" **/\n")
      + QString("typedef struct td_test {\n"
	    "/* TODO: put your data here */\n"
        "} $CLASSNAME$, *$CLASSNAME$Ptr;\n\n\n"
        "$CLASSNAME$* $CLASSNAME$_new(void);\n"
        "void $CLASSNAME$_delete($CLASSNAME$* self);\n"
        "gboolean $CLASSNAME$_init($CLASSNAME$* self);\n"
        "void $CLASSNAME$_end($CLASSNAME$* self);\n\n\n"
        "#endif\n");
  }
  else {
    classIntf = QString("\n"
			"#ifndef $HEADERGUARD$\n"
			"#define $HEADERGUARD$\n"
			"\n"
			"$INCLUDEBASEHEADER$\n"
			"\n")
      + namespaceBeg
      + QString("/**\n"
		" * $DOC$\n"
		" * $AUTHOR$\n"
		" **/\n")
      + QString("class $CLASSNAME$$INHERITANCE$\n"
		"{\n"
		"$QOBJECT$"
		"public:\n")
      + ( advConstructorsHeader.isNull() ? QString("    $CLASSNAME$($ARGS$);\n") : advConstructorsHeader )
      + QString("\n    ~$CLASSNAME$();\n")
      + advH_public
      + (advH_public_slots.isNull() ? QString::fromLatin1("") : ("\n\npublic slots:" + advH_public_slots))
      + (advH_protected.isNull() ? QString::fromLatin1("") : ("\n\nprotected:" + advH_protected))
      + (advH_protected_slots.isNull() ? QString::fromLatin1("") : ("\n\nprotected slots:" + advH_protected_slots))
      + (advH_private.isNull() ? QString::fromLatin1("") : ("\n\nprivate:" + advH_private))
      + (advH_private_slots.isNull() ? QString::fromLatin1("") : ("\n\nprivate slots:" + advH_private_slots))
      + QString("};\n"
		"\n")
      + namespaceEnd
      +     "#endif\n";
  }

  QString headerGuard = namespaceStr.upper() + header.upper();
  headerGuard.replace(QRegExp("\\."),"_");
  QString includeBaseHeader;
  if (childClass) // TODO: do this only if this is a Qt class
  {
    includeBaseHeader = "#include <qwidget.h>";
  }
  else if (qobject && (dlg.baseclasses_view->childCount() == 0))
  {
    includeBaseHeader = "#include <qobject.h>";
  }

  if (objc) {
    if (dlg.baseclasses_view->firstChild())
      if (dlg.baseclasses_view->firstChild()->text(0) != "NSObject")
        includeBaseHeader = "#include \"" + dlg.baseclasses_view->firstChild()->text(0) + ".h\"";
  } else
  {
    QListViewItemIterator it( dlg.baseclasses_view );
    while ( it.current() )
    {
      if (!it.current()->text(0).isEmpty())
        if ((!childClass) || (it.current()->text(0) != "QWidget"))
          includeBaseHeader += "\n#include <" + it.current()->text(0).lower() + ".h>";
      ++it;
    }
  }

  QString author = DomUtil::readEntry(*dlg.m_part->projectDom(), "/general/author");

  QString inheritance;
  if (dlg.baseclasses_view->childCount() > 0)
  {
    inheritance += " : ";

    QListViewItemIterator it( dlg.baseclasses_view );
    while ( it.current() )
    {
      if (!it.current()->text(0).isEmpty())
      {
        if (inheritance != " : ")
          inheritance += ", ";
        if (it.current()->text(1).contains("virtual"))
          inheritance += "virtual ";
        if (it.current()->text(1).contains("public"))
          inheritance += "public ";
        if (it.current()->text(1).contains("protected"))
          inheritance += "protected ";
        if (it.current()->text(1).contains("private"))
          inheritance += "private ";
        inheritance += it.current()->text(0);
      }
      ++it;
    }
  }
  else if (qobject)
    inheritance += ": public QObject";

  QString qobjectStr;
  if (childClass || qobject)
    qobjectStr = "Q_OBJECT\n";

  classIntf.replace(QRegExp("\\$HEADERGUARD\\$"), headerGuard);
  classIntf.replace(QRegExp("\\$INCLUDEBASEHEADER\\$"), includeBaseHeader);
  classIntf.replace(QRegExp("\\$AUTHOR\\$"), author);
  classIntf.replace(QRegExp("\\$DOC\\$"), doc);
  classIntf.replace(QRegExp("\\$CLASSNAME\\$"), className);
  if (dlg.baseclasses_view->childCount() > 0)
    classIntf.replace(QRegExp("\\$BASECLASS\\$"), dlg.baseclasses_view->firstChild()->text(0));
  classIntf.replace(QRegExp("\\$INHERITANCE\\$"), inheritance);
  classIntf.replace(QRegExp("\\$QOBJECT\\$"), qobjectStr);
  classIntf.replace(QRegExp("\\$ARGS\\$"), args);
  classIntf.replace(QRegExp("\\$FILENAME\\$"), header);


  QFile hfile(headerPath);
  if (!hfile.open(IO_WriteOnly)) {
    KMessageBox::error(&dlg, "Cannot write to header file");
    return;
  }
  QTextStream hstream(&hfile);
  hstream << classIntf;
  hfile.close();

	QStringList fileList;

	if ( project->activeDirectory().isNull() )
	{
		fileList.append ( header );
		fileList.append ( implementation );
	}
	else
	{
		fileList.append ( project->activeDirectory() + "/" + header );
		fileList.append ( project->activeDirectory() + "/" + implementation );
	}

	project->addFiles ( fileList );
}

#include "cppnewclassdlg.moc"
