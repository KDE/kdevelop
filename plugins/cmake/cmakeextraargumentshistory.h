/*
    SPDX-FileCopyrightText: 2016 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKEEXTRAARGUMENTS_H
#define CMAKEEXTRAARGUMENTS_H

#include <KComboBox>

#include "cmakecommonexport.h"

#include <QStringList>

namespace Ui {
    class CMakeExtraArgumentsHistory;
}

/**
* This class augments CMake's extra arguments widget with a history feature.
* @author René Bertin
*/
class KDEVCMAKECOMMON_EXPORT CMakeExtraArgumentsHistory
{
    public:
        explicit CMakeExtraArgumentsHistory(KComboBox* widget);
        ~CMakeExtraArgumentsHistory();

        QStringList list() const;

    private:
        Q_DISABLE_COPY(CMakeExtraArgumentsHistory)

        KComboBox* m_arguments;
};

#endif
