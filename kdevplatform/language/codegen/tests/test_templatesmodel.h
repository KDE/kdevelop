/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTTEMPLATESMODEL_H
#define KDEVPLATFORM_TESTTEMPLATESMODEL_H

#include <QTest>

namespace KDevelop
{
class TemplatesModel;
}

class TestTemplatesModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void descriptionExtraction();
    void descriptionParsing();
    void templateIndexes();

private:
    KDevelop::TemplatesModel* model;
};

#endif // KDEVPLATFORM_TESTTEMPLATESMODEL_H
