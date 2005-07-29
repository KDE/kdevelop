/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "clearcaseintegrator.h"

#include <kdevgenericfactory.h>
#include <kdevplugininfo.h>

#include "ccintegratordlg.h"

static const KDevPluginInfo data("kdevclearcaseintegrator");
typedef KDevGenericFactory<ClearcaseIntegrator> ClearcaseIntegratorFactory;
K_EXPORT_COMPONENT_FACTORY( libclearcaseintegrator, ClearcaseIntegratorFactory(data) )

ClearcaseIntegrator::ClearcaseIntegrator(QObject* parent, const char* name,
    const QStringList // args
                                         )
    :KDevVCSIntegrator(parent, name)
{
}

ClearcaseIntegrator::~ClearcaseIntegrator()
{
}

VCSDialog* ClearcaseIntegrator::fetcher(QWidget* // parent
                                        )
{
    return 0;
}

VCSDialog* ClearcaseIntegrator::integrator(QWidget* parent)
{
    CCIntegratorDlg *dlg = new CCIntegratorDlg(parent);
    return dlg;
}

#include "clearcaseintegrator.moc"
