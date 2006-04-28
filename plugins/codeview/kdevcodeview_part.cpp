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

#include "kdevcodeview_part.h"
#include "kdevcodeview.h"

#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <kaction.h>
#include <klocale.h>
#include <ktrader.h>
#include <kmimetype.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>

#include <kdevplugininfo.h>
#include <kdevmainwindow.h>
#include <kdevgenericfactory.h>
#include <kdevlanguagesupport.h>

typedef KDevGenericFactory<KDevCodeViewPart> KDevCodeViewFactory;
static const KDevPluginInfo data( "kdevcodeview" );
K_EXPORT_COMPONENT_FACTORY( kdevcodeview,
                            KDevCodeViewFactory( data ) );

KDevCodeViewPart::KDevCodeViewPart( QObject *parent,
                                    const char *name,
                                    const QStringList& )
        : KDevPlugin( &data, parent )
{
    setObjectName( QString::fromUtf8( name ) );

    setInstance( KDevCodeViewFactory::instance() );

    if (!languageSupport())
        return;
    KDevCodeProxy * model = languageSupport() ->codeProxy();
    if ( !model )
        return ;

    QWidget *window = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout( window );
    vbox->setMargin( 0 );
    vbox->setSpacing( 0 );

    QFrame *toolBar = new QFrame( window );
    toolBar->setFrameShape( QFrame::StyledPanel );
    toolBar->setFrameShadow( QFrame::Raised );

    m_codeView = new KDevCodeView( this, window );
    m_codeView->setWindowIcon( KIcon( "view_tree" ) );
    m_codeView->setWindowTitle( i18n( "Code View" ) );

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

    connect( currentdoc, SIGNAL( triggered() ), m_codeView, SLOT( modeCurrent() ) );
    connect( normalize, SIGNAL( triggered() ), m_codeView, SLOT( modeNormalize() ) );
    connect( aggregate, SIGNAL( triggered() ), m_codeView, SLOT( modeAggregate() ) );

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
        connect( action, SIGNAL( triggered() ), m_codeView, SLOT( filterKind() ) );
    }
    filter->setMenu( filterMenu );

    hbox->addWidget( mode );
    hbox->addWidget( filter );
    hbox->addStretch( 1 );
    toolBar->setLayout( hbox );
    vbox->addWidget( toolBar );
    vbox->addWidget( m_codeView );
    window->setLayout( vbox );

    m_codeView->setModel( model );

    if ( KDevCodeDelegate * delegate = languageSupport() ->codeDelegate() )
        m_codeView->setItemDelegate( delegate );

    m_codeView->setWhatsThis( i18n( "Code View" ) );

    mainWindow() ->embedSelectViewRight( window,
                                         i18n( "Code View" ),
                                         i18n( "Code View" ) );

    setXMLFile( "kdevcodeview.rc" );
}

KDevCodeViewPart::~KDevCodeViewPart()
{
    if ( m_codeView )
    {
        mainWindow() ->removeView( m_codeView );
        delete m_codeView;
    }
}

void KDevCodeViewPart::import( RefreshPolicy /*policy*/ )
{}


#include "kdevcodeview_part.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
