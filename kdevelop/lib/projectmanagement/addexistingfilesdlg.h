/***************************************************************************
                             -------------------
    begin                : Thu Oct 5 2000
    copyright            : (C) 2000 by Sandy Meier (KDevelop Team)
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADDEXISTINGFILESDLG_H
#define ADDEXISTINGFILESDLG_H
#include "addexistingfilesbase.h"
#include <qstring.h>
#include <qlist.h>


class AddExistingFilesDlg : public AddExistingFilesBase
{ 
    Q_OBJECT

public:
    AddExistingFilesDlg( QWidget* parent = 0, const char* name = 0,QString startDir="/",
			 QStringList filters="");
    ~AddExistingFilesDlg();
    QStringList addedFiles();
    protected slots:
      void slotDirectorySelected(QString directory);
    void slotComboBoxActivated(const QString&);
    void slotAddClicked();
    void slotAddAllClicked();
    void slotAddTreeClicked();
    void slotRemoveClicked();
    void slotRemoveAllClicked();

      
 protected:
    QString m_currentDir;
    QStringList m_addedFiles;
    QStringList m_filters;
 
};


#endif // FILEPROPSPAGE_H
