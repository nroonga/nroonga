Groonga = require('../build/Release/groonga.node')
readline = require('readline')

db = if process.argv.length > 2
  new Groonga.Database(process.argv[2])
else
  new Groonga.Database()

rl = readline.createInterface(process.stdin, process.stdout)
prefix = '> '
rl.on 'line', (line) ->
  try
    data = db.commandSync line.trim()
    if data.length > 0
      result = JSON.parse(data)
      console.log(result)
  catch error
    console.log(error)

  rl.prompt()

rl.on 'close', ->
  process.exit(0)

rl.setPrompt prefix, prefix.length
rl.prompt()
