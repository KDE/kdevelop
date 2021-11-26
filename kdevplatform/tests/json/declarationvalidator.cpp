/*
    SPDX-FileCopyrightText: 2012 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "declarationvalidator.h"

#include "language/duchain/declaration.h"
#include "testsuite.h"
#include "jsondeclarationtests.h"

#include <QJsonDocument>

#include <utility>

namespace KDevelop {
class DeclarationValidatorPrivate
{
public:
    DeclarationValidator::TestDataEditor testDataAdjuster{};
    bool testsPassed = true;
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

DeclarationValidator::DeclarationValidator(TestDataEditor testDataAdjuster)
    : d_ptr(new DeclarationValidatorPrivate{std::move(testDataAdjuster)})
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
        if (d->testDataAdjuster) {
            d->testDataAdjuster(testData);
        }
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
