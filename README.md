## nroonga

[nroonga](http://nroonga.github.com) is a library for building groonga powered nodes.
You can write your custom full-text search backend on the top of [node.js](http://nodejs.org) and [groonga](http://groonga.org).


### Requirements:

    [groonga](http://groonga.org) built with MessagePack support


### To install:

    % npm install nroonga

### To build and run tests:

    % npm install
    % npm test

### To run examples:

Super simple test script:

    % node examples/test.js

A CLI example (like groonga stand-alone mode):

    % coffee examples/prompt.coffee

A http daemon example (like groonga server mode):

    % coffee examples/server.coffee

### Examples

    var nroonga = require('nroonga');
    var db = new nroonga.Database('database');
    
    // Synchronous
    console.log(db.commandSync('status'));
    
    // Asynchronous
    db.command('status', function(error, data) {
      console.log(data);
    });

### new nroonga.Database([[path], openOnly])

Open a groonga database.

If [path] is given, create a persistent db. Otherwise, create a temporary db.

If [openOnly] is set to `true`, do not attempt to create even if open failed. Otherwise, try to create a new database.

### database.commandSync(command)

Send `command` to groonga. Block until results returned.

### database.command(command, [options], callback)

Asynchronously send `command` to groonga. Callback will be given two arguments `(error, data)`.

### License

LGPL 2.1 or later. See license/lgpl-2.1.txt.
(Yoji Shidara has a right to change the license including contributed patches.)

