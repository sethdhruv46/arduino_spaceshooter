#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(7,8,9,10,11,12);

// ---------------- PINS ----------------
const int VRY = A1;
const int SW  = 2;
const int BUZZER = 3;

// ---------------- CONSTANTS ----------------
const byte PLAYER_COL = 0;
const byte MAX_COL = 15;

// ---------------- PLAYER ----------------
byte playerRow = 1;

// ---------------- ENEMY ----------------
byte enemyRow = 0;
int enemyCol = 15;

byte enemyType = 0;      // 0=Scout 1=Fast 2=Tank
byte enemyHealth = 1;

const int bossSpeed = 700;
unsigned long bossTimer = 0;

// ---------------- BULLET ----------------
bool bulletActive = false;
byte bulletRow = 0;
int bulletCol = 1;

// ---------------- GAME ----------------
int score = 0;
int highScore = 0;
byte lives = 3;

bool gameOver = false;
bool explosion = false;

bool invincible = false;
unsigned long invincibleTimer = 0;
const int INVINCIBLE_TIME = 1000;

// ---------------- BOSS ----------------
bool bossActive = false;
byte bossHealth = 7;
int bossCol = 15;
const byte BOSS_MAX_HEALTH = 7;
int nextBossScore = 25;

// ---------------- SPEED ----------------
int enemySpeed = 500;
const int bulletSpeed = 70;

// ---------------- TIMERS ----------------
unsigned long enemyTimer = 0;
unsigned long bulletTimer = 0;
unsigned long explosionTimer = 0;

// ---------------- SCREEN BUFFER ----------------
char screenBuffer[2][16];
char previousBuffer[2][16];

// ---------------- CUSTOM CHARACTERS ----------------

// Ship
byte ship[8]={
B00100,
B01110,
B11111,
B10101,
B11111,
B01110,
B00100,
B00000
};

// Alien
byte alien[8]={
B01110,
B10101,
B11111,
B11111,
B01110,
B10101,
B00000,
B00000
};

// Heart
byte heart[8]={
B00000,
B01010,
B11111,
B11111,
B11111,
B01110,
B00100,
B00000
};

// Explosion
byte boom[8]={
B00100,
B10101,
B01110,
B11111,
B01110,
B10101,
B00100,
B00000
};

byte fastAlien[8] = {
B00100,
B01110,
B11111,
B10101,
B11111,
B00100,
B01010,
B00000
};

byte tankAlien[8] = {
B11111,
B11111,
B10101,
B11111,
B11111,
B10101,
B11111,
B00000
};

byte damagedTank[8] = {
B11111,
B11011,
B10001,
B11111,
B10101,
B11011,
B11111,
B00000
};

void splashScreen(){

lcd.clear();

lcd.setCursor(1,0);
lcd.print("SPACE SHOOTER");

lcd.setCursor(4,1);
lcd.print("by Dhruv");

delay(2000);

lcd.clear();
}

void resetGame(){

score=0;
lives=3;
enemySpeed=500;

playerRow=1;

bulletActive=false;

gameOver=false;

explosion=false;

enemyCol=15;
enemyRow=random(0,2);

}

void spawnEnemy() {

  if (!bossActive && score >= nextBossScore) {
    bossActive = true;
    bossHealth = 7;
    bossCol = 15;

    nextBossScore += 25;
}

  enemyCol = 15;

  // 60% chance to spawn in the player's lane
  if (random(100) < 60)
    enemyRow = playerRow;
  else
    enemyRow = random(0, 2);

  int r = random(100);

  if (r < 60) {

    enemyType = 0;
    enemyHealth = 1;

  }
  else if (r < 90) {

    enemyType = 1;
    enemyHealth = 1;

  }
  else {

    enemyType = 2;
    enemyHealth = 2;

  }

}
void readJoystick(){

int y=analogRead(VRY);

if(y<350)
playerRow=0;

if(y>700)
playerRow=1;

}

void fireBullet(){

static bool previous=HIGH;

bool current=digitalRead(SW);

if(previous==HIGH && current==LOW && !bulletActive){

bulletActive=true;
bulletRow=playerRow;
bulletCol=1;

fireSound();

}

previous=current;

}

void setup(){

lcd.begin(16,2);

clearBuffer();
for(int r=0;r<2;r++)
  for(int c=0;c<16;c++)
    previousBuffer[r][c]='?';

lcd.createChar(0,ship);
lcd.createChar(1,alien);
lcd.createChar(2,heart);
lcd.createChar(3,boom);
lcd.createChar(4, fastAlien);
lcd.createChar(5, tankAlien);
lcd.createChar(6, damagedTank);

pinMode(SW,INPUT_PULLUP);
pinMode(BUZZER,OUTPUT);

randomSeed(analogRead(A5));

EEPROM.get(0,highScore);

if(highScore<0 || highScore>999)
highScore=0;

splashScreen();
startSound();
resetGame();

}

void fireSound() {
  tone(BUZZER, 1800, 30);
  delay(15);
  tone(BUZZER,1800,20);
}

void hitSound() {

  tone(BUZZER, 1200, 20);
  delay(20);

  tone(BUZZER, 1700, 20);
  delay(20);

  tone(BUZZER, 2300, 30);
}

void damageSound() {

  tone(BUZZER, 900, 40);
  delay(45);

  tone(BUZZER, 500, 120);
}

void gameOverSound() {

  for (int f = 1000; f >= 200; f -= 40) {
    tone(BUZZER, f, 15);
    delay(15);
  }
}

void startSound() {

  int notes[] = {523, 659, 784, 988};

  for (byte i = 0; i < 4; i++) {
    tone(BUZZER, notes[i], 50);
    delay(60);
  }
}

void moveBullet() {

  if (!bulletActive) return;

  if(bossActive && bulletCol==bossCol){
    bulletActive=false;
    bossHealth--;
    hitSound();
    if(bossHealth==0){
      bossActive=false;
      explosion=true;
      explosionTimer=millis();
      enemyRow=0; enemyCol=bossCol;
      score+=5;
      spawnEnemy();
    }
    return;
  }

  if (millis() - bulletTimer >= bulletSpeed) {

    bulletTimer = millis();

    bulletCol++;

    if (bulletCol > MAX_COL) {
      bulletActive = false;
    }
  }
}

void moveEnemy(){

  if(bossActive){
    if(millis()-enemyTimer<700) return;
    enemyTimer=millis();
    bossCol--;
    tone(BUZZER,500,40);
    if(bossCol<0){
      bossActive=false;
      if(!invincible){
        lives--;
        damageSound();
        invincible=true;
        invincibleTimer=millis();
        if(lives==0){ gameOver=true; return; }
      }
      spawnEnemy();
    }
    return;
  }

  int speed = enemySpeed;

  if(enemyType == 1)
      speed = enemySpeed * 0.7;

  if(millis() - enemyTimer < speed)
      return;

  enemyTimer = millis();

  enemyCol--;

    if(enemyCol == 3) tone(BUZZER,900,30);
    else if(enemyCol == 2) tone(BUZZER,1100,30);
    else if(enemyCol == 1) tone(BUZZER,1400,40);

  if(enemyCol < 0){

      if(!invincible && enemyRow == playerRow){

          lives--;
          damageSound();
          invincible = true;
          invincibleTimer = millis();

          if(lives == 0){

              gameOver = true;
              return;
          }

      }

      spawnEnemy();
  }

}

void checkCollision() {

  if (!bulletActive) return;

  if(bossActive && bulletCol==bossCol){
    bulletActive=false;
    bossHealth--;
    hitSound();
    if(bossHealth==0){
      bossActive=false;
      explosion=true;
      explosionTimer=millis();
      enemyRow=0; enemyCol=bossCol;
      score+=5;
      spawnEnemy();
    }
    return;
  }

  if(bulletActive &&
   bulletRow == enemyRow &&
   bulletCol == enemyCol){

    bulletActive = false;

    enemyHealth--;

    if(enemyHealth <= 0){

        explosion = true;
        explosionTimer = millis();

        switch(enemyType){

            case 0:
                score += 1;
                break;

            case 1:
                score += 2;
                break;

            case 2:
                score += 3;
                break;
        }

        hitSound();

        if(score % 5 == 0 && enemySpeed > 150)
            enemySpeed -= 25;
    }
}
}

void updateExplosion() {

  if (!explosion)
    return;

  if (millis() - explosionTimer > 120) {

    explosion = false;

    spawnEnemy();
  }
}


void clearBuffer() {
  for (int r = 0; r < 2; r++)
    for (int c = 0; c < 16; c++)
      screenBuffer[r][c] = ' ';
}

void renderBuffer() {
  for (int r = 0; r < 2; r++) {
    for (int c = 0; c < 16; c++) {
      if (screenBuffer[r][c] != previousBuffer[r][c]) {
        lcd.setCursor(c, r);
        char ch = screenBuffer[r][c];
        if ((unsigned char)ch <= 7)
          lcd.write((byte)ch);
        else
          lcd.print(ch);
        previousBuffer[r][c] = screenBuffer[r][c];
      }
    }
  }
}

void drawHUD() {

  lcd.setCursor(13,0);

  if(score<10)
    lcd.print(" ");

  lcd.print(score);

  lcd.setCursor(13,1);

  for(int i=0;i<lives;i++)
    lcd.write(byte(2));

  for(int i=lives;i<3;i++)
    lcd.print(" ");
}

void drawGame() {

  clearBuffer();

  screenBuffer[playerRow][PLAYER_COL] = 0;

  if (bulletActive && bulletCol >= 0 && bulletCol < 16)
    screenBuffer[bulletRow][bulletCol] = '-';

  if(bossActive && bossCol>=0 && bossCol<16){
    screenBuffer[0][bossCol]=5;
    screenBuffer[1][bossCol]=5;
  }
  else if (enemyCol >= 0 && enemyCol < 16) {
    if (explosion) {
      screenBuffer[enemyRow][enemyCol] = 3;
    } else {
      switch (enemyType) {
        case 0:
          screenBuffer[enemyRow][enemyCol] = 1;
          break;
        case 1:
          screenBuffer[enemyRow][enemyCol] = 4;
          break;
        case 2:
          screenBuffer[enemyRow][enemyCol] = (enemyHealth == 2) ? 5 : 6;
          break;
      }
    }
  }

  renderBuffer();
  drawHUD();
}

void updateGame(){

    if (invincible && millis() - invincibleTimer >= INVINCIBLE_TIME) {
        invincible = false;
    }


  readJoystick();

  fireBullet();

  moveBullet();

  moveEnemy();

  checkCollision();

  updateExplosion();

}

void saveHighScore() {

  if (score > highScore) {

    highScore = score;
    EEPROM.put(0, highScore);
  }
}

void showGameOver() {

  saveHighScore();

  lcd.clear();

  lcd.setCursor(3,0);
  lcd.print("GAME OVER");

  lcd.setCursor(0,1);
  lcd.print("S:");
  lcd.print(score);

  lcd.print(" H:");
  lcd.print(highScore);

  gameOverSound();

  delay(2500);

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Press Button");

  lcd.setCursor(2,1);
  lcd.print("to Restart");

  while(digitalRead(SW)==HIGH);

  delay(300);

  resetGame();
}

void loop() {

  if(gameOver){

    showGameOver();
  }

  updateGame();

  drawGame();

  delay(25);

}