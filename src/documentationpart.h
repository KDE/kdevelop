#ifndef __DOCUMENTATIONPART_H__
#define __DOCUMENTATIONPART_H__


#include <khtml_part.h>


class DocumentationPart : public KHTMLPart
{
  Q_OBJECT

public:

  DocumentationPart();

  void setContext(const QString &context);
  QString context() const;
  virtual bool openURL(const KURL &url);


private slots:
 
  void openURLRequest(const KURL &url);


private:

  QString m_context;

};


#endif
