/*
    SPDX-FileCopyrightText: 2017 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

.import QtQuick.LocalStorage 2.0 as Storage

function getDatabase() {
    return Storage.LocalStorage.openDatabaseSync("WelcomePageSettings", "0.1", "WelcomePageSettings", 100);
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
