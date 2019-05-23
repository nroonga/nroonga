'use strict'
if (process.env.GROONGA_PATH) {
  const path = require('path')
  process.env.PATH = process.env.GROONGA_PATH + path.sep + 'bin' + path.delimiter + process.env.PATH
}
module.exports = require('../build/Release/nroonga_bindings.node')
