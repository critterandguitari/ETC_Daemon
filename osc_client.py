import liblo, sys

# send all messages to port 1234 on the local machine
try:
    target = liblo.Address(4001)
except liblo.AddressError, err:
    print str(err)
    sys.exit()

liblo.send(target, "/getknobs", 1)
