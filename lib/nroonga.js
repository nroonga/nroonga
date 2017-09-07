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

const formatResult = (result, command) => {
  if (command === 'dump') {
    return result.toString('UTF-8')
  }
  if (result.length > 0) {
    return JSON.parse(result)
  }
  return null
}

nroonga.Database.prototype.commandSync = function (command, options) {
  const overridedOptions = overrideOutputType(options || {}, 'json')
  const commandString = optionsToCommandString(command, overridedOptions)

  const result = this.commandSyncString(commandString)
  return formatResult(result, command)
}

nroonga.Database.prototype.command = function (command, options, callback) {
  let wrappedCallback = null

  if (arguments.length === 2) {
    callback = options
    options = {}
  }

  if (callback) {
    wrappedCallback = (error, data) => {
      if (error) {
        return callback(error)
      }
      return callback(null, formatResult(data, command))
    }
  }

  const overridedOptions = overrideOutputType(options, 'json')
  const commandString = optionsToCommandString(command, overridedOptions)

  return this.commandString(commandString, wrappedCallback)
}
