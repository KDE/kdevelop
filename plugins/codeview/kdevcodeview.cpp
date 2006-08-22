/*
* This file is part of KDevelop
*
* Copyright (c) 2006 Adam Treat <treat@kde.org>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kdevcodeview.h"

#include "kdevcodetree.h"

#include <QMenu>
#include <QLabel>
#include <QAction>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <kicon.h>
#include <kmenu.h>
#include <kfile.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include <kdevcore.h>
#include <kdevcodeproxy.h>
#include <kdevlanguagesupport.h>
#include <kdevdocumentcontroller.h>

KDevCodeView::KDevCodeView( QWidget *parent )
        : QWidget( parent )
{
    setObjectName( i18n( "Code View" ) );

    setWindowIcon( SmallIcon( "kdevelop" ) ); //FIXME
    setWindowTitle( i18n( "Code View" ) );
    setWhatsThis( i18n( "Code View" ) );

    QVBoxLayout * vbox = new QVBoxLayout( this );
    vbox->setMargin( 0 );
    vbox->setSpacing( 0 );

    if ( !KDevCore::activeLanguage() )
    {
        QLabel * label = new QLabel( this );
        label->setMargin( 20 );
        label->setWordWrap( true );
        label->setFrameStyle( QFrame::Panel | QFrame::Sunken );
        label->setText( i18n( "No active language part loaded!" ) );
        label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
        vbox->addWidget( label );
        setLayout( vbox );
        return ;
    }

    KDevCodeProxy * model = KDevCore::activeLanguage() ->codeProxy();
    if ( !model )
    {
        QLabel * label = new QLabel( this );
        label->setMargin( 20 );
        label->setWordWrap( true );
        label->setFrameStyle( QFrame::Panel | QFrame::Sunken );
        label->setText( i18n( "Active language part does not offer a proper codemodel!" ) );
        label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
        vbox->addWidget( label );
        setLayout( vbox );
        return ;
    }

    QFrame *toolBar = new QFrame( this );
    toolBar->setFrameShape( QFrame::StyledPanel );
    toolBar->setFrameShadow( QFrame::Raised );

    KDevCodeTree *codeTree = new KDevCodeTree( this );
    codeTree->setWindowIcon( KIcon( "view_tree" ) );
    codeTree->setWindowTitle( i18n( "Code View" ) );

    QHBoxLayout *hbox = new QHBoxLayout( toolBar );
    hbox->setMargin( 2 );

    QToolButton *mode = new QToolButton( toolBar );
    mode->setText( i18n( "Mode" ) );
    mode->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    mode->setArrowType( Qt::DownArrow );
    mode->setPopupMode( QToolButton::InstantPopup );
    QMenu *modeMenu = new QMenu( i18n( "Mode" ) );
    QAction *currentdoc = modeMenu->addAction( i18n( "&Current" ) );
    QAction *normalize = modeMenu->addAction( i18n( "&Normalize" ) );
    QAction *aggregate = modeMenu->addAction( i18n( "&Aggregate" ) );
    mode->setMenu( modeMenu );

    connect( currentdoc, SIGNAL( triggered() ), codeTree, SLOT( modeCurrent() ) );
    connect( normalize, SIGNAL( triggered() ), codeTree, SLOT( modeNormalize() ) );
    connect( aggregate, SIGNAL( triggered() ), codeTree, SLOT( modeAggregate() ) );

    QToolButton *filter = new QToolButton( toolBar );
    filter->setText( i18n( "Filter" ) );
    filter->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    filter->setArrowType( Qt::DownArrow );
    filter->setPopupMode( QToolButton::InstantPopup );
    QMenu *filterMenu = new QMenu( i18n( "Filter" ) );

    QMap<QString, int> kindFilterList = model->kindFilterList();
    QMap<QString, int>::ConstIterator kind = kindFilterList.begin();
    for ( ; kind != kindFilterList.end(); ++kind )
    {
        QAction *action = filterMenu->addAction( kind.key() );
        action->setData( kind.value() );
        action->setCheckable( true );
        connect( action, SIGNAL( triggered() ), codeTree, SLOT( filterKind() ) );
    }
    filter->setMenu( filterMenu );

    hbox->addWidget( mode );
    hbox->addWidget( filter );
    hbox->addStretch( 1 );
    toolBar->setLayout( hbox );
    vbox->addWidget( toolBar );
    vbox->addWidget( codeTree );
    setLayout( vbox );

    codeTree->setModel( model );

    if ( KDevCodeDelegate * delegate = KDevCore::activeLanguage() ->codeDelegate() )
        codeTree->setItemDelegate( delegate );

    setWhatsThis( i18n( "Code View" ) );
}

KDevCodeView::~KDevCodeView()
{}

#include "kdevcodeview.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
