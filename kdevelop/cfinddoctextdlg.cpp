/***************************************************************************
                          cfinddoctextdlg.cpp  -  description                              
                             -------------------                                         
    begin                : Thu Feb 25 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    email                : Ralf.Nolden@post.rwth-aachen.de

    Adapted from finddlg.h  and finddlg.cpp of khelp
    Copyright (C) 1998 Martin Jones (mjones@kde.org)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "cfinddoctextdlg.h"

#include <kiconloader.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>

CFindDocTextDlg::CFindDocTextDlg(QWidget *parent, const char *name ) :
  QDialog(parent,name, true)
{
    resize(250,120);
    setCaption(i18n("Search for Help On"));
    setIcon(SmallIcon("filefind"));
    QVBoxLayout *vl = new QVBoxLayout( this, 15 );

    QHBoxLayout *hl = new QHBoxLayout( 15 );
    vl->addLayout( hl );

    QLabel *label = new QLabel( i18n( "Search for:" ), this );
    label->setFixedSize( label->sizeHint() );

    QLineEdit *edit = new QLineEdit( this );
    edit->setFixedHeight( edit->sizeHint().height() );
    edit->setFocus();
    connect( edit, SIGNAL( textChanged( const QString& ) ),
		    SLOT( slotTextChanged( const QString& ) ) );

    hl->addWidget( label );
    hl->addWidget( edit );

    hl = new QHBoxLayout;
    vl->addLayout( hl );

    hl->addStretch();

    hl = new QHBoxLayout( 15 );
    vl->addLayout( hl );

    QPushButton *btn = new QPushButton( i18n( "&Search" ), this );
    btn->setFixedSize( btn->sizeHint() );
    btn->setDefault( true );
    connect( btn, SIGNAL( clicked() ), this, SLOT( slotFind() ) );
    hl->addWidget( btn );

    hl->addStretch();

    btn = new QPushButton( i18n( "&Close" ), this );
    btn->setFixedSize( btn->sizeHint() );
    connect( btn, SIGNAL( clicked() ), this, SLOT( slotClose() ) );
    hl->addWidget( btn );

    vl->activate();
}
CFindDocTextDlg::~CFindDocTextDlg(){
}



void CFindDocTextDlg::slotTextChanged( const QString& t )
{
    text = t;
}

void CFindDocTextDlg::slotClose()
{
    close();
}

void CFindDocTextDlg::slotFind()
{
  emit signalFind( text );
}

//***************************************************************************

CManpageTextDlg::CManpageTextDlg(QWidget *parent, const char *name ) :
  QDialog(parent,name, true)
{
    resize(250,120);
    setCaption(i18n("Show Manpage On"));
    setIcon(SmallIcon("filefind"));
    QVBoxLayout *vl = new QVBoxLayout( this, 15 );

    QHBoxLayout *hl = new QHBoxLayout( 15 );
    vl->addLayout( hl );

    QLabel *label = new QLabel( i18n( "Manpage:" ), this );
    label->setFixedSize( label->sizeHint() );

    edit = new QLineEdit( this );
    edit->setFixedHeight( edit->sizeHint().height() );
    edit->setFocus();

    hl->addWidget( label );
    hl->addWidget( edit );

    hl = new QHBoxLayout;
    vl->addLayout( hl );

    hl->addStretch();

    hl = new QHBoxLayout( 15 );
    vl->addLayout( hl );

    QPushButton *btn = new QPushButton( i18n( "&OK" ), this );
    btn->setFixedSize( btn->sizeHint() );
    btn->setDefault( true );
    connect( btn, SIGNAL( clicked() ), this, SLOT( accept() ) );
    hl->addWidget( btn );

    hl->addStretch();

    btn = new QPushButton( i18n( "&Cancel" ), this );
    btn->setFixedSize( btn->sizeHint() );
    connect( btn, SIGNAL( clicked() ), this, SLOT( reject() ) );
    hl->addWidget( btn );

    vl->activate();
}

CManpageTextDlg::~CManpageTextDlg()
{
}

QString CManpageTextDlg::manpageText()
{
  QString manpage = edit->text();;
  if (!manpage.isEmpty())
    manpage="man:/"+manpage;    //+"(3)";
  return manpage;
}

#include "cfinddoctextdlg.moc"
