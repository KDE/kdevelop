/*
    SPDX-FileCopyrightText: 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef OKTETATOOLVIEWFACTORY_H
#define OKTETATOOLVIEWFACTORY_H

// KDev
#include <interfaces/iuicontroller.h>

namespace Kasten {
class AbstractToolViewFactory;
class AbstractToolFactory;
}

namespace KDevelop
{

class OktetaToolViewFactory : public IToolViewFactory
{
  public:
    OktetaToolViewFactory( Kasten::AbstractToolViewFactory* toolViewFactory,
                           Kasten::AbstractToolFactory* toolFactory );

    ~OktetaToolViewFactory() override;

  public: // KDevelop::IToolViewFactory API
    QWidget* create( QWidget* parent ) override;
    Qt::DockWidgetArea defaultPosition() const override;

    QString id() const override;

  protected:
    Kasten::AbstractToolViewFactory* mToolViewFactory;
    Kasten::AbstractToolFactory* mToolFactory;
};

}

#endif
