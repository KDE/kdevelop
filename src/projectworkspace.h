
#ifndef _PROJECTWORKSPACE_H_
#define _PROJECTWORKSPACE_H_


class QDomElement;

/**
 * 
 * KDevelop Authors
 **/
class ProjectWorkspace
{
public:
  static void save();
  static void restore();
  
private:
  static void saveFileList( QDomElement& );
  static void restoreFileList( const QDomElement& );
  
  ProjectWorkspace();
  ~ProjectWorkspace();
};

#endif
