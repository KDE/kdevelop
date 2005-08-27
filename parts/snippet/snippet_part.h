/*
 *  File : snippet_part.h
 *
 *  Author: Robert Gruber <rgruber@users.sourceforge.net>
 *
 *  Copyright: See COPYING file that comes with this distribution
 */


#ifndef __KDEVPART_SNIPPET_H__
#define __KDEVPART_SNIPPET_H__


#include <qguardedptr.h>
#include <kdevplugin.h>


class SnippetWidget;
class KAboutData;
class KDialogBase;

/*
This is the KDevPlugin for the CodeSnippet feature
For more info read the README.dox file
@author Robert Gruber
*/
class SnippetPart : public KDevPlugin
{
  Q_OBJECT

public:

  SnippetPart(QObject *parent, const char *name, const QStringList &);
  ~SnippetPart();
  KAboutData * aboutData();
    QStringList getAllLanguages();
    QStringList getProjectLanguages();

private:
  QGuardedPtr<SnippetWidget> m_widget;

public slots:
  void slotConfigWidget( KDialogBase *dlg );
};


#endif
