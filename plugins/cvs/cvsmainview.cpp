/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsmainview.h"

#include <KLocale>

#include "cvspart.h"
#include "cvsjob.h"
#include "cvsgenericoutputview.h"

CvsMainView::CvsMainView( CvsPart *part, QWidget* parent )
 : QWidget( parent ),
   Ui::CvsMainViewBase(),
   m_part(part)
{
    Ui::CvsMainViewBase::setupUi(this);

    // CvsPart will notify when a job finished
    connect(m_part, SIGNAL(jobFinished(KJob*)),
            this, SLOT(slotJobFinished(KJob*)));

    // allow appending of new views
    connect(m_part, SIGNAL(addNewTabToMainView(QWidget*, QString)),
            this, SLOT(slotAddTab(QWidget*, QString)) );

    // create a default output view
    m_mainview = new CvsGenericOutputView(m_part);
    tabwidget->addTab( m_mainview, i18n("CVS") );

    // add a close button as corner widget
    m_closeButton = new QToolButton(tabwidget);
    m_closeButton->setIcon( KIcon( "tab-remove" ) );
    m_closeButton->adjustSize();
    m_closeButton->setAutoRaise(true);
    m_closeButton->setEnabled(false);
    tabwidget->setCornerWidget( m_closeButton );
    connect(m_closeButton, SIGNAL( clicked() ),
            this, SLOT( slotTabClose() ));
}

CvsMainView::~CvsMainView()
{
    kDebug(9500) <<  k_funcinfo ;
    delete m_mainview;
}

void CvsMainView::slotAddTab(QWidget * tab, const QString& label)
{
    kDebug(9500) <<  k_funcinfo << label;

    int idx = tabwidget->addTab( tab, label );
    tabwidget->setCurrentIndex(idx);

    if (tabwidget->count() > 1)
        m_closeButton->setEnabled(true);
}

void CvsMainView::slotJobFinished(KJob * job)
{
    kDebug(9500) <<  k_funcinfo ;
    m_mainview->slotJobFinished(job);
    tabwidget->setCurrentIndex(0);
}

void CvsMainView::slotTabClose()
{
    int idx = tabwidget->currentIndex();

    // don't allow to close the first tab
    if (idx != 0)
        tabwidget->removeTab( idx );

    // if only the first tab remains, disable the close button
    if (tabwidget->count() <= 1)
        m_closeButton->setEnabled(false);
}

#include "cvsmainview.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
