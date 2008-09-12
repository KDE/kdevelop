/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef FILECREATE_NEWFILE_H
#define FILECREATE_NEWFILE_H

#include <qptrlist.h>
#include <qmap.h>
#include <kurl.h>
#include <kdialogbase.h>

#include "filecreate_filetype.h"

class KURLRequester;
class QGridLayout;
class KComboBox;
class QCheckBox;
class KLineEdit;

namespace FileCreate {
  class NewFileChooser : public KDialogBase {
      Q_OBJECT
  public:
    NewFileChooser(QWidget *parent=0);
    virtual ~NewFileChooser();
    void setFileTypes(QPtrList<FileType> typelist);
    KURL url() const;
    bool addToProject() const;
    const FileType *selectedType() const;
    void setCurrent(const FileType *filetype);
    void setDirectory(const QString & url);
    void setName(const QString & name);
    
    void setInProjectMode(bool m);
  private slots:
      void slotFileNameChanged(const QString &);
  protected:
    void addType(const FileType * filetype);
	void accept();

    QGridLayout *m_grid;
    KURLRequester *m_urlreq;
    KComboBox *m_filetypes;
    QCheckBox *m_addToProject;
    KLineEdit *m_filename;

    QMap<int,const FileType*> m_typeInCombo;

  };

}

#endif
