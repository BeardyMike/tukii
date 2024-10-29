/*//|_   _|/////(_) h| (_)/////| (_)/////////| |/(_)/////////////////////////////////////////////////
//    | |  _ __  _| |_ _  __ _| |_ ___  __ _| |_ _  ___  _ __                                      //
//    | | | '_ \| | __| |/ _` | | / __|/ _` | __| |/ _ \| '_ \                                     //
//   _| |_| | | | | |_| | (_| | | \__ \ (_| | |_| | (_) | | | |                                    //
//  |_____|_| |_|_|\__|_|\__,_|_|_|___/\__,_|\__|_|\___/|_| |_|                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////*/
// 
#include <Wire.h>              // github.com/esp8266/Arduino/tree/master/libraries/Wire
#include <Adafruit_GFX.h>      // github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h>  // github.com/adafruit/Adafruit-SD1306
#include <RotaryEncoder.h>     // github.com/mathertel/RotaryEncoder
#include <LittleFS.h>
#include <TinyUSB_Mouse_and_Keyboard.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

/*-------------------------------------------------------------------------------------------------*/

// Pin definitions for buttons
#define button1Pin 28  // Left Button
#define button2Pin 26  // Right Button

/*-------------------------------------------------------------------------------------------------*/

// Define rotary encoder pins
#define ROTARY_ENCODER_A_PIN 3  // Dial Pin A (CCW)
#define ROTARY_ENCODER_B_PIN 2  // Dial Pin B (CW)
#define ROTARY_BUTTON 14        // Dial Switch Pin (BTN)

/*-------------------------------------------------------------------------------------------------*/

// Define important variables outside the scope of functions
const char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
const char *action_list[] = {
  "copy",
  "paste",
  "cut",
  "undo",
  "redo",
  "selall",
  "save",
  "print",
  "find",
  "replce",
  "tskman",
  "close"
};

/*-------------------------------------------------------------------------------------------------*/

// Define the display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*-------------------------------------------------------------------------------------------------*/

// Initialize the rotary encoder
RotaryEncoder encoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, RotaryEncoder::LatchMode::FOUR3);

/*-------------------------------------------------------------------------------------------------*/

// character screen vars
int ll_index = 60;
int l_index = 61;
int mid_index = 0;
int r_index = 1;
int rr_index = 2;

/*-------------------------------------------------------------------------------------------------*/

// Define or initialise important vars
int pos = 0;
int leftKey;
int leftVal;
int rightKey;
int rightVal;
int dial_still_pressed;
int action;  // 0 = copy, 1 = paste, 2 = cut, 3 = undo, 4 = redo, 5 = selall, 6 = save, 7 = print, 8 = find, 9 = replace, 10 = tskman, 11 = close

// text from patorjk.com/software/taag/#p=display&f=Big&t=BeardyMike
// ^ - Init
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////






/*//// ____|////| |//////////////////////////////////////////////////////////////////////////////////
//  | (___   ___| |_ _   _ _ __                                                                    //
//   \___ \ / _ \ __| | | | '_ \                                                                   //
//   ____) |  __/ |_| |_| | |_) |                                                                  //
//  |_____/ \___|\__|\__,_| .__/                                                                   //
//////////////////////////| |//////////////////////////////////////////////////////////////////////*/
void setup() {

  // Initialise the Keyboard and Mouse
  Keyboard.begin();
  Mouse.begin();

  /*-------------------------------------------------------------------------------------------------*/

  // Initialize the buttons
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(ROTARY_BUTTON, INPUT_PULLUP);

  /*-------------------------------------------------------------------------------------------------*/

  // Initialise the serial port
  Serial.begin(115200);
  delay(1000);
  Serial.println("tukiiOS is ready!");

  /*-------------------------------------------------------------------------------------------------*/

  // Initialise LittleFS
  if (!LittleFS.begin()) {
    Serial.println("An error occurred during LittleFS initialization!");
    return;
  }

  // Check if buttons.txt exists
  if (LittleFS.exists("/buttons.txt")) {
    Serial.println("buttons.txt exists.");
  } else {
    // Create buttons.txt and write the default values
    File file = LittleFS.open("/buttons.txt", "w");
    if (!file) {
      Serial.println("Failed to create buttons.txt");
      return;
    }
    file.println("22");
    delay(0.01);
    file.println("1");
    delay(0.01);
    file.println("24");
    delay(0.01);
    file.println("1");
    delay(0.01);
    file.close();
    Serial.println("buttons.txt created with default values.");
  }

  // Read the left and right keys from buttons.txt
  File file = LittleFS.open("buttons.txt", "r");
  if (!file) {
    Serial.println("Failed to open buttons.txt");
    return;
  }

  leftKey = file.parseInt();
  // Serial.println(leftKey);
  delay(0.01);

  leftVal = file.parseInt();
  // Serial.println(leftVal);
  delay(0.01);

  rightKey = file.parseInt();
  // Serial.println(rightKey);
  delay(0.01);

  rightVal = file.parseInt();
  // Serial.println(rightVal);
  delay(0.01);

  file.close();

  /*-------------------------------------------------------------------------------------------------*/

  // Initialize the display, then clear it
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Address 0x3C for 128x64

  /*-------------------------------------------------------------------------------------------------*/

  main_screen();
}
//
// ^ Setup
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////






/*/|  ____|//////////////| |/(_)/////////////////////////////////////////////////////////////////////
// | |__ _   _ _ __   ___| |_ _  ___  _ __  ___                                                    //
// |  __| | | | '_ \ / __| __| |/ _ \| '_ \/ __|                                                   //
// | |  | |_| | | | | (__| |_| | (_) | | | \__ \                                                   //
// |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////*/
// Common Functions

void press_button(int val) {
  if (val == 0) {
    Keyboard.print(characters[leftKey]);
    while (digitalRead(button1Pin) == LOW) {
      delay(10);
    }
  } else if (val == 1) {
    Keyboard.print(characters[rightKey]);
    while (digitalRead(button2Pin) == LOW) {
      delay(10);
    }
  }
  return;
}

/*-------------------------------------------------------------------------------------------------*/

void action_press(int action) {
  while (digitalRead(button1Pin) == LOW || digitalRead(button2Pin) == LOW) {
    delay(10);
  }
  if (action == 0) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('c');
    delay(100);
    Keyboard.releaseAll();
  } else if (action == 1) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('v');
    delay(100);
    Keyboard.releaseAll();
  } else if (action == 2) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('x');
    delay(100);
    Keyboard.releaseAll();
  } else if (action == 3) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('z');
    delay(100);
    Keyboard.releaseAll();
  } else if (action == 4) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('y');
    delay(100);
    Keyboard.releaseAll();
  } else if (action == 5) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('a');
    delay(100);
    Keyboard.releaseAll();
  } else if (action == 6) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('s');
    delay(100);
    Keyboard.releaseAll();
  } else if (action == 7) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('p');
    delay(100);
    Keyboard.releaseAll();
  } else if (action == 8) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('f');
    delay(100);
    Keyboard.releaseAll();
  } else if (action == 9) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('h');
    delay(100);
    Keyboard.releaseAll();
  } else if (action == 10) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(KEY_LEFT_SHIFT);
    Keyboard.press(KEY_ESC);
    delay(100);
    Keyboard.releaseAll();
  } else if (action == 11) {
    Keyboard.press(KEY_LEFT_ALT);
    Keyboard.press(KEY_F4);
    delay(100);
    Keyboard.releaseAll();
  }
  return;
}

/*-------------------------------------------------------------------------------------------------*/

void write_buttons_char(int Lkey, int Lval, int Rkey, int Rval) {
  File file = LittleFS.open("/buttons.txt", "w");
  if (!file) {
    Serial.println("Failed to open buttons.txt");
    return;
  }
  file.println(Lkey);
  delay(0.01);
  file.println(Lval);
  delay(0.01);
  file.println(Rkey);
  delay(0.01);
  file.println(Rval);
  delay(0.01);
  file.close();
  return;
}

// ^ - Common Functions
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////






/*//// ____|/////////////////////////////////////////////////////////////////////////////////////////
//  | (___   ___ _ __ ___  ___ _ __  ___                                                           //
//   \___ \ / __| '__/ _ \/ _ \ '_ \/ __|                                                          //
//   ____) | (__| | |  __/  __/ | | \__ \                                                          //
//  |_____/ \___|_|  \___|\___|_| |_|___/                                                          //
///////////////////////////////////////////////////////////////////////////////////////////////////*/

void main_screen() {
  display.clearDisplay();  // Clear the display
  display.fillRect(0, 0, 64, 64, WHITE);
  display.fillRect(64, 0, 64, 64, BLACK);

  // Left button display
  if (leftVal == 1) {
    display.setTextSize(5);
    display.setTextColor(BLACK);
    display.setCursor(20, 14);  // Centered in the white square
    display.print(characters[leftKey]);
  } else {
    display.setTextSize(2);
    display.setTextColor(BLACK);
    display.setCursor(2, 24);  // Adjusted to fit 6 characters
    display.print(action_list[leftKey]);
  }

  // Right button display
  if (rightVal == 1) {
    display.setTextSize(5);
    display.setTextColor(WHITE);
    display.setCursor(88, 14);  // Centered in the black square
    display.print(characters[rightKey]);
  } else {
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(66, 24);  // Adjusted to fit 6 characters
    display.print(action_list[rightKey]);
  }

  display.display();                           // Display the screen
  Serial.println("Main Screen Loaded");
  // Serial.println("Left Button is: " + String(characters[leftKey]));
  // Serial.println("Right Button is: " + String(characters[rightKey]));
  while (digitalRead(ROTARY_BUTTON) == LOW) {  // Wait until the rotary button is released
    delay(10);
  }
  return;
}

/*-------------------------------------------------------------------------------------------------*/

void main_menu() {
  Serial.println("Main Menu 1/2 Loaded");
  int menu1_postion = 1;
  int menu1_position_drawn = 1;
  display.clearDisplay();
  display.fillRect(0, 14, 128, 17, WHITE);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(2, 1);
  display.print("Menu 1/2");
  display.setTextSize(2);
  display.setCursor(2, 15);
  display.setTextColor(BLACK, WHITE);
  display.print("Characters");
  display.setCursor(2, 33);
  display.setTextColor(WHITE);
  display.print("Actions");
  display.setCursor(2, 55);
  display.setTextSize(1);
  display.print("Next Page >");
  display.setCursor(103, 55);
  display.print("Exit");
  display.display();
  while (digitalRead(ROTARY_BUTTON) == LOW) {  // Wait until the rotary button is released
    delay(10);
  }
  while (true) {
    encoder.tick();                      // Check the encoder for movement
    int newPos = encoder.getPosition();  // Get the direction of the encoder
    if (newPos > pos) {
      menu1_postion++;
      if (menu1_postion > 4) menu1_postion = 1;  // Wrap around to first menu
    } else if (newPos < pos) {
      menu1_postion--;
      if (menu1_postion < 1) menu1_postion = 4;  // Wrap around to last menu
    }
    pos = newPos;
    if (menu1_postion != menu1_position_drawn) {  // Display the current menu option only if it has changed
      if (menu1_postion == 1) {
        display.clearDisplay();
        display.fillRect(0, 14, 128, 17, WHITE);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(2, 1);
        display.print("Menu 1/2");
        display.setTextSize(2);
        display.setCursor(2, 15);
        display.setTextColor(BLACK, WHITE);
        display.print("Characters");
        display.setCursor(2, 33);
        display.setTextColor(WHITE);
        display.print("Actions");
        display.setCursor(2, 55);
        display.setTextSize(1);
        display.print("Next Page >");
        display.setCursor(103, 55);
        display.print("Exit");
        display.display();
        menu1_position_drawn = 1;
      } else if (menu1_postion == 2) {
        display.clearDisplay();
        display.fillRect(0, 32, 128, 17, WHITE);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(2, 1);
        display.print("Menu 1/2");
        display.setTextSize(2);
        display.setCursor(2, 15);
        display.print("Characters");
        display.setCursor(2, 33);
        display.setTextColor(BLACK, WHITE);
        display.print("Actions");
        display.setCursor(2, 55);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.print("Next Page >");
        display.setCursor(103, 55);
        display.print("Exit");
        display.display();
        menu1_position_drawn = 2;
      } else if (menu1_postion == 3) {
        display.clearDisplay();
        display.fillRect(0, 54, 70, 9, WHITE);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(2, 1);
        display.print("Menu 1/2");
        display.setTextSize(2);
        display.setCursor(2, 15);
        display.print("Characters");
        display.setCursor(2, 33);
        display.print("Actions");
        display.setCursor(2, 55);
        display.setTextSize(1);
        display.setTextColor(BLACK, WHITE);
        display.print("Next Page >");
        display.setCursor(103, 55);
        display.setTextColor(WHITE);
        display.print("Exit");
        display.display();
        menu1_position_drawn = 3;
      } else if (menu1_postion == 4) {
        display.clearDisplay();
        display.fillRect(101, 54, 70, 9, WHITE);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(2, 1);
        display.print("Menu 1/2");
        display.setTextSize(2);
        display.setCursor(2, 15);
        display.print("Characters");
        display.setCursor(2, 33);
        display.print("Actions");
        display.setCursor(2, 55);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.print("Next Page >");
        display.setTextColor(BLACK, WHITE);
        display.setCursor(103, 55);
        display.print("Exit");
        display.display();
        menu1_position_drawn = 4;
      }
    }
    if (digitalRead(ROTARY_BUTTON) == LOW) {  // Check rotary button to select the current menu option
      delay(100);                             // Debounce delay
      if (menu1_postion == 1) {
        character_menu();
      } else if (menu1_postion == 2) {
        action_menu();
      } else if (menu1_postion == 3) {
        main_menu2();
      } else if (menu1_postion == 4) {
        while (digitalRead(ROTARY_BUTTON) == LOW) {
          delay(10);
        }
        main_screen();
      }
      break;
    }
  }
}

/*-------------------------------------------------------------------------------------------------*/

void main_menu2() {
  Serial.println("Main Menu 2/2 Loaded");
  int menu2_postion = 1;
  int menu2_position_drawn = 1;
  display.clearDisplay();
  display.fillRect(0, 14, 128, 17, WHITE);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(2, 1);
  display.print("Menu 2/2");
  display.setTextSize(2);
  display.setCursor(2, 15);
  display.setTextColor(BLACK, WHITE);
  display.print("Games");
  display.setCursor(2, 33);
  display.setTextColor(WHITE);
  display.print("About");
  display.setCursor(2, 55);
  display.setTextSize(1);
  display.print("< Last Page");
  display.setCursor(103, 55);
  display.print("Exit");
  display.display();
  while (digitalRead(ROTARY_BUTTON) == LOW) {  // Wait until the rotary button is released
    delay(10);
  }
  while (true) {
    encoder.tick();                      // Check the encoder for movement
    int newPos = encoder.getPosition();  // Get the direction of the encoder
    if (newPos > pos) {
      menu2_postion++;
      if (menu2_postion > 4) menu2_postion = 1;  // Wrap around to first menu
    } else if (newPos < pos) {
      menu2_postion--;
      if (menu2_postion < 1) menu2_postion = 4;  // Wrap around to last menu
    }
    pos = newPos;
    if (menu2_postion != menu2_position_drawn) {  // Display the current menu option only if it has changed
      if (menu2_postion == 1) {
        display.clearDisplay();
        display.fillRect(0, 14, 128, 17, WHITE);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(2, 1);
        display.print("Menu 2/2");
        display.setTextSize(2);
        display.setCursor(2, 15);
        display.setTextColor(BLACK, WHITE);
        display.print("Games");
        display.setCursor(2, 33);
        display.setTextColor(WHITE);
        display.print("About");
        display.setCursor(2, 55);
        display.setTextSize(1);
        display.print("< Last Page");
        display.setCursor(103, 55);
        display.print("Exit");
        display.display();
        menu2_position_drawn = 1;
      } else if (menu2_postion == 2) {
        display.clearDisplay();
        display.fillRect(0, 32, 128, 17, WHITE);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(2, 1);
        display.print("Menu 2/2");
        display.setTextSize(2);
        display.setCursor(2, 15);
        display.print("Games");
        display.setCursor(2, 33);
        display.setTextColor(BLACK, WHITE);
        display.print("About");
        display.setCursor(2, 55);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.print("< Last Page");
        display.setCursor(103, 55);
        display.print("Exit");
        display.display();
        menu2_position_drawn = 2;
      } else if (menu2_postion == 3) {
        display.clearDisplay();
        display.fillRect(0, 54, 70, 9, WHITE);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(2, 1);
        display.print("Menu 2/2");
        display.setTextSize(2);
        display.setCursor(2, 15);
        display.print("Games");
        display.setCursor(2, 33);
        display.print("About");
        display.setCursor(2, 55);
        display.setTextSize(1);
        display.setTextColor(BLACK, WHITE);
        display.print("< Last Page");
        display.setCursor(103, 55);
        display.setTextColor(WHITE);
        display.print("Exit");
        display.display();
        menu2_position_drawn = 3;
      } else if (menu2_postion == 4) {
        display.clearDisplay();
        display.fillRect(101, 54, 70, 9, WHITE);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(2, 1);
        display.print("Menu 2/2");
        display.setTextSize(2);
        display.setCursor(2, 15);
        display.print("Games");
        display.setCursor(2, 33);
        display.print("About");
        display.setCursor(2, 55);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.print("< Last Page");
        display.setTextColor(BLACK, WHITE);
        display.setCursor(103, 55);
        display.print("Exit");
        display.display();
        menu2_position_drawn = 4;
      }
    }
    if (digitalRead(ROTARY_BUTTON) == LOW) {  // Check rotary button to select the current menu option
      delay(100);                             // Debounce delay
      if (menu2_postion == 1) {
        games_menu();
      } else if (menu2_postion == 2) {
        about_screen();
      } else if (menu2_postion == 3) {
        main_menu();
      } else if (menu2_postion == 4) {
        while (digitalRead(ROTARY_BUTTON) == LOW) {
          delay(10);
        }
        main_screen();
      }
      break;
    }
  }
}

/*-------------------------------------------------------------------------------------------------*/
void character_menu() {
  Serial.println("Character Menu Loaded");
  display.clearDisplay();
  display.fillRect(49, 0, 24, 100, SSD1306_WHITE);
  // Print characters to screen in a gentle arch
  display.setTextSize(4);
  display.setCursor(1, 30);
  display.setTextColor(SSD1306_WHITE);
  display.print(characters[ll_index]);
  display.setTextSize(4);
  display.setCursor(26, 20);
  display.setTextColor(SSD1306_WHITE);
  display.print(characters[l_index]);
  display.setTextSize(4);
  display.setCursor(51, 15);
  display.setTextColor(SSD1306_BLACK);
  display.print(characters[mid_index]);
  display.setTextSize(4);
  display.setCursor(76, 20);
  display.setTextColor(SSD1306_WHITE);
  display.print(characters[r_index]);
  display.setTextSize(4);
  display.setCursor(101, 30);
  display.setTextColor(SSD1306_WHITE);
  display.print(characters[rr_index]);
  display.display();
  while (digitalRead(ROTARY_BUTTON) == LOW) {  // Wait until the rotary button is released
    delay(10);
  }
  while (true) {
    encoder.tick();                      // Check the encoder for movement
    int newPos = encoder.getPosition();  // Get the direction of the encoder
    if (newPos > pos) {
      ll_index++;
      l_index++;
      mid_index++;
      r_index++;
      rr_index++;
      if (ll_index > 61) ll_index = 0;    // Wrap around to first character
      if (l_index > 61) l_index = 0;      // Wrap around to first character
      if (mid_index > 61) mid_index = 0;  // Wrap around to first character
      if (r_index > 61) r_index = 0;      // Wrap around to first character
      if (rr_index > 61) rr_index = 0;    // Wrap around to first character
      display.clearDisplay();             // Clear the display
      display.fillRect(49, 0, 24, 100, SSD1306_WHITE);
      display.setTextSize(4);
      display.setCursor(1, 30);
      display.setTextColor(SSD1306_WHITE);
      display.print(characters[ll_index]);
      display.setTextSize(4);
      display.setCursor(26, 20);
      display.setTextColor(SSD1306_WHITE);
      display.print(characters[l_index]);
      display.setTextSize(4);
      display.setCursor(51, 15);
      display.setTextColor(SSD1306_BLACK);
      display.print(characters[mid_index]);
      display.setTextSize(4);
      display.setCursor(76, 20);
      display.setTextColor(SSD1306_WHITE);
      display.print(characters[r_index]);
      display.setTextSize(4);
      display.setCursor(101, 30);
      display.setTextColor(SSD1306_WHITE);
      display.print(characters[rr_index]);
      display.display();
    } else if (newPos < pos) {
      int newdraw = 1;
      ll_index--;
      l_index--;
      mid_index--;
      r_index--;
      rr_index--;
      if (ll_index < 0) ll_index = 61;    // Wrap around to last character
      if (l_index < 0) l_index = 61;      // Wrap around to last character
      if (mid_index < 0) mid_index = 61;  // Wrap around to last character
      if (r_index < 0) r_index = 61;      // Wrap around to last character
      if (rr_index < 0) rr_index = 61;    // Wrap around to last character
      display.clearDisplay();             // Clear the display
      display.fillRect(49, 0, 24, 100, SSD1306_WHITE);
      display.setTextSize(4);
      display.setCursor(1, 30);
      display.setTextColor(SSD1306_WHITE);
      display.print(characters[ll_index]);
      display.setTextSize(4);
      display.setCursor(26, 20);
      display.setTextColor(SSD1306_WHITE);
      display.print(characters[l_index]);
      display.setTextSize(4);
      display.setCursor(51, 15);
      display.setTextColor(SSD1306_BLACK);
      display.print(characters[mid_index]);
      display.setTextSize(4);
      display.setCursor(76, 20);
      display.setTextColor(SSD1306_WHITE);
      display.print(characters[r_index]);
      display.setTextSize(4);
      display.setCursor(101, 30);
      display.setTextColor(SSD1306_WHITE);
      display.print(characters[rr_index]);
      display.display();
    }
    pos = newPos;
    if (digitalRead(ROTARY_BUTTON) == LOW) {  // Check rotary button to select the current character
      delay(10);                              // Debounce delay
      char L_or_R = LR_menu();                // Print the selected character to the keyboard
      if (L_or_R == 'L') {
        leftKey = mid_index;
        leftVal = 1;
        write_buttons_char(leftKey, leftVal, rightKey, rightVal);
      } else if (L_or_R == 'R') {
        rightKey = mid_index;
        rightVal = 1;
        write_buttons_char(leftKey, leftVal, rightKey, rightVal);
      }
      while (digitalRead(button1Pin) == LOW || digitalRead(button2Pin) == LOW) {
        delay(10);
      }
      main_screen();
      break;
    }
  }
}

/*-------------------------------------------------------------------------------------------------*/
void action_menu() {
  int newdraw_action = 0;
  Serial.println("Action Menu Loaded");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(1, 1);
  display.print("copy");
  display.setCursor(1, 11);
  display.setTextColor(WHITE);
  display.print("paste");
  display.setCursor(1, 21);
  display.print("cut");
  display.setCursor(1, 31);
  display.print("undo");
  display.setCursor(1, 41);
  display.print("redo");
  display.setCursor(1, 51);
  display.print("selall");
  // Right side actions
  display.setCursor(64, 1);
  display.print("save");
  display.setCursor(64, 11);
  display.print("print");
  display.setCursor(64, 21);
  display.print("find");
  display.setCursor(64, 31);
  display.print("replace");
  display.setCursor(64, 41);
  display.print("tskman");
  display.setCursor(64, 51);
  display.print("close");
  // Display the screen
  display.display();
  // Wait until the rotary button is released
  while (digitalRead(ROTARY_BUTTON) == LOW) {
    delay(10);
  }
  while (true) {
    encoder.tick();                      // Check the encoder for movement
    int newPos = encoder.getPosition();  // Get the direction of the encoder
    if (newPos > pos) {
      action++;
      newdraw_action = 1;
      if (action > 11) action = 0;  // Wrap around to first action
    } else if (newPos < pos) {
      action--;
      newdraw_action = 1;
      if (action < 0) action = 11;  // Wrap around to last action
    }
    pos = newPos;
    if (newdraw_action == 1) {
      display.clearDisplay();
      display.setTextSize(1);

      // Display first column (0 to 5)
      for (int i = 0; i < 6; i++) {
        if (i == action % 6 && action < 6) {
          display.setTextColor(BLACK, WHITE);  // Highlight selected item
        } else {
          display.setTextColor(WHITE);  // Regular text color
        }
        display.setCursor(1, i * 10 + 1);
        display.print(action_list[i]);
      }

      // Display second column (6 to 11)
      for (int i = 6; i < 12; i++) {
        if (i == action) {
          display.setTextColor(BLACK, WHITE);  // Highlight selected item
        } else {
          display.setTextColor(WHITE);  // Regular text color
        }
        display.setCursor(64, (i - 6) * 10 + 1);
        display.print(action_list[i]);
      }

      display.display();  // Render the updated display content
      newdraw_action = 0;
    }
    if (digitalRead(ROTARY_BUTTON) == LOW) {
      delay(100);
      char L_or_R = LR_menu();
      if (L_or_R == 'L') {
        leftKey = action;
        leftVal = 0;
        write_buttons_char(leftKey, leftVal, rightKey, rightVal);
      } else if (L_or_R == 'R') {
        rightKey = action;
        rightVal = 0;
        write_buttons_char(leftKey, leftVal, rightKey, rightVal);
      }
      while (digitalRead(button1Pin) == LOW || digitalRead(button2Pin) == LOW) {
        delay(10);
      }
      main_screen();
      break;
    }
  }
}

/*-------------------------------------------------------------------------------------------------*/

char LR_menu() {
  Serial.println("LR Menu Loaded");
  // clear the display
  display.clearDisplay();
  display.fillRect(0, 0, 64, 64, SSD1306_WHITE);
  display.fillRect(64, 0, 64, 64, SSD1306_BLACK);
  display.setCursor(10, 15);
  display.setTextSize(5);
  display.setTextColor(SSD1306_BLACK);
  display.print("L");
  display.setCursor(74, 15);
  display.setTextColor(SSD1306_WHITE);
  display.print("R");
  display.display();
  while (digitalRead(ROTARY_BUTTON) == LOW) {
    delay(10);
  }
  while (true) {
    if (digitalRead(button1Pin) == LOW) {
      return 'L';
    } else if (digitalRead(button2Pin) == LOW) {
      return 'R';
    }
  }
}

/*-------------------------------------------------------------------------------------------------*/

void games_menu() {      
  while (digitalRead(button1Pin) == LOW || digitalRead(button2Pin) == LOW || digitalRead(ROTARY_BUTTON) == LOW) {
    delay(10);  // Wait until all buttons are released
  }
  
  Serial.println("Games Menu Loaded");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.print("Games Menu");
  display.setCursor(10, 10);
  display.print("Coming Soon!");
  display.setCursor(10, 40);
  display.print("Press any button");
  display.setCursor(10, 50);
  display.print("to return");
  display.display();

  while (true) {
    if (digitalRead(button1Pin) == LOW || digitalRead(button2Pin) == LOW || digitalRead(ROTARY_BUTTON) == LOW) {
      while (digitalRead(button1Pin) == LOW || digitalRead(button2Pin) == LOW || digitalRead(ROTARY_BUTTON) == LOW) {
        delay(10);  // Wait until all buttons are released
      }
      main_screen();
      break;
    }
  }
}

/*-------------------------------------------------------------------------------------------------*/
void about_screen() {
  Serial.println("About Screen Loaded");
  display.clearDisplay();
  
  // Display "tukii" in large font
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.print("tukii");
  
  // Display "Made by BeardyMike" underneath in smaller fontchange this to show tukii in large font, with Made by BeardyMike underneath.
  display.setTextSize(1);
  display.setCursor(10, 40);
  display.print("Made by BeardyMike");
  
  // Display the screen
  display.display();
  
  // Wait until the rotary button is released
  while (digitalRead(ROTARY_BUTTON) == LOW) {
    delay(10);
  }
  // Wait until the rotary button is pressed again
  while (true) {
    if (digitalRead(ROTARY_BUTTON) == LOW) {
      while (digitalRead(ROTARY_BUTTON) == LOW) {
        delay(10);  // Wait until the rotary button is released
      }
      main_screen();
      break;
    }
  }
}
//
// ^ Screens
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////





/*//| |//////////////////////////////////////////////////////////////////////////////////////////////
//  | |     ___   ___  _ __                                                                        //
//  | |    / _ \ / _ \| '_ \                                                                       //
//  | |___| (_) | (_) | |_) |                                                                      //
//  |______\___/ \___/|  __/                                                                       //
//////////////////////| |//////////////////////////////////////////////////////////////////////////*/
void loop() {

  // if left button pressed, send leftKey to keyboard
  if (digitalRead(button1Pin) == LOW) {
    if (leftVal == 1) {
      press_button(0);
    } else {
      action_press(leftKey);
    }
    delay(10);
  }
  // if right button pressed, send rightKey to keyboard
  if (digitalRead(button2Pin) == LOW) {
    if (rightVal == 1) {
      press_button(1);
    } else {
      action_press(rightKey);
    }
    delay(10);
  }
  // if rotary button pressed, open the main_menu
  if (digitalRead(ROTARY_BUTTON) == LOW) {
    delay(100);  // Debounce delay
    if (dial_still_pressed == 0) {
      main_menu();
    }
    dial_still_pressed = 1;
  }
  dial_still_pressed = 0;
}
//
// ^ Loop
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////