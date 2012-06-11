nroonga = module.exports = require('../build/Release/nroonga_bindings.node')
msgpack = require('msgpack2')

optionsToCommandString = (command, options) ->
  args = [command]
  if options?
    for key, value of options
      args.push '--' + key
      args.push JSON.stringify(value)
  args.join(' ')

overrideOutputType = (optionsGiven, type) ->
  options = {}
  for key, value of optionsGiven
    options[key] = value
  options.output_type = type
  options

formatResult = (result, command) ->
  if command == 'dump'
    result.toString('UTF-8')
  else
    msgpack.unpack(result)

nroonga.Database.prototype.commandSync = (command, options={}) ->
  options = overrideOutputType(options, 'msgpack')
  result = this.commandSyncString(optionsToCommandString(command, options))
  if result.length > 0
    formatResult(result, command)
  else
    undefined

nroonga.Database.prototype.command = (command, options, callback) ->
  if arguments.length == 2
    callback = options
    options = {}
  options = overrideOutputType(options, 'msgpack')

  wrappedCallback = if callback?
    (error, data) ->
      if error?
        callback error
      else
        callback undefined, formatResult(data, command)
  else
    undefined

  this.commandString optionsToCommandString(command, options), wrappedCallback
