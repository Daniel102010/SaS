#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <ClickButton.h>

LiquidCrystal_I2C lcd(0x27,16,2);

// PIN MAP
const int BTN_STBY=2;
const int BTN_SLOT[4]={3,4,5,6};
const int LED_STBY=7;
const int LED_SLOT[4]={8,9,10,11};
const int RELAY[8]={A0,A1,A2,A3,A6,A7,12,13};
const int RESET_PIN=1;
const int EEPROM_ADDR=0;
enum SystemState {
  STANDBY,
  ACTIVE,
  SETUP_SLOT,
  SETUP_MENU,
  SETUP_CARD,
  SETUP_GROUP
};

SystemState state=STANDBY;
int activeSlot=0;
const int EEPROM_A2_SLOT_ADDR=10;
int appleSlot=6;
const char* cardList[]={
  "MyCard 1",
  "MyCard 2",
  "MyCard 3",
  "MyCard 4",
  "A-B",
  "C-M",
  "N-S",
  "T-Z"
};

const int CARD_COUNT=8;
const char* groupAB[]={
  "A1",
  "A2",
  "A3",
  "A4",
  "B1",
  "B2",
  "B3",
  "B4"
};

const int GROUP_AB_COUNT=8;

int slotCard[4]={0,0,0,0};
int editSlot=0;
int groupIndex=0;
int groupCard[4]={0,0,0,0};

// ClickButton objects
ClickButton btnStandby(BTN_STBY, LOW, CLICKBTN_PULLUP);
ClickButton btnA(BTN_SLOT[0], LOW, CLICKBTN_PULLUP);
ClickButton btnB(BTN_SLOT[1], LOW, CLICKBTN_PULLUP);
ClickButton btnC(BTN_SLOT[2], LOW, CLICKBTN_PULLUP);
ClickButton btnD(BTN_SLOT[3], LOW, CLICKBTN_PULLUP);

void activateSlot(int slot){
 activeSlot=slot;
 setSlotRelays(slot);
 showReady(slot);
 EEPROM.update(EEPROM_ADDR,slot);
 doReset();
}


void allRelaysOff(){ for(int i=0;i<8;i++) digitalWrite(RELAY[i],LOW); }
void setSlotRelays(int slot){ allRelaysOff(); digitalWrite(RELAY[slot*2],HIGH); digitalWrite(RELAY[slot*2+1],HIGH); }
void allSlotLedsOff(){ for(int i=0;i<4;i++) digitalWrite(LED_SLOT[i],LOW); }

void showStandby(){
 lcd.clear(); lcd.setCursor(0,0); lcd.print("Standby");
 digitalWrite(LED_STBY,HIGH);
 allSlotLedsOff();
 allRelaysOff();
 state=STANDBY;
}

void showReady(int slot){
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("Slot ");
 lcd.print(char('A'+slot));
 //lcd.setCursor(0,1);
 //lcd.print("Ready");
  lcd.setCursor(0,1);
lcd.print("                ");   // șterge linia
lcd.setCursor(0,1);
if(slotCard[slot]==4)
    lcd.print(groupAB[groupCard[slot]]);
else
    lcd.print(cardList[slotCard[slot]]);
 digitalWrite(LED_STBY,LOW);
 allSlotLedsOff();
 digitalWrite(LED_SLOT[slot],HIGH);
 state=ACTIVE;
}

void doReset(){
 digitalWrite(RESET_PIN,HIGH); delay(150);
 digitalWrite(RESET_PIN,LOW); delay(250);
}

void setup(){
 pinMode(BTN_STBY,INPUT_PULLUP);
 for(int i=0;i<4;i++) pinMode(BTN_SLOT[i],INPUT_PULLUP);
 pinMode(LED_STBY,OUTPUT);
 for(int i=0;i<4;i++) pinMode(LED_SLOT[i],OUTPUT);
 for(int i=0;i<8;i++) pinMode(RELAY[i],OUTPUT);
 pinMode(RESET_PIN,OUTPUT);
 digitalWrite(RESET_PIN,LOW);
 lcd.init(); lcd.backlight();
 showStandby();
  for(int i=0;i<4;i++){
  slotCard[i]=EEPROM.read(20+i);
groupCard[i]=EEPROM.read(30+i);

if(groupCard[i]>=GROUP_AB_COUNT){
  groupCard[i]=0;
}
  if(slotCard[i]>=CARD_COUNT){
    slotCard[i]=0;
  }
}
}

void loop() {

  btnStandby.Update();
  btnA.Update();
  btnB.Update();
  btnC.Update();
  btnD.Update();

  switch(state){

    //=================================================
    case STANDBY:
    case ACTIVE:

      if(btnStandby.clicks==1 && state==ACTIVE){
        showStandby();
      }

      if(btnStandby.clicks==-1){
        state=SETUP_SLOT;
        appleSlot=EEPROM.read(EEPROM_A2_SLOT_ADDR);
        if(appleSlot<1 || appleSlot>7) appleSlot=6;

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Apple II Slot");
        lcd.setCursor(0,1);
        lcd.print("Current: ");
        lcd.print(appleSlot);
      }

      if(btnA.clicks==1) activateSlot(0);
      if(btnB.clicks==1) activateSlot(1);
      if(btnC.clicks==1) activateSlot(2);
      if(btnD.clicks==1) activateSlot(3);

    break;

    //=================================================
    case SETUP_SLOT:

      if(btnA.clicks==1){
        appleSlot--;
        if(appleSlot<1) appleSlot=7;

        lcd.setCursor(9,1);
        lcd.print(" ");
        lcd.setCursor(9,1);
        lcd.print(appleSlot);
      }

      if(btnB.clicks==1){
        appleSlot++;
        if(appleSlot>7) appleSlot=1;

        lcd.setCursor(9,1);
        lcd.print(" ");
        lcd.setCursor(9,1);
        lcd.print(appleSlot);
      }

      if(btnStandby.clicks==1){
groupCard[editSlot]=groupIndex;
        EEPROM.update(EEPROM_A2_SLOT_ADDR,appleSlot);

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Setup");

        lcd.setCursor(0,1);
        lcd.print("A B C D");

        state=SETUP_MENU;
      }

    break;

    //=================================================
    case SETUP_MENU:

      if(btnStandby.clicks==-1){
        showStandby();
      }

      if(btnA.clicks==1) editSlot=0;
      if(btnB.clicks==1) editSlot=1;
      if(btnC.clicks==1) editSlot=2;
      if(btnD.clicks==1) editSlot=3;

      if(btnA.clicks==1 ||
         btnB.clicks==1 ||
         btnC.clicks==1 ||
         btnD.clicks==1){

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Slot ");
        lcd.print(char('A'+editSlot));

        lcd.setCursor(0,1);
        lcd.print(cardList[slotCard[editSlot]]);

        state=SETUP_CARD;
      }

    break;

    //=================================================
    case SETUP_CARD:

      if(btnA.clicks==1){

        slotCard[editSlot]--;
        if(slotCard[editSlot]<0)
          slotCard[editSlot]=CARD_COUNT-1;

        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print(cardList[slotCard[editSlot]]);
      }

      if(btnB.clicks==1){

        slotCard[editSlot]++;
        if(slotCard[editSlot]>=CARD_COUNT)
          slotCard[editSlot]=0;

        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print(cardList[slotCard[editSlot]]);
      }
if(btnStandby.clicks==1){

  EEPROM.update(20+editSlot,slotCard[editSlot]);

  if(slotCard[editSlot] < 4){

    lcd.clear();
    lcd.print("Saved");
    delay(700);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Setup");

    lcd.setCursor(0,1);
    lcd.print("A B C D");

    state=SETUP_MENU;
}else{
 
    groupIndex=groupCard[editSlot];
 

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(cardList[slotCard[editSlot]]);

  lcd.setCursor(0,1);
  lcd.print(groupAB[groupIndex]);

  state=SETUP_GROUP;

}

}
     if(btnStandby.clicks==-1){

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Setup");

  lcd.setCursor(0,1);
  lcd.print("A B C D");

  state=SETUP_MENU;
} 

    break;
    //=================================================
case SETUP_GROUP:

  if(btnA.clicks==1){

    groupIndex--;
    if(groupIndex<0)
      groupIndex=GROUP_AB_COUNT-1;

    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print(groupAB[groupIndex]);
  }

  if(btnB.clicks==1){

    groupIndex++;
    if(groupIndex>=GROUP_AB_COUNT)
      groupIndex=0;

    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print(groupAB[groupIndex]);
  }

  if(btnStandby.clicks==1){
groupCard[editSlot]=groupIndex;
EEPROM.update(30+editSlot,groupIndex);
    lcd.clear();
   // lcd.print("Saved");
    lcd.print(groupIndex);
    delay(700);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Setup");
    lcd.setCursor(0,1);
    lcd.print("A B C D");

    state=SETUP_MENU;
  }

  if(btnStandby.clicks==-1){

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Setup");
    lcd.setCursor(0,1);
    lcd.print("A B C D");

    state=SETUP_MENU;
  }

  break;
  }
}
