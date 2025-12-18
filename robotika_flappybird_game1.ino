#include <LiquidCrystal.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

// LCD lábak: RS, E, D4, D5, D6, D7
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// HT16K33 7-szegmens kijelző
Adafruit_7segment matrix = Adafruit_7segment();

// Nyomógomb
const int BUTTON_PIN = 8;

// Madár (kis figura)
byte birdChar[8] = {
  B00110,
  B01101,
  B11111,
  B11110,
  B01110,
  B00100,
  B00000,
  B00000
};

// Akadály (teli blokk)
byte wallChar[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

int birdY = 1;          // Madár magassága (0: felső sor, 1: alsó sor)
int score = 0;          // Pontszám
bool isGameOver = false;
bool gameStarted = false;

// Pálya (16 oszlop). 0: üres, 1: akadály lent, 2: akadály fent
int terrain[16]; 
unsigned long lastMoveTime = 0;
int gameSpeed = 400;    // Játék sebessége (ms), minél kisebb, annál gyorsabb

void setup() {
  // Gomb beállítása belső felhúzóellenállással
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // LCD indítása
  lcd.begin(16, 2);
  lcd.createChar(0, birdChar); // 0-ás karakter a madár
  lcd.createChar(1, wallChar); // 1-es karakter a fal

  // 7-szegmens kijelző indítása
  matrix.begin(0x70);
  matrix.setBrightness(10); // Fényerő állítása (0-15)
  
  showStartScreen();
}

void loop() {
  // Gomb állapotának olvasása (LOW ha le van nyomva)
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  if (!gameStarted) {
    // Ha még nem megy a játék, gombnyomásra indul
    if (buttonPressed) {
      resetGame();
      gameStarted = true;
      delay(200); // Pergésmentesítés
    }
    return;
  }

  if (isGameOver) {
    // Ha vége a játéknak, gombnyomásra újraindul
    if (buttonPressed) {
      resetGame();
      delay(200);
    }
    return;
  }


  // Ha nyomjuk a gombot, felmegy (0), ha elengedjük, leesik (1)
  if (buttonPressed) {
    birdY = 0;
  } else {
    birdY = 1;
  }

  // Pálya léptetése időzítve
  if (millis() - lastMoveTime > gameSpeed) {
    moveTerrain();
    lastMoveTime = millis();
    
    // Minden lépésnél nőhet a pontszám, ha átjutottunk egy akadályon
    if(terrain[1] != 0) {
       score++;
       updateScoreBoard();
       // Nehezítés: minden 5. pontnál gyorsul kicsit
       if (score % 5 == 0 && gameSpeed > 150) {
         gameSpeed -= 20;
       }
    }
  }

  if (terrain[1] == 1 && birdY == 1) { // Fal lent, madár lent
    gameOver();
  } else if (terrain[1] == 2 && birdY == 0) { // Fal fent, madár fent
    gameOver();
  }

  drawGame();
}


void resetGame() {
  score = 0;
  gameSpeed = 400;
  birdY = 1;
  isGameOver = false;
  
  // Pálya törlése
  for (int i = 0; i < 16; i++) {
    terrain[i] = 0;
  }
  
  updateScoreBoard();
  lcd.clear();
}

void moveTerrain() {
  // Minden elemet balra léptetünk
  for (int i = 0; i < 15; i++) {
    terrain[i] = terrain[i+1];
  }

  // Új elem generálása a jobb szélen
  // Véletlenszerűen generálunk akadályt, de hagyunk néha szünetet is
  int newBlock = random(0, 4); 
  // 0: Üres, 1: Alsó fal, 2: Felső fal, 3: Üres
  
  if (newBlock == 3) newBlock = 0;
  
  // Ha az előző fal volt, most legyen üres
  if (terrain[14] != 0) newBlock = 0;

  terrain[15] = newBlock;
}

void drawGame() {  
  // Felső sor rajzolása
  lcd.setCursor(0, 0);
  for (int i = 0; i < 16; i++) {
    if (i == 1 && birdY == 0) {
      lcd.write(byte(0)); // Madár fent
    } else if (terrain[i] == 2) {
      lcd.write(byte(1)); // Fal fent
    } else {
      lcd.print(" ");     // Üres
    }
  }

  // Alsó sor rajzolása
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    if (i == 1 && birdY == 1) {
      lcd.write(byte(0)); // Madár lent
    } else if (terrain[i] == 1) {
      lcd.write(byte(1)); // Fal lent
    } else {
      lcd.print(" ");     // Üres
    }
  }
}

void updateScoreBoard() {
  matrix.print(score);
  matrix.writeDisplay();
}

void showStartScreen() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Flappy Bird");
  lcd.setCursor(0, 1);
  lcd.print("Nyomd a gombot");
  
  matrix.print(0);
  matrix.writeDisplay();
}

void gameOver() {
  isGameOver = true;
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("GAME OVER");
  lcd.setCursor(2, 1);
  lcd.print("Pontszam: ");
  lcd.print(score);
  
  // Villogtatjuk a pontszámot a 7-szegmensen
  for(int i=0; i<3; i++) {
    matrix.blinkRate(1); // Villogás be
    matrix.writeDisplay();
    delay(500);
    matrix.blinkRate(0); // Villogás ki
    matrix.writeDisplay();
  }
}