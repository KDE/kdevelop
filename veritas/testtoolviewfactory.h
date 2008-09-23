/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef VERITAS_TESTTOOLVIEWFACTORY_H
#define VERITAS_TESTTOOLVIEWFACTORY_H

#include <interfaces/iuicontroller.h> // for IToolViewFactory
#include "veritasexport.h"

class QWidget;
namespace Sublime { class View; }

namespace Veritas
{

class ITestFramework;

/*! A factory implementation for veritas test-runner toolviews */
class VERITAS_EXPORT TestToolViewFactory : public KDevelop::IToolViewFactory
{
public:
    TestToolViewFactory(ITestFramework *framework);
    virtual ~TestToolViewFactory();

    // IToolViewFactory
    virtual QWidget* create(QWidget *parent);
    virtual Qt::DockWidgetArea defaultPosition();
    virtual QString id() const;
    virtual void viewCreated(Sublime::View* view);

private:
    class TestToolViewFactoryPrivate;
    TestToolViewFactoryPrivate * const d;
};

}

#endif
