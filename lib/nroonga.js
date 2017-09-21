'use strict'

const nroonga = module.exports = require('../build/Release/nroonga_bindings.node')

const optionsToCommandString = (command, options) => {
  const args = [command]

  if (options) {
    for (let key in options) {
      if (options[key] != null) {
        args.push('--' + key)
        args.push(JSON.stringify(options[key]))
      }
    }
  }
  return args.join(' ')
}

const overrideOutputType = (optionsGiven, type) => {
  const options = {}
  for (let key in optionsGiven) {
    options[key] = optionsGiven[key]
  }
  options.output_type = type
  return options
}

nroonga.Database.prototype.commandSync = function (command, options) {
  const overridedOptions = overrideOutputType(options || {}, 'json')
  const commandString = optionsToCommandString(command, overridedOptions)

  return this.commandSyncString(commandString)
}

nroonga.Database.prototype.command = function (command, options, callback) {
  if (arguments.length === 2) {
    callback = options
    options = {}
  }

  const overridedOptions = overrideOutputType(options, 'json')
  const commandString = optionsToCommandString(command, overridedOptions)

  return this.commandString(commandString, callback)
}
