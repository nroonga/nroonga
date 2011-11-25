express = require('express')
nroonga = require('../lib/nroonga')

db = if process.argv.length > 2
  new nroonga.Database(process.argv[2])
else
  new nroonga.Database()

port = 3000

app = express.createServer()
app.get '/d/:command', (req, res) ->
  db.command req.params.command, req.query, (error, data) ->
    if error?
      res.send(error.toString() + "\n", 400)
    else
      res.send(data)

app.listen(port)
console.log "Server listining at port #{port}."
