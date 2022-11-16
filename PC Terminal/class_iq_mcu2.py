import  os
import  socket
import  threading
import  time
from    threading   import Thread, Event
from    time        import sleep

BUFFER_SIZE = 2048

class IQ_MCU:
    BYTE_FORMAT = "utf-8"

    # Default Members and state
    port_open       = False
    current_state   = "WAITING"
    socket          = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    event           = Event()
    filepath        = ""
    current_date    = 0
    current_min     = 0
    command_ready   = False
    command_error   = False
    
    def __init__(self, addr, port):
        self.ip     = addr
        self.port   = port
        self.addr   = (self.ip, self.port)
        

    def setAddr(self, ip, port):
        self.ip = ip
        self.port = port
        self.addr = (self.ip, self.port)    

    def setFilePath(self, path):
        self.filepath = path + "//"

    def openPort(self):
        if(self.port_open == False):
            self.port_open  = True
            self.rx_thread  = Thread(target = self.rx_data_handler)
            
            self.socket.connect(self.addr)
            self.rx_thread_start()

    def closePort(self):
        if(self.port_open == True):
            self.setState_Waiting()
            self.socket.close()
            self.port_open = False
            self.rx_thread_stop()
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def rx_thread_start(self):
        if(self.port_open == False):
            return "[THREAD] Port Closed"
        
        if(self.rx_thread.is_alive() == True):
            return "[THREAD] Already Running"
        
        t = time.localtime(time.time())
        self.current_min = t.tm_min
        self.rx_thread.start()
        return "[THREAD] Started Successfully"
    
    def rx_thread_stop(self):
        if(self.rx_thread.is_alive() == False):
            return "[THREAD] Not Running"

        self.event.set()
        self.rx_thread.join()
        return "[THREAD] Stopped Successfully"

    def rx_data_handler(self):
        if(self.rx_thread.is_alive() == False):
            return "[THREAD] Not Running"

        # Wait until an EVENT is triggered through the RX_STOP function
        while(self.event.is_set() == False):
            # Wait for data to load into the buffer
            buffer = self.socket.recv(BUFFER_SIZE)

            # Data is received, call the appropriate function
            if(self.current_state == "WAITING"):
                self.state_rx_handler_waiting(buffer)
            elif(self.current_state == "IDLE"):
                self.state_rx_handler_idle(buffer)
            elif(self.current_state == "LOGGING"):
                self.state_rx_handler_logging(buffer)

    def rx_string_format(self, data):
        # Get the first 20 characters of the string
        state = data[0:20]
        # Convert the reply to a string
        state_string = state.decode(self.BYTE_FORMAT)
        
        # Find the [ and ] in this string and extract the reply
        return state_string[state_string.find("[")+2:state_string.find("]")-1]

        

    def state_rx_handler_waiting(self, data):

        # Is the reply as expected?
        if(self.rx_string_format(data) == "STATE - Idle"):
            # Data Received Successfully
            self.command_error = False
        else:
            # Something went wrong when receiving the data
            self.command_error = True
            
        # Command Proccessed
        self.command_ready = True

    def state_rx_handler_idle(self, data):
        # Get string reply
        reply = self.rx_string_format(data)
        
        # Is the reply as expected?
        if(reply == "STATE - Logging" or reply == "STATE - Waiting"):
            # Data Received Successfully
            self.command_error = False
        else:
            # Something went wrong when receiving the data
            self.command_error = True
            
        # Command Proccessed
        self.command_ready = True


    def state_rx_handler_logging(self, data):
        # Is the reply as expected?
        if(self.rx_string_format(data) == "STATE - Idle"):
            # Data Received Successfully
            self.command_error = False
        else:
            t = time.localtime(time.time())
            subfolder = f"{t.tm_year:04d}.{t.tm_mon:02d}.{t.tm_mday:02d}//"
            if(t.tm_mday != self.current_date):
                self.current_date = t.tm_mday
                path_to_check = os.path.isdir(self.filepath + subfolder)
                if not(path_to_check):
                    os.makedirs(self.filepath + subfolder)
                
            if(t.tm_min != self.current_min):
                self.current_min = t.tm_min

            filename = self.filepath + subfolder + f"{t.tm_hour:02d}.{t.tm_min:02d}" + ".bin"
            
            # Convert the Values to an Integer list
            int_list = [int(i) for i in data]
            
            # Write the data into a file, with a new line for each received UDP Datagram
            with open(filename, "a") as bin_file:
                bin_file.write(str(int_list))
                bin_file.write("\n")
            
        # Command Proccessed
        self.command_ready = True

    def sendCommand(self, command):
        message = command.encode(self.BYTE_FORMAT)
        self.command_ready = False
        self.socket.sendto(message, self.addr)

        while self.command_ready == False:
            # Wait for the data to come
            pass

        if(self.command_error == True):
            return f"[CMD STATE] Command {command} failed"
        return  f"[CMD STATE] Command {command} was successful"
        
    
    def setState_Waiting(self):
        if(self.port_open == True):
            # Port is open
            if(self.current_state == "LOGGING"):
                # Go from LOGGING to IDLE through the command 'STOP'
                self.sendCommand("STOP")
                self.current_state = "IDLE"

            if(self.current_state == "IDLE"):
                # Go from IDLE to WAITING through the command 'RESET'
                self.sendCommand("RESET")
                self.current_state = "WAITING"

    def setState_Idle(self):
        if(self.port_open == True):
            # Port is open
            if(self.current_state == "LOGGING"):
                # Go from LOGGING to IDLE through the command 'STOP'
                self.sendCommand("STOP")
                self.current_state = "IDLE"

            if(self.current_state == "WAITING"):
                # Go from WAITING to IDLE through the command 'START'
                self.sendCommand("CONFIG")
                self.current_state = "IDLE"

    def setState_Logging(self):
        if(self.port_open == True):
            # Port is open
            if(self.current_state == "WAITING"):
                # Go from WAITING to IDLE through the command 'CONFIG'
                self.sendCommand("CONFIG")
                self.current_state = "IDLE"

            if(self.current_state == "IDLE"):
                # Go from IDLE to LOGGING through the command 'START'
                self.sendCommand("START")
                self.current_state = "LOGGING"


        
SERVER  = "192.168.1.195"
PORT    = 7
if __name__ == "__main__":
    mcu = IQ_MCU(SERVER, PORT)
    
    mcu.openPort()
    mcu.setState_Logging()

    sleep(5)
    
    mcu.setState_Waiting()
    mcu.closePort()
    
