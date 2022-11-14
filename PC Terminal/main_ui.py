from tkinter        import *
from tkinter        import filedialog
from tkinter        import messagebox
from class_ui       import UI_Comms, UI_Flow,  UI_Logging
from ping           import ping
from class_iq_mcu   import IQ_MCU

# Default Server and Port
SERVER  = "192.168.1.195"   
PORT    = 7

mcu = IQ_MCU(SERVER, PORT)

def OpenComms(ui_comms, ui_flow, btn):
    if(ui_comms.getComms_State() == False):
        # COMMS are Closed, Now we will open them
        target_ip = ui_comms.getIP_MCU()
        if(target_ip == ""):
            ui_comms.setStatus_Comms("Missing IP Address")
            return
        
        ui_comms.setStatus_Comms("Verifying IP Address ... ")
        if(ping(target_ip) == False):
            ui_comms.setStatus_Ping("Target not reached")
            ui_comms.setStatus_Comms("Connection Closed")
            return

        ui_comms.setStatus_Ping("Target reached")

        mcu.setAddr(ui_comms.getIP_MCU(), int(ui_comms.getPort_MCU()))
        
        mcu.openPort()
        ui_flow.setState(mcu.current_state)
        
        ui_comms.setStatus_Comms("Connection Open")
        ui_comms.setComms_State(True)
        btn["text"] = "Close Comms"
    else:
        # COMMS are Open, Now we will close them
        mcu.rx_stop()
        mcu.closePort()

        ui_comms.setComms_State(False)
        ui_comms.setStatus_Comms("Connection Closed")
        ui_comms.setStatus_Ping(" ")
        btn["text"] = "Open Comms"

def Flow_Prev(ui_comms, ui_flow):
    if(mcu.port_state == "Closed"):
        return
    print("Port State: "+str(mcu.port_state))
    if(mcu.current_state == "WAITING"):
        pass
    elif(mcu.current_state == "IDLE"):
        mcu.setState_Waiting()
    elif(mcu.current_state == "LOGGING"):
        mcu.rx_stop()
        mcu.setState_Idle()
    else:
        print('Invalid')
    ui_flow.setState(mcu.current_state)

def Flow_Next(ui_comms, ui_flow):
    if(mcu.port_state == "Closed"):
        return
    print("Port State: "+str(mcu.port_state))
    if(mcu.current_state == "WAITING"):
        mcu.setState_Idle()
    elif(mcu.current_state == "IDLE"):
        if(mcu.filepath == ""):
            messagebox.showwarning("Missing Data", "No directory selected")
        else:
            mcu.setState_Logging()
            mcu.rx_start()
    elif(mcu.current_state == "LOGGING"):
        pass
    else:
        print('Invalid')
    ui_flow.setState(mcu.current_state)
        

def Logging_OpenFolder(ui_comms, ui_flow, ui_logging):    
    path = filedialog.askdirectory()
    
    if(str(path) == ""):
        return

    ui_logging.getPath().delete(0, END)
    ui_logging.getPath().insert(0, str(path))

    mcu.setFilePath(path)

def Create_UI():
    ## ##########################################################################################
    ##
    ## Create Objects for all UI Elements except Buttons
    ##
    ## ##########################################################################################
    
    # Create the Different UI Sections
    frame_comms     = LabelFrame(root, text="Ethernet Settings")
    frame_flow      = LabelFrame(root, text="Logger System Flow Control")
    frame_logging   = LabelFrame(root, text="Logger System Save Location")
    
    # Ethernet Section
    label_ip_pc     = Label(frame_comms, width = 15, text = "PC IP Address")
    label_ip_mcu    = Label(frame_comms, width = 15, text = "MCU IP Address")
    label_port_pc   = Label(frame_comms, width = 15, text = "PC Port Address")
    label_port_mcu  = Label(frame_comms, width = 15, text = "MCU Port Address")
    label_state     = Label(frame_comms, width = 15, text = "Comms. State")
    label_ping      = Label(frame_comms, width = 15, text = "Ping State")

    entry_ip_mcu        = Entry(frame_comms, width = 30)
    entry_port_mcu      = Entry(frame_comms, width = 30)
    entry_status_ping   = Entry(frame_comms, width = 30, justify='center')
    entry_status_coms   = Entry(frame_comms, width = 30, justify='center')

    # Flow Section    
    entry_current = Entry(frame_flow, width = 30, justify='center')
    entry_current.configure(state = "disable")
    
    # Logging Section
    label_path      = Label(frame_logging, width = 15, text = "Folder Location") 
    entry_path      = Entry(frame_logging, width = 30, justify='center')

    ## ##########################################################################################
    ##
    ## Create UI Objects
    ##
    ## ##########################################################################################
    ui_comms    = UI_Comms(entry_ip_mcu, entry_port_mcu, entry_status_ping, entry_status_coms)
    ui_flow     = UI_Flow(entry_current)
    ui_logging  = UI_Logging(entry_path)

    ## ##########################################################################################
    ##
    ## Create All buttons
    ##
    ## ##########################################################################################

    # Ethernet Section
    button_open  = Button(frame_comms, text = "Open Comms",  width = 15, command = lambda: OpenComms(ui_comms, ui_flow, button_open))
    # Flow Section
    button_prev  = Button(frame_flow, text = "Previous State",  width = 15, command = lambda: Flow_Prev(ui_comms, ui_flow))
    button_next  = Button(frame_flow, text = "Next State",  width = 15, command = lambda: Flow_Next(ui_comms, ui_flow))
    # Logging Section
    button_setPath  = Button(frame_logging, text = "Set Save Location",  width = 15, command = lambda: Logging_OpenFolder(ui_comms, ui_flow, ui_logging))


    ## ##########################################################################################
    ##
    ## Layout the Entire UI
    ##
    ## ##########################################################################################

    # Frame Layout
    frame_comms.grid    (row=0, column = 0, padx = 10, pady = 5, sticky = "w")
    frame_flow.grid     (row=1, column = 0, padx = 10, pady = 5, sticky = "w")
    frame_logging.grid  (row=2, column = 0, padx = 10, pady = 5, sticky = "w")

    # Ethernet Section
    label_ip_mcu.grid       (row = 0, column = 0, padx = 10, pady = 5, sticky = "w")
    label_port_mcu.grid     (row = 1, column = 0, padx = 10, pady = 5, sticky = "w")
    label_state.grid        (row = 2, column = 0, padx = 10, pady = 5, sticky = "w")
    label_ping.grid         (row = 3, column = 0, padx = 10, pady = 5, sticky = "w")
    entry_ip_mcu.grid       (row = 0, column = 1, padx = 10, pady = 5, sticky = "w")
    entry_port_mcu.grid     (row = 1, column = 1, padx = 10, pady = 5, sticky = "w")
    entry_status_coms.grid  (row = 2, column = 1, padx = 10, pady = 5, sticky = "w")
    entry_status_ping.grid  (row = 3, column = 1, padx = 10, pady = 5, sticky = "w")
    button_open.grid        (row = 0, column= 2, padx = 10, pady = 5, sticky = "w")

    # Flow Section
    button_prev.grid    (row = 0, column = 0, padx = 10, pady = 5, sticky = "w")
    entry_current.grid  (row = 0, column = 1, padx = 10, pady = 5, sticky = "w")
    button_next.grid    (row = 0, column = 2, padx = 10, pady = 5, sticky = "w")

    # Logging Section
    label_path.grid     (row = 0, column = 0, padx = 10, pady = 5, sticky = "w")
    entry_path.grid     (row = 0, column = 1, padx = 10, pady = 5, sticky = "w")
    button_setPath.grid (row = 0, column = 2, padx = 10, pady = 5, sticky = "w")

    # Write Default Settings
    entry_ip_mcu.insert(0, SERVER)
    entry_port_mcu.insert(0, PORT)

    return {ui_comms, ui_flow, ui_logging}


root = Tk()
root.title("IQ-Ethernet Terminal")
Create_UI()
