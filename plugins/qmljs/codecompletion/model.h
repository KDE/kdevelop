/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef QMLJS_MODEL_H
#define QMLJS_MODEL_H

#include "codecompletionexport.h"

#include <language/codecompletion/codecompletionmodel.h>

namespace QmlJS {

class KDEVQMLJSCOMPLETION_EXPORT CodeCompletionModel : public KDevelop::CodeCompletionModel
{
    Q_OBJECT
public:
    explicit CodeCompletionModel(QObject* parent);
protected:
    KDevelop::CodeCompletionWorker* createCompletionWorker() override;
};

}

#endif // QMLJS_MODEL_H
