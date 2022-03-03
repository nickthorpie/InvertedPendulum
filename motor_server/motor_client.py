import subprocess
import threading
import sys
import os, tempfile
from time import sleep

# Update the c++ reader script before running. Take this out once happy with the final product
subprocess.run(["g++ ./motor_client.cpp -o ./motor_client"],shell=True)
# motor = Motor_Helper_Multiprocessing(direction, step, (21,21,21), "A4988",min_stepdelay=0.0005)



class motor_client():
    def __init__(self,dir_pin,step_pin,enable_pin,freq = 10000):
        self.dir_pin = dir_pin
        self.step_pin = step_pin
        self.enable_pin = enable_pin
        self.freq = freq

        self.init_pipe()

    def init_pipe(self):
        tmpdir = tempfile.mkdtemp()
        filename = os.path.join(tmpdir, "myfifo")
        try:
            os.mkfifo(filename)
        except OSError as e:
            print("Failed to create FIFO: %s" % e)
        else:
            # subprocess.call([f"./reader {filename}"])
            step_pin = 1
            dir_pin = 2
            freq = 1000
            self.p = subprocess.Popen([f'./motor_client {filename} {self.step_pin} {self.dir_pin} {self.freq}'], shell=True)

            fifo = open(filename, 'w')
            fifo.write("0\0")
            self.fifo = fifo
            print("Pipeline Opened on Python Side")

    def send_command(self,value):
        assert type(value) == int, "command must be type int"
        self.fifo.write(f"{value}\0")
        self.fifo.flush()

    def close_pipe(self):
        fifo.close()
        os.remove(filename)
        os.rmdir(tmpdir)
        print("Successful exit")


# tmpdir = tempfile.mkdtemp()
# filename = os.path.join(tmpdir,"myfifo")
# print(filename)
# try:
#     os.mkfifo(filename)
# except OSError as e:
#     print("Failed to create FIFO: %s" % e)
# else:
#     # subprocess.call([f"./reader {filename}"])
#     step_pin = 1
#     dir_pin = 2
#     freq = 1000
#     subprocess.Popen([f'./reader {filename} {step_pin} {dir_pin} {freq}'],shell=True)
#
#     fifo = open(filename, 'w')
#     print("MADE IT HERE")
#     fifo.write("0\0")
#     # write stuff to fifo
#     sleeptime = 0.0001
#     # sleep(1)
#     while(True):
#         # print("Sending hello")
#         # print("hello",file=fifo)
#         fifo.write("1\0")
#         fifo.flush()
#         sleep(sleeptime)
#         # print("Sending world")
#         fifo.write("0\0")
#         fifo.flush()
#         sleep(sleeptime)

# finally:
#     fifo.close()
#     os.remove(filename)
#     os.rmdir(tmpdir)
#     print("Successful exit")

# writer_thread = threading.Thread(target=writer)
# writer_thread.daemon = True
# writer_thread.start()
# print("Started Daemon")
# #CALL THE READER PROCESS
# subprocess.call(["./reader"])

# sys.stdout.write("Done\n")