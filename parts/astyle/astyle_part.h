/*
 *  Copyright (C) 2001 Matthias H�zer-Klpfel <mhk@caldera.de>
 */


#ifndef __KDEVPART_ASTYLE_H__
#define __KDEVPART_ASTYLE_H__

class KDialogBase;
#include <kdevsourceformatter.h>
#include <kaction.h>
#include <kparts/part.h>
#include <kdevplugin.h>
#include <kdebug.h>
#include <qmap.h>
#include <qvariant.h>

class AStyleWidget;
class QPopupMenu;
class Context;
class ConfigWidgetProxy;

class AStylePart : public KDevSourceFormatter
{
  Q_OBJECT

public:

  AStylePart(QObject *parent, const char *name, const QStringList &);
  ~AStylePart();

  QString formatSource(const QString text, AStyleWidget * widget, const QMap<QString, QVariant>& options);
  virtual QString formatSource(const QString text);
  QString indentString() const;
  void saveGlobal();
  void setExtensions(QString ext);
  QString getExtensions();
  void restorePartialProjectSession(const QDomElement * el);
  void savePartialProjectSession(QDomElement * el);

  QMap<QString, QVariant>& getProjectOptions(){return m_project;}
  QMap<QString, QVariant>& getGlobalOptions(){return m_global;}


private slots:

  void activePartChanged(KParts::Part *part);
  void beautifySource();
  void formatFiles();
  void formatFilesSelect();
  void insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo);
  void contextMenu(QPopupMenu *popup, const Context *context);

private:

  void cursorPos( KParts::Part *part, uint * col, uint * line );
  void setCursorPos( KParts::Part *part, uint col, uint line );

  void loadGlobal();

  ConfigWidgetProxy *m_configProxy;
  KAction *formatTextAction;
  KAction *formatFileAction;

  // the configurable options.
  QMap<QString, QVariant>  m_project;
  QMap<QString, QVariant> m_global;
  QMap<QString, QString> m_searchExtensions;
  QStringList m_displayExtensions;
  KURL::List m_urls;


};


#endif
