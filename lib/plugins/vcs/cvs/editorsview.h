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

#ifndef EDITORSVIEW_H
#define EDITORSVIEW_H

#include <QDialog>
#include <KJob>

#include "ui_editorsview.h"

class CvsPart;
class CvsJob;

/**
 * Shows the output from @code cvs editors @endcode in a nice way.
 * Create a CvsJob by calling CvsProxy::editors() and connect the job's
 * result(KJob*) signal to EditorsView::slotJobFinished(KJob* job)
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class EditorsView : public QWidget, private Ui::EditorsViewBase
{
Q_OBJECT
public:
    EditorsView(CvsPart* part, CvsJob* job=0, QWidget *parent = 0);
    virtual ~EditorsView();

private slots:
    void slotJobFinished(KJob* job);

private:
    CvsPart* m_part;
    QString m_output;
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
