
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

    CreatedFile()
      : status( STATUS_NOTCREATED ) {}
    
    CreatedFile( const CreatedFile& source )
      : dir( source.dir ), filename( source.filename ),
        ext( source.ext ), subtype( source.subtype ),
	status( source.status ) {}
	
    CreatedFile& operator = ( const CreatedFile& source )
    {
	dir = source.dir;
	filename = source.filename;
	ext = source.ext;
	subtype = source.subtype;
	status = source.status;
	return( *this );
    }
    
    bool operator == ( const CreatedFile& source ) const
    {
        return
           dir == source.dir &&
	   filename == source.filename &&
	   ext == source.ext &&
	   subtype == source.subtype &&
	   status == source.status;
    }
    
    // this should be private
    QString dir;
    QString filename;
    QString ext;
    QString subtype;
    Status status;
  };
                     

public:

  KDevCreateFile() { }
  virtual ~KDevCreateFile() { }
  
  /**
   * Call this method to create a new file, within or without the project. Supply as
   * much information as you know. Leave what you don't know as QString::null.
   * The user will be prompted as necessary for the missing information, and the
   * file created, and added to the project as necessary.
   */ 
  virtual CreatedFile createNewFile(QString ext = QString::null,
                     QString dir = QString::null,
                     QString name = QString::null,
                     QString subtype = QString::null) = 0;
                     
  
};

#endif
