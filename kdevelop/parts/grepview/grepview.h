/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GREPVIEW_H_
#define _GREPVIEW_H_

#include <qguardedptr.h>
#include "kdevcomponent.h"


class GrepWidget;
class ProjectSpace;
class KDevEditorManager;

class GrepView : public KDevComponent
{
    Q_OBJECT

public:
    GrepView( QObject *parent=0, const char *name=0 );
    ~GrepView();
   	ProjectSpace* projectSpace();
   	KDevEditorManager* editorManager();

public slots:	
	void slotProjectSpaceOpened();
	void slotProjectSpaceClosed();
	void slotStopButtonClicked();
	void slotConfigWidgetRequested(KDialogBase*);

protected:
    virtual void setupGUI();

private:
    QGuardedPtr<GrepWidget> m_widget;
    friend class GrepWidget;
};

#endif
