/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_OUTPUTMODEL_H
#define KDEVPLATFORM_TEST_OUTPUTMODEL_H

#include <QObject>

namespace KDevelop
{
class OutputModel;

class TestOutputModel : public QObject
{
Q_OBJECT
public:
    explicit TestOutputModel(QObject* parent = nullptr);

private Q_SLOTS:
    void bench();
    void bench_data();
};

}
#endif // KDEVPLATFORM_TEST_OUTPUTMODEL_H
