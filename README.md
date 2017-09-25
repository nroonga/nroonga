## nroonga

[![Build Status](https://secure.travis-ci.org/nroonga/nroonga.svg?branch=master)](http://travis-ci.org/nroonga/nroonga)

[nroonga](http://nroonga.github.com) is a library for building groonga powered nodes.
You can write your custom full-text search backend on the top of [node.js](http://nodejs.org) and [groonga][].


### Requirements:

* [groonga][]
* [pkg-config][]

  [groonga]:http://groonga.org
  [pkg-config]:http://www.freedesktop.org/wiki/Software/pkg-config

#### For debian and ubuntu users

If you are using debian or ubuntu, the simplest way to install is to use packages. [Install instructions][groonga-install] of groonga is available at the groonga website. Follow one of these:

* [Debian GNU/Linux](http://groonga.org/docs/install/debian.html)
* [Ubuntu](http://groonga.org/docs/install/ubuntu.html)

NOTE: You need to install `libgroonga-dev` package in order to install nroonga.

  [groonga-install]:http://groonga.org/docs/install.html

### To install:

After groonga installed, just do

    % npm install nroonga

### To build and run tests:

    % npm install
    % npm test

### To run examples:

Super simple test script:

    % node examples/test.js

A CLI example (like groonga stand-alone mode):

    % coffee examples/prompt.coffee

### Examples

```javascript
var nroonga = require('nroonga');
var db = new nroonga.Database('database');

// Synchronous
console.log(db.commandSync('status'));

// Asynchronous
db.command('status', function(error, data) {
  console.log(data);
});
```

### new nroonga.Database([[path], openOnly])

Open a groonga database.

If [path] is given, create a persistent db. Otherwise, create a temporary db.

If [openOnly] is set to `true`, do not attempt to create even if open failed. Otherwise, try to create a new database.

### database.commandSync(command)

Send `command` to groonga. Block until results returned.

### database.command(command, [options], callback)

Asynchronously send `command` to groonga. Callback will be given two arguments `(error, data)`.

### database.close()

Close database. After `close` called, any API calls for the database raise an exception.

### License

LGPL 2.1 or later. See license/lgpl-2.1.txt.
(Yoji Shidara has a right to change the license including contributed patches.)

