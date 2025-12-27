#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <vector>
#include "FS.h"
#include "SD.h"
//#include <WiFi.h>
//#include "time.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MCP23017.h>
#include <Keypad.h>
// Fonts
#include "tinos_fnt.h" 
#include "Orbitron_access_fnt.h"
#include "orbitron_fnt.h" 
#include "dseg14_fnt.h" 
// Bitmaps
#include "pfp-stallone_bmp.h"
#include "GRANDSTAND_bmp.h"
#include "thumbprint_bmp.h" 

//Function Prototypes
void displayImg(String name);
void displayDoc(String name);
uint8_t getFingerprintID();
void updatecursor(int ubound, int lbound);
void drawcursor();
void displayLine(String text, int line);
void listDirMem(fs::FS &fs, const char *dirname, uint8_t levels);
uint16_t read16(File &f);
uint32_t read32(File &f);

//fingerprint
HardwareSerial serialPort(2);  // use UART2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&serialPort);
uint8_t getFingerprintID();

// declaring Sprites
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite curs = TFT_eSprite(&tft);
TFT_eSprite logo = TFT_eSprite(&tft);
TFT_eSprite rights = TFT_eSprite(&tft);
TFT_eSprite launch = TFT_eSprite(&tft);
TFT_eSprite world = TFT_eSprite(&tft);
TFT_eSprite menu = TFT_eSprite(&tft); 
TFT_eSprite screen = TFT_eSprite(&tft);


// set LCD address, number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// Oled setup Declaration for an SSD1306 display connected to I2C
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
Adafruit_SSD1306 oled1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SSD1306 oled2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Initialize MCP23017
Adafruit_MCP23017 mcp;

//variables declaration
int dt = 100;
int potpin = 14;
int h = 100;
int xpin = 32;
int ypin = 33;
int xval;
int yval;
int cursx = 240;
int cursy = 160;
int deltacursx = 0;
int deltacursy = 0;
int booth = 65;

int tftw = 450;
int tfth = 306;
int screenw = 290;
int screenh = 218;
int indentw = 30;
int indenth = 38;

int pfpw = 102;
int titleh  = 40;
int labelh  = 20;
int menuh   = 218;

int filno = 0;
int dirno = 0;
int iconw = 30;
int iconh = 30;

/*//credentials for time
const char* ssid       = "PTCL-BB";
const char* password   = "07860786";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 18000;
const int   daylightOffset_sec = 3600;*/

// Keypad setup

  const byte ROWS = 4;   // Number of rows
  const byte COLS = 10;  // Number of columns
  char keysUpper[ROWS][COLS] = {
    { 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'O', 'P', '<' },
    { 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '\n' },
    { 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '.' },
    { '1', '2', '3', ' ', '5', '6', 'I', '8', ' ', '0' }
  };

  char keysLower[ROWS][COLS] = {
    { 'q', 'w', 'e', 'r', 't', 'y', 'u', 'o', 'p', '<' },
    { 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\n' },
    { 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '.' },
    { '1', '2', '3', ' ', '4', '5', 'i', '6', '7', '8' }
  };

  byte rowPins[ROWS] = { 12, 15, 14, 13 };                // MCP23017 pins connected to rows
  byte colPins[COLS] = { 0, 1, 2, 3, 4, 5, 7, 6, 9, 8 };  // MCP23017 pins connected to columns 8, 9, 6, 7, 5, 4, 3, 2, 1,0

  // Custom Keypad class to work with MCP23017
  class MCP23017_Keypad : public Keypad {
  public:
    MCP23017_Keypad(char *userKeymap, byte *row, byte *col, byte numRows, byte numCols, Adafruit_MCP23017 &expander)
      : Keypad(userKeymap, row, col, numRows, numCols), mcp(expander) {}

    void pin_mode(byte pinNum, byte mode) {
      if (mode == INPUT_PULLUP) {
        mcp.pullUp(pinNum, HIGH);
      } else {
        mcp.pinMode(pinNum, mode);
      }
    }

    void pin_write(byte pinNum, boolean level) {
      mcp.digitalWrite(pinNum, level);
    }

    int pin_read(byte pinNum) {
      return mcp.digitalRead(pinNum);
    }
  private:
    Adafruit_MCP23017 &mcp;
  };

  // Initialize keypad with upper case keys
  MCP23017_Keypad keypadUpper(makeKeymap(keysUpper), rowPins, colPins, ROWS, COLS, mcp);
  MCP23017_Keypad keypadLower(makeKeymap(keysLower), rowPins, colPins, ROWS, COLS, mcp);
  Keypad *keypad = &keypadUpper;

  String inputText = "";  // Variable to store typed text
  int currentLine = 0;    // Variable to track the current line number
  int istate;
  const int maxLines = 8;  // Max number of lines on the display
  String lines[8];         // Array to store each line of text

  bool isUpperCaseMode = true;  // Flag to indicate if the case is upper (true) or lower (false)

 struct fileType {
  String search_name; 
  String label_name;
  std::vector<String> array;

 };

//array of vectors (expandable arrays) for each filytype
fileType application[3] = {
  {"/Documents", "DOCUMENTS", {}},
  {"/Images", "IMAGES", {}},
  {"/Projects", "PROJECTS", {}}
};

//struct for people
  struct person {
    int ID;
    String name;
    String alias;
    String company;

  };

//coordinate datatype setup
  struct coordinates {
    int x;
    int y;
  };

  //setting coordinates to default value
  coordinates cursor = { 240, 160 };

//peripheral flag datatype setup
  struct peripherals {
    String name;
    bool alert;
  };

  peripherals devices[5]{
    { "keyboard", 0 },
    { "thm sensor", 0 },
    { "terminal", 0 },
    { "status view", 0 },
    { "mem drive", 1 }
  };

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

/*//connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);*/


//initialize fingerprint
  while (!Serial)
    ;  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    devices[2].alert = 1;
    /*while (1)
         {
             delay(1);
         }*/
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);

  finger.getTemplateCount();
  finger.LEDcontrol(0);

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  } else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains ");
    Serial.print(finger.templateCount);
    Serial.println(" templates");
  }


// initalize SD card
  if (!SD.begin(5)) {
    Serial.println("Card Mount Failed");
    return;
    devices[5].alert = 1;
  }


// pin_definitions
  pinMode(xpin, INPUT);
  pinMode(ypin, INPUT);
  pinMode(potpin, INPUT);

// Initialize MCP23017
  mcp.begin();  // Default address 0x20 keyboard 
  Wire.begin();


// initialize TFT
  tft.init();
  tft.setRotation(1);
  curs.setColorDepth(1); 
  curs.createSprite(21, 21);
  logo.createSprite(480, 89);
  rights.setColorDepth(1); 
  rights.createSprite(480, 110);
  launch.setColorDepth(1); 
  launch.createSprite(200, 20);
  menu.setColorDepth(1); 
  menu.createSprite(tftw-screenw-5, menuh-1);
  screen.setColorDepth(1); 
  screen.createSprite(screenw,screenh);

  tft.fillScreen(TFT_BLACK);

// initialize LCD
  lcd.init();
  lcd.backlight();

// initialize Oled1
  if (!oled1.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    devices[3].alert = 1;
  }
  oled1.clearDisplay();
  oled1.display();

// initialize Oled2
  if (!oled2.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    devices[4].alert = 1;
  }
  oled2.clearDisplay();
  oled2.display();

  oled2.drawRect(0,0, 128, 64, WHITE);
  oled2.display();


//###############################   BEGIN   ####################################


// GRANDSTAND LOGO
  logo.setSwapBytes(true);
  logo.pushImage(20, 0, 480, 89, GRANDSTAND);  // x, y, width, height, file
  logo.pushSprite(0, booth);
  logo.deleteSprite();

  delay(50);
  rights.drawString("(C) Copyright GRANDSTAND Corp. 1985.", 125, 0, 2);
  delay(50);
  rights.drawString("All Rights Reserved.", 190, 15, 2);
  delay(80);
  rights.drawString("This GRANDSTAND PC MFC.103 is property of stallone_85.", 60, 50, 2);
  rights.pushSprite(0, booth + 115);
  rights.deleteSprite();

  delay(2000);

// Peripheral check

  launch.drawString("Launching KMT.os.", 0, 0, 2);
  launch.pushSprite(30, booth + 215);
  delay(700);
  launch.drawString("Launching KMT.os_.", 0, 0, 2);
  launch.pushSprite(30, booth + 215);
  delay(700);

  lcd.setCursor(0, 0);

  lcd.print("Peripherals...");
  oled2.setRotation(2);
  oled2.setTextSize(1);
  oled2.fillRect(0, 0, SCREEN_WIDTH, 10, WHITE);
  oled2.setTextColor(BLACK, WHITE);
  oled2.setCursor(0, 1);
  oled2.print(" SCANNING DEVICES...");
  oled2.display();

  oled1.setRotation(0);
  oled1.setTextSize(1);
  oled1.fillRect(0, 0, SCREEN_WIDTH, 10, WHITE);
  oled1.setTextColor(BLACK, WHITE);
  oled1.setCursor(0, 1);
  oled1.print(">LAUNCHING TERMINAL.");
  oled1.display();

  lcd.setCursor(0, 1);
  lcd.print("(");
  delay(300);

  for (int i; i < 5; i++) {
    lcd.setCursor(1 + (i * 2), 1);
    lcd.print("#");
    delay(50);
    lcd.setCursor(2 + (i * 2), 1);
    lcd.print("#");
    lcd.setCursor(11, 1);
    lcd.print((i + 1) * 20);
    oled2.setTextColor(WHITE, BLACK);
    oled2.setCursor(3, 11 + (i * 11));  //both 11's must be same??
    oled2.println(devices[i].name);
    oled2.display();
    if (devices[i].alert == 0) {
      oled2.setTextColor(WHITE, BLACK);
      oled2.setCursor(80, 11 + (i * 11));  //both 11's must be same??
      oled2.println("ACTIVE");
      oled2.display();
    } else {
      oled2.setTextColor(BLACK, WHITE);
      oled2.setCursor(80, 11 + (i * 11));  //both 11's must be same??
      oled2.println("INACTIVE");
      oled2.display();
      while (devices[i].alert == 1) {
        char key = keypad->getKey();
        if (key) {
          if (key == '3') {
            devices[i].alert = 0;
            break;
          }
        }
      }
    }

    delay(50);
  }
  lcd.clear();
  oled2.clearDisplay();
  oled2.display();
  oled2.drawRect(0,0, 128, 64, WHITE);
  oled2.display();

// Memory Drive check

  launch.drawString("Launching KMT.os_ _.", 0, 0, 2);
  launch.pushSprite(30, booth + 215);

  oled2.setTextColor(BLACK, WHITE);
  oled2.setCursor(1, 1); 
  oled2.println("  MEMORY DRIVE CHECK.");
  oled2.display();
  delay (100);

  oled2.setTextColor(WHITE, BLACK);
  oled2.setCursor(4, 12); 
  oled2.println("MEM_TYPE: ");
  oled2.display();
  delay (100);
  uint8_t cardType = SD.cardType();
if(cardType == CARD_MMC){
  oled2.setCursor(80, 12);  
  oled2.println("MMC_DISK");
  oled2.display();
} else if(cardType == CARD_SD){
  oled2.setCursor(80, 12);  
  oled2.println("SDSC");
  oled2.display();
} else if(cardType == CARD_SDHC){
  oled2.setCursor(80, 12);  
  oled2.println("SDHC");
  oled2.display();
} else {
  oled2.setCursor(80, 12); 
  oled2.println("UNKNOWN");
  oled2.display();
}

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  oled2.setCursor(4, 24); 
  oled2.println("DISK_CAP: ");
  oled2.display();
  delay(50);
  oled2.setCursor(80, 24); 
  oled2.println(String(cardSize)+ " MB");
  oled2.display();
  delay (50);
  oled2.setTextColor(BLACK, WHITE);
  oled2.setCursor(1,36); 
  oled2.println("     INDEX COUNT...  ");
  oled2.drawFastHLine(0, 35, 128, WHITE);
  oled2.display();
  delay (50);
  oled2.setTextColor(WHITE, BLACK);
  oled2.setCursor(4, 48); 
  oled2.println("DIR: ");
  oled2.setCursor(64, 48); 
  oled2.println("FILE: ");
  oled2.drawRect(28, 43, 32, 21, WHITE); // BOX FOR DIRECTORY NUMBER (dirno)
  oled2.drawRect(94, 43, 34, 21, WHITE); // BOX FOR FILE NUMBER (filno)
  oled2.display();
  listDirMem (SD, "/", 2);

  launch.drawString("Launching KMT.os_ _ _.", 0, 0, 2);
  launch.pushSprite(30, booth + 215);
  launch.deleteSprite();
  delay(700);






// ID SCREEN

  tft.fillScreen(TFT_BLACK);
  tft.drawRect(indentw, indenth, tftw, titleh, TFT_WHITE);   //TITLE WINDOW
  tft.drawRect(indentw, indenth+titleh+5, tftw-((pfpw*2)+3+5), tfth-(indenth+titleh+5), TFT_WHITE);   //LEFT WINDOW

  tft.drawRect(indentw+tftw-(pfpw*2)-3, indenth+titleh+5, (pfpw*2)+3, tfth-(indenth+titleh+5), TFT_WHITE);  //RIGHT WINDOW 
  tft.drawRect(indentw+tftw-(pfpw*2)-3, indenth+titleh+5, (pfpw*2)+1, 161, TFT_WHITE);  //Space for pfp and thumbprint
  tft.drawRect(indentw+tftw-(pfpw*2)-3, indenth+titleh+5, pfpw+2, 146, TFT_WHITE); // divider

  delay(700); 

  tft.setFreeFont(&Orbitron_Medium_30);  // & followed by font name specified at the end  Tinos_Regular_21
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  char title [] = "PROTOTYPE_85";
  for (int i = 0; i < strlen(title); i++){
    tft.drawChar( title[i], indentw+6+((tftw/(strlen(title)))*(i)), indenth+32); // Title
    delay(100);
  }
  

  delay(2000);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.unloadFont();   
  tft.setTextFont(2);
  tft.drawString("Initializing ID Verification Sequence", indentw+1, indenth+titleh+5, 2);

  delay(700);

  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString(" ID Verification Sequence Initialized ", indentw+1, indenth+titleh+5, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  delay(900);
  tft.drawString(" Place finger ", indentw+1, indenth+titleh+5+18, 2);
  finger.LEDcontrol(1);
  delay(900);
  for (int a = 0; a < 3; a++) {
    tft.drawString(" *", indentw+1+ (a*15), (indenth+titleh+5+(18*2)), 2);
    delay(900);
  }

  //identification

  int ID;

  ID = getFingerprintID();
  if (ID == 1) {
    tft.drawString(" Found ID # 001", indentw+1, indenth+titleh+5+(18*7), 2);
    tft.drawString(" Confidence of", indentw+1, indenth+titleh+5+(18*8), 2);
    tft.drawString(String(finger.confidence), 130, indenth+titleh+5+(18*8), 2);
    tft.setSwapBytes(true);
    tft.pushImage(indentw + tftw-pfpw-1, indenth+titleh+6, pfpw, 159, pfp_stallone);  // x, y, width, height, file
    tft.pushImage(indentw + tftw-(pfpw*2)-2, indenth+titleh+6, pfpw, 144, print);  // x, y, width, height, file
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("stallone_85/23-", indentw+tftw-(pfpw*2)-3,  tfth -(18*2), 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Sultan Haidar Ali.11052008", indentw+tftw-(pfpw*2)-3, (indenth+titleh+5+tfth-(indenth+titleh+5))-18, 2);

  }

  if (ID == 3) {
    tft.fillScreen(TFT_BLACK);
    tft.setFreeFont(&Orbitron_Medium_38);  // & followed by font name specified at the end  Tinos_Regular_21
    tft.drawString("ACESS DENIED", 55, 140);
    for (;;)
      ;
  }

  finger.LEDcontrol(0);

  delay(1500);
  tft.fillScreen(TFT_BLACK);
}


void loop() {

  drawcursor();

  tft.drawRect(indentw, indenth+labelh+menuh, tftw, iconh, TFT_WHITE);  //Icon Tray
  for(int z = 0; z<3; z++){
    tft.drawRect(indentw*(z+1), indenth+labelh+menuh, iconw, iconh, TFT_WHITE);   // Icon
    if (cursor.x < (indentw+1+iconw)*(z+1) && cursor.x > (indentw+1)*(z+1) && cursor.y > indenth+labelh+menuh && cursor.y < indenth+labelh+menuh+iconh) { 
      tft.fillRect(indentw*(z+1), indenth+labelh+menuh, iconw, iconh, TFT_WHITE); // selected Icon
      char key = keypad->getKey();
        if (key == '2') {
          tft.drawRect(indentw+1, indenth, tftw-screenw, labelh, TFT_WHITE);   //menu label box
          tft.drawRect(indentw+1, indenth+labelh , tftw-screenw-3, menuh, TFT_WHITE);  //menu boundary

          tft.setTextColor(TFT_WHITE, TFT_BLACK);
          tft.drawString(application[z].label_name, indentw+3, indenth+2, 2); //menu label

          while (cursor.x < 180) {
            updatecursor(4, application[z].array.size()*18 ); //give function upper & lower bound of menu 
            int potval = analogRead(potpin);
            //smothing linval
            int lastUsedValue;
            int linvalraw = map(potval, 700, 4095, 2 , -(application[z].array.size()-12)*18 );
            float alpha = 0.22; //response speed
            int linval = alpha * linvalraw + (1 - alpha) * linval;
            if (abs(linval - lastUsedValue) > 6) { // Hysteresis check
              lastUsedValue = linval;
            }

            char key = keypad->getKey();
            if (key == '1') {
              tft.fillScreen(BLACK);
              cursor.x = 240;
              cursor.y = 160;
              break;
            }
            
            Serial.println(potval);
            for (size_t j = 0; j < application[z].array.size(); j++) { // loop through each of the items in the array
              String name = " " + application[z].array[j]; // process the name string 
              while(name.length() < 26){
                name += " "; 
              } 
              if (cursor.y > (3 + (j * 18)) && cursor.y < (21 + (j * 18))) { // check if cursor is within the bounds of the given item

                menu.setTextColor(TFT_BLACK, TFT_WHITE);
                menu.drawString(name, 0, linval + (j * 18), 2); // display the name
                if (key == '2'){ //if key is pressed
                  displayImg(application[z].array[j]);
                }
              }

              else {
                menu.setTextColor(TFT_WHITE, TFT_BLACK);
                menu.drawString(name, 0, linval + (j * 18), 2);
              }
            }
            menu.pushSprite(indentw+2,indenth+labelh);
          }}
      
    }
  }
/*
  if (cursor.x < 91 && cursor.x > 61 && cursor.y > 270 && cursor.y < 300) {
    char key = keypad->getKey();

    if (mcp.digitalRead(10) == HIGH) {
      Serial.println("on");
      if (isUpperCaseMode == true) {
        char key = 'I';
      }
      if (isUpperCaseMode == false) {
        char key = 'i';
      }
    }


    if (key) {
      if (key == '>') {  // Toggle case with '>'
        isUpperCaseMode = !isUpperCaseMode;
        keypad = isUpperCaseMode ? (Keypad *)&keypadUpper : (Keypad *)&keypadLower;
        updateDisplay();
      } else {
        // Handle special keys
        if (key == '<') {  // Backspace
          if (inputText.length() > 0) {
            inputText.remove(inputText.length() - 1);  // Remove the last character
            updateDisplay();
          }
        } else if (key == ' ') {  // Space
          inputText += " ";       // Add a space character
          updateDisplay();
        } else if (key == '\n') {  // Newline
          if (currentLine < maxLines - 1) {
            lines[currentLine] = inputText;  // Save the current line
            currentLine++;                   // Move to the next line
            inputText = "";                  // Clear the input text for the new line
            updateDisplay();
          }
        } else {
          // Append the typed character to inputText
          inputText += key;
          updateDisplay();
        }
      }
    }
  }*/
}

void updatecursor(int ubound, int lbound) {
  xval = analogRead(xpin);
  yval = analogRead(ypin);
  int xbase=2500;
  int ybase=2700;

  if (yval <= ybase-100) {
    deltacursy = map(yval, ybase-100, 0, 0, 15);
    cursor.y = cursor.y - deltacursy;
  }

  if (yval >= ybase+100) {
    deltacursy = map(yval, ybase+100, 4095, 0, 15);
    cursor.y = cursor.y + deltacursy;
  }

  if (xval <= xbase-100) {
    deltacursx = map(xval, xbase-100, 0, 0, 15);
    cursor.x = cursor.x + deltacursx;
  }

  if (xval >= xbase+100) {
    deltacursx = map(xval, xbase+100, 4095, 0, 15);
    cursor.x = cursor.x - deltacursx;
  }

  if (cursor.y < ubound) {
    cursor.y = ubound;
  }

  if (cursor.y > lbound) {
    cursor.y = lbound;
  }

  if (cursor.x > 450) {
    cursor.x = 450;
  }

  if (cursor.x < 25) {
    cursor.x = 15;
  }
}

void drawcursor() {
  xval = analogRead(xpin);
  yval = analogRead(ypin);
  int xbase=2500;
  int ybase=2700;
  Serial.print(xval);
  Serial.print("  ");
  Serial.println(yval);

  curs.drawCircle(10, 10, 10, TFT_BLACK);  //centre,x,y,rad,color
  curs.pushSprite(cursor.x, cursor.y);

  

  if (yval <= ybase-100) {
    deltacursy = map(yval, ybase-100, 0, 0, 15);
    cursor.y = cursor.y - deltacursy;
  }

  if (yval >= ybase+100) {
    deltacursy = map(yval, ybase+100, 4095, 0, 15);
    cursor.y = cursor.y + deltacursy;
  }

  if (xval <= xbase-100) {
    deltacursx = map(xval, xbase-100, 0, 0, 15);
    cursor.x = cursor.x + deltacursx;
  }

  if (xval >= xbase+100) {
    deltacursx = map(xval, xbase+100, 4095, 0, 15);
    cursor.x = cursor.x - deltacursx;
  }

  if (cursor.y < 30) {
    cursor.y = 30;
  }

  if (cursor.y > 280) {
    cursor.y = 280;
  }

  if (cursor.x > 450) {
    cursor.x = 450;
  }

  if (cursor.x < 25) {
    cursor.x = 15;
  }


  curs.drawCircle(10, 10, 10, TFT_WHITE);  //centre,x,y,rad,color
  curs.pushSprite(cursor.x, cursor.y);

  delay(50);
}

void updateDisplay() {
  oled1.clearDisplay();  // Clear the screen

  for (int i = 0; i < currentLine; i++) {
    displayLine(lines[i], i);
  }

  displayLine(inputText + "_", currentLine);  // Display the current line with cursor
}

void displayLine(String text, int line) {
  // Calculate line position
  int y = 10 + line * 12;  // Adjust the multiplier (20) based on text size
  oled1.clearDisplay();
  oled1.setTextSize(1);
  oled1.setTextColor(WHITE);
  oled1.setCursor(0, y);
  // Display static text
  oled1.println(text);
  oled1.display();
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString(" Image taken", indentw+1, (indenth+titleh+5+(18*3)), 2);
      delay(150);
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      tft.drawString(" No finger detected", indentw+1, (indenth+titleh+5+(18*3)), 2);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      tft.drawString(" Communication error", indentw+1, (indenth+titleh+5+(18*3)), 2);
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      tft.drawString(" Imaging error", indentw+1, (indenth+titleh+5+(18*3)), 2);
      return p;
    default:
      Serial.println("Unknown error");
      tft.drawString(" Unknown error", indentw+1,(indenth+titleh+5+(18*3)), 2);
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      tft.drawString(" Image converted", indentw+1, (indenth+titleh+5+(18*4)), 2);
      delay(150);
      tft.drawString(" Acessing Dactylograph databases", indentw+1, (indenth+titleh+5+(18*5)), 2);
      delay(300);
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      tft.drawString(" Image unclear", indentw+1, (indenth+titleh+5+(18*4)), 2);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      tft.drawString(" Communication error", indentw+1, (indenth+titleh+5+(18*4)), 2);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      tft.drawString(" Unable to identify features ", indentw+1, (indenth+titleh+5+(18*4)), 2);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      tft.drawString(" Unable to identify features ", 32,(indenth+titleh+5+(18*4)), 2);
      return p;
    default:
      Serial.println("Unknown error");
      tft.drawString(" Unknown error", 32,(indenth+titleh+5+(18*4)), 2);
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    tft.drawString(" Match found", 32, (indenth+titleh+5+(18*6)), 2);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #");

  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);

  return finger.fingerID;
}

void listDirMem(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);
  
  oled2.setTextSize(2);
  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      dirno = dirno + 1;
      oled2.setCursor(32, 47); 
      oled2.println(String (dirno));
      oled2.display();
      delay (50);

      if (levels) {
        listDirMem(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      //If the directory is "Documents", add each file name into the array
      String strDir = String(dirname);
      for (int i =0; i<3; i++){ 
        if (strDir == application[i].search_name){
          String strFil =  String(file.name());
          application[i].array.push_back(strFil);
        }
      }
      Serial.print("  SIZE: ");
      Serial.println(file.size());
      filno = filno + 1;
      Serial.println(filno);
      oled2.setCursor(98, 47); 
      oled2.println(String (filno));
      oled2.display();
      delay (10);
    }
    file = root.openNextFile();
  }
}

void displayDoc(String name){

  int a = 0;
  int b = 0;
  File file = SD.open("/Documents/"+ name, FILE_READ);
  if (!file) {
    Serial.println("File open failed");
    return;
  }
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim(); // Remove \r or trailing whitespace
    screen.setCursor(a, b);
    screen.setTextColor(TFT_WHITE, TFT_BLACK);
    screen.setTextSize(1);
    screen.drawString(line, a, b);
    b += 18;
  } 
  screen.pushSprite(tftw-screenw+indentw, indenth+labelh);
}

void displayImg(String name){
  Serial.print(name);
  // Open BMP file
  File bmpFile = SD.open("/Images/" + name, FILE_READ);
  if (!bmpFile) {
    Serial.println("Failed to open BMP file!");
    return;
  }

  // Verify BMP header
  if (read16(bmpFile) != 0x4D42) {
    Serial.println("Not a valid BMP file");
    bmpFile.close();
    return;
  }

  // Skip unneeded BMP header fields
  bmpFile.seek(10);
  uint32_t pixelDataOffset = read32(bmpFile);
  bmpFile.seek(18);
  int32_t bmpWidth  = read32(bmpFile);
  int32_t bmpHeight = read32(bmpFile);
  bmpFile.seek(28);
  uint16_t bpp = read16(bmpFile);

  if (bmpWidth > screenw || abs(bmpHeight) > screenh || bpp != 1) {
    Serial.println("BMP not supported: wrong size or color depth");
    bmpFile.close();
    return;
  }

  screen.fillSprite(TFT_BLACK);

  // Move to pixel array
  bmpFile.seek(pixelDataOffset);

  int rowSize = ((bmpWidth + 31) / 32) * 4;  // row size aligned to 4 bytes

  uint8_t rowBuf[rowSize];
  bool flip = bmpHeight > 0;

  for (int y = 0; y < abs(bmpHeight); y++) {
    int row = flip ? abs(bmpHeight) - 1 - y : y;
    bmpFile.seek(pixelDataOffset + row * rowSize);
    bmpFile.read(rowBuf, rowSize);

    for (int x = 0; x < bmpWidth; x++) {
      uint8_t byte = rowBuf[x / 8];
      bool pixelOn = byte & (0x80 >> (x % 8));
      screen.drawPixel(x, y, pixelOn ? TFT_WHITE : TFT_BLACK);
    }
  }

  bmpFile.close();

  // Push sprite to screen
  screen.pushSprite(tftw-screenw+indentw, indenth+labelh);
}

// --- Helper functions for BMP reading ---
uint16_t read16(File &f) {
  uint16_t result;
  result = f.read();       // LSB
  result |= f.read() << 8; // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  result = f.read();
  result |= f.read() << 8;
  result |= f.read() << 16;
  result |= f.read() << 24;
  return result;
}
//Redundant Functions

// returns -1 if failed, otherwise returns ID #
/*
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)
    return -1;

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  return finger.fingerID;
} 



void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}*/
