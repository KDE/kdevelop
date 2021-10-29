/*
    SPDX-FileCopyrightText: 2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QProcess>

namespace Heaptrack
{

class Visualizer : public QProcess
{
    Q_OBJECT

public:
    /// Creates new Heaptrack visualizer (heaptrack_gui) process. Executable path is taken
    /// from plugin settings, results (data) file - from passed resultsFile parameter. After
    /// process finishing the results file will be automatically removed.
    Visualizer(const QString& resultsFile, QObject* parent);
    ~Visualizer() override;

protected:
    QString m_resultsFile;
};

}
