#ifndef __FILECREATE_WIDGET2_H__
#define __FILECREATE_WIDGET2_H__


#include <qtable.h>
#include <qstring.h>
#include <qmap.h>

//#include <klistview.h>

#include "filecreate_typechooser.h"

class KIconLoader;
class KDevProject;
class FileCreatePart;

class FileCreateFileType;

class FileCreateWidget2 : public QTable, public FileCreateTypeChooser
{
  Q_OBJECT
    
public:
		  
  FileCreateWidget2(FileCreatePart *part);
  ~FileCreateWidget2();

  void refresh();

private:
  void setRow(int row, FileCreateFileType * filetype);
  void empty();
  void setDefaultColumnWidths();

  KIconLoader * m_iconLoader;
  QMap<int,FileCreateFileType*> typeForRow;
  FileCreateFileType * m_selected;

private slots:
  void slotCellSelected(int row, int col);
  void slotDoSelection();
  
};


#endif
