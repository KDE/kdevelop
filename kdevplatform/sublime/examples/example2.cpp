/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <sublime/area.h>
#include <sublime/urldocument.h>
#include <sublime/controller.h>

#include <QApplication>
#include <QUrl>

#include "example2main.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    auto *controller = new Sublime::Controller(&app);
    auto* area = new Sublime::Area(controller, QStringLiteral("Area"));
    controller->addDefaultArea(area);
    Sublime::Document *doc = new Sublime::UrlDocument(controller, QUrl::fromLocalFile(QStringLiteral("~/foo.cpp")));
    area->addView(doc->createView());
    auto *window = new Example2Main(controller);
    controller->showArea(area, window);
    window->resize(800, 600);
    window->show();

    return app.exec();
}

