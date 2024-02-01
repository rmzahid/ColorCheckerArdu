#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);        // select the pins used on the LCD panel

int photoTran = A5;
int reading = 0;

int redLED = 11;//3;
int greenLED = 12;// 5;
int blueLED = 13;//6;

int wait = 250;
int waitF = 500;
int nextReading = 60;//60 sec
int maxLimit = 5; //maximun sample reading

int maxColorLimit = 3; //how many color sample to check

float maxReadingR=1.0, maxReadingG=1.0, maxReadingB=1.0; //empty data reading
float sampleR[3], sampleG[3], sampleB[3];  // three sample color to trace
int bPress=0;  //button press
int menuFlag = 0; //which position of menu
// 0-calibrate;
// 1-1st standard sample;
// 2-2nd SS;
// 3-3rd SS;
// 4-Set interval period
// 5-Actual sample to trace

int activationFlag = 0; //button press activation

int maxTime = 4;
int timeSelection;

const String pressNow = "PRESS NOW     ";
unsigned long interval = 60 * 60 * 1000UL; // needs to be unsigned to handle rollover correctly?
unsigned long lastTime;

const int UP = 0;
const int DOWN = 1;
const int LEFT = 2;
const int RIGHT = 3;
const int SELECT = 5;
const int NONE = -1;

void setup() {
  // put your setup code here, to run once:
  pinMode(photoTran, INPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  Serial.begin(9600);
  delay(1000);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  lcd.setCursor(0, 0);                   // set the LCD cursor   position
  lcd.print("Welcome to ");
  delay(100);
  lcd.setCursor(0, 1);                   // set the LCD cursor   position
  lcd.print("ColorChecker 1");
  lastTime=millis();
  delay(waitF);
  delay(waitF);

}


void readColor(float &ReadingR, float &ReadingG, float &ReadingB) {
  float readingR = 0;
  float readingG = 0;
  float readingB = 0;
  int i;
  for (i = 0; i < maxLimit; i++) {
    digitalWrite(blueLED, LOW);
    digitalWrite(redLED, HIGH); delay(wait);
    readingR += analogRead(photoTran);
    Serial.print(readingR);
    Serial.print(",");
    delay(wait);

    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH); delay(wait);
    readingG += analogRead(photoTran);
    Serial.print(readingG);
    Serial.print(",");
    delay(wait);

    digitalWrite(greenLED, LOW);
    digitalWrite(blueLED, HIGH); delay(wait); //delay(wait);
    readingB += analogRead(photoTran);
    Serial.print(readingB);
    Serial.println("");
    delay(wait);

  }
  digitalWrite(blueLED, LOW);

  ReadingR = readingR / maxLimit / maxReadingR;
  ReadingG = readingG / maxLimit / maxReadingG;
  ReadingB = readingB / maxLimit / maxReadingB;
  lcd.setCursor(0, 0);
  lcd.print("Color read ends");
  lcd.setCursor(0, 1);
  delay(100);
  lcd.print(ReadingR);
  delay(100);
  lcd.print(";");
  lcd.print(ReadingG);
  lcd.print(";");
  lcd.print(ReadingB);
  lcd.println(";");
  delay(100);
}


//function to check nearest from a set of color samples
// Here out limitation is 3 samples
void nearestColor(float Rx, float Gx, float Bx, int &index) {
  //Find the nearest color
  float dist[maxColorLimit]; int i = 0;
  float colorDist;
  //Find distance
  for (i = 0; i < maxColorLimit; i++) {
    //if(((sampleR[i]+sampleG[i]+sampleB[i])/3.0) <0.5)
    if (((sampleR[i] + Rx) / 2.0) < 0.5)
      dist[i] = sqrt(2 * pow((sampleR[i] - Rx), 2) + 4 * pow((sampleG[i] - Gx), 2) + 3 * pow((sampleB[i] - Bx), 2));
    else
      dist[i] = sqrt(3 * pow((sampleR[i] - Rx), 2) + 4 * pow((sampleG[i] - Gx), 2) + 2 * pow((sampleB[i] - Bx), 2));

  }

  // Find the minimum nearest distance and its index.
  // we can do in above loop
  colorDist = dist[0];
  index = 0;
  for (i = 1; i < maxColorLimit; i++) {
    if (colorDist > dist[i]) {
      index = i;
      colorDist = dist[i];
    }
  }


}

void calibrate() {
  //reading empty for ambient light effect
  float readingR = 0;
  float readingG = 0;
  float readingB = 0;
  int i;

  for (i = 0; i < maxLimit; i++) {
    digitalWrite(blueLED, LOW);
    digitalWrite(redLED, HIGH); delay(wait);
    readingR += analogRead(photoTran);
    Serial.print(readingR);
    Serial.print(",");
    delay(wait);

    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH); delay(wait);
    readingG += analogRead(photoTran);
    Serial.print(readingG);
    Serial.print(",");
    delay(wait);

    digitalWrite(greenLED, LOW);
    digitalWrite(blueLED, HIGH); delay(wait); //delay(wait);
    readingB += analogRead(photoTran);
    Serial.print(readingB);
    Serial.println("X");
    delay(wait);
    digitalWrite(blueLED, LOW);
  }
  
  Serial.print("C-");
  maxReadingR = readingR / maxLimit;
  maxReadingG = readingG / maxLimit;
  maxReadingB = readingB / maxLimit;
  //lcd.setCursor(0,1);
  
  Serial.print(maxReadingR);
  Serial.print(";");
  Serial.print(maxReadingG);
  Serial.print(";");
  Serial.print(maxReadingB);
  Serial.println(";");

}

void setInterval() {
  //lcd.setCursor(0,1);
  //lcd.print("Select time: Up/DOWN; SELECT");
  const String timeString[4][9] = {
    "1 minute", 
    "30 minute", 
    "1 hour", 
    "2 hours"
   };

  int activeFlag = 1;
  keyPress();
  while (activeFlag) {
    switch (bPress) {
      case UP:
        if (++timeSelection >= maxTime)
          timeSelection = 0;
        break;
      case DOWN:
        if (--timeSelection <= 0) timeSelection = maxTime - 1;
        break;
      case SELECT:
        activeFlag = 0;
        break;
      default:
        keyPress();
    }
    //Serial.print(timeString[timeSelection][0]);
    //delay(100);
  }
  lcd.print(timeString[timeSelection][0]);
  delay(100);
  switch(timeSelection) {
    case 0:
      interval=1UL*60*1000;
      break;
    case 1:
      interval=15UL*60*1000;
      break;
    case 2:
      interval=30UL*60*1000;
      break;
    case 3:
      interval=60UL*60*1000;
      break;
      
  }
}//setInterval ends

void myCleanScreen(){
  int i;
  lcd.scrollDisplayRight();
  lcd.setCursor(0,0);
  delay(100);
  for(i=0;i<16;i++){
    
    lcd.print(" ");
    delay(100);
  }
  
  lcd.setCursor(0,1);
  delay(100);
  for(i=0;i<16;i++){
    
    lcd.print(" ");
    delay(100);
  }
}

void myMenu() { //int mFlag, int mActuation) {
  
  String menu[6][1] = {

    "0-",
  // "Calibrate      ",
    "1-",
  //"Insert 1st std ",
    "2-"
  //"Insert 2nd SS  ",
    "3-",
  //"Insert 3rd SS  ",
    "4-",
  //"Set T Interval ",
    "5-",
  //"Put main sample"
  };
  
  Serial.print("M-");
  Serial.print(menuFlag);
  Serial.print("-");
  Serial.print(bPress);
  Serial.print(menu[menuFlag][0]);
  Serial.println(activationFlag);
  if(menuFlag!=5)
    myCleanScreen();
  lcd.setCursor(0, 0);
  lcd.print(menu[menuFlag][0]);
  delay(100);
  switch (menuFlag) {
    case 0:
      //calibrate
      //if (activationFlag) {
        calibrate();
        activationFlag = 0;
      //}
      //else
        {
          lcd.setCursor(0, 0);                   // set the LCD cursor   position
        
          //lcd.print("Calibrate Press Select");
          //lcd.print(menu[menuFlag][0]);
        }
  
      break;
    case 1:
      //read first color
      if (activationFlag) {
        readColor(sampleR[0], sampleG[0], sampleB[0]);
        Serial.println(sampleR[0]);
        activationFlag = 0;
      }
      else
        {
        //lcd.setCursor(0, 0);                   // set the LCD cursor   position
        //lcd.print(menu[menuFlag][0]);
        }
      //lcd.print("Sample 1 Press Select");
      break;
    case 2:
      //read color for second point
      if (activationFlag) {
        readColor(sampleR[1], sampleG[1], sampleB[1]);
        Serial.println(sampleR[1]);
        activationFlag = 0;
      }
      else
        {
          //lcd.setCursor(0, 0);                   // set the LCD cursor   position
      //lcd.print("Sample 2 Press Select");
        //lcd.print(menu[menuFlag][0]);
        }
      break;
    case 3:
      //read color third point
      if (activationFlag) {
        readColor(sampleR[2], sampleG[2], sampleB[2]);
        Serial.println(sampleR[2]);
        activationFlag = 0;
      }
      else
        {
          //lcd.setCursor(0, 0);                   // set the LCD cursor   position
        
        //lcd.print("Sample 3 Press Select");
        //lcd.print(menu[menuFlag][0]);
        }
      break;
    case 4:
      //set of sample reading interval
      if (activationFlag) {
        //setInterval();
        interval=1UL*60*1000;
        activationFlag = 0;
      }
      else
        {
          //lcd.setCursor(0, 0);                   // set the LCD cursor   position
        

        //lcd.print("Interval Press Select");
          //lcd.print(menu[menuFlag][0]);
        }
      break;
    case 5:
      //find nearest color
      //lcd.setCursor(0, 0);                   // set the LCD cursor   position

      //lcd.print("Interval Press Select");
      //lcd.print(menu[menuFlag][0]);
      //delay(100);
      while(millis() - lastTime <=  interval) {
        lcd.setCursor(0, 1);                   // set the LCD cursor   position
        lcd.print(millis());
        delay(1000);
      }
      if (millis() - lastTime >=  interval) {
        // do something every hour
        float R, G, B;
        int index;
        readColor(R, G, B);
        delay(100);
        nearestColor(R, G, B, index);
        Serial.print("Index: ");
        Serial.println(index);
        lcd.setCursor(0, 0);
        delay(150);
        lcd.print("Index: ");
        lcd.print(index);
        delay(100);
        //printFile(index);//with time interval
        activationFlag = 0;
        lastTime = millis();                  // reset timer
        
        break;
      }
  }

}

//keyPress
void keyPress() {
  int val;                               // variable to store the value coming from the analog pin
  //double data;                           // variable to store the temperature value coming from the conversion formula
  val = analogRead(0); //multiplexed L-R-U-D-Select button
  lcd.setCursor(0, 1);                   // set the LCD cursor   position
  lcd.print(pressNow);
  delay(100);
  Serial.print("Analog K --");
  Serial.println(val);

  while (val <= 1023) {
    //lcd.setCursor(1, 0);                   // set the LCD cursor   position
    //lcd.print("Val ---");
    //lcd.print(val);

    if (val < 10)
    {
      bPress = RIGHT;//R
      break;
    }
    else if (val < 145) //140
    {
      bPress = UP; //UP
      break;
    }
    else if (val < 329)
    {
      bPress = DOWN; //Down
      break;
    }
    else if (val < 505)
    {
      bPress = LEFT; //Left
      break;
    }
    else if (val < 741)
    {
      bPress = SELECT; //Select
      break;
    }
    else
    {
      bPress = NONE;
      lcd.setCursor(0, 1);                   // set the LCD cursor   position
      lcd.print(pressNow);
      delay(100);
      val = analogRead(0); //multiplexed L-R-U-D-Select button
      //Serial.print("Analog data --");
      //Serial.println(val);
    }
  }



  Serial.print("bPress");
  Serial.print(val);
  Serial.print(bPress);
  if (bPress == SELECT) {
    activationFlag = 1;
    Serial.println("Activation");
  }

}
void loop() {
  // put your main code here, to run repeatedly:

  int i, index;

  //lcd.setCursor(1, 0);                   // set the LCD cursor   position
  //lcd.print("Val ---");
  //lcd.print(val);

  Serial.print("Analog 0 --");
  //Serial.println(val);

  //lcd.setCursor(0, 0);
  //lcd.print(menu[menuFlag][0]);
  //lcd.setCursor(0, 1);                   // set the LCD cursor   position
  //lcd.print(pressNow);
  //Auto mode

  //myCleanScreen();
  delay(100);
  if(menuFlag==0){ //Only for first time
    myMenu();
    menuFlag++;
  }
  
  if (menuFlag>0 && menuFlag < 5) {
    keyPress();
    myMenu();
    menuFlag++;
    
    Serial.print("NOW-------Auto -----");
    Serial.println(menuFlag);
    delay(waitF);
    //scanf enter
  }
  else
  {
    Serial.println("Sample Insert----   ");
    menuFlag = 5;
    myMenu();
    delay(waitF);
    //delay(waitF);
  }
}
