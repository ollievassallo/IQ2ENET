from tkinter import *

class UI_Comms:
    def __init__(self, ip_pc, ip_mcu, port_pc, port_mcu, ping, comms):
        self.comms_open = False
        self.ip_pc      = ip_pc
        self.ip_mcu     = ip_mcu
        self.port_pc    = port_pc
        self.port_mcu   = port_mcu
        self.status_ping    = ping
        self.status_comms   = comms
        self.status_ping.configure(state="disabled")
        self.status_comms.configure(state="disabled")

    def setIP_PC(self, obj):
        self.ip_pc = obj

    def setIP_MCU(self, obj):
        self.ip_mcu = obj

    def setPort_PC(self, obj):
        self.port_pc = obj

    def setPort_MCU(self, obj):
        self.port_mcu = obj

    def setComms_State(self, state):
        self.comms_open = state

    def setStatus_Ping(self, new_state):
        self.status_ping.configure(state = "normal")
        self.status_ping.delete(0, END)
        self.status_ping.insert(0, new_state)
        self.status_ping.configure(state = "disable")

    def setStatus_Comms(self, new_state):
        self.status_comms.configure(state = "normal")
        self.status_comms.delete(0, END)
        self.status_comms.insert(0, new_state)
        self.status_comms.configure(state = "disable")

    def getCommsState(self):
        return self.comms_open

    def getIP_PC(self):
        return self.ip_pc
    
    def getIP_MCU(self):
        return self.ip_mcu
    
    def getPort_PC(self):
        return self.port_pc

    def getPort_MCU(self):
        return self.port_mcu

    def getComms_State(self):
        return self.comms_open


class UI_Flow:
    def __init__(self, current_state):
        self.current_state = current_state

class UI_Logging:
    def __init__(self, path):
        self.path = path

    def setPath(self, path):
        self.path = path

    def getPath(self):
        return self.path
    
