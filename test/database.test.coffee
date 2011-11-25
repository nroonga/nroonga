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
