Groonga = require('../build/Release/groonga.node')

db = new Groonga.Database()

db.command('status', function(error, data) {
  console.log(data);
  result = JSON.parse(data);
  console.log(result);
});
