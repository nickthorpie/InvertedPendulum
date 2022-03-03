import subprocess
import threading
import sys
import os, tempfile
from time import sleep

# Update the c++ reader script before running. Take this out once happy with the final product
subprocess.run(["g++ ./reader.cpp -o ./reader"],shell=True)

print("wrote execs")
def writer():
    #CALL THE WRITER PROCESS
    subprocess.call(["./writer"])



tmpdir = tempfile.mkdtemp()
filename = os.path.join(tmpdir,"myfifo")
print(filename)
try:
    os.mkfifo(filename)
except OSError as e:
    print("Failed to create FIFO: %s" % e)
else:
    # subprocess.call([f"./reader {filename}"])
    step_pin = 1
    dir_pin = 2
    freq = 100
    subprocess.Popen([f'./reader {filename} {step_pin} {dir_pin} {freq}'],shell=True)

    fifo = open(filename, 'w')
    print("MADE IT HERE")
    fifo.write("0\0")
    # write stuff to fifo
    sleeptime = 0.0001
    # sleep(1)
    while(True):
        # print("Sending hello")
        # print("hello",file=fifo)
        fifo.write("1\0")
        fifo.flush()
        sleep(sleeptime)
        # print("Sending world")
        fifo.write("0\0")
        fifo.flush()
        sleep(sleeptime)

finally:
    fifo.close()
    os.remove(filename)
    os.rmdir(tmpdir)
    print("Successful exit")

# writer_thread = threading.Thread(target=writer)
# writer_thread.daemon = True
# writer_thread.start()
# print("Started Daemon")
# #CALL THE READER PROCESS
# subprocess.call(["./reader"])

sys.stdout.write("Done\n")