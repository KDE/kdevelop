/***************************************************************************
 *   Copyright (C) 1999 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "adddocitemdlg.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qdom.h>
#include <kbuttonbox.h>
#include <kfiledialog.h>
#include <klocale.h>


AddDocItemDialog::AddDocItemDialog(KFile::Mode mode, QString filter, bool checkQtDocTitle, QString title, QString url, QWidget *parent, const char *name)
    : QDialog(parent, name, true), m_mode(mode), m_filter(filter)
{
    setCaption(i18n("Add Documentation Entry"));

    title_check = 0;
    if (checkQtDocTitle)
        title_check = new QCheckBox(i18n("Custom title"), this);
    
    QLabel *title_label = new QLabel(i18n("&Title:"), this);
    title_edit = new QLineEdit(this);
    title_edit->setText(title);
    title_edit->setFocus();
    title_label->setBuddy(title_edit);

    QLabel *url_label = new QLabel(i18n("&Location:"), this);
    url_edit = new KURLRequester(this);
    url_label->setBuddy(url_edit);
    QFontMetrics fm(url_edit->fontMetrics());
    url_edit->setURL(url);
    url_edit->setMinimumWidth(fm.width('X')*35);
    url_edit->setFilter(m_filter);
    url_edit->setMode((int) m_mode);

/*    QPushButton *url_button = new QPushButton("...", this);
    url_button->setFixedSize(30, 25);

    connect( url_button, SIGNAL(clicked()), this, SLOT(fileButtonClicked()));
*/
    QString s = i18n("Enter the name of the entry here.");
    QWhatsThis::add(title_label, s);
    QWhatsThis::add(title_edit, s);
    s = i18n("Enter the location of the entry here.");
    QWhatsThis::add(url_label, s);
    QWhatsThis::add(url_edit, s);
/*    s = i18n("Here you can browse through your file system to select a location for the entry.");
    QWhatsThis::add(url_button, s);*/

    QVBoxLayout *layout = new QVBoxLayout(this, 10);

    QGridLayout *grid = new QGridLayout(2, 3);
    if (checkQtDocTitle)
    {
        layout->addWidget(title_check);
    }
    layout->addLayout(grid);
    grid->addWidget(title_label, 0, 0);
    grid->addMultiCellWidget(title_edit, 0, 0, 1, 2);
    grid->addWidget(url_label, 1, 0);
    grid->addWidget(url_edit, 1, 1);
//    grid->addWidget(url_button, 1, 2);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok = buttonbox->addButton(i18n("&OK"));
    QPushButton *cancel = buttonbox->addButton(i18n("Cancel"));
    ok->setDefault(true);
    connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    if (checkQtDocTitle)
    {
        title_edit->setEnabled(false);
        connect(title_check, SIGNAL(toggled(bool)), title_edit, SLOT(setEnabled(bool)));
        connect(url_edit, SIGNAL(textChanged(const QString&)), this, SLOT(setTitle(const QString&)));
    }
}


AddDocItemDialog::~AddDocItemDialog()
{}

void AddDocItemDialog::setTitle(const QString &str)
{
    if (title_check == 0)
        return;
    else if (title_check->isChecked())
        return;

    title_edit->setText("");
    QFileInfo fi(str);
    if (!fi.exists())
        return;
    
    QFile f(str);
    if (!f.open(IO_ReadOnly)) {
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "DCF") {
        return;
    }
    f.close();

    QDomElement docEl = doc.documentElement();
    
    title_edit->setText(docEl.attribute("title", QString::null));
    
}

#include "adddocitemdlg.moc"
