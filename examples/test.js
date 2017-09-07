'use strict'

const nroonga = require('../lib/nroonga')

const db = new nroonga.Database()

db.command('status', (_, data) => {
  console.log(data)
})
