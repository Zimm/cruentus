# CRUENTUS
Cruentus is a next generation web server (lulz not really i suck at coding)

Cruentus was created because:

1. I wanted to learn many new programming ideas
2. I was tired of using a server that I didn't know the code to
3. I feel like the other servers are much too bloated for what I wanted

## CAPABILITIES
Cruentus can be anything from an echo server to an http server.
It uses threading or new processes (your choice, I prefer threading... i guess you could do both... but thatd be lamesauce) to communicate with a function. It sends the socket descriptor to the new thread (or process) and the function can then read or write or anything to it. This allows you to create any sort of server you want, just create one function and youre off! Possibilities are endless, you could just have the function be a wrapper to other languages, to other functions or anything your little heart desires.

## LIBCRUENTUS
libcruentus is the main library I use for anything having to do with the server. Currently it only has a Socket class. This helps for creating plugins to he web server which I shall talk about next

## HTTP SERVER
* The http server that I built in can act as a normal http server where it looks for html and css files and such and serves them up.
* It also supports dynamic html pages through the use of cruxes
* Finally it supports full on plugins in any language that supports unix sockets

## CRUXES
Cruxes are plugins for cruentus. There are two types so far:

1. HTML cruxes
An HTML crux supports dynamically created web pages. Based on what is put in the address bar similar to how php works (http://smth.asdf/asdf.php?asdf) will show up in the page. An example page is in the repo, index.html. Basically you add <*smth*> to the html file, the default value is smth but if you add ?asdf to the end of the url then smth will be replaced with asdf.
EX: http://asdf.asdf.asdf.hcrux?poop
EX2: http://asdf.asdf.asdf/?poop it automatically looks for index files so you can just add the ?blah to the end after the /!
2. Socket cruxes
You create a directory with the extension .crux on it. You have your program create a unix sock at ./sock in said directory. The server will pass requests to that directory through that socket. It will listen for a response from your program and send it back to the client. NOTE it sends the full request to the plugin, not just the file requested or something of the such
EXAMPLE IN ACTION: hello.indomit.us 
You can do hello.indomit.us/?WORLD ;)

## FEATURES
You can have directives, aka for noobs like me a file that tells the server what file path to look for files for a certain url. it defaults to the path that you start the server in if you don't have a conf file.

## INSTRUCTIONS
Coming soon... gotta go pillage a village

