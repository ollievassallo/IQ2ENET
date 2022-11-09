from tkinter import *
from tkinter import filedialog
from class_ui import UI_Comms, UI_Flow,  UI_Logging
from ping import ping

def OpenComms(ui_comms, btn):
    if(ui_comms.getComms_State() == False):
        # COMMS are Closed, Now we will open them
        target_ip = ui_comms.getIP_MCU().get()
        if(target_ip == ""):
            ui_comms.setStatus_Comms("Missing IP Address")
            return
        
        ui_comms.setStatus_Comms("Verifying IP Address ... ")
        if(ping(target_ip) == False):
            ui_comms.setStatus_Ping("Target not reached")
            ui_comms.setStatus_Comms("Connection Closed")
            return
        
        ui_comms.setStatus_Ping("Target reached")
        ui_comms.setStatus_Comms("Connection Open")
        ui_comms.setComms_State(True)
        btn["text"] = "Close Comms"
    else:
        # COMMS are Open, Now we will close them
        ui_comms.setComms_State(False)
        ui_comms.setStatus_Comms("Connection Closed")
        ui_comms.setStatus_Ping(" ")
        btn["text"] = "Open Comms"

def Flow_Prev(ui_comms, ui_flow):
    print("Going to Previous")

def Flow_Next(ui_comms, ui_flow):
    print("Going to Next")

def Logging_OpenFolder(ui_comms, ui_flow, ui_logging):
    
    path = filedialog.askdirectory()
    
    if(str(path) == ""):
        return

    ui_logging.getPath().delete(0, END)
    ui_logging.getPath().insert(0, str(path))

def Create_UI():
    root = Tk()
    root.title("IQ-Ethernet Terminal")

    # Create the Frame and container for details related to the ETHERNET connection between the PC and the MCU
    frame_comms = LabelFrame(root, text="Ethernet Settings")
    frame_comms.grid(row=0, column = 0, columnspan = 2, padx = 10, pady = 5, sticky = "w")

    label_ip_pc     = Label(frame_comms, width = 15, text = "PC IP Address")
    label_ip_mcu    = Label(frame_comms, width = 15, text = "MCU IP Address")
    label_port_pc   = Label(frame_comms, width = 15, text = "PC Port Address")
    label_port_mcu  = Label(frame_comms, width = 15, text = "MCU Port Address")
    label_state     = Label(frame_comms, width = 15, text = "Comms. State")
    label_ping      = Label(frame_comms, width = 15, text = "Ping State")

    entry_ip_pc         = Entry(frame_comms, width = 30)
    entry_ip_mcu        = Entry(frame_comms, width = 30)
    entry_port_pc       = Entry(frame_comms, width = 30)
    entry_port_mcu      = Entry(frame_comms, width = 30)
    entry_status_ping   = Entry(frame_comms, width = 30)
    entry_status_coms   = Entry(frame_comms, width = 30)

    label_ip_pc.grid    (row = 0, column = 0, padx = 10, pady = 5, sticky = "w")
    label_ip_mcu.grid   (row = 1, column = 0, padx = 10, pady = 5, sticky = "w")
    label_port_pc.grid  (row = 0, column = 2, padx = 10, pady = 5, sticky = "w")
    label_port_mcu.grid (row = 1, column = 2, padx = 10, pady = 5, sticky = "w")
    label_state.grid    (row = 2, column = 0, padx = 10, pady = 5, sticky = "w")
    label_ping.grid     (row = 3, column = 0, padx = 10, pady = 5, sticky = "w")

    entry_ip_pc.grid        (row = 0, column = 1, padx = 10, pady = 5, sticky = "w")
    entry_ip_mcu.grid       (row = 1, column = 1, padx = 10, pady = 5, sticky = "w")
    entry_port_pc.grid      (row = 0, column = 3, padx = 10, pady = 5, sticky = "w")
    entry_port_mcu.grid     (row = 1, column = 3, padx = 10, pady = 5, sticky = "w")
    entry_status_coms.grid  (row = 2, column = 1, padx = 10, pady = 5, sticky = "w")
    entry_status_ping.grid  (row = 3, column = 1, padx = 10, pady = 5, sticky = "w")

    ui_comms = UI_Comms(entry_ip_pc, entry_ip_mcu, entry_port_pc, entry_port_mcu, entry_status_ping, entry_status_coms)

    button_open  = Button(frame_comms, text = "Open Comms",  width = 15, command = lambda: OpenComms(ui_comms, button_open))
    button_open.grid(row = 2, column= 3, columnspan = 2, padx = 10, pady = 5, sticky = "w")

    # Create the Frame and container for the details related to controlling the flow of the program
    frame_flow = LabelFrame(root, text="Logger System Flow Control")
    frame_flow.grid(row=1, column = 0, padx = 10, pady = 5, sticky = "w")

    entry_current= Entry(frame_flow, width = 30)

    ui_flow = UI_Flow(entry_current)
    
    button_prev  = Button(frame_flow, text = "Previous State",  width = 15, command = lambda: Flow_Prev(ui_comms, ui_flow))
    button_next  = Button(frame_flow, text = "Next State",  width = 15, command = lambda: Flow_Next(ui_comms, ui_flow))

    button_prev.grid    (row = 0, column = 0, padx = 10, pady = 5, sticky = "w")
    entry_current.grid  (row = 0, column = 1, padx = 10, pady = 5, sticky = "w")
    button_next.grid    (row = 0, column = 2, padx = 10, pady = 5, sticky = "w")

    # Create the Frame and container for the details related to Logging Data
    frame_logging = LabelFrame(root, text="Logger System Flow Control")
    frame_logging.grid(row=1, column = 1, padx = 10, pady = 5, sticky = "w")

    label_path      = Label(frame_logging, width = 15, text = "Folder Location") 
    entry_path      = Entry(frame_logging, width = 30)

    ui_logging  = UI_Logging(entry_path)
    
    button_setPath  = Button(frame_logging, text = "Set Save Location",  width = 15, command = lambda: Logging_OpenFolder(ui_comms, ui_flow, ui_logging))

    label_path.grid     (row = 0, column = 0, padx = 10, pady = 5, sticky = "w")
    entry_path.grid     (row = 0, column = 1, padx = 10, pady = 5, sticky = "w")
    button_setPath.grid (row = 1, column = 0, padx = 10, pady = 5, sticky = "w")

    return {ui_comms, ui_flow, ui_logging}

if __name__ == "__main__":
    Create_UI()
