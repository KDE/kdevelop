/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsmainview.h"

#include <KLocale>

#include "cvs_part.h"
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
}

CvsMainView::~CvsMainView()
{
    kDebug() <<  k_funcinfo <<endl;
    delete m_mainview;
}

void CvsMainView::slotAddTab(QWidget * tab, QString label)
{
    kDebug() <<  k_funcinfo << label << endl;
    tabwidget->addTab( tab, label );
}

void CvsMainView::slotJobFinished(KJob * job)
{
    kDebug() <<  k_funcinfo <<endl;
    m_mainview->slotJobFinished(job);
}

#include "cvsmainview.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
