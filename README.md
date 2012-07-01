# Gist It!

Gist It is an application to create gists from console. It allows to create
gist from input (ie, `git diff | gistit`) or from a specific file (ie,
`gistit -f file.txt`).

## Compiling

```shell
	# Compile jansson
	cd jansson
	autoreconf -i
	./configure
	make

	# Compile gistit
	cd ..
	make
```

## Web Application

The web application uses [node.js](http://nodejs.org/). To run, it requires two
environment variables:

- `GITHUB_CLIENT_ID` It contains the client ID of GitHub application
- `GITHUB_SECRET_KEY` It contains the secret key of GitHub application

### Deploying

The web application can be deployed easily to [heroku](http://www.heroku.com/).

- Create the application: `heroku create <APPLICATION_NAME>`
- Push the code: `git push heroku master`
- Set the GitHub application information: `heroku config:add GITHUB_CLIENT_ID=<CLIENT_ID>`
and `heroku config:add GITHUB_SECRET_KEY=<SECRET_KEY>`
