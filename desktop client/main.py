##################################################################################
#                                                                                #
#   This is the companion app for the tukii:DE. This app reads data from the     #
#   tukii and performs the actions, communication happens over the serial bus.   #
#   The tukii uses TinyUSB. Configuring the tukii from the app will send the     #
#   new config via TinyUSB, and also to the webserver, so the user config can    #
#   be saved and applied as needed. Profiles can also be made and applied using  #
#   the app. Profiles are loaded to the tukii over TinyUSB.   
# 
#   The app connects to the BeardyMike API server to get the latest weather and  #
#   crypto data, which will be sent to the tukii and shown on the screensaver.   #
#   The user needs to enter their API key to enable this feature, as the server  #
#   is restricted, and the user needs to be authenitcated.                       #
#                                                                                #
#   @Author: BeardyMike                                                          #
#                                                                                #
##################################################################################



##################################################################################
# import modules

# pip install customtkinter
import customtkinter
import serial
import serial.tools.list_ports

# pip install pystray
import pystray
import os
from PIL import Image
import configparser
import threading
import time

# Set the working directory to the directory the script is running from
os.chdir(os.path.dirname(os.path.abspath(__file__)))
##############################################################################



##############################################################################
# Global variables

# temp variable to store the previous event for the hover function
temp = ""

# Define the image for the system tray icon
image = Image.open("data/image/logo.ico")

# Set the size of the main app window
gui_X = 650
gui_Y = 450
gui_sidebar_width = 140
gui_main_frame_width = gui_X - gui_sidebar_width

# Images
tray_icon = "data/image/logo.ico"
lcd_image = "data/image/lcd.png"
logo_image = "data/image/logo.ico"
tukii_image = "data/image/img.png"

# file paths
data_ini = "data/data.ini"

##############################################################################



##############################################################################
# Functions and Commands

# Function to compare the current event to the previous event
def compare(event, temp):
    return event == temp

# Function to print to the console when a button is hovered over
# add -->    .bind("<Enter>", lambda eff: on_hover(eff="None", event="Button Name"), add='+')     <-- to something to call this function
def on_hover(eff="None", event=""):
    global temp
    if compare(event, temp) == True:
        return
    print("Hovering over " + event)
    temp = event
    return

# Function to quit the app, called when the Quit option is selected from the system tray icon menu
def quit_window():
    icon.stop()
    app.quit()

# Function to hide the window, called when the window is closed
def withdraw_window():
    app.withdraw()

# Function to toggle the window, called when the .Starter option is chosen from the system tray
def toggle_window():
    if app.state() == "withdrawn":
        app.deiconify()
    else:
        app.withdraw()

def ini_write(section, key, value):
    config = configparser.ConfigParser()
    config.read(data_ini)
    config.set(section, key, value) 
    with open(data_ini, 'w') as configfile:
        config.write(configfile)

customtkinter.set_appearance_mode("System")  # Modes: "System" (standard), "Dark", "Light"
customtkinter.set_default_color_theme("dark-blue")  # Themes: "blue" (standard), "green", "dark-blue"
main_frame_colour = ("#EBEBEB","#1A1A1A") # https://www.google.com/search?q=%23ebebeb & https://www.google.com/search?q=%231a1a1a

##############################################################################



##############################################################################
# Classes

# This class will be the main app window
class App(customtkinter.CTk):
    def __init__(self):
        super().__init__()

        customtkinter.deactivate_automatic_dpi_awareness()

        def page1_button():
            print("Home Button Pressed")
            self.main_frame.destroy()
            self.main_frame = Page1Frame(self)
            ini_write("main", "active_page", "1")

        def page2_button():
            print("Characters Button Pressed")
            self.main_frame.destroy()
            self.main_frame = Page2Frame(self)
            ini_write('main', 'active_page', "2") 
           
        def page3_button():
            print("Actions Button Pressed")
            self.main_frame.destroy()
            self.main_frame = Page3Frame(self)
            ini_write('main', 'active_page', "3") 
   
        def page4_button():
            print("Screen Button Pressed")
            self.main_frame.destroy()
            self.main_frame = Page4Frame(self)
            ini_write('main', 'active_page', "4") 
       
        def page5_button():
            print("Settings Button Pressed")
            self.main_frame.destroy()
            self.main_frame = Page5Frame(self)
            ini_write('main', 'active_page', "5") 
        # Configure window
        self.title("")
        self.resizable(False, False)
           
        # Configure grid layout (4x4)
        self.grid_columnconfigure(0, weight=0)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)
        
        # Create sidebar frame with widgets
        self.sidebar_frame = customtkinter.CTkFrame(self, width=gui_sidebar_width , corner_radius=0)
        self.sidebar_frame.grid(row=0, column=0, rowspan=6, sticky="nsew")

        # Create the logo label, and bind it to the left, middle, and right mouse buttons with lambda functions that print to the console
        self.image = Image.open(lcd_image)
        self.photo = customtkinter.CTkImage(self.image, size=(128,64))
        self.logo_label = customtkinter.CTkLabel(self.sidebar_frame, image=self.photo, text="")
        self.logo_label.bind('<Button-1>', lambda e: print("The tukii logo was clicked with the left mouse button"), add='+')
        self.logo_label.bind('<Button-2>', lambda p: print("The tukii logo was clicked with the middle mouse button"), add='+')
        self.logo_label.bind('<Button-3>', lambda p: print("The tukii logo was clicked with the right mouse button"), add='+')
        #self.logo_label.bind("<Enter>", lambda eff: on_hover(eff="None", event="the app logo"), add='+')
        self.logo_label.grid(row=0, column=0, padx=20, pady=25)

        # Create the sidebar buttons, each with a command that prints to the console
        # Page1 Button
        self.sidebar_button_1 = customtkinter.CTkButton(self.sidebar_frame, text="Home", font=("Segoe UI", 18), command=page1_button)
        self.sidebar_button_1.grid(row=1, column=0, padx=20, pady=10)
        #self.sidebar_button_1.bind("<Enter>", lambda eff: on_hover(eff="None", event="Page1 Button"), add='+')

        # Page2 Button
        self.sidebar_button_2 = customtkinter.CTkButton(self.sidebar_frame, text="Characters", font=("Segoe UI", 18), command=page2_button)
        self.sidebar_button_2.grid(row=2, column=0, padx=20, pady=10)
        #self.sidebar_button_2.bind("<Enter>", lambda eff: on_hover(eff="None", event="Page2 Button"), add='+')

        # Page3 Button
        self.sidebar_button_3 = customtkinter.CTkButton(self.sidebar_frame, text="Actions", font=("Segoe UI", 18), command=page3_button)
        self.sidebar_button_3.grid(row=3, column=0, padx=20, pady=10)
        #self.sidebar_button_3.bind("<Enter>", lambda eff: on_hover(eff="None", event="Page3 Button"), add='+')

        # Page4 Button
        self.sidebar_button_4 = customtkinter.CTkButton(self.sidebar_frame, text="Screens", font=("Segoe UI", 18), command=page4_button)
        self.sidebar_button_4.grid(row=4, column=0, padx=20, pady=10)
        #self.sidebar_button_4.bind("<Enter>", lambda eff: on_hover(eff="None", event="Page4 Button"), add='+')

        # Page5 Button
        self.sidebar_button_5 = customtkinter.CTkButton(self.sidebar_frame, text="Settings", font=("Segoe UI", 18), command=page5_button)
        self.sidebar_button_5.grid(row=6, column=0, padx=20, pady=10)
        #self.sidebar_button_4.bind("<Enter>", lambda eff: on_hover(eff="None", event="Page4 Button"), add='+')

        # Set font
        customtkinter.FontManager.load_font("Ardy.ttf")
        arduino_font = customtkinter.CTkFont(family="Ardy", size=128)

        # Create page1_frame with widgets
        class Page1Frame(customtkinter.CTkFrame):
            def __init__(self, parent):
                super().__init__(parent, corner_radius=0, width=gui_main_frame_width, height=600, fg_color="transparent")
                self.grid(row=0, column=1, padx=5, pady=5)

                self.config = configparser.ConfigParser()
                self.config.read(data_ini)
                self.config.sections()


                # Create a frame to hold the labels with a transparent background
                self.frame = customtkinter.CTkFrame(self, width=128, height=64, fg_color="transparent")
                self.frame.grid(row=1, column=0, padx=15, pady=25)

                # Create the labels
                self.t_label = customtkinter.CTkLabel(self.frame, height=150, width=150, text="selall", font=("Ardy", 64), text_color="black", fg_color="white")
                self.k_label = customtkinter.CTkLabel(self.frame, height=150, width=150, text="replce", font=("Ardy", 64), text_color="white", fg_color="black")

                # Add the labels to the frame side by side
                self.t_label.grid(row=0, column=0)
                self.k_label.grid(row=0, column=1)

        # Create page2_frame with widgets, it has lots of buttons and labels
        class Page2Frame(customtkinter.CTkFrame):
            def __init__(self, parent):
                super().__init__(parent, corner_radius=0, width=gui_main_frame_width, fg_color="transparent")
                self.grid(row=0, column=1, rowspan=8, columnspan=2, sticky="nsew", padx=150)
                
                def set_active_button(button, button_name):
                    # Remove border from all buttons
                    for btn in [self.button1, self.button2, self.button3, self.button4, self.button5, self.button6]:
                        btn.configure(border_color=main_frame_colour)
                    
                    # Set green border around the active button
                    if button_name != "6":
                        button.configure(border_color="green")
                    else:
                        button.configure(border_color="red")
                    
                    # Store the active button in the ini file
                    ini_write("page2", "active_button", button_name)

                # Create a blank label for row 1
                self.row1label = customtkinter.CTkLabel(self, text="")
                self.row1label.grid(row=0, column=1, padx=15, pady=25)
                # Create a set of buttons
                self.button1 = customtkinter.CTkButton(self, corner_radius=10, border_width=4, border_color=main_frame_colour, text="Button 1", font=("Segoe UI", 16), command=lambda: [set_active_button(self.button1, "1"), self.button6.configure(state="disabled", text="Disabled")])
                self.button1.grid(row=1, column=0, padx=15, pady=10)

                self.button2 = customtkinter.CTkButton(self, corner_radius=10, border_width=4, border_color=main_frame_colour, text="Button 2", font=("Segoe UI", 16), command=lambda: set_active_button(self.button2, "2"))
                self.button2.grid(row=2, column=0, padx=15, pady=10)

                self.button3 = customtkinter.CTkButton(self, corner_radius=10, border_width=4, border_color=main_frame_colour, text="Button 3", font=("Segoe UI", 16), command=lambda: set_active_button(self.button3, "3"))
                self.button3.grid(row=3, column=0, padx=15, pady=10)

                self.button4 = customtkinter.CTkButton(self, corner_radius=10, border_width=4, border_color=main_frame_colour, text="Button 4", font=("Segoe UI", 16), command=lambda: set_active_button(self.button4, "4"))
                self.button4.grid(row=4, column=0, padx=15, pady=10)

                self.button5 = customtkinter.CTkButton(self, corner_radius=10, border_width=4, border_color=main_frame_colour, text="Button 5", font=("Segoe UI", 16), command=lambda: [set_active_button(self.button5, "5"), self.button6.configure(state="normal", text="Button 6")])
                self.button5.grid(row=5, column=0, padx=15, pady=10)

                self.button6 = customtkinter.CTkButton(self, corner_radius=10, border_width=4, border_color=main_frame_colour, text="Disabled", font=("Segoe UI", 16), state="disabled", command=lambda: set_active_button(self.button6, "6"))
                self.button6.grid(row=6, column=0, padx=15, pady=10)

                # Read the active button from the ini file, and set the active button to the correct button
                config = configparser.ConfigParser()
                config.read(data_ini)
                active_button = config['page2']['active_button']
                if active_button == "1":
                    set_active_button(self.button1, "1")
                elif active_button == "2":
                    set_active_button(self.button2, "2")
                elif active_button == "3":
                    set_active_button(self.button3, "3")
                elif active_button == "4":
                    set_active_button(self.button4, "4")
                elif active_button == "5":
                    set_active_button(self.button5, "5")
                else:
                    set_active_button(self.button1, "1")
                
        # Create page3_frame with widgets
        class Page3Frame(customtkinter.CTkFrame):
            def __init__(self, parent):
                super().__init__(parent, corner_radius=0, width=gui_main_frame_width, fg_color="transparent")
                self.grid(row=0, column=1, rowspan=6, sticky="nsew", padx=25)

                # Create a blank label for row 1
                self.row1label = customtkinter.CTkLabel(self, text="Actions", font=("Segoe UI", 36))
                self.row1label.grid(row=0, column=0, padx=15, pady=25)
        
        # Create page4_frame with widgets
        class Page4Frame(customtkinter.CTkFrame):
            def __init__(self, parent):
                super().__init__(parent, corner_radius=0, width=gui_main_frame_width, fg_color="transparent")
                self.grid(row=0, column=1, rowspan=6, sticky="nsew", padx=25)

                # Create a blank label for row 1
                self.row1label = customtkinter.CTkLabel(self, text="Screens", font=("Segoe UI", 36))
                self.row1label.grid(row=0, column=0, padx=15, pady=25)

        # Create page4_frame with widgets
        class Page5Frame(customtkinter.CTkFrame):
            def __init__(self, parent):
                super().__init__(parent, corner_radius=0, width=gui_main_frame_width, fg_color="transparent")
                self.grid(row=0, column=1, rowspan=6, sticky="nsew", padx=25)

                # Create a blank label for row 1
                self.row1label = customtkinter.CTkLabel(self, text="Settings", font=("Segoe UI", 36))
                self.row1label.grid(row=0, column=0, padx=15, pady=25)

        # read the active page from the ini file, and set the main frame to the correct page
        config = configparser.ConfigParser()
        config.read(data_ini)
        active_page = config['main']['active_page']
        if active_page == "1":
            self.main_frame = Page1Frame(self)
        elif active_page == "2":
            self.main_frame = Page2Frame(self)
        elif active_page == "3":
            self.main_frame = Page3Frame(self)
        elif active_page == "4":
            self.main_frame = Page4Frame(self)
        elif active_page == "5":
            self.main_frame = Page5Frame(self)
        else:
            self.main_frame = Page1Frame(self)

class Cereal(): # Serial Communication Class
    class Cereal:
        def __init__(self):
            self.ser = None
            self.scan_ports()
            self.read_thread = threading.Thread(target=self.read_data)
            self.read_thread.daemon = True
            self.read_thread.start()

        def scan_ports(self):
            ports = serial.tools.list_ports.comports()
            for port in ports:
                try:
                    self.ser = serial.Serial(port.device, 115200, timeout=1)
                    self.ser.write(b'TK_are_you_tukii_TK')
                    response = self.ser.readline().decode().strip()
                    if response == 'TK_thats_me_TK':
                        print(f"Tukii found on port {port.device}")
                        ini_write('connection', 'port', port.device)
                        global tukii_port
                        tukii_port = port.device
                        return
                    else:
                        self.ser.close()
                except:
                    continue
            self.ser = None
            print("Tukii not found")


        def send_data(self, keyvalue, data):
            if self.ser:
                command = f"TK_keyvalue={keyvalue}_{data}_TK"
                self.ser.write(command.encode())

        def read_data(self):
            while True:
                if self.ser:
                    try:
                        line = self.ser.readline().decode().strip()
                        if line.startswith("TK_") and line.endswith("_TK"):
                            content = line[3:-3]
                            keyvalue, value = content.split("_", 1)
                            keyvalue = keyvalue.split("=")[1]
                            if keyvalue == "0":
                                ini_write('received', 'character', value)
                            elif keyvalue == "1":
                                ini_write('received', 'action', value)
                            elif keyvalue == "2":
                                ini_write('received', 'other', value)
                    except:
                        print("Error reading from Tukii")
                        pass
                time.sleep(0.33)

        def runner(self):
            self.read_thread.start()

##############################################################################



##############################################################################
# Main
#
# Create the system tray icon, with a menu that has two options: Quit and Show
icon = pystray.Icon("tukii", image, title="tukii", menu=pystray.Menu(pystray.MenuItem("tukii", toggle_window, default=True),pystray.MenuItem("Quit", quit_window)))
#
# Run the system tray icon in the background, detached from the main app. this way the icon will continue to run even if the main app is closed.
icon.run_detached()
#
# Create the Serial Communication applet
cereal = Cereal()
# Run the Serial Communication applet in the background
#cereal.runner()
# Create the main app, this will be the main window that is shown when the Show option is selected from the system tray icon menu
app = App()
#
# Set the size of the app window
app.geometry(f"{gui_X}x{gui_Y}")
#
# Set the icon for the app, using logo.ico as the icon file
app.iconbitmap("data/image/logo.ico")
#
# When the window is closed, call the withdraw_window function, this way the window is not destroyed.
app.protocol('WM_DELETE_WINDOW', withdraw_window)
#
# Start the main app
# app.withdraw()

app.mainloop()
#
##############################################################################