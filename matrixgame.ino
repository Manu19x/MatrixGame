#include <LiquidCrystal.h>
#include "LedControl.h"
#include <EEPROM.h>

const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;

const int redPin = A2;
const int greenPin = 13;
bool greenLEDState = LOW;
bool redLEDState = LOW;
const int buzzerPin = 3;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

byte matrixBrightness = 2;

byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;


const byte moveInterval = 100;     // Timing variable to control the speed of LED movement
unsigned long long lastMoved = 0;  // Tracks the last time the LED moved
unsigned long startTime = 0;
bool gameEnded = false;
const byte matrixSize = 8;  // Size of the LED matrix
bool matrixChanged = true;  // Flag to track if the matrix display needs updating
// 2D array representing the state of each LED (on/off) in the matrix
bool matrix[matrixSize][matrixSize] = {
  { 1, 1, 1, 1, 1, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 1, 1, 1, 1, 1, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 1, 1, 1, 1, 1, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 1, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0 }
};

bool initialMatrix[matrixSize][matrixSize];

#define LCD_WIDTH 16

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int joyX = A0;
const int joyY = A1;
const int joySW = 2;

int xValue = 0;
int yValue = 0;
int minThreshold = 200;
int maxThreshold = 600;
bool joyMovedX = false;
bool joyMovedY = false;
bool swState = false;
bool lastSwState = false;

unsigned long lastBlink = 0;
unsigned long blinkInterval = 500;
bool currentLedState = HIGH;

byte leftArr[8] = {
  B00100,
  B01100,
  B11100,
  B11111,
  B11100,
  B01100,
  B00100,
  B00000
};
byte rightArr[8] = {
  B00100,
  B00110,
  B00111,
  B11111,
  B00111,
  B00110,
  B00100,
  B00000
};

byte bell[8] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B00000,
  B00100,
  B00000

};
byte crown[8] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};
byte questionmark[8] = {
  B00110,
  B01001,
  B01001,
  B00001,
  B00010,
  B00100,
  B00000,
  B00100
};
int score = 0;
int highscore = 0;
long playMillis = 0;

int playerRow = 0;  // Rândul inițial al jucătorului
int playerCol = 0;

bool soundPlaying = false;
unsigned long soundStart = 0;
unsigned long soundDuration = 0;
unsigned int soundFrequency = 0;

enum MenuStates {
  START_GAME,
  HIGH_SCORE,
  SETTINGS,
  ABOUT,
  PLAYING,
  DETAILS,
  GAME_OVER,
  WIN,
  ADJUSTING
} menuState;

bool displayAboutText = false;
bool soundEnabled = true;
unsigned long previousMillis = 0;
long greenInterval = 0; // Intervalul de timp pentru LED-ul verde (în milisecunde)
long redInterval = 0;   // Intervalul de timp pentru LED-ul roșu (în milisecunde)
bool isGreen = true; // Variabilă pentru a ține evidența stării LED-ului verde

void setup() {
  for (int i = 0; i < matrixSize; ++i) {
    for (int j = 0; j < matrixSize; ++j) {
      initialMatrix[i][j] = matrix[i][j];
    }
  }
  lcd.begin(16, 2);
  lcd.createChar(0, leftArr);
  lcd.createChar(1, rightArr);
  lcd.createChar(2, bell);
  lcd.createChar(3, crown);
  lcd.createChar(4, questionmark);

  pinMode(joySW, INPUT_PULLUP);

  showIntroMessage("PRESS JSW.");  // Afiseaza mesajul introductiv cu un text la alegere

  // Setează starea inițială a meniului după ce a fost afișat mesajul introductiv
  menuState = START_GAME;

  lc.shutdown(0, false);  // Pornire matrice de LED-uri
  lc.setIntensity(0, 8);  // Setare intensitate luminoasă
  lc.clearDisplay(0);

  pinMode(redPin, OUTPUT);  // Setează pinul pentru LED-ul roșu ca ieșire
  pinMode(greenPin, OUTPUT);

  xPos = 7;
  yPos = 7;

  pinMode(buzzerPin, OUTPUT);

  digitalWrite(redPin, LOW);  // Aprinde LED-ul roșu
  digitalWrite(greenPin, LOW); // Stinge LED-ul verde
   previousMillis = millis();
   Serial.begin(9600);
  
}

void loop() {

  //alternate();
  switch (menuState) {
    case START_GAME:
      lcd.setCursor(1, 0);
      lcd.write(byte(1));
      centerTextOnLcd("Start game", 0);
      lcd.setCursor(14, 0);
      lcd.write(byte(0));
      centerTextOnLcd("JS button=start", 1);
      break;

    case HIGH_SCORE:
      lcd.setCursor(1, 0);
      lcd.write(byte(3));
      centerTextOnLcd("HighScore", 0);
      lcd.setCursor(14, 0);
      lcd.write(byte(3));
      centerTextOnLcd("JS button=enter", 1);
      //centerTextOnLcd(String("Your HighScore:") + String(highscore), 1);
      break;

    case SETTINGS:
      lcd.setCursor(1, 0);
      lcd.write(byte(2));
      centerTextOnLcd("Settings", 0);
      lcd.setCursor(14, 0);
      lcd.write(byte(2));
      centerTextOnLcd("JS button=enter", 1);
      break;

    case ABOUT:
      lcd.setCursor(1, 0);
      lcd.write(byte(4));
      centerTextOnLcd("About", 0);
      lcd.setCursor(14, 0);
      lcd.write(byte(4));
      centerTextOnLcd("JS button=enter", 1);
      break;


    case DETAILS:
      lcd.setCursor(1, 0);
      centerTextOnLcd("Joc creat de", 0);
      centerTextOnLcd("Marian Matea", 1);
      break;

    case PLAYING:
      // unsigned long seconds = (millis() - startTime) / 1000;  // Calculează timpul în secunde
      centerTextOnLcd("Have Fun", 0);  
      centerTextOnLcd("Watch The LEDS!!!", 1);
      //centerTextOnLcd(String(seconds), 1); // Afișează timpul în secunde pe a doua linie
      movegame();
      //playShortStartSound();
       //playSound(1000, 500); // 
       //updateSound();
      
      break;
    
    case GAME_OVER:
      lcd.setCursor(1, 0);
      centerTextOnLcd("GAME OVER", 0);
      centerTextOnLcd("INCEARCA DIN NOU", 1);
      //playGameOverSound();
      break;
    case WIN:
      lcd.setCursor(1, 0);
      centerTextOnLcd("Ai castigat :D", 0);
      centerTextOnLcd("INCEARCA DIN NOU", 1);
      //playWinSound();
      //updateSound();
      break;
    case ADJUSTING:
      lcd.setCursor(1,0);
      centerTextOnLcd("Control Sunete", 0);
      displaySoundSetting(); // Afișează starea sunetului pe ecran
       joystickEventCheck();


  }
  joystickEventCheck();
  
  
}
void showIntroMessage(const char* message) {
  lcd.clear();
  centerTextOnLcd("WELCOME!", 0);  // Afiseaza mesajul introductiv centrat pe prima linie

  // Afiseaza mesajul primit ca parametru centrat pe a doua linie
  centerTextOnLcd(message, 1);

  while (digitalRead(joySW) == HIGH)
    ;
  delay(500);

  lcd.clear();
}

void centerTextOnLcd(String text, short line) {  ///afiseaza textul - *text* centrat - la linia -*line*
  short l = text.length();
  short spaces = (LCD_WIDTH - l) / 2;  /// LCD width - latimea totala a ecranului lcd --- formula pentru nr spatii necesare pt a centra textul
  lcd.setCursor(spaces, line);
  lcd.print(text);
}

void changeMenuState(bool fw) {  /// primim un bool pentru a decide in ce directie mergem -> (true - inainte) -> (false - inapoi)
  if (fw) {
    if (menuState == 3) {
      menuState = 0;
    } else {
      menuState = menuState + 1;
    }
  } else {
    if (menuState == 0) {
      menuState = 3;
    } else {
      menuState = menuState - 1;
    }
  }
  lcd.clear();
}
void joystickEventCheck() {
  yValue = analogRead(joyY);
  if (yValue > maxThreshold && joyMovedY == false) {
    switch (menuState) {
      case START_GAME:
      case HIGH_SCORE:
      case SETTINGS:
      case ABOUT:
        changeMenuState(true);  // inapoi în meniu
      case ADJUSTING:
        toggleSoundSetting(); // Inversează starea sunetului la apăsarea butonului în meniu
        lcd.clear();
        break;
    }
    joyMovedY = true;
  }

  if (yValue < minThreshold && joyMovedY == false) {
    switch (menuState) {
      case START_GAME:
      case HIGH_SCORE:
      case SETTINGS:
      case ABOUT:
        changeMenuState(false);  // inainte în meniu
      case ADJUSTING:
        toggleSoundSetting(); // Inversează starea sunetului la apăsarea butonului în meniu
        lcd.clear();
        break;
    }
    joyMovedY = true;
  }

  if (yValue >= minThreshold && yValue <= maxThreshold) {
    joyMovedY = false;
  }

  swState = digitalRead(joySW);
  if (swState != lastSwState) {
    if (swState == LOW) {
      switch (menuState) {
        case ABOUT:
          menuState = DETAILS;
          lcd.clear();
          break;
        case DETAILS:
          menuState = ABOUT;
          lcd.clear();
          break;
        case START_GAME:
          menuState = PLAYING;
          if(soundEnabled){
              int frequency = 2000; 
              unsigned long duration = 500; 
              tone(buzzerPin, frequency, duration);
          }
          lcd.clear();
          break;
       // case PLAYING: /// ar trebui scos
        //  menuState = START_GAME;
        //  lcd.clear();
        //  break;
        case GAME_OVER:
        case WIN:
          resetGame();
          menuState = START_GAME;
          lcd.clear();
          break;
       /* case WIN:
          resetGame();
          menuState = START_GAME;
          lcd.clear();
          break;
        */
        case SETTINGS:
          menuState = ADJUSTING;
          lcd.clear();
          break;
        
        case ADJUSTING:
          menuState = START_GAME;
          lcd.clear(); 
          break;

      }
    }
    lastSwState = swState;
  }
}
void movegame() {
  //alternate();
  if (millis() - lastMoved > moveInterval) {
    updatePositions();     // Update the LED position based on joystick input
    lastMoved = millis();  // Reset the movement timer
  }
  // Update the LED matrix display if there's been a change
  if (matrixChanged) {
    updateMatrix();
    matrixChanged = false;
  }
  if (millis() - lastBlink > blinkInterval) {
    // Inversează starea LED-ului
    currentLedState = !currentLedState;

    // Actualizează starea LED-ului la poziția curentă în matrice
    lc.setLed(0, xPos, yPos, currentLedState);

    lastBlink = millis();  // Actualizează timpul ultimului blink
  }
  alternate();
}

void updateMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, matrix[row][col]);  // Update each LED state
    }
  }
}
/*void updatePositions() {
  int xValue = analogRead(joyX);  // Read the X-axis value
  int yValue = analogRead(joyY);  // Read the Y-axis value
  // Store the last positions
  xLastPos = xPos;
  yLastPos = yPos;
  // Update xPos based on joystick movement
  if (xValue < minThreshold) {
    xPos = (xPos + 1) % matrixSize;
  } else if (xValue > maxThreshold) {
    xPos = (xPos > 0) ? xPos - 1 : matrixSize - 1;
  }
  // Update yPos based on joystick movement
  if (yValue < minThreshold) {
    yPos = (yPos > 0) ? yPos - 1 : matrixSize - 1;
  } else if (yValue > maxThreshold) {
    yPos = (yPos + 1) % matrixSize;
  }
  // Check if the position has changed and update the matrix accordingly
  if (xPos != xLastPos || yPos != yLastPos) {
    matrixChanged = true;
    matrix[xLastPos][yLastPos] = 0;  // Turn off the LED at the last position
    matrix[xPos][yPos] = 1;          // Turn on the LED at the new position
  }
}
*/
void changeState(int led) {
  digitalWrite(greenPin, LOW); // Stinge LED-ul verde
  digitalWrite(redPin, LOW);   // Stinge LED-ul roșu

  digitalWrite(led, HIGH); // Aprinde LED-ul specificat ca argument
  
  // Aici poți adăuga logica pentru mișcare sau alte acțiuni legate de starea LED-urilor
}

void alternate()
{
 unsigned long currentMillis = millis(); // Obține timpul curent

  if (isGreen && (currentMillis - previousMillis >= greenInterval)) {
    previousMillis = currentMillis;
    changeState(redPin); // Trecem la starea roșie
    isGreen = false;
    greenInterval = random(1000, 2700); // Generăm un nou interval aleatoriu pentru LED-ul verde
  } else if (!isGreen && (currentMillis - previousMillis >= redInterval)) {
    previousMillis = currentMillis;
    changeState(greenPin); // Trecem la starea verde
    isGreen = true;
    redInterval = random(1000, 3300); // Generăm un nou interval aleatoriu pentru LED-ul roșu
  }
}

void updatePositions() {
  int xValue = analogRead(joyX);  // Citirea valorii axei X
  int yValue = analogRead(joyY);  // Citirea valorii axei Y

  // Stocarea ultimelor poziții
  xLastPos = xPos;
  yLastPos = yPos;

  // Verificare dacă jucătorul se poate mișca
  if (isGreen) { // Jucătorul se poate mișca doar când LED-ul verde este activ
    // Actualizarea poziției pe baza mișcării joystick-ului
    if (xValue < minThreshold) {
      if (xPos < matrixSize - 1 && !matrix[xPos + 1][yPos]) {
        xPos++;
      }
    } else if (xValue > maxThreshold) {
      if (xPos > 0 && !matrix[xPos - 1][yPos]) {
        xPos--;
      }
    }

    if (yValue < minThreshold) {
      if (yPos > 0 && !matrix[xPos][yPos - 1]) {
        yPos--;
      }
    } else if (yValue > maxThreshold) {
      if (yPos < matrixSize - 1 && !matrix[xPos][yPos + 1]) {
        yPos++;
      }
    }
  } 
  if (!isGreen && (xValue < minThreshold || xValue > maxThreshold || yValue < minThreshold || yValue > maxThreshold)) {
    menuState = GAME_OVER; // Setăm starea de GAME_OVER în cazul unei mișcări când LED-ul roșu este activ
    if(soundEnabled){
      int frequency = 1000; 
      unsigned long duration = 500; 
      tone(buzzerPin, frequency, duration);

    }
    resetGame();
  }
  

  // Actualizare matrice în cazul în care poziția jucătorului s-a schimbat
  if (xPos != xLastPos || yPos != yLastPos) {
    matrixChanged = true;
    matrix[xLastPos][yLastPos] = 0;  // Stinge LED-ul la ultima poziție
    matrix[xPos][yPos] = 1;          // Aprinde LED-ul la noua poziție
  }
  
  if(xPos==1&&yPos==0){
    menuState=WIN;
    if(soundEnabled){
      int frequency = 3000; 
      unsigned long duration = 500; 
      tone(buzzerPin, frequency, duration);
    }
    
    resetGame();
  }
  
 /*
  if (menuState == GAME_OVER || menuState == WIN) {
    // Dacă starea jocului este GAME_OVER sau WIN, stinge LED-ul la poziția curentă
    matrix[xPos][yPos] = 0;
    matrix[xLastPos][yLastPos] =0;
  }
  */
}

void resetGame() {
  // Reinițializarea variabilelor și stării jocului la valorile inițiale
  matrix[xLastPos][yLastPos] = 0;
  matrix[xPos][yPos] = 0;
  xPos = 7;
  yPos = 7;
  //xLastPos = 0;
  //yLastPos = 0;
  //menuState = START_GAME;  // Sau starea inițială corespunzătoare
  // Reinițializarea matricei sau a altor variabile necesare pentru joc la valorile inițiale
  // Exemplu:
  for (int i = 0; i < matrixSize; ++i) {
    for (int j = 0; j < matrixSize; ++j) {
      matrix[i][j] = initialMatrix[i][j];  // Resetarea matricei la valorile inițiale
    }
  }
  digitalWrite(redPin, LOW);  
  digitalWrite(greenPin, LOW); 
}


void toggleSoundSetting() {
  soundEnabled = !soundEnabled; // Invertește starea sunetului
}

void displaySoundSetting() {
  lcd.setCursor(0, 1);
  if (soundEnabled) {
    centerTextOnLcd("ON", 1);
  } else {
    centerTextOnLcd("OFF", 1);
  }
}


/*
void alternate()
{
 unsigned long currentMillis = millis(); // Obține timpul curent

  if (isGreen && (currentMillis - previousMillis >= greenInterval)) {
    previousMillis = currentMillis;
    changeState(redPin); // Trecem la starea roșie
    isGreen = false;
  } else if (!isGreen && (currentMillis - previousMillis >= redInterval)) {
    previousMillis = currentMillis;
    changeState(greenPin); // Trecem la starea verde
    isGreen = true;
  }
}*/
  