/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FILEVIEWPART_H_
#define _FILEVIEWPART_H_

#include <qpointer.h>
#include <configwidgetproxy.h>
#include "kdevplugin.h"
#include "vcscolorsconfigwidget.h"

class PartWidget;
class KDialogBase;
class KDevVersionControl;

class FileViewPart : public KDevPlugin
{
    Q_OBJECT

public:
    FileViewPart( QObject *parent, const char *name, const QStringList & );
    virtual ~FileViewPart();

    //! Colors to use for VCS visual feed-back
    static VCSColors vcsColors;
    
    KDevVersionControl *versionControl();

private slots:
	void insertConfigWidget( const KDialogBase* dlg, QWidget * page, unsigned int );
    void init();

private:
    void loadSettings();
    void storeSettings();

    QPointer<PartWidget> m_widget;
	ConfigWidgetProxy * _configProxy;
};

#endif
