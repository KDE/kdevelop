/***************************************************************************
                             grepdlg.cpp
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


#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qwhatsthis.h>
#include <qstringlist.h>
#include <kbuttonbox.h>
#include <kfiledialog.h>
#include <kapp.h>
#include <kiconloader.h>
#include <klocale.h>
#include "grepdlg.h"
#include "misc.h"


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

const char *filepatterns[] = {
    "*.h,*.hxx,*.cpp,*.cc,*.C,*.cxx,*.idl,*.c",
    "*.cpp,*.cc,*.C,*.cxx,*.c",
    "*.h,*.hxx,*.idl",
    "*"
};


GrepDialog::GrepDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, false)
{
    setCaption(i18n("Search in Files"));
    
    QGridLayout *layout = new QGridLayout(this, 6, 2, 10, 4);
    layout->addRowSpacing(4, 10);
    layout->setRowStretch(4, 0);
    layout->setColStretch(0, 0);
    layout->setColStretch(1, 20);

    QLabel *pattern_label = new QLabel(i18n("&Pattern:"), this);
    layout->addWidget(pattern_label, 0, 0, AlignRight | AlignVCenter);

    pattern_edit = new QLineEdit(this);
    pattern_label->setBuddy(pattern_edit);
    pattern_edit->setFocus();
    layout->addWidget(pattern_edit, 0, 1);
    
    QLabel *template_label = new QLabel(i18n("&Template:"), this);
    layout->addWidget(template_label, 1, 0, AlignRight | AlignVCenter);

    QBoxLayout *template_layout = new QHBoxLayout(4);
    layout->addLayout(template_layout, 1, 1);
    
    template_edit = new QLineEdit(this);
    template_label->setBuddy(template_edit);
    template_edit->setText(template_str[0]);
    template_layout->addWidget(template_edit, 1);

    QComboBox *template_combo = new QComboBox(false, this);
    template_combo->insertStrList(template_desc);
    template_layout->addWidget(template_combo, 0);

    QLabel *files_label = new QLabel(i18n("&Files:"), this);
    layout->addWidget(files_label, 2, 0, AlignRight | AlignVCenter);

    files_combo = new QComboBox(true, this);
    files_label->setBuddy(files_combo->focusProxy());
    files_combo->insertStrList(filepatterns);
    layout->addWidget(files_combo, 2, 1);

    QLabel *dir_label = new QLabel(i18n("Di&rectory:"), this);
    layout->addWidget(dir_label, 3, 0, AlignRight | AlignVCenter);

    QBoxLayout *dir_layout = new QHBoxLayout(4);
    layout->addLayout(dir_layout, 3, 1);
    
    dir_edit = new QLineEdit(this);
    dir_label->setBuddy(dir_edit);
    dir_edit->setMinimumWidth(dir_edit->fontMetrics().maxWidth()*25);
    dir_layout->addWidget(dir_edit, 10);

    QPushButton *dir_button = new QPushButton(this, "dirButton");
    dir_button->setPixmap(BarIcon("open"));
    dir_layout->addWidget(dir_button);
    
    recursive_box = new QCheckBox(i18n("&Recursive"), this);
    recursive_box->setChecked(true);
    dir_layout->addSpacing(10);
    dir_layout->addWidget(recursive_box);

    QBoxLayout *button_layout = new QHBoxLayout(4);
    layout->addLayout(button_layout, 5, 1);
    QPushButton *search_button = new QPushButton(i18n("&Search"), this);
    QPushButton *done_button = new QPushButton(i18n("&Done"), this);
    button_layout->addStretch();
    button_layout->addWidget(search_button);
    button_layout->addWidget(done_button);

    resize(sizeHint());

    QWhatsThis::add(pattern_edit,
		    i18n("Enter the regular expression you want to search for here.\n"
			 "Possible meta characters are:\n"
			 "<b>.</b> - Matches any character\n"
			 "<b>^</b> - Matches the beginning of a line\n"
			 "<b>$</b> - Matches the end of a line\n"
	                 "<b>\\\\\\<</b> - Matches the beginning of a word\n"
                         "<b>\\\\\\></b> - Matches the end of a word\n"
			 "\n"
			 "The following repetition operators exist:\n"
			 "<b>?</b> - The preceding item is matches at most once\n"
			 "<b>*</b> - The preceding item is matched zero or more times\n"
			 "<b>+</b> - The preceding item is matched once or more times\n"
			 "<b>{<i>n</i>}</b> - The preceding item is matched exactly <i>n</i> times\n"
			 "<b>{<i>n</i>,}</b> - The preceding item is matched <i>n</i> or more times\n"
			 "<b>{,<i>n</i>}</b> - The preceding item is matched at most <i>n</i> times\n"
			 "<b>{<i>n</i>,<i>m</i>}</b> - The preceding item is matched at least <i>n</i>,\n"
			 "   but at most <i>m</i> times.\n"
			 "\n"
			 "Furthermore, backreferences to bracketed subexpressions are\n"
			 "available via the notation \\\\<i>n</i>."
			 ));
    QWhatsThis::add(files_combo,
		    i18n("Enter the file name pattern of the files to search here.\n"
			 "You may give several patterns separated by commas"));
    QWhatsThis::add(template_edit,
		    i18n("You can choose a template for the pattern from the combo box\n"
			 "and edit it here. The string %s in the template is replaced\n"
			 "by the pattern input field, resulting in the regular expression\n"
			 "to search for."));

    connect( template_combo, SIGNAL(activated(int)),
	     SLOT(templateActivated(int)) );
    connect( dir_button, SIGNAL(clicked()),
	     SLOT(dirButtonClicked()) );
    connect( search_button, SIGNAL(clicked()),
	     SLOT(slotSearchClicked()) );
    connect( done_button, SIGNAL(clicked()),
	     SLOT(hide()) );
}


GrepDialog::~GrepDialog()
{}


void GrepDialog::dirButtonClicked()
{
    dir_edit->setText(KFileDialog::getExistingDirectory(dir_edit->text()));
}


void GrepDialog::templateActivated(int index)
{
    template_edit->setText(template_str[index]);
}


void GrepDialog::slotSearchClicked()
{
    emit searchClicked();
    hide();
}
