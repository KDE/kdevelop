/***************************************************************************
 *   Copyright (C) 2005 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ANNOTATEPAGE_H
#define ANNOTATEPAGE_H

#include "cvsservicedcopIface.h"
#include <qwidget.h>
#include <qmap.h>
#include <q3listview.h>

class CvsJob_stub;
class CvsService_stub;
class Q3TextBrowser;
class AnnotateView;
class QStringList;
class KLineEdit;
class KPushButton;

/**
 * Implementation for the page displaying 'cvs annotate' output.
 * To the top of the page the user can enter a revision and request
 * a annotate run for it.
 * The main widget of a page is the AnnotateView. It holds the output
 * of the cvs annotate job. See there for further detail.
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class AnnotatePage : public QWidget, virtual public CVSServiceDCOPIface
{
    Q_OBJECT

    friend class AnnotateDialog;
    friend class AnnotateView;

public:
    AnnotatePage( CvsService_stub *cvsService, QWidget *parent=0, const char *name=0, int flags=0 );
    virtual ~AnnotatePage();

    /**
     * Call cvs annotate for the given file and revistion.
     * @param pathName The filename to annotate
     * @param revision The CVS revision number
     */
    void startAnnotate( const QString pathName, const QString revision="" );

    /**
     * Cancels the current operation if any
     */
    void cancel();

signals:
    /**
     * This signal is ment to be emitted by the nested AnnotateView.
     * The dialog that holds this page catches it in order to create
     * a new page with the annotate output for the given revision.
     * @param rev The revision for which a new annotate run is requested
     */
    void requestAnnotate(const QString rev);

private slots:
    // DCOP Iface
    virtual void slotJobExited( bool normalExit, int exitStatus );
    virtual void slotReceivedOutput( QString someOutput );
    virtual void slotReceivedErrors( QString someErrors );

    /**
     * This slot is connected to the button next to m_leRevision.
     * It clears the AnnotateView and reruns cvs annotate with the
     * revision the user entered into m_leRevision
     */
    void slotNewAnnotate();

private:
    /**
     * This method is executed after the cvs annotate job finished.
     * It parses the output and passes it to the AnnotateView
     */
    void parseAnnotateOutput(QStringList& lines);


    /**
     * This is the output buffer for the cvs annotate job.
     * Everytime slotReceivedOutput() is called by dcop
     * we append the gained data to this buffer.
     */
    QString m_output;
    /**
     * This is the AnnotateView. It gets nested into this page.
     */
    AnnotateView *m_annotateView;
    /**
     * The file for which this page holds the annotate output.
     */
    QString m_pathName;
    /**
     * Maps the checkin comments to revision numbers
     */
    QMap<QString, QString>  m_comments;

    /**
     * With this KLineEdit and the KPushButton next to it
     * the user can rerun cvs annotate for any revision he
     * enters into this KLineEdit.
     */
    KLineEdit *m_leRevision;
    /**
     * With this KPushButton the user can rerun cvs annotate
     * for the revision he entered into m_leRevision
     */
    KPushButton *m_btnAnnotate;

    CvsService_stub *m_cvsService;
    CvsJob_stub *m_cvsAnnotateJob;
};

#endif
