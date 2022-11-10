import socket
from threading import Thread, Event
import threading

from time import sleep

class IQ_MCU:
    BYTE_FORMAT = "utf-8"

    def __cmd_send(self, message, expected_reply):
        if(self.port_state == "Closed"):
            return -1
        else:
            # ENCODE Message into the correct format
            message = message.encode(self.BYTE_FORMAT)
            # Send the command
            self.socket.sendto(message, (self.addr))
            while True:
                # Wait for reply back
                data = self.socket.recv(1024)

                # Make sure reply is as expected
                data = data.decode(self.BYTE_FORMAT)
                data = data[data.find("[")+2:data.find("]")-1]
                if(data == expected_reply):
                    # Command was received sucessfully
                    return 0
                else:
                    # Command was wrong
                    return -1
    
    def __init__(self, addr, port):
        self.port_state     = "Closed"
        self.current_state  = "WAITING"
        self.ip             = addr
        self.port           = port
        self.socket         = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.addr           = (self.ip, self.port)
        self.event          = Event()
        self.rx_thread      = Thread(target = self.receive_data, args=[self.event])

    def setAddr(self, ip, port):
        self.ip = ip
        self.port = port
        self.addr = (self.ip, self.port)    
      
    def openPort(self):
        if(self.port_state == "Closed"):
            self.port_state = "Open"
            self.socket.connect( (self.addr) )
        return "PORT OPENED"

    def closePort(self):
        if(self.port_state == "Open"):
            cmd_state = self.setState_Waiting()
            if(cmd_state == "SUCCESS"):
                self.socket.close()
                self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            else:
                return "FAILED TO CLOSE\n\t"+str(cmd_state)
        return "PORT CLOSED"

    def cmd_send_config(self):
        self.__cmd_send("CONFIG", "STATE - Idle")
        self.current_state = "IDLE"


    def cmd_send_start(self):
        self.__cmd_send("START", "STATE - Logging")
        self.current_state = "LOGGING"        

    def cmd_send_stop(self):
        self.__cmd_send("STOP", "STATE - Idle")
        self.current_state = "IDLE"

        
    def cmd_send_reset(self):
        self.__cmd_send("RESET", "STATE - Waiting")
        self.current_state = "WAITING"
        
    def setState_Waiting(self):
        if(self.port_state == "Open"):
            # Move from LOGGING to IDLE through 'STOP' Command
            if(self.current_state == "LOGGING"):
                self.cmd_send_stop()

            # Move from IDLE to WAITING through 'RESET' Command
            if(self.current_state == "IDLE"):
                self.cmd_send_reset()

            if(self.current_state == "WAITING"):
                # MOVED to WAITING state successfully
                return "SUCCESS"
            else:
                # Failed to enter WAITING state
                return "FAILED"
        else:
            return "PORT CLOSED"

    def setState_Idle(self):
        if(self.port_state == "Open"):
        # Move from LOGGING to IDLE through 'STOP' Command
            if(self.current_state == "LOGGING"):
                self.cmd_send_stop()
                
            # Move from WAITING to IDLE through 'CONFIG' Command
            if(self.current_state == "WAITING"):
               self.cmd_send_config()

            if(self.current_state == "IDLE"):
                # MOVED to IDLE state successfully
                return "SUCCESS"
            else:
                # Failed to enter IDLE state
                return "FAILED"
        else:
            return "PORT CLOSED"

    def setState_Logging(self):
        if(self.port_state == "Open"):
        # Move from WAITING to IDLE through 'CONFIG' Command
            if(self.current_state == "WAITING"):
               self.cmd_send_config()

            # Move from IDLE to LOGGING through 'START' Command   
            if(self.current_state == "IDLE"):
               self.cmd_send_start()

            if(self.current_state == "LOGGING"):
                # MOVED to LOGGING state successfully
                return "SUCCESS"
            else:
                # Failed to enter LOGGING state
                return "FAILED"
        else:
            return "PORT CLOSED"

    def rx_start(self):
        if(self.rx_thread.is_alive() == False):
            self.event      = Event()
            self.rx_thread  = threading.Thread(target = self.receive_data, args=[self.event])
            self.rx_thread.start()

    def rx_stop(self):
        if(self.rx_thread.is_alive()):
            self.event.set()
            self.rx_thread.join()

    def receive_data(self, event):
        while self.event.is_set() == False:
            data = self.socket.recv(1024)
            print(data.decode(self.BYTE_FORMAT))


SERVER  = "192.168.1.195"
PORT    = 7
if __name__ == "__main__":
    mcu = IQ_MCU(SERVER, PORT)
    
    print(mcu.openPort())
    print(mcu.setState_Logging())

    mcu.rx_start()
    print(str(threading.active_count()))
    sleep(5)
    mcu.rx_stop()
    
    print(mcu.setState_Waiting())
    print(mcu.closePort())
    

