import liblo, sys
import time

stopwatch = time.time()
maxt = 0
mint = 100000

# send all messages to port 1234 on the local machine
try:
    target = liblo.Address(4001)
except liblo.AddressError, err:
    print str(err)
    sys.exit()

# create server, listening on port 1234
try:
    server = liblo.Server(4000)
except liblo.ServerError, err:
    print str(err)
    sys.exit()

def blob_callback(path, args):
    global maxt, mint
    etime = time.time() - stopwatch
    if etime > maxt :
        maxt = etime
    if etime < mint :
        mint = etime
#    print "received message: " + str(args)

# register method taking an int and a float
server.add_method("/mblob", 'b', blob_callback)

# loop and dispatch messages every 100ms
count = 0
while True:
    stopwatch = time.time()
    liblo.send(target, "/nf", 1)
    server.recv(100)
    time.sleep(.03)

    count += 1
    if count == 10 :
        count = 0
        print "max time: " + str(maxt) + "\tmin time : " + str(mint)
#        mint = 100000
#        maxt = 0

