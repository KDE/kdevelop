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

#ifndef SETUPQTPAGE_H
#define SETUPQTPAGE_H

#include "setupqtpagebase.h"

class CppSupportPart;

class SetupQtPage : public SetupQtPageBase
{
  Q_OBJECT

public:
  SetupQtPage(CppSupportPart* cppSupport, QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
  ~SetupQtPage();
  /*$PUBLIC_FUNCTIONS$*/

public slots:
  /*$PUBLIC_SLOTS$*/

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/

private:
    CppSupportPart* m_part;
};

#endif

