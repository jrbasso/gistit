const express = require('express');
const path = require('path');
const http = require('https');
const PORT = process.env.PORT || 5000;

var app = express();

app.use(express.static(path.join(__dirname, 'public')));
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'pug');

app.get('/', function(req, res) {
  var githubGetToken = 'https://github.com/login/oauth/authorize?client_id=' + process.env.GITHUB_CLIENT_ID + '&scope=gist';

  if (req.query.code) {
    var ghreq = getAccessToken(req.query.code, function(ghres) {
      var ghdata = '';

      ghres.setEncoding('utf8');
      ghres.on('data', function(chunk) {
        ghdata += chunk;
      });
      ghres.on('end', function() {
        var token = null;

        ghdata = JSON.parse(ghdata);
        if (ghdata.access_token) {
          token = ghdata.access_token;
        }
        res.render('index', { githubGetToken: githubGetToken, token: token });
      });
    });
    ghreq.on('error', function(e) {
      console.log("Error getting access token: " + e.message);
      res.render('index', { githubGetToken: githubGetToken, token: null });
    });
    ghreq.end();
  } else {
    res.render('index', { githubGetToken: githubGetToken, token: null });
  }
});

app.listen(PORT, function() {
  console.log("Listening on " + PORT);
});

function getAccessToken(code, callback) {
  var options = {
    method: 'POST',
    host: 'github.com',
    port: 443,
    path: '/login/oauth/access_token?client_id=' + process.env.GITHUB_CLIENT_ID + '&client_secret=' + process.env.GITHUB_SECRET_KEY + '&code=' + code,
    headers: {
      'Accept': 'application/json'
    }
  }
  return request = http.request(options, callback);
}