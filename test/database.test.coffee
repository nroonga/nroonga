nroonga = require('../lib/nroonga')
fs = require('fs')

temporaryDatabase = (callback) ->
  tempdir = 'test/tmp'
  fs.mkdir tempdir, ->
    databaseName = "tempdb-#{process.pid}-#{(new Date()).valueOf()}"
    db = new nroonga.Database(tempdir + '/' + databaseName)

    try
      callback(db)
    finally
      fs.readdir tempdir, (err, files) ->
        throw err if err?
        re = RegExp('^' + databaseName)
        for file in files when file.match(re)
          fs.unlink(tempdir + '/' + file)

withTestDatabase = (callback) ->
  temporaryDatabase (db) ->
    db.commandSync 'table_create',
      name: 'Site'
      flags: 'TABLE_HASH_KEY'
      key_type: 'ShortText'
    db.commandSync 'column_create',
      table: 'Site'
      name: 'title'
      flags: 'COLUMN_SCALAR'
      type: 'ShortText'

    db.commandSync 'table_create',
      name: 'Terms'
      flags: 'TABLE_PAT_KEY|KEY_NORMALIZE'
      key_type: 'ShortText'
      default_tokenizer: 'TokenBigram'
    db.commandSync 'column_create',
      table: 'Terms'
      name: 'entry_title'
      flags: 'COLUMN_INDEX|WITH_POSITION'
      type: 'Site'
      source: 'title'

    db.commandSync 'load',
      table: 'Site'
      values: JSON.stringify [
        _key: "http://groonga.org/"
        title: "groonga - An open-source fulltext search engine and column store"
      ,
        _key: "http://groonga.rubyforge.org/"
        title: "Fulltext search by Ruby with groonga - Ranguba"
      ,
        _key: "http://mroonga.github.com/"
        title: "Groonga storage engine - Fast fulltext search on MySQL"
      ]

    callback(db)

module.exports =
  'get groonga status by Database#commandSync': (beforeExit, assert) ->
    db = new nroonga.Database()
    status = db.commandSync('status')
    assert.isDefined status.version

  'get groonga status by Database#command': (beforeExit, assert) ->
    db = new nroonga.Database()
    status = null
    db.command 'status', (error, data) ->
      status = data

    beforeExit ->
      assert.isDefined status.version

  'open database whose name is not string': (beforeExit, assert) ->
    errorThrown = null
    try
      new nroonga.Database(1)
    catch error
      errorThrown = error

    beforeExit ->
      assert.ok(errorThrown, 'No error thrown')

  'create table and store data': (beforeExit, assert) ->
    withTestDatabase (db) ->
      assert.equal 3, db.commandSync('select', table: 'Site')[0][0][0]

  'search data': (beforeExit, assert) ->
    withTestDatabase (db) ->
      matched = db.commandSync 'select',
        table: 'Site'
        match_columns: 'title'
        query: 'ruby'
      assert.equal 1, matched[0][0][0]

  'query including space': (beforeExit, assert) ->
    withTestDatabase (db) ->
      matched = db.commandSync 'select',
        table: 'Site'
        match_columns: 'title'
        query: 'search ranguba'
      assert.equal 1, matched[0][0][0]
