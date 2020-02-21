import { Database } from '..'

const db = new Database()

// Synchronous
console.log('// Synchronous')
console.log(db.commandSync('status'))

// Asynchronous
db.command('status', (_, data) => {
  console.log('// Asynchronous')
  console.log(data)
})
