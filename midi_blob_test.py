import liblo, sys
import time

# client
try:
    target = liblo.Address(4001)
except liblo.AddressError, err:
    print str(err)
    sys.exit()

# create server
try:
    server = liblo.Server(4000)
except liblo.ServerError, err:
    print str(err)
    sys.exit()

def blob_callback(path, args):
    print "received message: " + str(args)

# register method taking an int and a float
server.add_method("/mblob", 'b', blob_callback)

# loop and dispatch messages every 100ms
count = 0
while True:
    stopwatch = time.time()
    liblo.send(target, "/nf", 1)
    while (server.recv(1)):
        pass
    time.sleep(.1)


