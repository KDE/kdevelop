/*
 *  Copyright (C) 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>   
 */


#ifndef __KDEVPART_TOOLS_H__
#define __KDEVPART_TOOLS_H__


#include <qguardedptr.h>
#include <kdevplugin.h>


class KDialogBase;


class ToolsWidget;


class ToolsPart : public KDevPlugin
  {
    Q_OBJECT

public:

    ToolsPart( QObject *parent, const char *name, const QStringList & );
    ~ToolsPart();


private slots:

    void configWidget(KDialogBase *dlg);

    void updateMenu();

	void slotToolActivated();

  };


#endif
