# Gist It!

Gist It is an application to create gists from console. It allows to create
gist from input (ie, `git diff | gistit`) or from a specific file (ie,
`gistit file.txt`).

## Dependencies

- [cURL](http://curl.haxx.se)
- [jansson](https://github.com/akheron/jansson)

If you are using Ubuntu:
```shell
	sudo apt-get install libcurl4-openssl-dev libjansson-dev
```

## Usage

```shell
	# Creating a public gist from other app response
	ls | gistit

	# Creating a private gist from other app response
	ls | gistit -priv

	# Specifying the gist filename
	ls | gistit -i list.txt

	# Sending files
	gistit file.txt

	# Sending multiple files in a private gist
	gistit -priv file1.txt file2.c

	# Setting gist description
	gistit -d "This is just a sample" sample.txt

	# Setting gist description, private and with multiple files
	gistit -d "Sample" -priv file1.txt file2.txt file3.txt

	# Help
	gistit -h

	# Version
	gistit -v
```

By default the `gistit` create the GitHub Gist as anonymous, but you can configure to
associate with your account. See instructions on [http://gistit.herokuapp.com/].

## Compiling

```shell
	# Install cURL
	# Install jansson

	./autogen.sh
	make
	make install
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
