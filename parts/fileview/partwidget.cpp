/***************************************************************************
 *   Copyright (C) 2003 by KDevelop authors                                *
 *   kdevelop-devel@kde.org                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qwhatsthis.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qdom.h>
#include <kcombobox.h>
#include <kaction.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"

#include "fileviewpart.h"
#include "filetreewidget.h"
#include "partwidget.h"

///////////////////////////////////////////////////////////////////////////////
// class PartWidget
///////////////////////////////////////////////////////////////////////////////

PartWidget::PartWidget( FileViewPart *part, QWidget *parent )
    : QVBox( parent, "fileviewpartwidget" ), m_filetree( 0 ),
    m_filter( 0 ), m_btnFilter( 0 ), m_part( part )
{
    m_filetree = new FileTreeWidget( m_part, this );
    m_filetree->setCaption(i18n("File Tree"));
    m_filetree->setIcon(SmallIcon("folder"));
    QWhatsThis::add(m_filetree, i18n("File Tree\n\n"
                                    "The file viewer shows all files of the project "
                                    "in a tree layout."));

    QHBox* filterBox = new QHBox( this );
    m_btnFilter = new QToolButton( filterBox );
    m_btnFilter->setIconSet( SmallIconSet("filter" ) );
    m_btnFilter->setToggleButton( true );
    m_filter = new KHistoryCombo( true, filterBox, "filter");
    m_filter->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
    filterBox->setStretchFactor(m_filter, 2);

    connect( m_btnFilter, SIGNAL( clicked() ), this, SLOT( slotBtnFilterClick() ) );
    connect( m_filter, SIGNAL( activated(const QString&) ), this, SLOT( slotFilterChange(const QString&) ) );
    connect( m_filter, SIGNAL( returnPressed(const QString&) ),
             m_filter, SLOT( addToHistory(const QString&) ) );

    QDomDocument &dom = *m_part->projectDom();
    QString patterns = DomUtil::readEntry( dom, "/kdevfileview/tree/hidepatterns" );
    if (patterns.isEmpty())
        patterns = "*.o,*.lo,CVS";
    m_filter->insertItem( patterns );
    // m_hidePatterns = QStringList::split(",", patterns);
    slotFilterChange( patterns );
}

///////////////////////////////////////////////////////////////////////////////

PartWidget::~PartWidget()
{
    QDomDocument &dom = *m_part->projectDom();
    DomUtil::writeEntry( dom, "/kdevfileview/tree/hidepatterns", m_filetree->filters().join(",") );
}

///////////////////////////////////////////////////////////////////////////////

void PartWidget::slotFilterChange( const QString & nf )
{
    QString f = nf.stripWhiteSpace();
    bool empty = f.isEmpty() || f == "*";
    if ( empty )
    {
        m_filter->lineEdit()->setText( QString::null );
        QToolTip::add( m_btnFilter, QString( i18n("Apply last filter (\"%1\")") ).arg( m_lastFilter ) );
    }
    else
    {
        m_lastFilter = f;
        QToolTip::add( m_btnFilter, i18n("Clear filter") );
    }
    m_btnFilter->setOn( !empty );
    // this will be never true after the m_filter has been used;)
    m_btnFilter->setEnabled( !( empty && m_lastFilter.isEmpty() ) );

    m_filetree->applyFilters( QStringList::split( ",", f ) );
}

///////////////////////////////////////////////////////////////////////////////
/*
   When the button in the filter box toggles:
   If off:
   If the name filer is anything but "" or "*", reset it.
   If on:
   Set last filter.
*/
void PartWidget::slotBtnFilterClick()
{
    if ( !m_btnFilter->isOn() )
    {
        slotFilterChange( QString::null );
    }
    else
    {
        m_filter->lineEdit()->setText( m_lastFilter );
        slotFilterChange( m_lastFilter );
    }
}


#include "partwidget.moc"
