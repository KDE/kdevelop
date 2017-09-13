var m = require("module");
var m2 = require("./module2");
var not_found = require("this_module_doesn_exist"); // Don't loop forever in BackgroundParser
/* */
var tls = require("tls");
var url = require("url");

/**
 * "type" : {"toString" : "bool" }
 */
var a = m.bar(4);

/**
 * "type" : { "toString" : "string" }
 */
var b = m2.greetings;

/**
 * "type" : { "toString" : "CleartextStream" }
 */
var stream = new tls.CleartextStream();
var cert = stream.getPeerCertificate();
var u = url.parse("http://www.kde.org:80");

/**
 * "type" : { "toString" : "string" }
 */
var CN = cert.issuer.CN;

/**
 * "type" : { "toString" : "int" }
 */
var port = u.port;
