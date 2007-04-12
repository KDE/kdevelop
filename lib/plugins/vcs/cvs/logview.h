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

#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QDialog>
#include <KJob>

#include "ui_logview.h"

class CvsPart;
class CvsJob;

/**
 * Shows the output from @code cvs log @endcode in a nice way.
 * Create a CvsJob by calling CvsProxy::log() and connect the job's
 * result(KJob*) signal to LogView::slotJobFinished(KJob* job)
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class LogView : public QWidget, private Ui::LogViewBase
{
Q_OBJECT
public:
    LogView(CvsPart* part, CvsJob* job=0, QWidget *parent = 0);
    virtual ~LogView();

private slots:
    void slotJobFinished(KJob* job);

private:
    CvsPart* m_part;
    QString m_output;
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
