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
  delay(100);
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

  struct KeyAction {
    const uint8_t modifiers[3];
    uint8_t modifierCount;
    uint8_t key;
  };

  const KeyAction actions[] = {
    { { KEY_LEFT_CTRL }, 1, 'c' },                      // Copy
    { { KEY_LEFT_CTRL }, 1, 'v' },                      // Paste
    { { KEY_LEFT_CTRL }, 1, 'x' },                      // Cut
    { { KEY_LEFT_CTRL }, 1, 'z' },                      // Undo
    { { KEY_LEFT_CTRL }, 1, 'y' },                      // Redo
    { { KEY_LEFT_CTRL }, 1, 'a' },                      // Select All
    { { KEY_LEFT_CTRL }, 1, 's' },                      // Save
    { { KEY_LEFT_CTRL }, 1, 'p' },                      // Print
    { { KEY_LEFT_CTRL }, 1, 'f' },                      // Find
    { { KEY_LEFT_CTRL }, 1, 'h' },                      // Replace
    { { KEY_LEFT_CTRL, KEY_LEFT_SHIFT }, 2, KEY_ESC },  // Task Manager
    { { KEY_LEFT_ALT }, 1, KEY_F4 }                     // Close
  };

  if (action >= 0 && action < (sizeof(actions)/sizeof(actions[0]))) {
    const KeyAction &ka = actions[action];
    for (uint8_t i = 0; i < ka.modifierCount; i++) {
      Keyboard.press(ka.modifiers[i]);
    }
    Keyboard.press(ka.key);
    delay(100);
    Keyboard.releaseAll();
  }
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

/*-------------------------------------------------------------------------------------------------*/


void display_menu(int menuNum, int selected, const String menuItems[], const int cursorX[], const int cursorY[], int numItems) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(2, 1);
  if (menuNum == 1) {
    display.print("Menu 1/2");
  } else {
    display.print("Menu 2/2");
  }

  for (int i = 0; i < numItems; i++) {
    if (i == selected) {
      if (i >= 2) {
        display.fillRect(cursorX[i] - 1, cursorY[i] - 1, 70, 10, WHITE);
        display.setTextColor(BLACK);
        display.setTextSize(1);
      } else {
        display.fillRect(0, cursorY[i] - 1, 128, 17, WHITE);
        display.setTextColor(BLACK);
        display.setTextSize(2);
      }
    } else {
      display.setTextColor(WHITE);
      if (i >= 2) {
        display.setTextSize(1);
      } else {
        display.setTextSize(2);
      }
    }
    display.setCursor(cursorX[i], cursorY[i]);
    display.print(menuItems[i]);
  }
  display.display();
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
  int menu_number = 1;
  int menu_position = 0;
  const int num_menu_items = 4;
  const String menuItems[] = {"Characters", "Actions", "Next Page >", "Exit"};
  const int cursorY[] = {15, 33, 55, 55};
  const int cursorX[] = {2, 2, 2, 103};

  display_menu(menu_number, menu_position, menuItems, cursorX, cursorY, num_menu_items);

  while (digitalRead(ROTARY_BUTTON) == LOW) {  // Wait until the rotary button is released
    delay(10);
  }

  while (true) {
    encoder.tick();
    int newPos = encoder.getPosition();
    if (newPos != pos) {
      if (newPos > pos) {
        menu_position = (menu_position + 1) % num_menu_items;
      } else if (newPos < pos) {
        menu_position = (menu_position - 1 + num_menu_items) % num_menu_items;
      }
      pos = newPos;
      display_menu(menu_number, menu_position, menuItems, cursorX, cursorY, num_menu_items);
    }

    if (digitalRead(ROTARY_BUTTON) == LOW) {  // Check rotary button to select the current menu option
      delay(100);  // Debounce delay
      switch (menu_position) {
        case 0:
          character_menu();
          break;
        case 1:
          action_menu();
          break;
        case 2:
          main_menu2();
          break;
        case 3:
          while (digitalRead(ROTARY_BUTTON) == LOW) {
            delay(10);
          }
          main_screen();
          break;
      }
      break;
    }
  }
}

/*-------------------------------------------------------------------------------------------------*/

void main_menu2() {
  Serial.println("Main Menu 2/2 Loaded");
  int menu_number = 2;
  int menu_position = 0;
  const int num_menu_items = 4;
  const String menuItems[] = {"Games", "About", "< Last Page", "Exit"};
  const int cursorY[] = {15, 33, 55, 55};
  const int cursorX[] = {2, 2, 2, 103};

  display_menu(menu_number, menu_position, menuItems, cursorX, cursorY, num_menu_items);

  while (digitalRead(ROTARY_BUTTON) == LOW) {  // Wait until the rotary button is released
    delay(10);
  }

  while (true) {
    encoder.tick();
    int newPos = encoder.getPosition();
    if (newPos != pos) {
      if (newPos > pos) {
        menu_position = (menu_position + 1) % num_menu_items;
      } else if (newPos < pos) {
        menu_position = (menu_position - 1 + num_menu_items) % num_menu_items;
      }
      pos = newPos;
      display_menu(menu_number, menu_position, menuItems, cursorX, cursorY, num_menu_items);
    }

    if (digitalRead(ROTARY_BUTTON) == LOW) {  // Check rotary button to select the current menu option
      delay(100);  // Debounce delay
      switch (menu_position) {
        case 0:
          games_menu();
          break;
        case 1:
          about_screen();
          break;
        case 2:
          main_menu();
          break;
        case 3:
          while (digitalRead(ROTARY_BUTTON) == LOW) {
            delay(10);
          }
          main_screen();
          break;
      }
      break;
    }
  }
}
/*-------------------------------------------------------------------------------------------------*/

void character_menu() {
  Serial.println("Character Menu Loaded");
  pos = encoder.getPosition();
  display.clearDisplay();
  draw_character_menu();
  display.display();

  while (digitalRead(ROTARY_BUTTON) == LOW) {
    delay(10);
  }

  while (true) {
    encoder.tick();
    int newPos = encoder.getPosition();
    if (newPos != pos) {
      int delta = newPos - pos;
      pos = newPos;
      mid_index = (mid_index + delta + 62) % 62;
      draw_character_menu();
      display.display();
    }

    if (digitalRead(ROTARY_BUTTON) == LOW) {
      delay(10);
      char L_or_R = LR_menu();
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

void draw_character_menu() {
  display.clearDisplay();
  display.fillRect(49, 0, 24, 100, SSD1306_WHITE);

  int indices[5];
  indices[0] = (mid_index - 2 + 62) % 62;
  indices[1] = (mid_index - 1 + 62) % 62;
  indices[2] = mid_index;
  indices[3] = (mid_index + 1) % 62;
  indices[4] = (mid_index + 2) % 62;

  int x_positions[5] = {1, 26, 51, 76, 101};
  int y_positions[5] = {30, 20, 15, 20, 30};
  int colors[5] = {SSD1306_WHITE, SSD1306_WHITE, SSD1306_BLACK, SSD1306_WHITE, SSD1306_WHITE};

  display.setTextSize(4);
  for (int i = 0; i < 5; i++) {
    display.setCursor(x_positions[i], y_positions[i]);
    display.setTextColor(colors[i]);
    display.print(characters[indices[i]]);
  }
}

/*-------------------------------------------------------------------------------------------------*/

void action_menu() {
  Serial.println("Action Menu Loaded");
  display.clearDisplay();
  const int leftColumnX = 1;
  const int rightColumnX = 64;
  const int lineHeight = 10;

  display.setTextSize(1);

  // Initialize action and pos
  action = 0;
  encoder.tick(); // Update encoder state
  pos = encoder.getPosition();

  // Display actions and highlight the first one
  for (int i = 0; i < 12; i++) {
    if (i == action) {
      display.setTextColor(BLACK, WHITE);
    } else {
      display.setTextColor(WHITE);
    }
    int x = i < 6 ? leftColumnX : rightColumnX;
    int y = (i % 6) * lineHeight + 1;
    display.setCursor(x, y);
    display.print(action_list[i]);
  }

  display.display();

  // Wait until the rotary button is released
  while (digitalRead(ROTARY_BUTTON) == LOW) {
    delay(10);
  }

  while (true) {
    encoder.tick();
    int newPos = encoder.getPosition();

    if (newPos != pos) {
      int delta = newPos - pos;
      pos = newPos;
      action = (action + delta + 12) % 12;

      // Redraw the menu
      display.clearDisplay();
      display.setTextSize(1);

      for (int i = 0; i < 12; i++) {
        if (i == action) {
          display.setTextColor(BLACK, WHITE);
        } else {
          display.setTextColor(WHITE);
        }
        int x = i < 6 ? leftColumnX : rightColumnX;
        int y = (i % 6) * lineHeight + 1;
        display.setCursor(x, y);
        display.print(action_list[i]);
      }

      display.display();
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
  display.setCursor(20, 14);
  display.setTextSize(5);
  display.setTextColor(SSD1306_BLACK);
  display.print("L");
  display.setCursor(88, 14);
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

  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(15, 10);
  display.print("tukii");
  
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

  // If left button pressed, check if it's being held down and encoder is turned
  if (digitalRead(button1Pin) == LOW) {
    int initialPos = encoder.getPosition();  // Record the initial position
    bool hasRotated = false;

    // Keep looping while the button is held down
    while (digitalRead(button1Pin) == LOW) {
      encoder.tick();
      int currentPos = encoder.getPosition();
      int delta = currentPos - initialPos;

      if (delta != 0) {
        hasRotated = true;
        initialPos = currentPos;

        if (leftVal == 1) {
          // Adjust indices for character selection
          ll_index = (ll_index + delta + 62) % 62;
          l_index = (l_index + delta + 62) % 62;
          mid_index = (mid_index + delta + 62) % 62;
          r_index = (r_index + delta + 62) % 62;
          rr_index = (rr_index + delta + 62) % 62;
          leftKey = mid_index;
        } else if (leftVal == 0) {
          // Increment or decrement through the actions
          leftKey = (leftKey + delta + 12) % 12;  // Assuming there are 12 actions
        }

        // Redraw the screen
        main_screen();
      }

      delay(10);  // Small delay
    }

    // After the button is released
    if (!hasRotated) {
      // If the encoder was not rotated, perform the normal action
      if (leftVal == 1) {
        press_button(0);
      } else {
        action_press(leftKey);
      }
    } else {
      // Save the new leftKey
      write_buttons_char(leftKey, leftVal, rightKey, rightVal);
    }
    delay(10);
  }
  // If right button pressed, check if it's being held down and encoder is turned
  if (digitalRead(button2Pin) == LOW) {
    int initialPos = encoder.getPosition();  // Record the initial position
    bool hasRotated = false;

    // Keep looping while the button is held down
    while (digitalRead(button2Pin) == LOW) {
      encoder.tick();
      int currentPos = encoder.getPosition();
      int delta = currentPos - initialPos;

      if (delta != 0) {
        hasRotated = true;
        initialPos = currentPos;

        if (rightVal == 1) {
          // Adjust indices for character selection
          ll_index = (ll_index + delta + 62) % 62;
          l_index = (l_index + delta + 62) % 62;
          mid_index = (mid_index + delta + 62) % 62;
          r_index = (r_index + delta + 62) % 62;
          rr_index = (rr_index + delta + 62) % 62;
          rightKey = mid_index;
        } else if (rightVal == 0) {
          // Increment or decrement through the actions
          rightKey = (rightKey + delta + 12) % 12;  // Assuming there are 12 actions
        }

        // Redraw the screen
        main_screen();
      }

      delay(10);  // Small delay
    }

    // After the button is released
    if (!hasRotated) {
      // If the encoder was not rotated, perform the normal action
      if (rightVal == 1) {
        press_button(1);
      } else {
        action_press(rightKey);
      }
    } else {
      // Save the new rightKey
      write_buttons_char(leftKey, leftVal, rightKey, rightVal);
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