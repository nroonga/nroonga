nroonga = module.exports = require('./nroonga_bindings.node')

optionsToCommandString = (command, options) ->
  args = [command]
  if options?
    for key, value of options
      args.push '--' + key
      args.push JSON.stringify(value)
  args.join(' ')

nroonga.Database.prototype.commandSync = (command, options) ->
  result = this.commandSyncString(optionsToCommandString(command, options))
  if result.length > 0
    JSON.parse(result)
  else
    undefined

nroonga.Database.prototype.command = (command, options, callback) ->
  if arguments.length == 2
    callback = options
    options = undefined

  wrappedCallback = if callback?
    (error, data) ->
      if error?
        callback error
      else
        callback undefined, JSON.parse(data)
  else
    undefined

  this.commandString optionsToCommandString(command, options), wrappedCallback
