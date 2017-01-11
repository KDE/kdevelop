/* KDevelop
 *
 * Copyright 2017 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

.import QtQuick.LocalStorage 2.0 as Storage

function getDatabase() {
    return Storage.LocalStorage.openDatabaseSync("WelcomePage", "0.1", "HelloAppDatabase", 100);
}

function createTable(tx) {
    tx.executeSql('CREATE TABLE IF NOT EXISTS settings(setting TEXT UNIQUE, value TEXT)');
}

function set(setting, value) {
    var db = getDatabase();
    var res = false;
    db.transaction(function(tx) {
        createTable(tx);
        var rs = tx.executeSql('INSERT OR REPLACE INTO settings VALUES (?,?);', [setting,value]);
        if (rs.rowsAffected > 0) {
            res = true;
        } else {
            res = false;
        }
    });
    return res;
}

function get(setting, defaultValue) {
    var db = getDatabase();
    var res = "";
    try {
        db.transaction(function(tx) {
            createTable(tx);
            var rs = tx.executeSql('SELECT value FROM settings WHERE setting=?;', [setting]);
            if (rs.rows.length > 0) {
                res = rs.rows.item(0).value;
            } else {
                res = defaultValue;
            }
        });
    } catch (err) {
        console.log("Database error:" + err);
        res = defaultValue;
    };
    return res
}
