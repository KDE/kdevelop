#ifndef __FILECREATE_WIDGET_H__
#define __FILECREATE_WIDGET_H__


#include <qwidget.h>
#include <qstring.h>

#include <klistview.h>

#include "filecreate_typechooser.h"

class KDevProject;
class FileCreatePart;

class FileCreateFileType;

class FileCreateWidget : public KListView, public FileCreateTypeChooser
{
  Q_OBJECT
    
public:
		  
  FileCreateWidget(FileCreatePart *part);
  ~FileCreateWidget();

  void refresh();

public slots:
  void slotTypeSelected(QListViewItem * item);
signals:
  void filetypeSelected(const FileCreateFileType * filetype);

};


#endif
