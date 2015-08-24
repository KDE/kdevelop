/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
    
var stream = require("stream");

var buffer = require("buffer");

var event = require("event");

exports.rename = function (oldPath, newPath, callback) { return ; };
exports.rename("", "", function(){});

exports.renameSync = function (oldPath, newPath) { return 1; };
exports.renameSync("", "");

exports.ftruncate = function (fd, len, callback) { return ; };
exports.ftruncate(1, 1, function(){});

exports.ftruncateSync = function (fd, len) { return 1; };
exports.ftruncateSync(1, 1);

exports.truncate = function (path, len, callback) { return ; };
exports.truncate("", 1, function(){});

exports.truncateSync = function (path, len) { return 1; };
exports.truncateSync("", 1);

exports.chown = function (path, uid, gid, callback) { return ; };
exports.chown("", 1, 1, function(){});

exports.chownSync = function (path, uid, gid) { return 1; };
exports.chownSync("", 1, 1);

exports.fchown = function (fd, uid, gid, callback) { return ; };
exports.fchown(1, 1, 1, function(){});

exports.fchownSync = function (fd, uid, gid) { return 1; };
exports.fchownSync(1, 1, 1);

exports.lchown = function (path, uid, gid, callback) { return ; };
exports.lchown("", 1, 1, function(){});

exports.lchownSync = function (path, uid, gid) { return 1; };
exports.lchownSync("", 1, 1);

exports.chmod = function (path, mode, callback) { return ; };
exports.chmod("", 1, function(){});

exports.chmodSync = function (path, mode) { return 1; };
exports.chmodSync("", 1);

exports.fchmod = function (fd, mode, callback) { return ; };
exports.fchmod(1, 1, function(){});

exports.fchmodSync = function (fd, mode) { return 1; };
exports.fchmodSync(1, 1);

exports.lchmod = function (path, mode, callback) { return ; };
exports.lchmod("", 1, function(){});

exports.lchmodSync = function (path, mode) { return 1; };
exports.lchmodSync("", 1);

exports.stat = function (path, callback) { return ; };
exports.stat("", function(){});

exports.lstat = function (path, callback) { return ; };
exports.lstat("", function(){});

exports.fstat = function (fd, callback) { return ; };
exports.fstat(1, function(){});

exports.statSync = function (path) { return new Stats; };
exports.statSync("");

exports.lstatSync = function (path) { return new Stats; };
exports.lstatSync("");

exports.fstatSync = function (fd) { return new Stats; };
exports.fstatSync(1);

exports.link = function (srcpath, dstpath, callback) { return ; };
exports.link("", "", function(){});

exports.linkSync = function (srcpath, dstpath) { return 1; };
exports.linkSync("", "");

exports.symlink = function (srcpath, dstpath, type, callback) { return ; };
exports.symlink("", "", "", function(){});

exports.symlinkSync = function (srcpath, dstpath, type) { return 1; };
exports.symlinkSync("", "", "");

exports.readlink = function (path, callback) { return ; };
exports.readlink("", function(){});

exports.readlinkSync = function (path) { return 1; };
exports.readlinkSync("");

exports.realpath = function (path, cache, callback) { return ; };
exports.realpath("", new Object(), function(){});

exports.realpathSync = function (path, cache) { return ""; };
exports.realpathSync("", new Object());

exports.unlink = function (path, callback) { return ; };
exports.unlink("", function(){});

exports.unlinkSync = function (path) { return 1; };
exports.unlinkSync("");

exports.rmdir = function (path, callback) { return ; };
exports.rmdir("", function(){});

exports.rmdirSync = function (path) { return 1; };
exports.rmdirSync("");

exports.mkdir = function (path, mode, callback) { return ; };
exports.mkdir("", 1, function(){});

exports.mkdirSync = function (path, mode) { return 1; };
exports.mkdirSync("", 1);

exports.readdir = function (path, callback) { return ; };
exports.readdir("", function(){});

exports.readdirSync = function (path) { return []; };
exports.readdirSync("");

exports.close = function (fd, callback) { return ; };
exports.close(1, function(){});

exports.closeSync = function (fd) { return 1; };
exports.closeSync(1);

exports.open = function (path, flags, mode, callback) { return ; };
exports.open("", "", 1, function(){});

exports.openSync = function (path, flags, mode) { return 1; };
exports.openSync("", "", 1);

exports.utimes = function (path, atime, mtime, callback) { return ; };
exports.utimes("", new Date(), new Date(), function(){});

exports.utimesSync = function (path, atime, mtime) { return 1; };
exports.utimesSync("", new Date(), new Date());

exports.futimes = function (fd, atime, mtime, callback) { return ; };
exports.futimes(1, new Date(), new Date(), function(){});

exports.futimesSync = function (fd, atime, mtime) { return 1; };
exports.futimesSync(1, new Date(), new Date());

exports.fsync = function (fd, callback) { return ; };
exports.fsync(1, function(){});

exports.fsyncSync = function (fd) { return 1; };
exports.fsyncSync(1);

exports.write = function (fd, n, offset, length, position, callback) { return ; };
exports.write(1, e, 1, 1, 1, function(){});

exports.writeSync = function (fd, n, offset, length, position) { return 1; };
exports.writeSync(1, e, 1, 1, 1);

exports.read = function (fd, n, offset, length, position, callback) { return ; };
exports.read(1, e, 1, 1, 1, function(){});

exports.readSync = function (fd, n, offset, length, position) { return 1; };
exports.readSync(1, e, 1, 1, 1);

exports.readFile = function (filename, options, callback) { return ; };
exports.readFile("", new Object(), function(){});

exports.readFileSync = function (filename, options) { return 1; };
exports.readFileSync("", new Object());

exports.writeFile = function (filename, data, options, callback) { return ; };
exports.writeFile("", "", new Object(), function(){});

exports.writeFileSync = function (filename, data, options) { return 1; };
exports.writeFileSync("", "", new Object());

exports.appendFile = function (filename, data, options, callback) { return ; };
exports.appendFile("", "", new Object(), function(){});

exports.appendFileSync = function (filename, data, options) { return 1; };
exports.appendFileSync("", "", new Object());

exports.watchFile = function (filename, options, callback) { return ; };
exports.watchFile("", new Object(), function(){});

exports.unwatchFile = function (filename, callback) { return ; };
exports.unwatchFile("", function(){});

exports.watch = function (filename, options, callback) { return new FSWatcher(); };
exports.watch("", new Object(), function(){});

exports.exists = function (path, callback) { return ; };
exports.exists("", function(){});

exports.existsSync = function (path) { return true; };
exports.existsSync("");

/*
 * exports.Stats
 */
function Stats() { return ; }
exports.Stats = Stats;

exports.Stats.prototype.isFile = function () { return true; };
exports.Stats.prototype.isFile();

exports.Stats.prototype.isDirectory = function () { return true; };
exports.Stats.prototype.isDirectory();

exports.Stats.prototype.isBlockDevice = function () { return true; };
exports.Stats.prototype.isBlockDevice();

exports.Stats.prototype.isCharacterDevice = function () { return true; };
exports.Stats.prototype.isCharacterDevice();

exports.Stats.prototype.isSymbolicLink = function () { return true; };
exports.Stats.prototype.isSymbolicLink();

exports.Stats.prototype.isFIFO = function () { return true; };
exports.Stats.prototype.isFIFO();

exports.Stats.prototype.isSocket = function () { return true; };
exports.Stats.prototype.isSocket();

exports.Stats.prototype.dev = 1;

exports.Stats.prototype.ino = 1;

exports.Stats.prototype.mode = 1;

exports.Stats.prototype.nlink = 1;

exports.Stats.prototype.uid = 1;

exports.Stats.prototype.gid = 1;

exports.Stats.prototype.rdev = 1;

exports.Stats.prototype.size = 1;

exports.Stats.prototype.blksize = 1;

exports.Stats.prototype.blocks = 1;

exports.Stats.prototype.atime = new Date();

exports.Stats.prototype.mtime = new Date();

exports.Stats.prototype.ctime = new Date();


exports.createReadStream = function (path, options) { return new ReadStream(); };
exports.createReadStream("", new Object());

/*
 * exports.ReadStream
 */
function ReadStream() { return ; }
exports.ReadStream = ReadStream;
exports.ReadStream.prototype = stream.Readable;


exports.createWriteStream = function (path, options) { return new WriteStream(); };
exports.createWriteStream("", new Object());

/*
 * exports.WriteStream
 */
function WriteStream() { return ; }
exports.WriteStream = WriteStream;
exports.WriteStream.prototype = stream.Writable;


/*
 * exports.FSWatcher
 */
function FSWatcher() { return ; }
exports.FSWatcher = FSWatcher;
exports.FSWatcher.prototype = event.EventEmitter;

exports.FSWatcher.prototype.close = function () { return ; };
exports.FSWatcher.prototype.close();


