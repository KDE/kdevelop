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

#ifndef ANNOTATEDIALOG_H
#define ANNOTATEDIALOG_H

#include <kdialogbase.h>

class CvsJob_stub;
class CvsService_stub;
class AnnotatePage;

/**
 * Implementation for the dialog displaying 'cvs annotate' output.
 *
 * This dialog hold a tab for each revision. The user just needs to 
 * click a line in the AnnotateView to get the annotate output for 
 * the selected revision.
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class AnnotateDialog : public KDialogBase
{
    Q_OBJECT
public:
    AnnotateDialog( CvsService_stub *cvsService, QWidget *parent=0, const char *name=0, int flags=0 );
    virtual ~AnnotateDialog();

    /**
     * Entrypoint from outside.
     * By calling this method, an annotate job is execuded for the given 
     * file and the specifed a revision. 
     * The output gets showen in the page which has already been created by the constructor.
     * 
     * You need to call this function in order to set the file which you want to annotate.
     * Any further operation will be execucted on the file specified by @param pathName
     *
     * @param pathName The file for which to run cvs annotate
     */
    void startFirstAnnotate( const QString pathName, const QString revision = "" );

private slots:
    /**
     * This slot runs cvs annotate for the given revision.
     * The output gets shown in a new page.
     * @param rev The revision which will be annotated
     */
    void slotAnnotate(const QString rev);

private:
    QString m_pathName;

    AnnotatePage *m_cvsAnnotatePage;
    CvsService_stub *m_cvsService;
};

#endif
