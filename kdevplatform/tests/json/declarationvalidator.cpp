/*
    SPDX-FileCopyrightText: 2012 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "declarationvalidator.h"

#include "language/duchain/declaration.h"
#include "testsuite.h"
#include "jsondeclarationtests.h"

#include <QJsonDocument>

namespace KDevelop {
class DeclarationValidatorPrivate
{
public:
    DeclarationValidatorPrivate() : testsPassed(true) {}
    bool testsPassed;
};

QByteArray preprocess(QByteArray json)
{
    int commentIndex = json.indexOf('#', 0);
    while (commentIndex > -1) {
        int commentEnd = json.indexOf('\n', commentIndex);
        if (commentEnd == -1) {
            json.truncate(commentIndex);
            break;
        }
        json.remove(commentIndex, commentEnd - commentIndex);
        commentIndex = json.indexOf('#', commentIndex);
    }

    return '{' + json + '}';
}

DeclarationValidator::DeclarationValidator()
    : d_ptr(new DeclarationValidatorPrivate)
{
}
DeclarationValidator::~DeclarationValidator()
{
}

bool DeclarationValidator::testsPassed() const
{
    Q_D(const DeclarationValidator);

    return d->testsPassed;
}

void DeclarationValidator::visit(DUContext*) { }
void DeclarationValidator::visit(Declaration* declaration)
{
    Q_D(DeclarationValidator);

    QJsonParseError error;
    const auto json = preprocess(declaration->comment());
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);

    if (error.error == 0) {
        QVariantMap testData = doc.toVariant().toMap();

        if (!KDevelop::runTests(testData, declaration))
            d->testsPassed = false;
    } else
    {
        d->testsPassed = false;
        QMessageLogger logger(declaration->topContext()->url().byteArray().constData(),
            declaration->range().start.line, nullptr);
        logger.warning() << "Error parsing JSON test data:" << error.errorString() << "at offset" << error.offset <<
            "JSON input was:\n" << json;
    }
}
}
