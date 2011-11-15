Nroonga = require('../build/Release/nroonga.node')

db = new Nroonga.Database()

db.command('status', function(error, data) {
  console.log(data);
  result = JSON.parse(data);
  console.log(result);
});
