 /**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef TEMPLATEWIZARDIFACE_H
#define TEMPLATEWIZARDIFACE_H

//
//  W A R N I N G  --  PRIVATE INTERFACES
//  --------------------------------------
//
// This file and the interfaces declared in the file are not
// public. It exists for internal purpose. This header file and
// interfaces may change from version to version (even binary
// incompatible) without notice, or even be removed.
//
// We mean it.
//
//

#include <private/qcom_p.h>

class QWidget;
struct DesignerFormWindow;

// {983d3eab-fea3-49cc-97ad-d8cc89b7c17b}
#ifndef IID_TemplateWizard
#define IID_TemplateWizard QUuid( 0x983d3eab, 0xfea3, 0x49cc, 0x97, 0xad, 0xd8, 0xcc, 0x89, 0xb7, 0xc1, 0x7b )
#endif

class TemplateWizardInterface : public QFeatureListInterface
{
public:
    virtual void setup( const QString &templ, QWidget *widget, DesignerFormWindow *fw, QUnknownInterface *appIface ) = 0;

};

#endif
