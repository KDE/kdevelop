/*
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_TEST_KTEXTEDITORPLUGININTEGRATION_H
#define KDEVPLATFORM_TEST_KTEXTEDITORPLUGININTEGRATION_H

#include <QObject>

class TestKTextEditorPluginIntegration : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testApplication();
    void testMainWindow();
    void testPlugin();
    void testPluginUnload();
};

#endif // KDEVPLATFORM_TEST_KTEXTEDITORPLUGININTEGRATION_H
