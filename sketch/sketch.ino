#include <WebUSB.h>

#include <SPFD5408_Adafruit_GFX.h>     // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h>  // Hardware-specific library

WebUSB WebUSBSerial(0, "localhost:3000");

#define Serial WebUSBSerial

// LCD Pin
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4  // Optional : otherwise connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0xE007
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// Init LCD
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// Dimensions
uint16_t width = 0;
uint16_t height = 0;

#define HEADER_HEIGHT 30
#define FOOTER_HEIGHT 30
#define BORDER 10

// Map dimensions
#define MAP_WIDTH 23
#define MAP_HEIGHT 25

const uint16_t size = MAP_WIDTH * MAP_HEIGHT;

// The tile values for the map
int8_t snakeMap[size];
int8_t prevSnakeMap[size];

// Snake head details
int8_t headxpos;
int8_t headypos;
int8_t direction;

// Amount of food the snake has (How long the body is)
int16_t food = 3;

// Determine if game is running
bool running;

// Snake function
void drawBorder();
void initGame();
void printMap();
void initMap();
void move(int8_t dx, int8_t dy);
void update();
void changeDirection(char key);
void clearScreen();
void generateFood();
char getMapValue(int8_t value);

void setup() {
  while (!Serial) {
    ;
  }
  Serial.begin(9600);
  Serial.write("Sketch begins.\r\n");
  Serial.flush();

  tft.reset();
  tft.begin(0x9341);
  tft.setRotation(0);

  width = tft.width() - 1;
  height = tft.height() - 1;

  // Initializing the game
  initGame();
}

void loop() {
  if (!running) {
    tft.setCursor(BORDER, height - 30);
    tft.setTextColor(CYAN);
    tft.print("Game over");
  }

  if (Serial && Serial.available()) {
    if (running) {
      char key = (char)Serial.read();
      changeDirection(key);
    } else {
      // start new game
      initGame();
    }
  }

  if (running) {
    // Upate the map
    update();

    // Print the map
    printMap();

    // wait 0.5 seconds
    uint16_t speed = min(food - 3, 10) * 30;
    delay(400 - speed);
  }
}

void drawBorder() {
  uint16_t width = tft.width() - 1;
  uint16_t height = tft.height() - 1;

  tft.fillScreen(GREEN);
  tft.fillRect(BORDER, BORDER + HEADER_HEIGHT, (width - BORDER * 2),
               (height - BORDER * 2 - HEADER_HEIGHT - FOOTER_HEIGHT), WHITE);

  tft.setTextSize(2);
  tft.setCursor(BORDER, 10);
  tft.setTextColor(WHITE);
  tft.print("WebUSB - Snake");
}

void initGame() {
  Serial.println("Starting new game...");
  Serial.flush();
  food = 3;
  direction = 4;
  drawBorder();
  initMap();
  running = true;
}

// Changes snake direction from input
void changeDirection(char key) {
  /*
      W
    A + D
      S

      1
    4 + 2
      3
    */
  switch (key) {
    case 'w':
      if (direction != 3) {
        direction = 1;
        Serial.println("Snake direction changed. Going up...");
        Serial.flush();
      }
      break;
    case 'd':
      if (direction != 4) {
        direction = 2;
        Serial.println("Snake direction changed. Going right...");
        Serial.flush();
      }
      break;
    case 's':
      if (direction != 1) {
        direction = 3;
        Serial.println("Snake direction changed. Going down...");
        Serial.flush();
      }
      break;
    case 'a':
      if (direction != 2) {
        direction = 4;
        Serial.println("Snake direction changed. Going left...");
        Serial.flush();
      }
      break;
  }
}

// Moves snake head to new location
void move(int8_t dx, int8_t dy) {
  for (uint16_t i = 0; i < size; i++) {
    prevSnakeMap[i] = snakeMap[i];
  }
  // determine new head position
  int8_t newx = headxpos + dx;
  int8_t newy = headypos + dy;

  // Check if there is food at location
  if (snakeMap[newx + newy * MAP_WIDTH] == -2) {
    for (uint16_t i = 0; i < size; i++) {
      if (snakeMap[i] > 0) {
        snakeMap[i]++;
      }
    }

    Serial.println("Snake has grown in size...");
    Serial.flush();

    // Increase food value (body length)
    food++;

    // Generate new food on map
    generateFood();
  }

  // Check location is free
  else if (snakeMap[newx + newy * MAP_WIDTH] != 0) {
    running = false;
    Serial.println("Game over");
    Serial.flush();
  }

  // Move head to new location
  headxpos = newx;
  headypos = newy;
  snakeMap[headxpos + headypos * MAP_WIDTH] = food + 1;
}

// Generates new food on map
void generateFood() {
  uint8_t x = 0;
  uint8_t y = 0;
  do {
    // Generate random x and y values within the map
    x = rand() % (MAP_WIDTH - 2) + 1;
    y = rand() % (MAP_HEIGHT - 2) + 1;

    // If location is not free try again
  } while (snakeMap[x + y * MAP_WIDTH] != 0);

  // Place new food
  snakeMap[x + y * MAP_WIDTH] = -2;
}

// Updates the map
void update() {
  // Move in direction indicated
  switch (direction) {
    case 1:
      move(0, -1);
      break;
    case 2:
      move(1, 0);
      break;
    case 3:
      move(0, 1);
      break;
    case 4:
      move(-1, 0);
      break;
  }

  // Reduce snake values on map by 1
  for (uint16_t i = 0; i < size; i++) {
    if (snakeMap[i] > 0) snakeMap[i]--;
  }
}

// Initializes map
void initMap() {
  for (uint16_t i = 0; i < size; i++) {
    prevSnakeMap[i] = 0;
    snakeMap[i] = 0;
  }

  // Places the initual head location in middle of map
  headxpos = MAP_WIDTH / 2;
  headypos = MAP_HEIGHT / 2;
  snakeMap[headxpos + headypos * MAP_WIDTH] = 3;
  snakeMap[headxpos + 1 + headypos * MAP_WIDTH] = 2;
  snakeMap[headxpos + 2 + headypos * MAP_WIDTH] = 1;

  // Places top and bottom walls
  for (uint8_t x = 0; x < MAP_WIDTH; ++x) {
    snakeMap[x] = -1;
    snakeMap[x + (MAP_HEIGHT - 1) * MAP_WIDTH] = -1;
  }

  // Places left and right walls
  for (uint8_t y = 0; y < MAP_HEIGHT; y++) {
    snakeMap[0 + y * MAP_WIDTH] = -1;
    snakeMap[(MAP_WIDTH - 1) + y * MAP_WIDTH] = -1;
  }

  // Generates first food
  generateFood();
}

// Prints the map to console
void printMap() {
  for (uint8_t x = 0; x < MAP_WIDTH; ++x) {
    for (uint8_t y = 0; y < MAP_HEIGHT; ++y) {
      // Prints the value at current x,y location
      char value = getMapValue(snakeMap[x + y * MAP_WIDTH]);
      char prevValue = getMapValue(prevSnakeMap[x + y * MAP_WIDTH]);

      uint8_t centerX = (x + 1) * 10;
      uint8_t centerY = (y + 1) * 10 + HEADER_HEIGHT;
      if (prevValue == '*' && value == ' ') {
        tft.fillCircle(centerX, centerY, 5, WHITE);
      }

      switch (value) {
        case '*':
          tft.fillCircle(centerX, centerY, 5, RED);
          break;
        case 'O':
          tft.fillCircle(centerX, centerY, 5, CYAN);
          break;
      }
    }
    // Ends the line for next x value
  }
}

// Returns graphical character for display from map value
char getMapValue(int8_t value) {
  // Returns a part of snake body
  if (value > 0) return '*';

  switch (value) {
      // Return wall
    case -1:
      return 'X';
      // Return food
    case -2:
      return 'O';
  }

  return ' ';
}