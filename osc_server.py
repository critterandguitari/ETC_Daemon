#!/usr/bin/env python

import liblo, sys

# create server, listening on port 1234
try:
    server = liblo.Server(4000)
except liblo.ServerError, err:
    print str(err)
    sys.exit()

def blob_callback(path, args):
    print "received message: " + str(args[0][0])

# register method taking an int and a float
server.add_method("/mblob", 'b', blob_callback)

# loop and dispatch messages every 100ms
while True:
    server.recv(100)
