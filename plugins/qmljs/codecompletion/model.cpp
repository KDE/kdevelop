/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "model.h"

#include "worker.h"

namespace QmlJS {

CodeCompletionModel::CodeCompletionModel(QObject* parent)
    : KDevelop::CodeCompletionModel(parent)
{

}

KDevelop::CodeCompletionWorker* CodeCompletionModel::createCompletionWorker()
{
    return new CodeCompletionWorker(this);
}

}
