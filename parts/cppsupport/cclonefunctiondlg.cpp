/***************************************************************************
                          cclonefunctiondlg.cpp  -  description
                             -------------------
    begin                : Sat Nov 18 2000
    copyright            : (C) 2000 by The KDevelop Team
                           (c) 2000-2001 August Hörandl
    email                : august.hoerandl@gmx.at
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cclonefunctiondlg.h"
#include "parsedclass.h"
#include "classparser.h"
#include "parsedattribute.h"
#include "parsedmethod.h"
//#include "cclassview.h"
#include <klocale.h>
#include <kmessagebox.h>
//#include <kapp.h>

#include "classstore.h"

#include <qmessagebox.h>
#include <qregexp.h>

static const QString templates(i18n("Templates"));

enum templatetype {
  c_copy,            /* copy constructor */
  c_unaer_member,    /* class operator@ () */
  c_member,          /* class operator@ (class) */
  c_bin_compare,   /* friend bool operator@ (class, class) */
  c_bin_nonmember_assign,   
          /* friend class operator@ (class, class) and class operator@ (class)
             friend class operator@= (class, class) and class operator@= (class)*/
  inp,               /* oper >> */
  outp               /* oper << */
};

static struct {
  QString name;
  templatetype typ;
} templatesdata[] = {
 { "", c_copy },

 { "!",  c_unaer_member },
 { "~",  c_unaer_member },
 { "++", c_unaer_member },
 { "--", c_unaer_member },

 { "=",  c_member },
 { "()", c_member },
 { "[]", c_member },
 { "->", c_member },

 { "+", c_bin_nonmember_assign },
 { "-", c_bin_nonmember_assign },
 { "/", c_bin_nonmember_assign },
 { "*", c_bin_nonmember_assign },
 { "%", c_bin_nonmember_assign },
 { "|", c_bin_nonmember_assign },
 { "&", c_bin_nonmember_assign },
 { "^", c_bin_nonmember_assign },

 { "<",  c_bin_compare },
 { ">",  c_bin_compare },
 { "<=", c_bin_compare },
 { ">=", c_bin_compare },
 { "==", c_bin_compare },
 { "!=", c_bin_compare },

 { "<<", outp },
 { ">>", inp }
};

static const int templatescount = sizeof templatesdata/(sizeof templatesdata[0]);

CCloneFunctionDlg::CCloneFunctionDlg( ClassStore *store, ClassStore *libstore, const QString& currentClass,
                                      QWidget *parent, const char *name )
  : QDialog(parent,name,true),
    classname(currentClass),
    classtree(store), libclasstree(libstore)
{
	// set up dialog
  setCaption( i18n("Select Function to Copy") );

    resize( 400, 110 );
    Form1Layout = new QGridLayout( this );
    Form1Layout->setSpacing( 6 );
    Form1Layout->setMargin( 11 );

    LayoutAll = new QVBoxLayout;
    LayoutAll->setSpacing( 6 );
    LayoutAll->setMargin( 0 );

    LayoutButton = new QHBoxLayout;
    LayoutButton->setSpacing( 6 );
    LayoutButton->setMargin( 0 );

  // combo boxes
  allclasses = new QComboBox( FALSE, this, "classes" );
//  allclasses->setAutoResize( TRUE );
  methods = new QComboBox( FALSE, this, "methods" );
//  methods->setAutoResize(TRUE);

  // Buttons
  okBtn = new QPushButton( i18n("&OK"), this );
  okBtn->setDefault( TRUE );

  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, 
					 QSizePolicy::Minimum );

  cancelBtn = new QPushButton( i18n("&Cancel"), this );

  // Layout
  LayoutButton->addWidget( okBtn );
  LayoutButton->addItem( spacer );
  LayoutButton->addWidget( cancelBtn );

  LayoutAll->addWidget( allclasses );
  LayoutAll->addWidget( methods );
  LayoutAll->addLayout( LayoutButton );

  Form1Layout->addLayout( LayoutAll, 0, 0 );
   // Ok and cancel buttons.
  connect( okBtn, SIGNAL( clicked() ), SLOT( OK() ) );
  connect( cancelBtn, SIGNAL( clicked() ), SLOT( reject() ) );

  initClasses();
}

/** fill the comboboxes with all classes */
void CCloneFunctionDlg::initClasses()
{
    // populate the comboboxes
    
    // first: templates
    allclasses->insertItem(templates);
	
    // create list of all classes
    ParsedClass *curr = classtree->getClassByName(classname);
    QString parentName;
    if ( curr && curr->parents.first() )
        parentName = curr->parents.first()->name();
    
    QValueList<ParsedClass*> all = classtree->getSortedClassList();
    QValueList<ParsedClass*>::ConstIterator it;
    for (it = all.begin(); it != all.end(); ++it) {
        allclasses->insertItem((*it)->name());
        if ((*it)->name() == parentName)
            allclasses->setCurrentItem(allclasses->count()-1);
    }

    all = libclasstree->getSortedClassList();
    for (it = all.begin(); it != all.end(); ++it) {
        allclasses->insertItem((*it)->name());
        if ((*it)->name() == parentName)
            allclasses->setCurrentItem(allclasses->count()-1);
    }
    
    slotNewClass( allclasses->currentText () );
    
    // change methods on class selection
    connect(allclasses, SIGNAL(highlighted(const QString&)),
            SLOT(slotNewClass(const QString&)) );
    
    // Set the default focus.
    allclasses->setFocus();
}


void CCloneFunctionDlg::OK()
{
    if( methods->currentText().isEmpty() )
        KMessageBox::information( this,
                                  i18n("You have to select a method."),
                                  i18n("No Method") );
    else
        accept();
}

/** update methods/slots */
void CCloneFunctionDlg::slotNewClass(const QString& name)
{
    methods->clear();

    if (name == templates) {	
        QString noarg (" ( )");
        QString oparg1( " (const "+classname+"& rhs)" );
        QString oparg2( " (const "+classname+"& lhs, const " + classname + "& rhs)" );

        // set/get attributes
	ParsedClass *theClass = classtree->getClassByName( classname );
	if (theClass) {
	  QValueList<ParsedAttribute*> attrList = theClass->getSortedAttributeList();
	  QValueList<ParsedAttribute*>::ConstIterator it;
	  for (it = attrList.begin(); it != attrList.end(); ++it) {
            QString name = (*it)->name();
            QString type = (*it)->type();
            type.replace( QRegExp("[&\\*]"), "" );
            type = type.stripWhiteSpace();
            methods->insertItem(type + "& get" + name + "()");
            methods->insertItem("void set" + name + "(const " + type + "& newval)" );
	  }
	}
        
        // operators
        for(int i=0; i < templatescount; i++) {
            QString op = templatesdata[i].name;
            switch (templatesdata[i].typ) {
            case c_copy:  /* copy constructor */
                methods->insertItem(" "+classname + oparg1);
                break;
            case c_unaer_member:    /* class operator@ () */
                methods->insertItem(classname + "& operator " + op + noarg);
                break;
                
            case c_member:  /* class operator@ (class) */
                methods->insertItem(classname + "& operator " + op + oparg1);
                break;
                
            case c_bin_nonmember_assign:   /* friend class operator@ (class, class)
                                              and class operator@ (class) */
                methods->insertItem(classname + " operator " + op + "=" + oparg1);
                //methods->insertItem("friend " + classname + "& operator " + op + "=" + oparg1 );
                methods->insertItem(classname + " operator " + op + oparg1);
                methods->insertItem("friend " + classname + " operator " + op + oparg2 );
                break;
                
            case c_bin_compare:
                methods->insertItem("bool operator " + op + oparg1);
                methods->insertItem("friend bool operator " + op + oparg2 );
                break;
                
            case inp:         /* oper >> */
 	        methods->insertItem(QString("friend istream& operator >> (istream& is, "
					    + classname + "& val)"));
                break;
                
            case outp:         /* oper << */
                methods->insertItem(QString("friend ostream& operator << (ostream& os, const "
					    + classname + "& val)"));
                break;
            }
        }
    } else {
        ParsedClass *theClass = classtree->getClassByName( name );
	if (theClass == 0)
	  theClass = libclasstree->getClassByName( name );
	if (theClass == 0)
	  return;
    
        // all Methods
        QValueList<ParsedMethod*> all;
        QValueList<ParsedMethod*>::ConstIterator it;

        all = theClass->getSortedMethodList();
        for (it = all.begin(); it != all.end(); ++it)
            if (! ((*it)->isDestructor() || (*it)->isConstructor()))
                methods->insertItem((*it)->asString());
        
        // all slots
        all = theClass->getSortedSlotList();
        for (it = all.begin(); it != all.end(); ++it)
            methods->insertItem((*it)->asString());
        
        // all signals
        all = theClass->getSortedSignalList();
        for (it = all.begin(); it != all.end(); ++it)
            methods->insertItem((*it)->asString());
    }
}
    
/** get the selected method */
bool CCloneFunctionDlg::getMethod(QString& type, QString& name, QString& decl, QString& comment,
                                  bool& ispriv, bool& isprot, bool& ispub,
                                  bool& isvirt, bool& isSlot, bool& isSignal, bool& isconst) {

  QString str;
  const QString curr = allclasses->currentText();
  const QString selected = methods->currentText();
  if(curr == templates) {
    static ParsedMethod result;
    
    int blank    = selected.find(' ') + 1;
    if (selected.contains(QRegExp("^friend")))
      blank    = selected.find(' ', blank) + 1;
    int argend   = selected.length();
    int argbegin = selected.findRev('(', argend);
    type = selected.left(blank);
    decl = selected.mid(blank, argend-blank+1);
    name = selected.mid(blank, argbegin-blank+1);
    comment = selected.mid(blank, argbegin-blank);
    comment.replace( QRegExp("^set"), "set ");
    comment.replace( QRegExp("^get"), "get ");
    if (selected.contains(QRegExp("^friend")))
      comment = "friend of class " + classname + "\n" + comment;
    ispub = true;
    ispriv = false;
    isprot = false;
    isvirt = false;
    isSlot = false;
    isSignal = false;
    isconst = false;
    return true;
  }

  ParsedClass *theClass = classtree->getClassByName( curr );
  if (theClass == 0)
    theClass = libclasstree->getClassByName( curr );
  if (theClass == 0)
    return false;
  ParsedMethod* res = searchMethod(theClass, selected);
  if (res) {
    type = res->type();
    name = res->name();
    decl = res->asString();
    comment = res->comment();
    ispub  = res->isPublic();
    ispriv = res->isPrivate();
    isprot = res->isProtected();
    isvirt = res->isVirtual();
    isSlot = res->isSlot();
    isSignal = res->isSignal();
    isconst = res->isConst();
    
    return true;
  } else
    return false;
}

ParsedMethod* CCloneFunctionDlg::searchMethod(ParsedClass *theClass, QString selected)
{
    QValueList<ParsedMethod*> all;
    QValueList<ParsedMethod*>::ConstIterator it;
    
    // check methods
    all = theClass->getSortedMethodList();
    for (it = all.begin(); it != all.end(); ++it)
        if (selected == (*it)->asString())
            return *it;
	
    // not found - try all slot
    all = theClass->getSortedSlotList();
    for (it = all.begin(); it != all.end(); ++it)
        if (selected == (*it)->asString())
            return *it;
	
    // not found - try all slot
    all = theClass->getSortedSignalList();
    for (it = all.begin(); it != all.end(); ++it)
        if (selected == (*it)->asString())
            return *it;
    
    // oops
    return 0;
}

#include "cclonefunctiondlg.moc"
