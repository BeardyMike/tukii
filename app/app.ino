/*//|_   _|/////(_) h| (_)/////| (_)/////////| |/(_)/////////////////////////////////////////////////
//    | |  _ __  _| |_ _  __ _| |_ ___  __ _| |_ _  ___  _ __                                      //
//    | | | '_ \| | __| |/ _` | | / __|/ _` | __| |/ _ \| '_ \                                     //
//   _| |_| | | | | |_| | (_| | | \__ \ (_| | |_| | (_) | | | |                                    //
//  |_____|_| |_|_|\__|_|\__,_|_|_|___/\__,_|\__|_|\___/|_| |_|                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////*/
// last edit - 21/01/25
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
#define button1Pin 28  // Left ButtonAAE94
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


/*-------------------------------------------------------------------------------------------------*/

// Define the arkanoid variables
int paddleX = 54;  // Paddle starting position
int ballX = 64, ballY = 32;  // Ball starting position
int ballSpeedX = 1, ballSpeedY = 1;  // Ball speed
bool gameRunning = false;


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

void setup1() {

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
void Serial_Instructor()
{
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("TK_") && command.endsWith("_TK")) {
      command = command.substring(3, command.length() - 3);  // Remove TK_ and _TK

      if (command == "main_screen") {
        main_screen();
      } else if (command == "main_menu") {
        main_menu();
      } else if (command == "main_menu2") {
        main_menu2();
      } else if (command == "character_menu") {
        character_menu();
      } else if (command == "action_menu") {
        action_menu();
      } else if (command == "games_menu") {
        games_menu();
      } else if (command == "about_screen") {
        about_screen();
      } else if (command == "screen_saver") {
        screen_saver();
      } else {
        Serial.println("Unknown command: " + command);
      }
    }
  }
}

class Arkanoid_target {
public:
  int x, y, width, height;
  int health;

  Arkanoid_target(int x, int y, int width, int height) 
    : x(x), y(y), width(width), height(height), health(2) {}

  void render(Adafruit_SSD1306 &display) {
    if (health == 2) {
      display.fillRect(x, y, width, height, WHITE);  // Solid rectangle
    } else if (health == 1) {
      display.drawRect(x, y, width, height, WHITE);  // Hollow rectangle
    }
  }

  void hit() {
    if (health > 0) {
      health--;
    }
  }

  bool isDestroyed() {
    return health == 0;
  }
};

void drawPaddle() {
  display.fillRect(paddleX, 60, 20, 2, WHITE);  // Reduced paddle height
}

void drawBall() {
  display.drawPixel(ballX, ballY, WHITE);  // Single pixel ball instead of rectangle
}

void updateBall() {
  // Update ball position
  ballX += ballSpeedX;
  ballY += ballSpeedY;

  // Simplified boundary checks
  static int topHitCount = 0;
  if (ballX <= 0 || ballX >= SCREEN_WIDTH-1) ballSpeedX = -ballSpeedX;
  if (ballY <= 0) {
    ballSpeedY = -ballSpeedY;
    topHitCount++;
    if (topHitCount >= 5 && abs(ballSpeedY) < 3) {
      ballSpeedY = (ballSpeedY > 0) ? ballSpeedY + 1 : ballSpeedY - 1;
      topHitCount = 0;  // Reset the counter
    }
  }

  // Check for paddle collision and adjust ball speed
  if (ballY >= 58 && ballY <= 62 && ballX >= paddleX && ballX <= paddleX + 20) {
    ballSpeedY = -ballSpeedY;

    int paddleHitPos = ballX - paddleX;
    if (paddleHitPos < 4) {
      // Ball hits the outer left edge of the paddle
      if (ballSpeedX > 0) {
        ballSpeedX = -ballSpeedX;
      } else {
        ballSpeedX = max(ballSpeedX - 1, -3);
      }
    } else if (paddleHitPos > 6) {
      // Ball hits the outer right edge of the paddle
      if (ballSpeedX < 0) {
        ballSpeedX = -ballSpeedX;
      } else {
        ballSpeedX = min(ballSpeedX + 1, 3);
      }
    } else if (paddleHitPos >= 9 && paddleHitPos <= 11) {
      // Ball hits the center 10% of the paddle
      if (ballSpeedX > 0) {
        ballSpeedX = max(ballSpeedX - 1, 1);
      } else {
        ballSpeedX = min(ballSpeedX + 1, -1);
      }
    }
  }

  // Game over check
  if (ballY >= SCREEN_HEIGHT-1) gameRunning = false;
}

void Arkanoid_game() {
  // Initialize game state
  paddleX = 54;
  ballX = random(0, SCREEN_WIDTH);
  ballY = 55; 
  ballSpeedX = (random(0, 2) == 0) ? 1 : -1;
  ballSpeedY = -1;
  gameRunning = true;
  // Create targets
  const int numTargets = 12;
  Arkanoid_target targets[numTargets] = {
    Arkanoid_target(5, 3, 15, 5),
    Arkanoid_target(25, 3, 15, 5),
    Arkanoid_target(45, 3, 15, 5),
    Arkanoid_target(65, 3, 15, 5),
    Arkanoid_target(85, 3, 15, 5),
    Arkanoid_target(105, 3, 15, 5),
    Arkanoid_target(5, 13, 15, 5),
    Arkanoid_target(25, 13, 15, 5),
    Arkanoid_target(45, 13, 15, 5),
    Arkanoid_target(65, 13, 15, 5),
    Arkanoid_target(85, 13, 15, 5),
    Arkanoid_target(105, 13, 15, 5)
  };

  bool allDestroyed = false;

  // Game loop
  while (gameRunning) {
    display.clearDisplay();

    // Optimized paddle movement
    if (digitalRead(button1Pin) == LOW) {
      paddleX = max(0, paddleX - 3);  // Increased speed, using max instead of constrain
    }
    if (digitalRead(button2Pin) == LOW) {
      paddleX = min(108, paddleX + 3);  // Increased speed, using min instead of constrain
    }

    updateBall();

    // Check for collisions with targets
    for (int i = 0; i < numTargets; i++) {
      if (!targets[i].isDestroyed() && ballX >= targets[i].x && ballX <= targets[i].x + targets[i].width &&
          ballY >= targets[i].y && ballY <= targets[i].y + targets[i].height) {
        targets[i].hit();
        ballSpeedY = -ballSpeedY;
      }
    }

    // Draw targets
    for (int i = 0; i < numTargets; i++) {
      targets[i].render(display);
    }

    drawPaddle();
    drawBall();
    display.display();

    // Check if all targets are destroyed
    bool allDestroyed = true;
    for (int i = 0; i < numTargets; i++) {
      if (!targets[i].isDestroyed()) {
        allDestroyed = false;
        break;
      }
    }

    if (allDestroyed) {
      gameRunning = false;
    }

    // Quick exit check
    if (digitalRead(ROTARY_BUTTON) == LOW) {
      while (digitalRead(ROTARY_BUTTON) == LOW);
      return main_screen();
    }
  }

  // Game complete screen
  if (allDestroyed) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 25);
    display.print("Well Done");
    display.display();
    delay(1000);  // Reduced delay
  } else {
    // Quick game over screen
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 25);
    display.print("Game Over");
    display.display();
    delay(500);  // Reduced delay
  }
  main_screen();

  // Game loop
  while (gameRunning) {
    display.clearDisplay();

    // Optimized paddle movement with speed increase
    static unsigned long lastButtonPressTime = 0;
    static int paddleSpeed = 1;

    if (digitalRead(button1Pin) == LOW) {
      unsigned long currentTime = millis();
      if (currentTime - lastButtonPressTime > 400) {  // Increase speed every 500ms
      paddleSpeed = min(paddleSpeed + 1, 3);  // Max speed is 3
      lastButtonPressTime = currentTime;
      }
      paddleX = max(0, paddleX - paddleSpeed);
    } else if (digitalRead(button2Pin) == LOW) {
      unsigned long currentTime = millis();
      if (currentTime - lastButtonPressTime > 400) {  // Increase speed every 500ms
      paddleSpeed = min(paddleSpeed + 1, 3);  // Max speed is 3
      lastButtonPressTime = currentTime;
      }
      paddleX = min(108, paddleX + paddleSpeed);
    } else {
      paddleSpeed = 1;  // Reset speed when button is released
    }

    updateBall();
    drawPaddle();
    drawBall();
    display.display();

    // Quick exit check
    if (digitalRead(ROTARY_BUTTON) == LOW) {
      while (digitalRead(ROTARY_BUTTON) == LOW);
      return main_screen();
    }
  }
}

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
// Screens
void screen_saver(){

// Icons  
// 'rain', 32x26px
const unsigned char bitmap_rain [] PROGMEM = {
	0x03, 0xfc, 0x00, 0x00, 0x04, 0x02, 0x00, 0x00, 0x08, 0x01, 0x1c, 0x00, 0x10, 0x01, 0x22, 0x00, 
	0x20, 0x00, 0xc1, 0x00, 0x20, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0xb8, 0x40, 0x00, 0x00, 0x44, 
	0x40, 0x00, 0x00, 0x04, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 
	0x20, 0x00, 0x00, 0x02, 0x10, 0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x04, 0x07, 0xff, 0xff, 0xf8, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x82, 0x20, 0x11, 0x11, 0x24, 0x40, 0x22, 0x22, 0x48, 0x80, 
	0x40, 0x10, 0x82, 0x10, 0x09, 0x20, 0x04, 0x20, 0x12, 0x41, 0x08, 0x40, 0x24, 0x02, 0x00, 0x00, 
	0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'snow', 32x26px
const unsigned char bitmap_snow [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 
	0x00, 0x11, 0x10, 0x00, 0x00, 0x09, 0x20, 0x00, 0x00, 0x05, 0x40, 0x00, 0x01, 0x0b, 0xa1, 0x00, 
	0x00, 0x41, 0x04, 0x00, 0x00, 0xd5, 0x56, 0x00, 0x04, 0x13, 0x90, 0x40, 0x02, 0x75, 0x5c, 0x80, 
	0x09, 0x09, 0x21, 0x20, 0x02, 0x75, 0x5c, 0x80, 0x04, 0x13, 0x90, 0x40, 0x00, 0xd5, 0x56, 0x00, 
	0x00, 0x41, 0x04, 0x00, 0x01, 0x0b, 0xa1, 0x00, 0x00, 0x05, 0x40, 0x00, 0x00, 0x09, 0x20, 0x00, 
	0x00, 0x11, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'sunny', 32x26px
const unsigned char bitmap_sunny [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x80, 0x00, 0x83, 0xc1, 0x00, 0x00, 0x4c, 0x32, 0x00, 0x00, 0x10, 0x08, 0x00, 
	0x00, 0x20, 0x04, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x80, 0x01, 0x00, 
	0x00, 0x80, 0x01, 0x00, 0x0e, 0x80, 0x01, 0x70, 0x00, 0x80, 0x01, 0x00, 0x00, 0x40, 0x02, 0x00, 
	0x00, 0x40, 0x02, 0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x4c, 0x32, 0x00, 
	0x00, 0x83, 0xc1, 0x00, 0x01, 0x00, 0x00, 0x80, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'wind', 32x26px
const unsigned char bitmap_wind [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 
	0x00, 0x00, 0x10, 0x40, 0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x00, 0x20, 
	0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x03, 0x80, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x1f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x30, 0x7f, 0xfc, 0x00, 0x08, 0x00, 0x02, 0x02, 0x04, 
	0x00, 0x01, 0x04, 0x02, 0x00, 0x40, 0x84, 0x02, 0x00, 0x80, 0x88, 0x02, 0x00, 0x80, 0x88, 0x02, 
	0x00, 0x80, 0x84, 0x04, 0x00, 0x81, 0x02, 0x08, 0x00, 0x42, 0x01, 0xf0, 0x00, 0x3c, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'overcast', 32x26px
const unsigned char bitmap_overcast [] PROGMEM = {
	0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x02, 0x00, 0x04, 0x00, 0x01, 0x1e, 0x08, 
	0x00, 0x00, 0x7f, 0x90, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x7b, 0xff, 0xf0, 
	0x00, 0x03, 0xff, 0xf0, 0x07, 0xf9, 0xff, 0xfb, 0x08, 0x05, 0xc7, 0xf8, 0x10, 0x02, 0xbb, 0xf8, 
	0x20, 0x02, 0x45, 0xf8, 0x40, 0x01, 0x82, 0xf0, 0x40, 0x00, 0x01, 0x00, 0x80, 0x00, 0x01, 0x70, 
	0x80, 0x00, 0x00, 0x88, 0x80, 0x00, 0x00, 0x08, 0x80, 0x00, 0x00, 0x04, 0x80, 0x00, 0x00, 0x04, 
	0x80, 0x00, 0x00, 0x04, 0x40, 0x00, 0x00, 0x04, 0x20, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x08, 
	0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00
};
// 'wallet', 16x13px
const unsigned char bitmap_wallet [] PROGMEM = {
	0x3f, 0xfc, 0x7f, 0xff, 0xe0, 0x01, 0xff, 0xf9, 0x80, 0x05, 0x81, 0x85, 0x82, 0x05, 0x87, 0x05, 
	0x82, 0x05, 0x82, 0x05, 0x87, 0x87, 0x80, 0x04, 0x7f, 0xf8
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 688)
const int bitmap_allArray_LEN = 6;
const unsigned char* bitmap_Array[6] = {
	bitmap_overcast,
	bitmap_rain,
	bitmap_snow,
	bitmap_sunny,
	bitmap_wallet,
	bitmap_wind
};

  display.clearDisplay();
  display.drawBitmap(7, 7, bitmap_Array[4], 16, 13, WHITE);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(28,7);
  display.print("130,401");
  display.display();        // Display the screen
  return;
}

void main_screen() {
  display.clearDisplay();   // Clear the display
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
  delay(250); // Debounce delay to prevent the dial rotations being registered errantly.
  while (true) {
    
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
   // Update encoder state
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

  Arkanoid_game();
  
}

/*-------------------------------------------------------------------------------------------------*/

void about_screen() {
  Serial.println("About Screen Loaded");
  display.clearDisplay();

  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(5, 10);
  display.print("tukii");
  
  display.setTextSize(1);
  display.setCursor(10, 50);
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

// loop for CPU 0
void loop() {
  Serial_Instructor();
  
  // Helper function to handle button press and rotation
  auto handleButtonPress = [&](int buttonPin, int &key, int &val, int buttonIndex) {
    if (digitalRead(buttonPin) == LOW) {
      int initialPos = encoder.getPosition();
      bool hasRotated = false;
      while (digitalRead(buttonPin) == LOW) {
        delay(25);
        int currentPos = encoder.getPosition();
        int delta = currentPos - initialPos;

        if (delta != 0) {
          hasRotated = true;
          if (val == 1) {
            key = (key + delta + 62) % 62;
            
            main_screen();
          } else if (val == 0) {
            key = (key + delta + 12) % 12;
            
            main_screen();
          }
          
          initialPos = currentPos;
        }
      }

      if (!hasRotated) {
        if (val == 1) {
          press_button(buttonIndex);
        } else {
          action_press(key);
        }
      } else {
        write_buttons_char(leftKey, leftVal, rightKey, rightVal);
      }
      
    }
  };

  // Handle left and right button presses
  handleButtonPress(button1Pin, leftKey, leftVal, 0);
  handleButtonPress(button2Pin, rightKey, rightVal, 1);

  // Handle rotary button press
  if (digitalRead(ROTARY_BUTTON) == LOW) {
    delay(100);
    if (dial_still_pressed == 0) {
      main_menu();
    }
    dial_still_pressed = 1;
  }
  dial_still_pressed = 0;

  // Handle encoder scroll
  if (digitalRead(button1Pin) == HIGH && digitalRead(button2Pin) == HIGH && digitalRead(ROTARY_BUTTON) == HIGH) {
    
    int newPos = encoder.getPosition();
    int delta = newPos - pos;

    if (delta != 0) {
      Mouse.move(0, 0, delta * 1);
      pos = newPos;
    }
  }
}

// loop for CPU 1
void loop1(){
    encoder.tick();
}
//
// ^ Loop
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////