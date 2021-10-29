/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

var buffer = require("buffer");

/*
 * exports.StringDecoder
 */
function StringDecoder(encoding) { return ; }
exports.StringDecoder = StringDecoder;

exports.StringDecoder.prototype.write = function (buffer) { return ""; };
exports.StringDecoder.prototype.write(new buffer.Buffer());

exports.StringDecoder.prototype.end = function () { return ""; };
exports.StringDecoder.prototype.end();


