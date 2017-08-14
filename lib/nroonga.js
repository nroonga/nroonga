var nroonga = module.exports = require('../build/Release/nroonga_bindings.node');

function optionsToCommandString(command, options) {
  var args, key;
  args = [command];

  if (options) {
    for (key in options) {
      if (options[key] != null) {
        args.push('--' + key);
        args.push(JSON.stringify(options[key]));
      }
    }
  }
  return args.join(' ');
}

function overrideOutputType (optionsGiven, type) {
  var key, options;
  options = {};
  for (key in optionsGiven) {
    options[key] = optionsGiven[key];
  }
  options.output_type = type;
  return options;
}

function formatResult(result, command) {
  if (command === 'dump') {
    return result.toString('UTF-8');
  } else {
    if (result.length > 0) {
      return JSON.parse(result);
    } else {
      return null;
    }
  }
}

nroonga.Database.prototype.commandSync = function(command, options) {
  var overridedOptions = overrideOutputType(options || {}, 'json');
  var commandString = optionsToCommandString(command, overridedOptions);

  var result = this.commandSyncString(commandString);
  return formatResult(result, command);
};

nroonga.Database.prototype.command = function(command, options, callback) {
  var wrappedCallback = null;
  var overridedOptions;
  var commandString;

  if (arguments.length === 2) {
    callback = options;
    options = {};
  }

  if (callback) {
    wrappedCallback = function(error, data) {
      if (error) {
        return callback(error);
      } else {
        return callback(null, formatResult(data, command));
      }
    };
  }

  overridedOptions = overrideOutputType(options, 'json');
  commandString = optionsToCommandString(command, overridedOptions);

  return this.commandString(commandString, wrappedCallback);
};
