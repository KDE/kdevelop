#ifndef __ASTYLE_ADAPTOR_H__
#define __ASTYLE_ADAPTOR_H__


#include <qstring.h>
#include <qtextstream.h>
#include <qmap.h>
#include <qvariant.h>

#include "astyle.h"

#define GLOBALDOC_OPTIONS 1
#define PROJECTDOC_OPTIONS 2


class ASStringIterator : public astyle::ASSourceIterator
{
public:

  ASStringIterator(const QString &string);
  virtual ~ASStringIterator();

  virtual bool hasMoreLines() const;
  virtual string nextLine();


private:

  QString _content;
  QTextStream *_is;

};

class AStyleWidget;

class KDevFormatter : public astyle::ASFormatter
{
public:

  KDevFormatter(const QMap<QString, QVariant>& options);
  KDevFormatter( AStyleWidget * widget );
  QString indentString() const
  {
	  return m_indentString;
  }
  
private:
	bool predefinedStyle( const QString & style );

private:
	QString m_indentString;
};


#endif
