/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for DVCS                                                      *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
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

namespace KDevelop
{
    class DVcsJob;
    class DistributedVersionControlPlugin;
}

/**
 * Shows the output from @code dvcs log @endcode in a nice way.
 * Create a DVCSjob by calling DVCSexecutor::log() and connect the job's
 * result(KJob*) signal to LogView::slotJobFinished(KJob* job)
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class LogView : public QWidget, private Ui::LogViewBase
{
Q_OBJECT
public:
    explicit LogView(KDevelop::DistributedVersionControlPlugin* plugin, KDevelop::DVcsJob* job=0, QWidget *parent = 0);
    virtual ~LogView();

private slots:
    /**
     * Connect a job's result() signal to this slot. When called, the output from the job
     * will be passed to the parseOutput() method and all found revisions will be displayed.
     * @note If you pass a DVCSjob object to the ctor, it's result() signal
     *       will automatically be connected to this slot.
     */
    void slotJobFinished(KJob* job);

private:
    KDevelop::DistributedVersionControlPlugin* m_plugin;
    QString m_output;
};

#endif
