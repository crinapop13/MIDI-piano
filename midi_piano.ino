#include <MIDI.h>
// Se creeaza si se leaga interfata MIDI la portul Serial default
MIDI_CREATE_DEFAULT_INSTANCE(); 

const byte ROWS = 4; // Numarul de randuri
const byte COLS = 4; // Numarul de coloane

char keys[COLS][ROWS];   //matricea in care vom retine starea butoanelor

// PINII DIGITALI
byte rowPins[ROWS] = {2,3,4,5}; //connect to the row pins (R0-R3) of the keypad
byte colPins[COLS] = {8,9,10,11}; //connect to the column pins (C0-C3) of the keypad

// PINII ANALOGICI
int analogpot1 = A0;  // Potentiometrul 1 si 2 ii vom conecta la pinii analogici A0 si A1
int analogpot2 = A1;  
                      
int analogpot1Old = 0;
int analogpot2Old = 0;
int analogpot1New = 0;
int analogpot2New = 0;
                       // Valoarea pinilor analogici = Recunoscuti de catre Ableton ca " C-54 " & " C-55 "
#define analogpot1CC 1  //valoare pentru cc = continuous controller
#define analogpot2CC 2

// debounce
unsigned long lastDebounceTime[COLS][ROWS] = { {0, 0, 0, 0}, 
                                               {0, 0, 0, 0},
                                               {0, 0, 0, 0},
                                               {0, 0, 0, 0}};  // ultima data cand butonul a fost apasat - in secunde
unsigned long debounceDelay = 200;    //* timpul de debounce, pana cand se stabilizeaza intrarea la aceeasi valoare

// MIDI
byte midiCh = 1; // canalul Midi
byte note[COLS][ROWS] = { {53, 55, 57, 59},  //C1 - S1,S5,S9,S13
                          {60, 61, 62, 63},  //C2 - S2,S6,S10,S14
                          {64, 65, 66, 67},  //C3 - S3,S7,S11,S15
                          {68, 69, 70, 71}}; //C4 - S4,S8,S12,S16

void setup() {
//Serial.begin(9600);  
Serial.begin(115200);
//MIDI.begin();   

    for(int x=0; x<ROWS; x++) {
        pinMode(rowPins[x], INPUT);
    }
 
    for (int x=0; x<COLS; x++) {
        pinMode(colPins[x], INPUT_PULLUP);
    }

pinMode(analogpot1, INPUT);
pinMode(analogpot2, INPUT);
}

void citesteMatrice() {
    // parcurgem coloanele
    for (int colIndex = 0; colIndex < COLS; colIndex++) {
        // col: set to output to low
        byte curCol = colPins[colIndex];
        pinMode(curCol, OUTPUT);
        digitalWrite(curCol, LOW);
 
        // parcurgem randurile
        for (int rowIndex = 0; rowIndex < ROWS; rowIndex++) {
            byte rowCol = rowPins[rowIndex];
            pinMode(rowCol, INPUT_PULLUP);
            keys[colIndex][rowIndex] = digitalRead(rowCol);
            pinMode(rowCol, INPUT);
        }
        // disable the column
        pinMode(curCol, INPUT);
    }
}
void loop() {
  
// Starea actuala a pinilor digitali
bool buttonsValueOld[COLS][ROWS] = { {HIGH, HIGH, HIGH, HIGH}, 
                                     {HIGH, HIGH, HIGH, HIGH},
                                     {HIGH, HIGH, HIGH, HIGH},
                                     {HIGH, HIGH, HIGH, HIGH} };
bool buttonsValueNew[COLS][ROWS] = { {HIGH, HIGH, HIGH, HIGH}, 
                                     {HIGH, HIGH, HIGH, HIGH},
                                     {HIGH, HIGH, HIGH, HIGH},
                                     {HIGH, HIGH, HIGH, HIGH} };

//Starea noua a butoanelor
citesteMatrice();
     
     for (int colIndex=0; colIndex < COLS; colIndex++) {
        for (int rowIndex=0; rowIndex < ROWS; rowIndex++) {  
          if ((millis() - lastDebounceTime[colIndex][rowIndex]) > debounceDelay) {
            if (keys[colIndex][rowIndex] != buttonsValueOld[colIndex][rowIndex]){
              lastDebounceTime[colIndex][rowIndex] = millis();
              if (keys[colIndex][rowIndex] == LOW){  //Daca apasam butonul valoarea pinului va fi trasa la ground, deci va avea valoarea 0
                MIDI.sendNoteOn(note[colIndex][rowIndex], 127, midiCh); //  (Nota,Volocitatea/Volumul,Canalul)
//                Serial.print("Note ");
//                Serial.print(rowIndex);
//                Serial.print(colIndex);
//                Serial.println(" on");
                } else {
                  MIDI.sendNoteOn(note[colIndex][rowIndex], 0, midiCh);
//                Serial.print("Note ");
//                Serial.print(rowIndex);
//                Serial.print(colIndex);
//                Serial.print(" of");
              }
              buttonsValueOld[colIndex][rowIndex] = keys[colIndex][rowIndex];
            }
          }
        }   
      }

//Potentiometrele
int pot1 = analogRead(A0);
int pot2 = analogRead(A1);
int analogpot1New = analogRead(A0);
int analogpot2New = analogRead(A1);

    if (analogpot1New - analogpot1Old >= 35 || analogpot1Old - analogpot1New >= 35) {
      analogpot1Old = analogpot1New;
      analogpot1New = (map(analogpot1New, 0, 1023, 0, 127));  //Mapeaza valoarea citita pe pinul analogic la o valoare care se poate folosi in MIDI
      analogpot1New = (constrain(analogpot1New, 0, 120));
      MIDI.sendControlChange(analogpot1CC, analogpot1New, 1);
//      Serial.print ("pot: ");
//      Serial.println(pot1);
//      Serial.print("potread: ");
//      Serial.println(analogpot1New); 
    }
    
    if (analogpot2New - analogpot2Old >= 35 || analogpot2Old - analogpot2New >= 35) {
      analogpot2Old = analogpot2New;
      analogpot2New = (map(analogpot2New, 0, 1023, 0, 127));   //map(value, fromLow, fromHigh, toLow, toHigh)
      analogpot2New = (constrain(analogpot2New, 0, 120));  //constrange valoarea citita de la potentiometru si mapata in intervalul [0,120] sa fie sigur in acest interval
      MIDI.sendControlChange(analogpot2CC, analogpot2New, 1);
//      Serial.print ("pot: ");
//      Serial.println(pot2);
//      Serial.print("potread: ");
//      Serial.println(analogpot2New); 
    }

delay(25);
}
