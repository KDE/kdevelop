/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

var buffer = require("buffer");

exports.getCiphers = function () { return []; };
exports.getCiphers();

exports.getHashes = function () { return []; };
exports.getHashes();

exports.createCredentials = function (details) { return new Object(); };
exports.createCredentials(new Object());

exports.createHash = function (algorithm) { return new Hash(); };
exports.createHash("");

/*
 * exports.Hash
 */
function Hash() { return ; }
exports.Hash = Hash;

exports.Hash.prototype.update = function (data, input_encoding) { return ; };
exports.Hash.prototype.update("", "");

exports.Hash.prototype.digest = function (encoding) { return ""; };
exports.Hash.prototype.digest("");


exports.createHmac = function (algorithm, key) { return new Hmac(); };
exports.createHmac("", "");

/*
 * exports.Hmac
 */
function Hmac() { return ; }
exports.Hmac = Hmac;

exports.Hmac.prototype.update = function (data) { return ; };
exports.Hmac.prototype.update("");

exports.Hmac.prototype.digest = function (encoding) { return ""; };
exports.Hmac.prototype.digest("");


exports.createCipher = function (algorithm, password) { return new Cipher(); };
exports.createCipher("", "");

exports.createCipheriv = function (algorithm, password, iv) { return new Cipher(); };
exports.createCipheriv("", "", "");

/*
 * exports.Cipher
 */
function Cipher() { return ; }
exports.Cipher = Cipher;

exports.Cipher.prototype.update = function (data, input_encoding, output_encoding) { return ; };
exports.Cipher.prototype.update("", "", "");

exports.Cipher.prototype.final = function (output_encoding) { return ""; };
exports.Cipher.prototype.final("");

exports.Cipher.prototype.setAutoPadding = function (auto_padding) { return ; };
exports.Cipher.prototype.setAutoPadding(true);


exports.createDecipher = function (algorithm, password) { return new Decipher(); };
exports.createDecipher("", "");

exports.createDecipheriv = function (algorithm, password, iv) { return new Decipher(); };
exports.createDecipheriv("", "", "");

/*
 * exports.Decipher
 */
function Decipher() { return ; }
exports.Decipher = Decipher;

exports.Decipher.prototype.update = function (data, input_encoding, output_encoding) { return ; };
exports.Decipher.prototype.update("", "", "");

exports.Decipher.prototype.final = function (output_encoding) { return ""; };
exports.Decipher.prototype.final("");

exports.Decipher.prototype.setAutoPadding = function (auto_padding) { return ; };
exports.Decipher.prototype.setAutoPadding(true);


exports.createSign = function (algorithm) { return new Sign(); };
exports.createSign("");

/*
 * exports.Sign
 */
function Sign() { return ; }
exports.Sign = Sign;

exports.Sign.prototype.update = function (data) { return ; };
exports.Sign.prototype.update("");

exports.Sign.prototype.sign = function (private_key, output_format) { return ""; };
exports.Sign.prototype.sign("", "");


exports.createVerify = function (algorithm) { return new Verify(); };
exports.createVerify("");

/*
 * exports.Verify
 */
function Verify() { return ; }
exports.Verify = Verify;

exports.Verify.prototype.update = function (data) { return ; };
exports.Verify.prototype.update("");

exports.Verify.prototype.verify = function (object, signature, signature_format) { return true; };
exports.Verify.prototype.verify("", "", "");


exports.createDiffieHellman = function (prime_length) { return new DiffieHellman(); };
exports.createDiffieHellman(1);

/*
 * exports.DiffieHellman
 */
function DiffieHellman() { return ; }
exports.DiffieHellman = DiffieHellman;

exports.DiffieHellman.prototype.generateKeys = function (encoding) { return ""; };
exports.DiffieHellman.prototype.generateKeys("");

exports.DiffieHellman.prototype.computeSecret = function (other_public_key, input_encoding, output_encoding) { return ""; };
exports.DiffieHellman.prototype.computeSecret("", "", "");

exports.DiffieHellman.prototype.getPrime = function (encoding) { return ""; };
exports.DiffieHellman.prototype.getPrime("");

exports.DiffieHellman.prototype.getGenerator = function (encoding) { return ""; };
exports.DiffieHellman.prototype.getGenerator("");

exports.DiffieHellman.prototype.getPublicKey = function (encoding) { return ""; };
exports.DiffieHellman.prototype.getPublicKey("");

exports.DiffieHellman.prototype.getPrivateKey = function (encoding) { return ""; };
exports.DiffieHellman.prototype.getPrivateKey("");

exports.DiffieHellman.prototype.setPublicKey = function (public_key, encoding) { return ; };
exports.DiffieHellman.prototype.setPublicKey("", "");

exports.DiffieHellman.prototype.setPrivateKey = function (private_key, encoding) { return ; };
exports.DiffieHellman.prototype.setPrivateKey("", "");


exports.getDiffieHellman = function (group_name) { return new DiffieHellman(); };
exports.getDiffieHellman("");

exports.pbkdf2 = function (password, salt, iterations, keylen, callback) { return ; };
exports.pbkdf2("", "", 1, 1, function(){});

exports.pbkdf2Sync = function (password, salt, iterations, keylen) { return ""; };
exports.pbkdf2Sync("", "", 1, 1);

exports.randomBytes = function (size, callback) { return new buffer.Buffer(); };
exports.randomBytes(1, function(){});

exports.pseudoRandomBytes = function (size, callback) { return new buffer.Buffer(); };
exports.pseudoRandomBytes(1, function(){});

exports.DEFAULT_ENCODING = "";

