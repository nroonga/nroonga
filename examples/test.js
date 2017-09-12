'use strict'

const nroonga = require('../lib/nroonga')
const db = new nroonga.Database()

// Synchronous
console.log('// Synchronous')
console.log(db.commandSync('status'))

// Asynchronous
db.command('status', (_, data) => {
  console.log('// Asynchronous')
  console.log(data)
})
