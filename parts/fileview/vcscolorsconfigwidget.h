/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VCSCOLORSCONFIGWIDGET_H
#define VCSCOLORSCONFIGWIDGET_H

#include <qcolor.h>
#include "vcscolorsconfigwidgetbase.h"

class FileViewPart;

struct VCSColors
{
    VCSColors();
    VCSColors( const VCSColors &others );
    VCSColors &operator=( const VCSColors &others );

    QColor modified;
    QColor added;
    QColor updated;
    QColor sticky;
    QColor conflict;
    QColor needsPatch;
    QColor needsCheckout;
    QColor unknown;
    QColor defaultColor;
};

class VCSColorsConfigWidget : public VCSColorsConfigWidgetBase
{
  Q_OBJECT

public:
  VCSColorsConfigWidget( FileViewPart *part, const VCSColors &vcsColors,
      QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
  virtual ~VCSColorsConfigWidget();
  /*$PUBLIC_FUNCTIONS$*/

  void getColors( VCSColors &vcsColors ) const;
  void setColors( const VCSColors &colors );

public slots:
  /*$PUBLIC_SLOTS$*/
  // Store settings if the user gives the ok
  void slotAccept();


protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/


private:
    FileViewPart *m_part;
};

#endif

