/***************************************************************************
                          grepdialog.cpp  -  grep frontend                              
                             -------------------                                         
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "grepdialog.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <qmessagebox.h>
#include <kquickhelp.h>
#include <kbuttonbox.h>
#include <kfiledialog.h>
#include <kapp.h>
#include <htmltoken.h>


const char *template_desc[] = {
    "normal",
    "assignment",
    "->MEMBER(",
    "class::MEMBER(",
    "OBJECT->member(",
    0
};

const char *template_str[] = {
    "%s",
    "\\<%s\\>[\t ]*=[^=]",
    "\\->[\\t ]*\\<%s\\>[\\t ]*(",
    "[a-z0-9_$]\\+[\\t ]*::[\\t ]*\\<%s\\>[\\t ]*(",
    "\\<%s\\>[\\t ]*\\->[\\t ]*[a-z0-9_$]\\+[\\t ]*(",
    0
};


GrepDialog::GrepDialog(QString dirname, QWidget *parent, const char *name)
    : QDialog(parent, name, false), childproc(0)
{
  setCaption(i18n("Search in Files..."));
    QGridLayout *layout = new QGridLayout(this, 4, 3, 10, 4);
    layout->addRowSpacing(1, 10);
    layout->addColSpacing(1, 10);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(1, 0);
    layout->setRowStretch(2, 10);
    layout->setColStretch(0, 5);
    layout->setColStretch(1, 0);
    layout->setColStretch(2, 1);
    
    QGridLayout *input_layout = new QGridLayout(4, 2, 10);
    layout->addLayout(input_layout, 0, 0);
    input_layout->setColStretch(0, 1);
    input_layout->setColStretch(1, 10);

    QLabel *pattern_label = new QLabel(i18n("Pattern:"), this);
    pattern_label->setAlignment(AlignRight | AlignVCenter);
    pattern_label->setMinimumSize(pattern_label->sizeHint());
    pattern_label->setFixedHeight(pattern_label->sizeHint().height());
    input_layout->addWidget(pattern_label, 0, 0);

    pattern_edit = new QLineEdit(this);
    pattern_edit->setFocus();
    pattern_edit->setMinimumSize(pattern_edit->sizeHint());
    input_layout->addWidget(pattern_edit, 0, 1);
    
    QLabel *template_label = new QLabel(i18n("Template:"), this);
    template_label->setAlignment(AlignRight | AlignVCenter);
    template_label->setMinimumSize(template_label->sizeHint());
    template_label->setFixedHeight(template_label->sizeHint().height());
    input_layout->addWidget(template_label, 1, 0);

    QBoxLayout *template_layout = new QHBoxLayout(4);
    input_layout->addLayout(template_layout, 1, 1);
    
    template_edit = new QLineEdit(this);
    template_edit->setText(template_str[0]);
    template_edit->setMinimumSize(template_edit->sizeHint());
    template_layout->addWidget(template_edit);

    QComboBox *template_combo = new QComboBox(false, this);
    template_combo->insertStrList(template_desc);
    template_combo->adjustSize();
    template_combo->setFixedSize(template_combo->size());
    template_layout->addWidget(template_combo);

    QLabel *files_label = new QLabel(i18n("Files:"), this);
    files_label->setAlignment(AlignRight | AlignVCenter);
    files_label->setMinimumSize(files_label->sizeHint());
    files_label->setFixedHeight(files_label->sizeHint().height());
    input_layout->addWidget(files_label, 2, 0);

    files_combo = new QComboBox(true,this);
    files_combo->setMinimumSize(files_combo->sizeHint());
    files_combo->insertItem("*.h,*.hxx,*.cpp,*.cc,*.C,*.cxx,*.idl,*.c");
    files_combo->insertItem("*.cpp,*.cc,*.C,*.cxx,*.c");
    files_combo->insertItem("*.h,*.hxx,*.idl");
    files_combo->insertItem("*");
    input_layout->addWidget(files_combo, 2, 1);

    QLabel *dir_label = new QLabel(i18n("Directory:"), this);
    dir_label->setAlignment(AlignRight | AlignVCenter);
    dir_label->setMinimumSize(files_label->sizeHint());
    dir_label->setFixedHeight(files_label->sizeHint().height());
    input_layout->addWidget(dir_label, 3, 0);

    QBoxLayout *dir_layout = new QHBoxLayout(4);
    input_layout->addLayout(dir_layout, 3, 1);
    
    dir_edit = new QLineEdit(this);
    dir_edit->setText(dirname);
    dir_edit->setMinimumSize(dir_edit->sizeHint());
    dir_layout->addWidget(dir_edit, 10);

    QPushButton *dir_button = new QPushButton("...", this);
    dir_button->setMinimumHeight(dir_button->sizeHint().height());
    dir_button->setFixedWidth(30);
    dir_layout->addWidget(dir_button);
    
    recursive_box = new QCheckBox(i18n("Recursive"), this);
    recursive_box->setMinimumSize(recursive_box->sizeHint());
    recursive_box->setChecked(true);
    dir_layout->addWidget(recursive_box);

    KButtonBox *actionbox = new KButtonBox(this, KButtonBox::VERTICAL);
    layout->addWidget(actionbox, 0, 2);
    actionbox->addStretch();
    search_button = actionbox->addButton(i18n("&Search"));
    search_button->setDefault(true);
    cancel_button = actionbox->addButton(i18n("Cancel"));
    cancel_button->setEnabled(false);
    actionbox->addStretch();
    actionbox->layout();

    resultbox = new QListBox(this);
    QFontMetrics fm(resultbox->fontMetrics());
    resultbox->setMinimumSize(fm.width("0")*40,
			      fm.lineSpacing()*15);
    layout->addMultiCellWidget(resultbox, 2, 2, 0, 2);

    KButtonBox *buttonbox = new KButtonBox(this);
    layout->addMultiCellWidget(buttonbox, 3, 3, 0, 2);
    QPushButton *clear_button = buttonbox->addButton(i18n("Clear"));
    buttonbox->addStretch();
    QPushButton *done_button = buttonbox->addButton(i18n("Done"));
    buttonbox->layout();

    layout->activate();

    KQuickHelp::add(pattern_edit,
		    i18n("Enter the regular expression you want to search for here.\n"
			 "Possible meta characters are:\n"
			 "<bold>.</bold> - Matches any character\n"
			 "<bold>^</bold> - Matches the beginning of a line\n"
			 "<bold>$</bold> - Matches the end of a line\n"
			 //			 "<bold>\\<</bold>  Matches the beginning of a word\n"
			 //			 "<bold>\\\></bold>  Matches the end of a word\n"
			 "\n"
			 "The following repetition operators exist:\n"
			 "<bold>?</bold> - The preceding item is matches at most once\n"
			 "<bold>*</bold> - The preceding item is matched zero or more times\n"
			 "<bold>+</bold> - The preceding item is matched once or more times\n"
			 "<bold>{<i>n</i>}</bold> - The preceding item is matched exactly <i>n</i> times\n"
			 "<bold>{<i>n</i>,}</bold> - The preceding item is matched <i>n</i> or more times\n"
			 "<bold>{,<i>n</i>}</bold> - The preceding item is matched at most <i>n</i> times\n"
			 "<bold>{<i>n</i>,<i>m</i>}</bold> - The preceding item is matched at least <i>n</i>,\n"
			 "   but at most <i>m</i> times"
			 // ... to be continued
			 ));
    KQuickHelp::add(files_combo,
		    i18n("Enter the file pattern of the files to search here.\n"));
    KQuickHelp::add(template_edit,
		    i18n("You can choose a template for the pattern from the combo box\n"
			 "and edit it here. The string %s in the template is replaced\n"
			 "by the pattern input field, resulting in the regular expression\n"
			 "to search for."));
    KQuickHelp::add(resultbox,
		    i18n("The results of the grep run are listed here. Select a\n"
			 "filename/line number combination and press Enter or doubleclick\n"
			 "on the item to show the respective line in the editor."));

    connect( template_combo, SIGNAL(activated(int)),
	     SLOT(templateActivated(int)) );
    connect( dir_button, SIGNAL(clicked()),
	     SLOT(dirButtonClicked()) );
    connect( resultbox, SIGNAL(selected(const char *)),
	     SLOT(itemSelected(const char *)) );
    connect( search_button, SIGNAL(clicked()),
	     SLOT(slotSearch()) );
    connect( cancel_button, SIGNAL(clicked()),
	     SLOT(slotCancel()) );
    connect( clear_button, SIGNAL(clicked()),
	     SLOT(slotClear()) );
    connect( done_button, SIGNAL(clicked()),
	     SLOT(accept()) );
}


GrepDialog::~GrepDialog()
{
    if (childproc)
      delete childproc;
}


void GrepDialog::done(int res)
{
    QDialog::done(res);
    //    delete this;
}


void GrepDialog::dirButtonClicked()
{
    dir_edit->setText(KDirDialog::getDirectory(dir_edit->text()));
}


void GrepDialog::templateActivated(int index)
{
    template_edit->setText(template_str[index]);
}


#include <iostream.h>
void GrepDialog::itemSelected(const char *item)
{
  int pos;
  QString filename, linenumber;
  
  QString str = item;
  if ( (pos = str.find(':')) != -1)
    {
      filename = str.left(pos);
      str = str.right(str.length()-1-pos);
      if ( (pos = str.find(':')) != -1)
	{
	  linenumber = str.left(pos);
	  emit itemSelected(filename,linenumber.toInt()-1);
	  //		    cout << "Selected file " << filename << ", line " << linenumber << endl;
	}
    }
}


void GrepDialog::processOutput()
{
  int pos;
  while ( (pos = buf.find('\n')) != -1)
    {
      QString item = buf.left(pos);
      if (!item.isEmpty())
	resultbox->insertItem(item);
      buf = buf.right(buf.length()-pos-1);
    }
}


// grep -e 'pattern' $(find 'directory' -name 'filepattern')


void GrepDialog::slotSearch()
{
  search_button->setEnabled(false);
  cancel_button->setEnabled(true);
  
  StringTokenizer tokener;
  QString files;
  QString files_temp = files_combo->currentText();
  if (files_temp.right(1) != ","){ 
    files_temp = files_temp + ",";
  }
  tokener.tokenize(files_temp,",");
  if(tokener.hasMoreTokens()){
    files = files + " '" + QString(tokener.nextToken())+"'" ;
  }
  while(tokener.hasMoreTokens()){
    files = files + " -o -name " + "'"+QString(tokener.nextToken())+ "'";
  }


    QString pattern = template_edit->text();
    pattern.replace(QRegExp("%s"), pattern_edit->text());
    pattern.replace(QRegExp("'"), "\\'");
    QString filepattern;
    if (recursive_box->isChecked())
	{
	    filepattern = QString("$(find '") + dir_edit->text() +
		"' -name"  + files + ")";
	}
    else
	{
	    filepattern = QString("$(find '") + dir_edit->text() +
		"' -maxdepth 1 -name "+ files + ")";
	}

    childproc = new KShellProcess();
    *childproc << "grep";
    *childproc << "-n";
    *childproc << (QString("-e '") + pattern + "'");
    *childproc << filepattern;
    *childproc << "/dev/null";
    *childproc << "< /dev/null";

    cerr << endl << filepattern;

    connect( childproc, SIGNAL(processExited(KProcess *)),
	     SLOT(childExited()) );
    connect( childproc, SIGNAL(receivedStdout(KProcess *, char *, int)),
	     SLOT(receivedOutput(KProcess *, char *, int)) );
    childproc->start(KProcess::NotifyOnExit, KProcess::Stdout);
}


void GrepDialog::slotCancel()
{
    search_button->setEnabled(true);
    cancel_button->setEnabled(false);

    buf += '\n';
    processOutput();
    if (childproc)
      delete childproc;
    childproc = 0;
}


void GrepDialog::childExited()
{
    if (childproc->exitStatus() == 1)
	QMessageBox::information(this, "Grep", i18n("No matches found"));
    slotCancel();
}


void GrepDialog::receivedOutput(KProcess *proc, char *buffer, int buflen)
{
    buf += QString(buffer, buflen+1);
    processOutput();
}


void GrepDialog::slotClear()
{
    //    slotCancel();
    resultbox->clear();
}


void  GrepDialog::setDirName(QString dir){
  dir_edit->setText(dir);
}
