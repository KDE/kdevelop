/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "settingsdialogbase.h"

class KEditListBox;

class SettingsDialog : public SettingsDialogBase
{
  Q_OBJECT

public:
  SettingsDialog(QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
  ~SettingsDialog();

  bool isValidQtDir( const QString& path ) const;

  QString qtDir() const;
  QString configuration() const;

  QString dbName() const;
  QStringList dirs() const;
  bool recursive() const;
  QString filePattern() const;

private slots:
  void validate();
  void validateDirectory( const QString & dir );

private:
  KEditListBox *elb;

};

#endif


