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
    # unpack notes
    notes = [0] * 128
    midi_blob = args[0]
    for i in range(0, 16):
        for j in range(0, 8):
            if midi_blob[i] & (1<<j) :
                notes[(i * 8) + j] = 1
            else :
                notes[(i * 8) + j] = 0
    # print str(midi_blob)
    line = ''
    for i in range(0, 128):
        line = line + str(notes[i])
    
    print line

# register method taking an int and a float
server.add_method("/mblob", 'b', blob_callback)

# loop and dispatch messages every 100ms
count = 0
while True:
    stopwatch = time.time()
    liblo.send(target, "/nf", 1)
    count += 1
    liblo.send(target, "/led", count % 8)
    while (server.recv(1)):
        pass
    time.sleep(.1)

