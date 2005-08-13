/***************************************************************************
 *   Copyright (C) 2004 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EDITORSDIALOG_H
#define EDITORSDIALOG_H

#include "editorsdialogbase.h"
#include "cvsservicedcopIface.h"

class CvsJob_stub;
class CvsService_stub;
class QStringList;

class EditorsDialog: public EditorsDialogBase, virtual public CVSServiceDCOPIface
{
Q_OBJECT
public:
    EditorsDialog(CvsService_stub *cvsService, QWidget *parent = 0, const char *name = 0);
    virtual ~EditorsDialog();
    
    void startjob(QString strDir);

private slots:
    // DCOP Iface
    virtual void slotJobExited( bool normalExit, int exitStatus );
    virtual void slotReceivedOutput( QString someOutput );
    virtual void slotReceivedErrors( QString someErrors );

private:    
    CvsService_stub *m_cvsService;
    CvsJob_stub *m_cvsJob;
    QString m_output;
};

#endif
