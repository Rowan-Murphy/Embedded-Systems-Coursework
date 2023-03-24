#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <EEPROM.h>
#include <avr/eeprom.h>
//#include <TimerOne.h>
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  Serial.println("BASIC");
  lcd.setBacklight(7);
#define DEBUG //comment out this line to hide debug statements
#ifdef DEBUG
  Serial.println("Debug statements will be shown");
#endif

#ifndef DEBUG
  Serial.println("Debug statements will not be shown");
#endif
}



enum state_e { INITIALISATION, WAITING_BUTTON_PRESS, WAITING_BUTTON_RELEASE, WAITING_SERIAL_INPUT }; //definition of states
//Structure to hold the current time
struct CurrentTime {
  String CurrentHours;
  String CurrentMinutes;
  String CurrentSeconds;
};
struct CurrentTime CTime = {"07", "29", "00"};
//might want to change this to a function which identifies the current time the arduino is stated at.


//Structure to hold the alarm time
struct AlarmTime {
  String AlarmHours;
  String AlarmMinutes;
  String AlarmSeconds;
};
unsigned long previousMillis = 0;

const int interval = 1000;
struct AlarmTime Alarm = {"07", "30", "00"};
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval ) {
    previousMillis = currentMillis;
    UpdateSeconds();
    CheckAlarmToTime();
  }

  static enum state_e state = INITIALISATION;//initial state here
  static int lastButtonPressed = 0;
  static int currentPressed;
  static long pressTime;
  switch (state) {
    case INITIALISATION: {
#ifdef DEBUG
        Serial.println("Initialisation");
#endif
        AddIDNumber();
        //UpdateAlarmOnDisplay();
        CurrentTime();
        state = WAITING_BUTTON_PRESS;
      }
    case WAITING_BUTTON_PRESS: {
#ifdef DEBUG
        Serial.println("Waiting button press");
#endif
        int buttonPressed = lcd.readButtons();
        #ifdef DEBUG
          Serial.print("Initial Button Press: ");
          Serial.println(buttonPressed);
        #endif
        int pressed = buttonPressed & ~lastButtonPressed;
        lastButtonPressed = buttonPressed;

        if (pressed & (BUTTON_UP | BUTTON_DOWN | BUTTON_SELECT)) {
          if (pressed & (BUTTON_SELECT)) {
            UpdateAlarmOnDisplay();
            lcd.setBacklight(2);
          } else {
            //code for other button inputs
          }
          currentPressed = pressed;
          pressTime = millis();
          state = WAITING_BUTTON_RELEASE;
        }
        break;
      }
    case WAITING_BUTTON_RELEASE: {
#ifdef DEBUG
        Serial.println("Waiting button release");
#endif
        if (millis() - pressTime >= 1000) {
          pressTime = millis();
          //code to enter alarm set mode
          #ifdef DEBUG
            Serial.println("Holding for 1 second");
          #endif
          lcd.setBacklight(5);
          bool alarmSet = true;
          lcd.clear();
          UpdateAlarmOnDisplay();
          unsigned long previousMillisAlarmMode = 0;
          int currentEdit = 0;
          bool displayed = true;
          String tempHoursString = Alarm.AlarmHours;
          String tempMinutesString = Alarm.AlarmMinutes;
          int tempHours = tempHoursString.toInt();
          int tempMinutes = tempMinutesString.toInt();
          String tempOnScreenValue = tempHoursString;
          while (alarmSet) {
            unsigned long currentMillisAlarmMode = millis();
            delay(250);
            int buttonInputAlarmMode = lcd.readButtons();
            
            
            #ifdef DEBUG
              Serial.println("Check toInt works: ");
              Serial.println(tempHours);
              Serial.println(tempMinutes);
            #endif
            switch (buttonInputAlarmMode) {
              case 1 : {
                //Select Button
                #ifdef DEBUG
                  Serial.println("Select Button Pushed");
                #endif
                //save the alarm
                if (tempHours < 10) {
                  
                  Alarm.AlarmHours = "0" + String(tempHours);
                } else {
                  Alarm.AlarmHours = String(tempHours);
                }

                if (tempMinutes < 10) {
                  Alarm.AlarmMinutes = "0" + String(tempMinutes);
                } else {
                  Alarm.AlarmMinutes = String(tempMinutes);
                }
                lcd.clear();
                alarmSet = false;
                AddIDNumber();
                CurrentTime();
                break;
              }
              case 2 : {
                //Right Button
                #ifdef DEBUG
                  Serial.println("Right Button Pushed");
                #endif
                currentEdit = 3;
                tempOnScreenValue = tempMinutesString;
                break;
              }
              case 4 : {
                //Down Button
                #ifdef DEBUG
                  Serial.println("Down Button Pushed");
                #endif
                if(currentEdit == 0) {
                  tempHours--;
                  if (tempHours <= 10 && tempHours > 0) {
                    lcd.setCursor(currentEdit, 1);
                    tempOnScreenValue = "0" + String(tempHours);
                    lcd.print(tempOnScreenValue);
                  } else if (tempHours > 10 && tempHours < 24) {
                    lcd.setCursor(currentEdit, 1);
                    tempOnScreenValue = String(tempHours);
                    lcd.print(tempHours);
                  } else if (tempHours < 0) {
                    tempHours = 23;
                    tempOnScreenValue = String(tempHours);
                    lcd.setCursor(currentEdit, 1);
                    lcd.print(tempHours);
                  } else {
                    //for some reason when decrementing it jumps from 11 to 0 
                    tempHours = 0;
                    lcd.setCursor(currentEdit, 1);
                    lcd.print("00");
                  }
                } else {
                  tempMinutes--;
                  if (tempMinutes <= 10 && tempMinutes > 0) {
                    lcd.setCursor(currentEdit, 1);
                    tempOnScreenValue = "0" + String(tempMinutes);
                    lcd.print(tempOnScreenValue);
                  } else if (tempMinutes > 10 && tempMinutes < 60) {
                    lcd.setCursor(currentEdit, 1);
                    tempOnScreenValue = String(tempMinutes);
                    lcd.print(tempMinutes);
                  } else if (tempMinutes >= 60) {
                    tempMinutes = 0;
                    lcd.setCursor(currentEdit, 1);
                    lcd.print("00");
                  } else if (tempMinutes < 0) {
                    tempMinutes = 59;
                    tempOnScreenValue = String(tempMinutes);
                    lcd.setCursor(currentEdit, 1);
                    lcd.print(tempMinutes);
                  }
                }
                break;
              }
              case 8 : {
                //Up Button
                #ifdef DEBUG
                  Serial.println("Up Button Pushed");
                #endif
                if(currentEdit == 0) {
                  tempHours++;
                  if (tempHours <= 10) {
                    lcd.setCursor(currentEdit, 1);
                    tempOnScreenValue = "0" + String(tempHours);
                    lcd.print(tempOnScreenValue);
                  } else if (tempHours >= 10 && tempHours < 24) {
                    lcd.setCursor(currentEdit, 1);
                    tempOnScreenValue = String(tempHours);
                    lcd.print(tempHours);
                  } else {
                    tempHours = 0;
                    lcd.setCursor(currentEdit, 1);
                    lcd.print("00");
                  }
                } else {
                  tempMinutes++;
                  if (tempMinutes <= 10) {
                    lcd.setCursor(currentEdit, 1);
                    tempOnScreenValue = "0" + String(tempMinutes);
                    lcd.print(tempOnScreenValue);
                  } else if (tempMinutes > 10 && tempMinutes < 60) {
                    lcd.setCursor(currentEdit, 1);
                    tempOnScreenValue = String(tempMinutes);
                    lcd.print(tempMinutes);
                  } else if (tempMinutes >= 60) {
                    tempMinutes = 0;
                    lcd.setCursor(currentEdit, 1);
                    lcd.print("00");
                  }
                }
                break;
              }
              case 16: {
                //Left Button
                #ifdef DEBUG
                  Serial.println("Left Button Pushed");
                #endif
                alarmSet = false;
                AddIDNumber();
                CurrentTime();
                break;
              }
              default : {
                //flash the current time being editied
                
                //lcd.print("  ");
                int flashInterval = 500;
                
                
                if (currentMillisAlarmMode - previousMillisAlarmMode >= flashInterval) {
                  previousMillisAlarmMode = currentMillisAlarmMode;
                  lcd.setCursor(currentEdit, 1);
                  if (displayed == true) {
                    lcd.print("  ");
                    displayed = false;
                  } else {
                    if (currentEdit == 0) {
                      lcd.print(tempOnScreenValue);
                      displayed = true;
                    } else if (currentEdit == 3) {
                      lcd.print(tempOnScreenValue);
                      displayed = true;
                    }
                    
                  }
                  
                }
              }
            }
            //code to flash the value
            
          }
          
        } else {
          int buttonPressed = lcd.readButtons();
          #ifdef DEBUG
            Serial.print("Held button press: ");
            Serial.println(buttonPressed);
          #endif
          int released = ~buttonPressed & lastButtonPressed;
          lastButtonPressed = buttonPressed;
          if (released & currentPressed ) {
            state = WAITING_BUTTON_PRESS;
            lcd.setBacklight(7);
          }
        }
       
      }
    case WAITING_SERIAL_INPUT : {
#ifdef DEBUG
        Serial.println("Waiting serial input");
#endif
        if (Serial.available() > 0) {
          //Will be true if the serial monitor has recieved an input
          char inputRecived[11];
          int inputByte = Serial.read();
          
#ifdef DEBUG
          Serial.println("Input Recieved");
          Serial.println(char(inputByte));
#endif
          
        }
      }
  }
}

static int AlarmCounter = 0;

void CurrentTime() {
  //A function which identifies the current time and updates the lcd accordingly
  //initially this function will set the time to midnight and print it to the lcd
#ifdef DEBUG
  Serial.println("Function CurrentTime");
#endif

  lcd.setCursor(0, 0);
  lcd.print(CTime.CurrentHours);
  lcd.print(":");
  //update minutes
  lcd.setCursor(3, 0);
  lcd.print(CTime.CurrentMinutes);
  lcd.print(":");
  //update seconds
  lcd.setCursor(6, 0);
  lcd.print(CTime.CurrentSeconds);


}

void AddIDNumber() {
  //A function which adds my ID number to the display
#ifdef DEBUG
  Serial.println("Function AddIDNumber");
#endif
  lcd.setCursor(9, 1);
  lcd.print("F116454");
}

void UpdateAlarmOnDisplay() {
  //A function which updates the current alarm time to the display
#ifdef DEBUG
  Serial.println("Function UpdateAlarmOnDisplay");
#endif
  //update hours
  lcd.setCursor(0, 1);
  lcd.print(Alarm.AlarmHours);
  lcd.print(":");
  //update minutes
  lcd.setCursor(3, 1);
  lcd.print(Alarm.AlarmMinutes);
  lcd.print(":");
  //update seconds
  lcd.setCursor(6, 1);
  lcd.print(Alarm.AlarmSeconds);

}

void UpdateSeconds() {

  //A function which updates the seconds after 1 second has passed, updates both the lcd and the structure CurrentTime
#ifdef DEBUG
  Serial.println("Function UpdateSeconds");
  Serial.print("Current value in the CurrentTime structure: ");
  Serial.println(CTime.CurrentSeconds);
#endif
  int currentSeconds = CTime.CurrentSeconds.toInt();
#ifdef DEBUG
  Serial.print("Check toInt() works: ");
  Serial.println(currentSeconds);
#endif
  currentSeconds++;
  if (currentSeconds >= 60) {
    UpdateMinutes();
    CTime.CurrentSeconds = "00";
    lcd.setCursor(6, 0);
    lcd.print(CTime.CurrentSeconds);
  } else {
    if (currentSeconds >= 10) {
      CTime.CurrentSeconds = String(currentSeconds);
      lcd.setCursor(6, 0);
      lcd.print(CTime.CurrentSeconds);
    } else {
      CTime.CurrentSeconds = "0" + String(currentSeconds);
      lcd.setCursor(6, 0);
      lcd.print(CTime.CurrentSeconds);
    }

  }


}

void UpdateMinutes() {
  //a function which updates the minutes once 60 seconds passes from the start of the program,
  //updates both the lcd and the value in the CurrentTime structure
#ifdef DEBUG
  Serial.println("Function UpdateMinutes");
  Serial.print("Current value in CurrentTime structure: ");
  Serial.println(CTime.CurrentMinutes);
#endif
  int currentMinutes = CTime.CurrentMinutes.toInt();
#ifdef DEBUG
  Serial.print("Check if toInt() works: ");
  Serial.println(currentMinutes);
#endif
  AlarmCounter = 0; // This is to ensure once the minute has passed the alarm can be set off again the next day

  currentMinutes++;
  if (currentMinutes >= 60) {
    UpdateHours();
    CTime.CurrentMinutes = "00";
    lcd.setCursor(3, 0);
    lcd.print(CTime.CurrentMinutes);
  } else {
    if (currentMinutes >= 10) {
      CTime.CurrentMinutes = String(currentMinutes);
      lcd.setCursor(3, 0);
      lcd.print(CTime.CurrentMinutes);
    } else {
      CTime.CurrentMinutes = "0" + String(currentMinutes);
      lcd.setCursor(3, 0);
      lcd.print(CTime.CurrentMinutes);
    }
  }
}

void UpdateHours() {
  //A function which updates the hours once 60 minutes has passed, updates both the lcd and the value in the CurrentTime structure
  //in here is when the 12/24 hours is edited
  //for BASIC it is a 24 hour clock

#ifdef DEBUG
  Serial.println("Function UpdateHours");
  Serial.print("Current value in CurrentTime structure: ");
  Serial.println(CTime.CurrentHours);
#endif
  int currentHours = CTime.CurrentHours.toInt();
#ifdef DEBUG
  Serial.print("Check if toInt() works: ");
  Serial.println(currentHours);
#endif
  currentHours++;
  if (currentHours >= 24) { // Here is where the 12/24 is changed
    CTime.CurrentHours = "00";
    lcd.setCursor(0, 0);
    lcd.print(CTime.CurrentHours);
  } else {
    if (currentHours >= 10) {
      CTime.CurrentHours = String(currentHours);
      lcd.setCursor(0, 0);
      lcd.print(CTime.CurrentHours);
    } else {
      CTime.CurrentHours = "0" + String(currentHours);
      lcd.setCursor(0, 0);
      lcd.print(CTime.CurrentHours);
    }
  }


}

void AlarmTimeReached() {
  //a function which prints alarm and flashed the lcd when the alarm time is reached
#ifdef DEBUG
  Serial.println("Funtcion AlarmTimeReached");
#endif
  unsigned long previousMillisAlarm = 0;
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("ALARM");
  int lcdInterval = 500;
  int lcdColour = 1;
  int previousLcdColour = 0;
  bool runLoop = true;
  while (runLoop) {
    unsigned long currentMillisAlarm = millis();
    if (lcd.readButtons() == 0) {
      if (currentMillisAlarm - previousMillisAlarm >= lcdInterval) {
        previousMillisAlarm = currentMillisAlarm;
        lcd.setBacklight(lcdColour);
        if (lcdColour != previousLcdColour) {
          lcdColour++;
          previousLcdColour = lcdColour;
        } else {
          lcdColour--;
        }
      }
    } else {
      runLoop = false;
      lcd.clear();
      lcd.setBacklight(7);
      AddIDNumber();
      //UpdateAlarmOnDisplay();
      CurrentTime();
      break;
    }

  }



}


void CheckAlarmToTime() {
  //A function to check the current time to the alarm time
#ifdef DEBUG
  Serial.println("Function CheckAlarmToTime");
#endif
  if (CTime.CurrentHours == Alarm.AlarmHours && CTime.CurrentMinutes == Alarm.AlarmMinutes && AlarmCounter == 0) {
    AlarmCounter++; // this is to stop the function being called again onces a button has been pressed
    AlarmTimeReached();
    
  }
}
