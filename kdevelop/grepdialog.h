/***************************************************************************
                          grepdialog.h  -  grep frontend                              
                             -------------------                                         
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef _GREPDIALOG_H_
#define _GREPDIALOG_H_

#include <qdialog.h>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QListBox;
class QPushButton;
class QLabel;
class KProcess;


class GrepDialog : public QDialog
{
    Q_OBJECT

public:
    GrepDialog(QString dirname, QWidget *parent=0, const char *name=0);
  ~GrepDialog();
  void  setDirName(QString);

signals:
    void itemSelected(QString abs_filename, int line);
    
public slots:
    void slotSearchFor(QString pattern);
		
private slots:
    void dirButtonClicked();
    void templateActivated(int index);
    void childExited();
    void receivedOutput(KProcess *proc, char *buffer, int buflen);
    void itemSelected(const QString &item);
    void slotSearch();
    void slotCancel();
    void slotClear();

private:
    void processOutput();
    void finish();
    
    QLineEdit *pattern_edit, *template_edit, *dir_edit;
    QComboBox *files_combo;
    QCheckBox *recursive_box;
    QListBox *resultbox;
    QPushButton *search_button, *cancel_button;
    QLabel *status_label, *matches_label;
    KProcess *childproc;
    QString buf;
};


#endif





