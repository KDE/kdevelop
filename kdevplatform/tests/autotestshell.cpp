/*
    SPDX-FileCopyrightText: 2013 Kevin Funk <kfunk@kde.org

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "autotestshell.h"

#include <sourcefilepaths.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTest>

using namespace KDevelop;

AutoTestShell::~AutoTestShell() = default;

QString AutoTestShell::xmlFile()
{
    return QStringLiteral(KDEVELOP_APP_DIR "/kdevelopui.rc");
}

void AutoTestShell::init(const QStringList& plugins)
{
    // TODO: Maybe generalize, add KDEVELOP_STANDALONE build option
    qputenv("CLEAR_DUCHAIN_DIR", "1"); // Always clear duchain dir (also to avoid popups asking the user to clear it)

    QStandardPaths::setTestModeEnabled(true);

    static auto instance = AutoTestShell();
    instance.m_plugins = plugins;
    s_instance = &instance;
}

void AutoTestShell::initializeNotifications()
{
    // Guard against installing to a non-test location.
    QVERIFY2(QStandardPaths::isTestModeEnabled(),
             "AutoTestShell::initializeNotifications() may be called only after AutoTestShell::init()");

    const QString destination = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
        + QLatin1String{"/knotifications6/"} + QCoreApplication::applicationName() + QLatin1String{".notifyrc"};

    const auto destinationDirectory = destination.first(destination.lastIndexOf(QLatin1Char{'/'}));
    QVERIFY(QDir().mkpath(destinationDirectory));

    // Remove the destination first because QFile::copy() does not overwrite
    // and would keep a possibly obsolete version of the file.
    QFile::remove(destination);

    const auto source = QStringLiteral(KDEVELOP_APP_DIR "/kdevelop.notifyrc");
    QVERIFY(QFile::copy(source, destination));
}
