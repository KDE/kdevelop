#ifndef __JAVASUPPORTPART_H__
#define __JAVASUPPORTPART_H__


#include "kdevlanguagesupport.h"


class JavaClassParser;


class JavaSupportPart : public KDevLanguageSupport
{
  Q_OBJECT

public:
   
  JavaSupportPart(QObject *parent, const char *name, const QStringList &);
  ~JavaSupportPart();
 
 
protected:

  Features features();
  QStringList fileFilters();

  void addClass();


private slots:

  void projectOpened();
  void projectClosed();

  void addedFileToProject(const QString &fileName);
  void removedFileFromProject(const QString &fileName);
  
  void initialParse();


private:

  void maybeParse(const QString &fileName);
  void parse(const QString &fileName);
 
  JavaClassParser *m_parser;

};


#endif
