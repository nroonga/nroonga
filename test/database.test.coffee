nroonga = require('../lib/nroonga')

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
