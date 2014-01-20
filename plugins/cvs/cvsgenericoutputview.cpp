/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsgenericoutputview.h"

#include "cvsjob.h"
#include <KI18n/KLocalizedString>

CvsGenericOutputView::CvsGenericOutputView(CvsJob* job, QWidget* parent)
    : QWidget(parent), Ui::CvsGenericOutputViewBase()
{
    Ui::CvsGenericOutputViewBase::setupUi(this);

    if (job) {
        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(slotJobFinished(KJob*)));
    }
}

CvsGenericOutputView::~CvsGenericOutputView()
{
}

void CvsGenericOutputView::appendText(const QString& text)
{
    textArea->append(text);
}

void CvsGenericOutputView::slotJobFinished(KJob * job)
{
    CvsJob * cvsjob = dynamic_cast<CvsJob*>(job);
    if (cvsjob) {
        appendText( cvsjob->cvsCommand() );
        appendText( cvsjob->output() );
        if (job->error() == 0) {
            appendText( i18n("Job exited normally") );
        } else {
            appendText( job->errorText() );
        }
    }
}

#include "cvsgenericoutputview.moc"
