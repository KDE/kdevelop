/*
    SPDX-FileCopyrightText: 2016 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MACDOCKPROGRESSVIEW_H

namespace KDevelop
{

class MacDockProgressView
{
public:
    static void setRange(int min, int max);

    static void setProgress(int value);

    static void setProgressVisible(bool visible);
};

}

#define MACDOCKPROGRESSVIEW_H
#endif
