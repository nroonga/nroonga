'use strict'
if (process.env.GROONGA_PATH) {
  const path = require('path')
  process.env.PATH = path.join(process.env.GROONGA_PATH, 'bin') + path.delimiter + process.env.PATH
}
module.exports = require('../build/Release/nroonga_bindings.node')
