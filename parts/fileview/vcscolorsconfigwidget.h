//
// C++ Interface:
//
// Description:
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

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
    QColor unknown;
    QColor defaultColor;
};

class VCSColorsConfigWidget : public VCSColorsConfigWidgetBase
{
  Q_OBJECT

public:
  VCSColorsConfigWidget( FileViewPart *part, const VCSColors &vcsColors,
      QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
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

