#include <WebUSB.h>
WebUSB WebUSBSerial(0, "localhost:3000");

#include <SPFD5408_Adafruit_GFX.h>    // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library

#define Serial WebUSBSerial

// LCD Pin

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4 // Optional : otherwise connect to Arduino's reset pin

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
uint8_t index = 0;
uint16_t width = 0;
uint16_t height = 0;

void run();
void printMap();
void initMap();
void move(int dx, int dy);
void update();
void changeDirection(char key);
void clearScreen();
void generateFood();

char getMapValue(int value);

// Map dimensions
const int8_t mapwidth = 23;
const int8_t mapheight = 25;

const int size = mapwidth * mapheight;

// The tile values for the map
int8_t snakeMap[size];
int8_t prevSnakeMap[size];

// Snake head details
int8_t headxpos;
int8_t headypos;
int8_t direction;

// Amount of food the snake has (How long the body is)
int8_t food = 3;

// Determine if game is running
bool running;

void setup()
{
  while (!Serial)
  {
    ;
  }
  Serial.begin(9600);
  Serial.write("Sketch begins.\r\n");
  Serial.flush();
  index = 0;

  tft.reset();

  tft.begin(0x9341);

  tft.setRotation(0); // Need for the Mega, please changed for your choice or rotation initial

  width = tft.width() - 1;
  height = tft.height() - 1;

  drawBorder();

  initMap();
  running = true;
}

uint8_t HEADER_HEIGHT = 30;
uint8_t FOOTER_HEIGHT = 30;
uint8_t border = 10;

void drawBorder()
{

  uint16_t width = tft.width() - 1;
  uint16_t height = tft.height() - 1;

  tft.fillScreen(GREEN);
  tft.fillRect(border, border + HEADER_HEIGHT, (width - border * 2), (height - border * 2 - HEADER_HEIGHT - FOOTER_HEIGHT), WHITE);

  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.setTextColor(WHITE);
  tft.print("WebUSB - Snake");
}

void loop()
{
  if (!running)
  {
    tft.setCursor(10, height - 30);
    tft.setTextColor(CYAN);
    tft.print("Game over");
  }

  if (Serial && Serial.available())
  {
    if (running)
    {
      char key = (char)Serial.read();
      // tft.setCursor(width - 110, height - 30);
      // tft.setTextColor(CYAN);
      // tft.print(key);
      changeDirection(key);
    }
    else
    {
      // start new game
      food = 3;
      drawBorder();
      initMap();
      running = true;
    }
    /*tft.setCursor(0, 0);
    tft.setTextSize(1);
    tft.setTextColor(WHITE);
    tft.print(count);

    count++;
    tft.setCursor(0, 0);
    tft.setTextColor(RED);
    tft.print(count);*/

    // color[index++] = Serial.read();

    /*if (index == 3)
    {
      Serial.print("Set LED to ");
      Serial.print(color[0]);
      Serial.print(", ");
      Serial.print(color[1]);
      Serial.print(", ");
      Serial.print(color[2]);
      Serial.print(".\r\n");
      Serial.flush();
      index = 0;
    }*/

    // If a key is pressed
  }

  if (running)
  {

    // Upate the map
    update();

    // Print the map
    printMap();

    // wait 0.5 seconds
    delay(500);
  }

  /*if (prev != color[0])
  {

    tft.setCursor(55, 150);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print(prev);

    tft.setCursor(55, 150);
    tft.setTextSize(2);
    tft.setTextColor(color[0]);
    tft.println(color[0]);

    // tft.fillScreen(color[0]);
  }
  prev = color[0];

  tft.setCursor(100, 100);
  tft.println(now());*/
}

// Changes snake direction from input
void changeDirection(char key)
{
  /*
      W
    A + D
      S

      1
    4 + 2
      3
    */
  switch (key)
  {
  case 'w':
    if (direction != 3)
      direction = 1;
    break;
  case 'd':
    if (direction != 4)
      direction = 2;
    break;
  case 's':
    if (direction != 1)
      direction = 3;
    break;
  case 'a':
    if (direction != 2)
      direction = 4;
    break;
  }
}

// Moves snake head to new location
void move(int dx, int dy)
{
  for (int i = 0; i < size; i++)
  {
    prevSnakeMap[i] = snakeMap[i];
  }
  // determine new head position
  int newx = headxpos + dx;
  int newy = headypos + dy;

  // Check if there is food at location
  if (snakeMap[newx + newy * mapwidth] == -2)
  {
    for (int i = 0; i < size; i++)
    {
      if (snakeMap[i] > 0)
      {
        snakeMap[i]++;
      }
    }

    // Increase food value (body length)
    food++;

    // Generate new food on map
    generateFood();
  }

  // Check location is free
  else if (snakeMap[newx + newy * mapwidth] != 0)
  {
    running = false;
  }

  // Move head to new location
  headxpos = newx;
  headypos = newy;
  snakeMap[headxpos + headypos * mapwidth] = food + 1;
}

// Clears screen
void clearScreen()
{
  // Clear the screen
  // drawBorder();
}

int8_t pos = 0;
// Generates new food on map
void generateFood()
{
  int x = 0;
  int y = 0;
  do
  {
    // Generate random x and y values within the map
    x = rand() % (mapwidth - 2) + 1;
    y = rand() % (mapheight - 2) + 1;

    // If location is not free try again
  } while (snakeMap[x + y * mapwidth] != 0);

  // Place new food
  snakeMap[x + y * mapwidth] = -2;

  pos = x + y * mapwidth;
}

// Updates the map
void update()
{
  // Move in direction indicated
  switch (direction)
  {
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
  for (int i = 0; i < size; i++)
  {
    if (snakeMap[i] > 0)
      snakeMap[i]--;
  }
}

// Initializes map
void initMap()
{
  for (int i = 0; i < size; i++)
  {
    snakeMap[i] = 0;
  }

  // Places the initual head location in middle of map
  headxpos = mapwidth / 2;
  headypos = mapheight / 2;
  snakeMap[headxpos + headypos * mapwidth] = 4;
  snakeMap[headxpos + 1 + headypos * mapwidth] = 3;
  snakeMap[headxpos + 2 + headypos * mapwidth] = 2;

  // Places top and bottom walls
  for (int x = 0; x < mapwidth; ++x)
  {
    snakeMap[x] = -1;
    snakeMap[x + (mapheight - 1) * mapwidth] = -1;
  }

  // Places left and right walls
  for (int y = 0; y < mapheight; y++)
  {
    snakeMap[0 + y * mapwidth] = -1;
    snakeMap[(mapwidth - 1) + y * mapwidth] = -1;
  }

  // Generates first food
  generateFood();
}

// Prints the map to console
void printMap()
{
  for (int x = 0; x < mapwidth; ++x)
  {
    for (int y = 0; y < mapheight; ++y)
    {
      // Prints the value at current x,y location
      char value = getMapValue(snakeMap[x + y * mapwidth]);
      char prevValue = getMapValue(prevSnakeMap[x + y * mapwidth]);

      if (prevValue == '*' && value == ' ')
      {
        tft.fillCircle((x + 1) * 10, (y + 1) * 10 + HEADER_HEIGHT, 5, WHITE);
      }

      switch (value)
      {
      case '*':
        tft.fillCircle((x + 1) * 10, (y + 1) * 10 + HEADER_HEIGHT, 5, RED);
        break;
      case ' ':
        // tft.fillCircle((x + 1) * 10, (y + 1) * 10 + HEADER_HEIGHT, 5, WHITE);
        break;
      case 'O':
        tft.fillCircle((x + 1) * 10, (y + 1) * 10 + HEADER_HEIGHT, 5, CYAN);
        break;
      }
    }
    // Ends the line for next x value
  }
}

// Returns graphical character for display from map value
char getMapValue(int value)
{
  // Returns a part of snake body
  if (value > 0)
    return '*';

  switch (value)
  {
    // Return wall
  case -1:
    return 'X';
    // Return food
  case -2:
    return 'O';
  }

  return ' ';
}