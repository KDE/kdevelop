
#ifndef _KDEVCREATEFILE_H_
#define _KDEVCREATEFILE_H_

#include <qstring.h>

/**
 * 
 * KDevelop Authors
 **/
class KDevCreateFile
{

public:
  class CreatedFile {
    
  public:  
    enum Status { STATUS_OK, STATUS_NOTCREATED, STATUS_NOTWITHINPROJECT };

    QString dir;
    QString filename;
    QString ext;
    QString subtype;
    Status status;
    
  };
                     

public:

  KDevCreateFile() { }
  virtual ~KDevCreateFile() { }
  
  
  virtual CreatedFile createNewFile(QString ext = QString::null,
                     QString dir = QString::null,
                     QString name = QString::null,
                     QString subtype = QString::null) = 0;
                     
  
};

#endif
