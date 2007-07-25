/* This file is part of KDevelop
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _VALGRIND_DIALOG_H_
#define _VALGRIND_DIALOG_H_

#include <kdialog.h>


namespace Ui { class ValgrindDialog; }

/**
 *
 * Harald Fernengel
 **/
class ValgrindDialog : public KDialog
{
    Q_OBJECT
public:
  enum Type { Memcheck = 0, Calltree = 1 };

  explicit ValgrindDialog( Type type, QWidget* parent = 0 );
  ~ValgrindDialog();

  // the app to check
  QString executableName() const;
  void setExecutable( const QString& url );

  // command line parameters for the app
  QString parameters() const;
  void setParameters( const QString& params );

  // name and/or path to the valgrind executable
  QString valExecutable() const;
  void setValExecutable( const QString& ve );

  // command line parameters for valgrind
  QString valParams() const;
  void setValParams( const QString& params );

  // name and/or path to the calltree executable
  QString ctExecutable() const;
  void setCtExecutable( const QString& ce );

  // command line parameters for calltree
  QString ctParams() const;
  void setCtParams( const QString& params );

  // name and/or path to the kcachegrind executable
  QString kcExecutable() const;
  void setKcExecutable( const QString& ke );

protected:
  bool isNewValgrindVersion() const;

private:
  Ui::ValgrindDialog* w;
  Type m_type;

private slots:
  void valgrindTextChanged();
};

#endif
