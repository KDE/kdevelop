/*
 *  Copyright (C) 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>   
 */
 

#ifndef __KDEVPART_UICHOOSER_H__
#define __KDEVPART_UICHOOSER_H__


class KDialogBase;
class ConfigWidgetProxy;

#include <kdevplugin.h>

class UIChooserPart : public KDevPlugin
{
  Q_OBJECT

public:
  UIChooserPart(QObject *parent, const char *name, const QStringList &);
  ~UIChooserPart();
  
private slots:
  void insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int );
  
private:
  ConfigWidgetProxy * _configProxy;

};


#endif
