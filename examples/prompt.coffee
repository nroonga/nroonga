Nroonga = require('../lib/nroonga')
readline = require('readline')

db = if process.argv.length > 2
  new Nroonga.Database(process.argv[2])
else
  new Nroonga.Database()

rl = readline.createInterface(process.stdin, process.stdout)
prefix = '> '
rl.on 'line', (line) ->
  try
    data = db.commandSync line.trim()
    if data?
      console.log(data)
  catch error
    console.log(error)

  rl.prompt()

rl.on 'close', ->
  process.exit(0)

rl.setPrompt prefix, prefix.length
rl.prompt()
