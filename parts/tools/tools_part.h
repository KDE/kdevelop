/*
 *  Copyright (C) 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>   
 */


#ifndef __KDEVPART_TOOLS_H__
#define __KDEVPART_TOOLS_H__


#include <qguardedptr.h>
#include <kdevplugin.h>
#include <kdevgenericfactory.h>


class QPopupMenu;
class KDialogBase;
class Context;
class ConfigWidgetProxy;

class ToolsWidget;


class ToolsPart : public KDevPlugin
  {
    Q_OBJECT

public:

    ToolsPart( QObject *parent, const char *name, const QStringList & );
    ~ToolsPart();


private slots:
	void insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int pagenumber );
    void updateMenu();

	void slotToolActivated();

    void contextMenu(QPopupMenu *popup, const Context *context);
    void updateToolsMenu();
    void toolsMenuActivated();
    void fileContextActivated(int id);
    void dirContextActivated(int id);
      
private:
    void startCommand(QString cmdline, bool captured, QString fileName);

    QPopupMenu *m_contextPopup;
    QString m_contextFileName;
	ConfigWidgetProxy * m_configProxy;
  };

typedef KDevGenericFactory<ToolsPart> ToolsFactory;

#endif
