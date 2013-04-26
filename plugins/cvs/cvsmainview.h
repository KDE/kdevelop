/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_CVSMAINVIEW_H
#define KDEVPLATFORM_PLUGIN_CVSMAINVIEW_H

#include <QWidget>
#include <KJob>
#include <QToolButton>

#include "ui_cvsmainview.h"

class CvsPlugin;
class CvsGenericOutputView;

/**
 * This class is the main output view of KDevelop's CVS plugin.
 * It only constists out of a KTabWidget.
 *
 * When created, a CvsGenericOutputView will be inserted.
 *
 * Inserting text into that default output view is possible via the
 * slotJobFinished() slot.
 *
 * Additional tabs can be added via slotAddTab().
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsMainView : public QWidget, private Ui::CvsMainViewBase {
    Q_OBJECT
public:
    CvsMainView(CvsPlugin *plugin, QWidget* parent);
    virtual ~CvsMainView();

public slots:
    /**
     * Inserts @p tag into the KTabWidget and calls it @p label .
     * This slot gets connected to CvsPlugin::addNewTabToMainView().
     */
    void slotAddTab(QWidget* tab, const QString& label);

    /**
     * When this slot gets called, the output of the job will be written to
     * the default outputview of the KTabWidget.
     * This slot gets connected to CvsPlugin::jobFinished().
     */
    void slotJobFinished(KJob* job);

    /**
     * Closes the current active tab (if it's not the first tab)
     */
    void slotTabClose();

private:
    CvsPlugin* m_plugin;
    CvsGenericOutputView* m_mainview;
    QToolButton* m_closeButton;
};

#endif
