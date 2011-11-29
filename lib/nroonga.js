(function() {
  var nroonga, optionsToCommandString;

  nroonga = module.exports = require('./nroonga_bindings.node');

  optionsToCommandString = function(command, options) {
    var args, key, value;
    args = [command];
    if (options != null) {
      for (key in options) {
        value = options[key];
        args.push('--' + key);
        args.push(JSON.stringify(value));
      }
    }
    return args.join(' ');
  };

  nroonga.Database.prototype.commandSync = function(command, options) {
    var result;
    result = this.commandSyncString(optionsToCommandString(command, options));
    if (result.length > 0) {
      return JSON.parse(result);
    } else {
      return;
    }
  };

  nroonga.Database.prototype.command = function(command, options, callback) {
    var wrappedCallback;
    if (arguments.length === 2) {
      callback = options;
      options = void 0;
    }
    wrappedCallback = callback != null ? function(error, data) {
      return callback(error, data != null ? JSON.parse(data) : void 0);
    } : void 0;
    return this.commandString(optionsToCommandString(command, options), wrappedCallback);
  };

}).call(this);
