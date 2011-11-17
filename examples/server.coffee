express = require('express')
Nroonga = require('../build/Release/nroonga.node')

db = if process.argv.length > 2
  new Nroonga.Database(process.argv[2])
else
  new Nroonga.Database()

port = 3000

app = express.createServer()
app.get '/d/:command', (req, res) ->
  args = [req.params.command]
  for key, value of req.query
    args.push("--#{key}")
    args.push(value)

  command = args.join(' ')
  db.command command, (error, data) ->
    if error?
      res.send(error.toString() + "\n", 400)
    else
      res.send(data)

app.listen(port)
console.log "Server listining at port #{port}."
