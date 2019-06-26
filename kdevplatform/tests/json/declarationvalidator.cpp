/* This file is part of KDevelop
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
