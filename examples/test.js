Nroonga = require('../lib/nroonga')

db = new Nroonga.Database()

db.command('status', function(error, data) {
  console.log(data);
});
