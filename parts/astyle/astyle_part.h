/*
 *  Copyright (C) 2001 Matthias H�zer-Klpfel <mhk@caldera.de>
 */


#ifndef __KDEVPART_ASTYLE_H__
#define __KDEVPART_ASTYLE_H__

class KDialogBase;
#include <kdevsourceformatter.h>
#include <kaction.h>
#include <kparts/part.h>

class AStyleWidget;

class AStylePart : public KDevSourceFormatter
{
  Q_OBJECT

public:

  AStylePart(QObject *parent, const char *name, const QStringList &);
  ~AStylePart();

  QString formatSource( const QString text, AStyleWidget * widget );
  virtual QString formatSource(const QString text);
  QString indentString() const;

private slots:

  void activePartChanged(KParts::Part *part);

  void beautifySource();

  void configWidget(KDialogBase *dlg);



private:

  void cursorPos( KParts::Part *part, uint * col, uint * line );
  void setCursorPos( KParts::Part *part, uint col, uint line );

  KAction *_action;

};


#endif
