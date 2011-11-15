Groonga = require('../build/Release/groonga.node')

module.exports =
  'get groonga status by Database#commandSync': (beforeExit, assert) ->
    db = new Groonga.Database()
    status = JSON.parse(db.commandSync('status'))
    assert.isDefined status.version

  'get groonga status by Database#command': (beforeExit, assert) ->
    db = new Groonga.Database()
    status = null
    db.command 'status', (error, data) ->
      status = JSON.parse(data)

    beforeExit ->
      assert.isDefined status.version
