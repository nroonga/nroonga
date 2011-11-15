Nroonga = require('../build/Release/nroonga.node')

module.exports =
  'get groonga status by Database#commandSync': (beforeExit, assert) ->
    db = new Nroonga.Database()
    status = JSON.parse(db.commandSync('status'))
    assert.isDefined status.version

  'get groonga status by Database#command': (beforeExit, assert) ->
    db = new Nroonga.Database()
    status = null
    db.command 'status', (error, data) ->
      status = JSON.parse(data)

    beforeExit ->
      assert.isDefined status.version
