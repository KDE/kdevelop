/***************************************************************************
                          cclonefunctiondlg.cpp  -  description
                             -------------------
    begin                : Sat Nov 18 2000
    copyright            : (C) 2000 by The KDevelop Team
    email                :
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
#include "cclassview.h"
#include "api.h"
#include <klocale.h>
#include <kmessagebox.h>
//#include <kapp.h>

#include <qmessagebox.h>
#include <qregexp.h>

static const QString templates("Templates");

enum templatetype {
  c_copy,            /* copy constructor */
  c_unaer_member,    /* class operator@ () */
  c_member,          /* class operator@ (class) */
  c_bin_compare,   /* friend bool operator@ (class, class) */
  c_bin_nonmember_assign,   /* friend class operator@ (class, class) and class operator@ (class)
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


CCloneFunctionDlg::CCloneFunctionDlg(CClassView* class_tree, QWidget *parent, const char *name )
  : QDialog(parent,name,true),
    classname(class_tree->getCurrentClass()->name()),
    classtree(class_tree)
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
  okBtn = new QPushButton( this, i18n("OK"));
  okBtn->setDefault( TRUE );
  okBtn->setText( i18n( "OK" ) );

  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );

  cancelBtn = new QPushButton( this, i18n("Cancel"));
  cancelBtn->setText( i18n("Cancel") );

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

  // populate the comboboxes
  // look up parent
  QString parentname;
  ParsedClass* curr = classtree->getCurrentClass();
	if ( curr && curr->parents.first() )
		parentname = curr->parents.first()->name();

	// first: templates
	allclasses->insertItem(templates);

	// create list of all classes
        QValueList<ParsedClass*> all = API::getInstance()->classStore()->getSortedClassList();
        QValueList<ParsedClass*>::ConstIterator allit;
        for (allit = all.begin(); allit != all.end(); ++allit) {
            ParsedClass *i = *allit;
		allclasses->insertItem(i->name());
	  if (i->name() == parentname)
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
 if( strlen( methods->currentText() ) == 0 )
    KMessageBox::information( this,
                            i18n("You have to select a method."),
                            i18n("No method") );
 else
    accept();
}

/** update methods/slots */
void CCloneFunctionDlg::slotNewClass(const QString& name)
{
	QString str;

	methods->clear();

	if(name == templates) {
	  QString noarg (" ( )");
	  QString oparg1( " (const "+classname+"& )" );
	  QString oparg2( " (const "+classname+"& , const " + classname + "& )" );
	  // set/get attributes
	  {
      ParsedClass *theClass = API::getInstance()->classStore()->getClassByName( classname );
    	QValueList<ParsedAttribute*> list = theClass->getSortedAttributeList();

      ParsedAttribute* attr;
      QValueList<ParsedAttribute*>::ConstIterator listit;
      for (listit = list.begin(); listit != list.end(); ++listit) {
         attr = *listit;
         QString name = attr->name();
         QString type = attr->type();
         type.replace( QRegExp("[&\\*]"), "" );
         type = type.stripWhiteSpace();
         methods->insertItem(type + "& get" + name + "()");
         methods->insertItem("void set" + name + "(const " + type + "& )" );
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
 	        methods->insertItem(QString("friend istream& operator >> (istream& , "+ classname + "& )"));
          break;

        case outp:         /* oper << */
       	  methods->insertItem(QString("friend ostream& operator << (ostream& , const"+ classname + "& )"));
          break;
      }
    }
	  return;
	}

	// all Methods
  ParsedClass *theClass = API::getInstance()->classStore()->getClassByName( name );
	QList<ParsedMethod> *implList = new QList<ParsedMethod>;
	QList<ParsedMethod> *availList = new QList<ParsedMethod>;
  QValueList<ParsedMethod*> all = theClass->getSortedMethodList();
  QValueList<ParsedMethod*>::ConstIterator allit;
  for (allit = all.begin(); allit != all.end(); ++allit) {
      ParsedMethod *i = *allit;
  	if (! (i->isDestructor() || i->isConstructor()))
            methods->insertItem(i->asString());
  }

	// all slots
  all = theClass->getSortedSlotList();
  for (allit = all.begin(); allit != all.end(); ++allit) {
      ParsedMethod *i = *allit;
      methods->insertItem(i->asString());
  }

  // all signals
  all = theClass->getSortedSignalList();
  for (allit = all.begin(); allit != all.end(); ++allit) {
      ParsedMethod *i = *allit;
      methods->insertItem(i->asString());
  }
}
/** get the selected method */
bool CCloneFunctionDlg::getMethod(QString& type, QString& decl, QString& comment,
                                  bool& ispriv, bool& isprot, bool& ispub){
 	QString str;
 	const QString name = allclasses->currentText();
 	const QString selected = methods->currentText();
 	if(name == templates) {
  	static ParsedMethod result;

  	int blank    = selected.find(' ') + 1;
  	if (selected.contains(QRegExp("^friend")))
      	blank    = selected.find(' ', blank) + 1;
  	int argend   = selected.length();
  	int argbegin = selected.findRev('(', argend);
  	type = selected.left(blank);
  	decl = selected.mid(blank, argend-blank+1);
  	comment = selected.mid(blank, argbegin-blank);
  	comment.replace( QRegExp("^set"), "set ");
  	comment.replace( QRegExp("^get"), "get ");
  	if (selected.contains(QRegExp("^friend")))
       comment = "friend of class " + classname + "\n" + comment;
   	ispub = true;
  	ispriv = false;
  	isprot = false;
  	return true;
  }

  ParsedClass *theClass = API::getInstance()->classStore()->getClassByName( name );
  ParsedMethod* res = searchMethod(theClass, selected);
  if (res) {
  	type = res->type();
  	decl = res->asString();
  	comment = res->comment();
  	ispub  =  res->isPublic();
  	ispriv =  res->isPrivate();
  	isprot =  res->isProtected();

		return true;
  } else
  	return false;
}

ParsedMethod* CCloneFunctionDlg::searchMethod(ParsedClass *theClass, QString selected)
{
 	QString str;
  QValueList<ParsedMethod*> all = theClass->getSortedMethodList();
  // check methods
  QValueList<ParsedMethod*>::ConstIterator allit;
  for (allit = all.begin(); allit != all.end(); ++allit) {
      ParsedMethod *i = *allit;
      if ( selected == i->asString()) {
	        return i;
	    }
  }

	// not found - try all slot
  all = theClass->getSortedSlotList();
  for (allit = all.begin(); allit != all.end(); ++allit) {
      ParsedMethod *i = *allit;
     if ( selected == i->asString()) {
	        return i;
	    }
  }
	// not found - try all slot
  all = theClass->getSortedSignalList();
  for (allit = all.begin(); allit != all.end(); ++allit) {
      ParsedMethod *i = *allit;
     if ( selected == i->asString()) {
	        return i;
	    }
  }

	// oops
	return NULL;
}
#include "cclonefunctiondlg.moc"
