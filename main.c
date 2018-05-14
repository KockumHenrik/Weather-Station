
#include "system_sam3x.h" 
#include "at91sam3x8.h" 
#include "core_cm3.h"
#include <stdio.h>
#include "limits.h"
#include "float.h"


volatile unsigned int *PIOD_PER = AT91C_PIOD_PER; //Pin Enable
volatile unsigned int *PIOD_ODR = AT91C_PIOD_ODR; // Output Disable Registerr
volatile unsigned int *PIOD_OER = AT91C_PIOD_OER; // Output Enable
volatile unsigned int *PIOD_PUDR = AT91C_PIOD_PPUDR; //Pull-up disable resistor
volatile unsigned int *PIOD_ISR = AT91C_PIOD_ISR; //Interrupt Status Register
volatile unsigned int *PIOD_SODR = AT91C_PIOD_SODR; //Set Output Data
volatile unsigned int *PIOD_PDSR = AT91C_PIOD_PDSR; //
volatile unsigned int *PIOD_CODR = AT91C_PIOD_CODR; //Clear Output Data
volatile unsigned int *PIOD_IFER = AT91C_PIOD_IFER; //
volatile unsigned int *PIOD_IFSR = AT91C_PIOD_IFSR; //
volatile unsigned int *PIOD_IER = AT91C_PIOD_IER; //Interrupt Enable
volatile unsigned int *PIOD_PUER = AT91C_PIOD_PUER; //
  
volatile unsigned int *PIOC_PER = AT91C_PIOC_PER; //Pin Enable
volatile unsigned int *PIOC_ODR = AT91C_PIOC_ODR; // Output Disable Registerr
volatile unsigned int *PIOC_OER = AT91C_PIOC_OER; //Output Enable
volatile unsigned int *PIOC_CODR = AT91C_PIOC_CODR; //Clear Output Data
volatile unsigned int *PIOC_SODR = AT91C_PIOC_SODR;
volatile unsigned int *PIOC_PDSR = AT91C_PIOC_PDSR; // Pin Data Status Register
volatile unsigned int *PIOC_PUDR = AT91C_PIOC_PPUDR; //Pull-up disable resistor
  
volatile unsigned int *PMC_PCER1 = AT91C_PMC_PCER1;
volatile unsigned int *PMC_PCER = AT91C_PMC_PCER; 

/*----------------------------------------------Functions--------------------------------------------------------*/  

void ButtonInit(void);
void DisplayInit(void);
void TempInit(void);
void ServoInit(void);
void LdrInit(void);   //Inititierar komponenter

void Delay(int Value);  //Delay function

int func(void);   //Används för att se vilken knapp som är intryckt
  
char Read_Status_Display(void);   //Kollar om skärmen är fungerande
void writeDataDisp(char cData);   //Skriver in en char på skärmen
void writeComDisp(char cCommand);   //Kommando till skärm t.ex. byta pixel
void moveCursor(void);    //Flyttar psoitionen på skärmen till (0,0)
void clearTextScreen(void);   //Rensar skärm
void clearTextRows(int col);    //Rensar raderna från rad 0 till rad (col)
void clearPixelScreen(void);    //Rensar skärm
void writeCharDisp(char cChar);   //Gör om charen till rätt värde, sen skriver ut den på skärmen
void writeWordsDisp(char sSentence[]);  //Skriver ut flera chars som en mening på skärmen
void writeNumberDisp(float dec);    //Skriver ut en float på skärmen
void writeNewRowDisp(int nCharOnScreen);  //Hoppar ner en rad

void tempMeasure(); //Mäter temperaturen
void TC0_Handler(); //Tar värdena från sensorn och gör om de till rätt grad

void turnServo(int nTurn); //Vrider på servon till angiver grad

void lightScan(void); //Kollar vad värdet på ljus sensorn är
void servoScan(void); //Avänder servon och ljus sensorn för att se vad värdena är på 180 grader

void startMenu(void); //Startup menyn där man kan välja vilket mode man ska ta
void tempMenu(void); //Visar nuvarande temperatur
void alarmMenu(void); //Här kan man sätta en maxgräns och en mingräns och får en varning om temperaturen överstiger eller går under de gränserna
void ldrMenu(void); //Kollar var största ljuskällan är
void loggingMenu(void); //Loggar värden som tas varje minut under en vecka, eller ett antal värden per sekund i 7 minuter
void logFunction(int mode); //Functionen som används i 



/*---------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------Variables--------------------------------------------------*/ 

int column;
int row;
int firstRow = 0x4;
int firstCol = 0x4;
int vec[] ={0x200, 0x100, 0x80};
int rowVec[] = {0x4, 0x8, 0x10, 0x20};
int button;
int and;

char cCharDisp[40*16];    //Tecken på skärmen
int nCharDisp = 0;        //Pekare -> Sista input värdet
int nLastInputDisp = 0;   //Pekare -> Föregånde sistas input värde

int nStatusTemp;
char cStatusTemp;
int nCommandTemp;

float nTempC;

float ldrValue;

float nLogTemp[10080];


/*---------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------Main-------------------------------------------------------*/

int main(){
  
  SystemInit();
  ButtonInit();
  ServoInit();
  DisplayInit();
  TempInit();
  LdrInit();
  
  
  while(1){
      startMenu();
  }
}

/*---------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------Initialization---------------------------------------------------*/

void ButtonInit(void){
    *PIOD_ISR = 1<<1;
    *PIOD_PER = 0xD;
    *PIOD_IER = 1<<1;
    *PIOD_PUDR = 1<<3;
    *PIOD_PUER = 0x8;
    *PIOD_OER = 0xC;
    *PIOD_IFER = 1<<1;
  
    *PMC_PCER  = 0x6000;                                    //Writes 1 to bit 14 (pin26)
    *PMC_PCER1 = 0x30;
    
    *PIOC_ODR  = 0x3C;
    *PIOC_OER  = 0x380;
    *PIOC_PER  = 0x3BC;                                      //Input enable register
    *PIOC_CODR = 0x380;
    *PIOC_SODR = 0x380;
  
    *PIOD_CODR = 0x4;
}

void DisplayInit(void){
    *PIOD_PER     = AT91C_PIO_PD0;                //Enable RESET PD0-Pin25
    *PIOD_PUDR   = AT91C_PIO_PD0;                //Disable Pull-up RESET PD0-Pin25
    *PIOD_OER     = AT91C_PIO_PD0;                //Enable output RESET PD0-Pin25
    *PIOC_PER     = 0xFF3FC;                      //Enable hela PIOC - OE KEYBOARD 
    *PIOC_PUDR   = AT91C_PIO_PC12 + AT91C_PIO_PC13;  //Disable Pull-up PC12 + PC13
    *PIOC_OER     = AT91C_PIO_PC12 + AT91C_PIO_PC13;  //Enable output PC12 + PC13
    *PIOC_OER     = 0x3C000;                      //Set PC14,15,16,17 as output 
    *PIOC_PUDR   = 0x3C3FC;                      //Disable Pull-up databus + PC14,15,16,17
    
    //Reset screen
    *PIOD_CODR    = 1;                            //Clear RESET (DISP)
    Delay(10000);                                       //Delay 20000
    *PIOD_SODR    = 1;                            //Set RESET (DISP)
    
    //Init screen
    writeDataDisp(0x00);
    writeDataDisp(0x00);
    writeComDisp(0x40);                                 //Set text home address
    writeDataDisp(0x00);
    writeDataDisp(0x40);
    writeComDisp(0x42);                                 //Set graphic home address
    writeDataDisp(0x28);                                //0x1E Standard
    writeDataDisp(0x00);
    writeComDisp(0x41);                                 //Set text area
    writeDataDisp(0x28);                                //0x28
    writeDataDisp(0x00);
    writeComDisp(0x43);                                 //Set graphic area
    writeComDisp(0x81);                                 //Text mode (0x80)
    writeComDisp(0x9C);                                 //Text on (0x94) graphic on(0x9C)
    
    //Visar inversa färger
    *PIOC_OER = 0x80000; 
    *PIOC_CODR = 0x80000;
    
    //Rensar display
    moveCursor();                                       //Flyttar cursor till (0,0)
    clearPixelScreen();                                 //Rensar skärmen från pixlar
    clearTextScreen();                                  //Rensar skärmen från tecken
}

void TempInit(){
    *AT91C_PMC_PCER = 0x08001000;                        //PMC = TC0 (0x8000000) & PIOB (0x1000) 
    *AT91C_PIOB_SODR = AT91C_PIO_PB25;                  
    *AT91C_PIOB_OER = AT91C_PIO_PB25;
    *AT91C_PIOB_PER = AT91C_PIO_PB25;
    *AT91C_TC0_CMR  = (*AT91C_TC0_CMR | AT91C_TC_CLKS_TIMER_DIV1_CLOCK); //Select Timer_Clock1 as TCCLK
    *AT91C_TC0_CCR  = 0x1;            //Enable counter and make a sw_reset in TC0_CCR0
    *AT91C_TC0_CMR  = (*AT91C_TC0_CMR | AT91C_TC_LDRA_FALLING);    //Load counter to A when TIOA falling in (TC0_CMR0)
    *AT91C_TC0_CMR  = (*AT91C_TC0_CMR | AT91C_TC_LDRB_RISING);    //Load counter to B when TIOA rising in (TC0_CMR0)
    
    //Enable the interrupt (NVIC) with the inline declared function
    NVIC_ClearPendingIRQ(TC0_IRQn);                       
    NVIC_SetPriority(TC0_IRQn, 200);
    NVIC_EnableIRQ(TC0_IRQn);
    //tempMeasure();
    //tempMeasure();
}

void ServoInit(){
  
  *AT91C_PMC_PCER = 0x0808700; 
  
  *AT91C_PIOB_PDR = 1<<17; // PIN DISABLE REGISTER PIN 27.
  *AT91C_PIOB_ABMR = 1<<17; // Pheriphrial AB Select Register pin 27.
  *AT91C_PWMC_ENA = 1<<1; // PWM Enable Register.
  *AT91C_PWMC_CH1_CMR = 0x5;  // 0b0101 SET MC_CKLDIV32
  *AT91C_PWMC_CH1_CPRDR = 52500;   // Channel Period Register.(20 MS)
  *AT91C_PWMC_CH1_CDTYR = 1690;    // Dead Time Register. 1500 = 0deg
}

void LdrInit(){
    *AT91C_PMC_PCER = 0x800;                            //PMC -> PIOA
    *AT91C_ADCC_MR  = 0x200;
    *AT91C_ADCC_CWR = 0x1000000;                        //Extended Mode Register, Möjligt att använda två
    lightScan();
    lightScan();                                        //Run program twice to get rid of unwanted values
}


/*---------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------Delay------------------------------------------------------*/

void Delay(int Value){
  int i;
  for(i=0;i<Value;i++)
    asm("nop");
}

/*---------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------Buttons----------------------------------------------------*/

int func(void){
  button = 0;
  *PIOD_CODR = 0x4;
  *PIOC_OER = 0x380;
  *PIOC_SODR  = 0x380;
  for(column = 0; column < 3; column++){
    *PIOC_SODR = 0x380;
    *PIOC_CODR = vec[column];
    for(row = 0; row < 4; row++){
      and = *PIOC_PDSR & rowVec[row];
      if(and==0){
        button = 3*row+column+1;
      }
    }
    *PIOC_SODR = vec[column];
  }
  *PIOC_OER = 0x380;
  return button;
}

/*---------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------Display----------------------------------------------------*/

char Read_Status_Display(void){
  nStatusTemp = 0;
  cStatusTemp = 0;
  
  *PIOC_ODR = 0x3FC;  
  *PIOC_SODR = 1<<13; //Set dir = 1 (input)
  *PIOC_CODR = 1<<12; //Enable output
  *PIOC_SODR = 1<<17; //Set C/D = 1;
  *PIOC_CODR = 1<<16; //Clear CS = 0;
  *PIOC_CODR = 1<<15; //Clear RD = 0;
  
  Delay(20);
  
  nStatusTemp = *PIOC_PDSR & 0xC; // Läser av skärmens bitar
  cStatusTemp = nStatusTemp>>2;   //Flyttar bitarna
  
  *PIOC_SODR = 1<<16; //Set CS = 1;
  *PIOC_SODR = 1<<15; //Set RD = 1;
  *PIOC_SODR = 1<<12; //Disable output
  *PIOC_CODR = 1<<13; //Clear dir = 0 (output)
  
  return cStatusTemp;
}

void writeDataDisp(char cData){
  int nDataTemp = 0; 
  
  while((Read_Status_Display()&0x3) != 0x3){
  }
  
  nDataTemp = cData;          
  nDataTemp = nDataTemp << 2; //Flyttar bitar
  
  *PIOC_CODR = 0x3FC;         //Clear bits
  *PIOC_SODR = nDataTemp;     //Set data to bits
  
  *PIOC_CODR = 1<<13;         //Clear dir = 0 (output)
  *PIOC_CODR = 1<<12;         //Enable output
  
  *PIOC_OER = 0x3FC;          //Set bits to output
  
  *PIOC_CODR = 1<<17;         //Clear C/D = 0
  *PIOC_CODR = 1<<16;         //Clear CS = 0
  *PIOC_CODR = 1<<14;         //Clear WR = 0
  
  Delay(20);
  
  *PIOC_SODR = 1<<16;         //Set CS = 1
  *PIOC_SODR = 1<<14;         //Set WR = 1
  *PIOC_SODR = 1<<12;         //Disable OE output
  
  *PIOC_ODR = 0x3FC;          //Clear bits (input)
}
        
void writeComDisp(char cCommand){
  int nCommandTemp = 0;
  
  while((Read_Status_Display()&0x3) != 0x3){
  }
         
  nCommandTemp = cCommand;
  nCommandTemp = nCommandTemp << 2;
  
  *PIOC_CODR = 0x3FC;
  *PIOC_SODR = nCommandTemp;
  
  *PIOC_CODR = 1<<13;
  *PIOC_CODR = 1<<12;
  *PIOC_OER = 0x3FC;
  
  *PIOC_SODR = 1<<17;
  *PIOC_CODR = 1<<16;
  *PIOC_CODR = 1<<14;
  
  Delay(20);
  
  *PIOC_SODR = 1<<16;
  *PIOC_SODR = 1<<14;
  *PIOC_SODR = 1<<12;
  
  *PIOC_ODR = 0x3FC;
}
         
void moveCursor(void){
    writeDataDisp(0x00);
    writeDataDisp(0x00);
    writeComDisp(0x24);
}

void clearTextScreen(void){
    moveCursor();                                       //Move cursor to (0,0)
    for(int i = 0; i<640; i++){                             //40 Kolumner * 16 Rader
        writeDataDisp(0x00);
        writeComDisp(0xC0);
    }
    moveCursor();                                       //Move cursor to (0,0)
    nCharDisp = 0;
    nLastInputDisp = 0;
}

void clearTextRows(int col){
     moveCursor();
     int counter = 0;
     moveCursor(); 
     for(int i = 0; i < 640; i++){
        if(counter < 40){
            if((col*40)<=i){
                writeDataDisp(0x00);
                counter++;
            }
            writeComDisp(0xC0);
        }
     }
     moveCursor();
}

void clearPixelScreen(void){
    //writeComDisp(0x98);
    int pix = 0;
    int mover;
    int col;
    int line;
    int bit;
    int move1 = 0;
    int move2 = 0;
    for(line = 1; line <= 128; line++){                 //128 kolumner
        for(col = 0; col < 40; col++){                  //40 rader
            mover = (col+pix);
            move1 = mover & 0xFF;
            move2 = (mover & 0xFF00)>>8;
            move2 = move2 + 0x40;                       //0x40 = graphic memory start

            writeDataDisp(move1);
            writeDataDisp(move2);
            writeComDisp(0x24);
            
            for(bit = 0; bit < 6; bit++){
                writeComDisp(0xF0 + bit);
            }
        }
        pix = pix + col;                                //Sparar föregåendes rads antal pixlar
    }
    moveCursor();                                       //Move cursor to (0,0)
}

void writeCharDisp(char cChar){
        cChar = cChar - 0x20;
        writeDataDisp(cChar);
        writeComDisp(0xC0);
        nCharDisp++;
}

void writeWordsDisp(char sSentence[]){
    writeComDisp(0x9C);
    nLastInputDisp = nCharDisp;
    int nCountColOnScreen = 0;                          //Räknar antal tecken på raden
    int nOneChar;                                       //Pekare -> tecken i sträng
    
    for(nOneChar = 0; nOneChar < strlen(sSentence); nOneChar++){
            writeCharDisp(sSentence[nOneChar]);         //Skriver ut tecken till display
            nCountColOnScreen++;
    }
}

void writeNumberDisp(float dec){
    char temp[30];
    snprintf(temp,sizeof(temp),"%f",dec);
    writeWordsDisp(temp);
}

void writeNewRowDisp(int nCharOnScreen){
    int nCharLeft;
    nCharLeft = nCharOnScreen/40;
    if(nCharLeft != 0){
        nCharOnScreen = nCharOnScreen - (40*nCharLeft);
    }
    while(nCharOnScreen < 40){
        writeCharDisp(' ');
        nCharOnScreen++;
    } 
}

/*---------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------Temprature-------------------------------------------------*/

void tempMeasure(){
    AT91C_BASE_TC0->TC_IER = AT91C_TC_LDRBS;            //Enable interrupt TC_IER_LDRBS
    AT91C_BASE_PIOB->PIO_OER = AT91C_PIO_PB25;
    AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB25;         //Set pin as low
    Delay(25);
    AT91C_BASE_PIOB->PIO_SODR = AT91C_PIO_PB25;
    AT91C_BASE_PIOB->PIO_ODR = AT91C_PIO_PB25;          //Create a startpuls with a Delay(25)...
    AT91C_BASE_TC0->TC_IER = AT91C_TC_LDRBS;            //(Re)enable interrupts (due to collition)
    AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;            //...and make a sw_reset in TC0_CCR0
}

void TC0_Handler(){
    Delay(5);
    int nKlockLow;
    int TC_RA = *AT91C_TC0_RA;
    int TC_RB = *AT91C_TC0_RB;    
    AT91C_BASE_TC0->TC_IDR = AT91C_TC_LDRBS;            //Disable interrupt TC_IER_LDRBS
    AT91C_BASE_TC0->TC_SR;
    nKlockLow = (TC_RB - TC_RA);            //Antalet klockpulser som signalen är låg 
    if(nKlockLow > 0x200){
        nTempC = (float)((((nKlockLow * (1 / 42.0)) / 5.0) - 273.15)-3);   //(-2) Kompensation,  timeout multiplier(µs/°K)
    }
}

/*---------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------Servo-----------------------------------------------------*/

void turnServo(int nTurn){
    if(nTurn < 0)
    {
      nTurn = 0;
    } 
    else if(nTurn > 180)
    {
      nTurn = 180;
    }
  
    *AT91C_PWMC_CH1_CDTYUPDR = 1690 + ((nTurn*5000)/180);  //Sätt start position på servon
}

/*---------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------LDR-------------------------------------------------------*/

void lightScan(void){
    *AT91C_ADCC_CHER = 0x4;
    *AT91C_ADCC_CR =0x2;
    *AT91C_ADCC_IER = 1<<24;
    ldrValue = (*AT91C_ADCC_LCDR & 0xFFF);
}

void servoScan(void){
    turnServo(0);
    int highPos = 0;
    float temp = FLT_MAX;
    for(int i = 0; i <= 180; i = i + 10){
        lightScan();
        Delay(10000);
        if(ldrValue <= temp){
            temp = ldrValue;
            highPos = i;
        }
        turnServo(i);
        Delay(5000000);
    }
    turnServo(highPos);
}

/*---------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------Menu------------------------------------------------------*/

void startMenu(void){
    writeWordsDisp("Start Menu");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("1. Temp Menu");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("2. Alarm Menu");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("3. Light Menu");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("4. Logging Menu");
    while(1){
        func();
        if(button==1){
            clearTextScreen();
            clearPixelScreen();
            tempMenu(); 
        }
        else if(button==2){
            clearTextScreen();
            clearPixelScreen();
            alarmMenu(); 
        }
        else if(button==3){
            clearTextScreen();
            clearPixelScreen();
            ldrMenu(); 
        }
        else if(button==4){
            clearTextScreen();
            clearPixelScreen();
            loggingMenu();
        }
      
      
    }
}

void tempMenu(void){
    writeWordsDisp("Temp Menu");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("1. Show Temperature");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("0. Back");
    
    
    while(1){
        func();
        if(button == 1){
            clearTextScreen();
            clearPixelScreen();
            writeNewRowDisp(nCharDisp);
            writeNewRowDisp(nCharDisp);
            writeWordsDisp("Showing Temperature");
            writeNewRowDisp(nCharDisp);
            writeWordsDisp("0. Back");
            while(1){
                 clearTextRows(1);
                 tempMeasure();
                 Delay(391667);
                 writeNumberDisp(nTempC);
                 
                 func();
                 if(button == 11){
                     clearTextScreen();
                     clearPixelScreen();
                     tempMenu();
                 }
                 Delay(2000000);
                 
            }
        }
        else if(button == 11){
            clearTextScreen();
            clearPixelScreen();
            startMenu();   
        }
    }
}

void alarmMenu(void){
    float highAlarm = 0;
    float lowAlarm = 0;
    float temp1 = 0;
    float temp2 = 0;
    int state = 0;
    writeWordsDisp("Alarm Menu");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("1. Set Alarm");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("0. Back");
    while(1){
        func();
        if(button == 1){
            clearTextScreen();
            clearPixelScreen();
            writeNewRowDisp(nCharDisp);
            writeNewRowDisp(nCharDisp);
            writeWordsDisp("Set max and min value on alarm, <= 9999");
            writeNewRowDisp(nCharDisp);
            writeWordsDisp("Press * too set max, then press it again to set min");
            clearTextRows(1);
            while(1){
                func();
                if(button != 10 && button != 12 && state == 0 && temp1 < 1000 && button != 0){
                  if( button == 11 && temp1 > 0){
                      temp1 = temp1 * 10;
                  }
                  else if(button > 0 && button < 10) {
                      temp1 = temp1 * 10 + button;
                  }   
                }
                else if(button != 10 && button != 12 && state == 1 && temp2 < 1000 && button != 0){
                  if(button == 11 && temp2 > 0){
                      temp2 = temp2 * 10;
                  }
                  else if(button > 0 && button < 10){
                      temp2 = temp2 * 10 + button;
                  }   
                }
                if(button == 10 && state ==  0 && temp1 > 0){
                    state = 1;
                    highAlarm = temp1;
                }
                if(button == 10 && state == 1 && temp2 > 0){
                    lowAlarm = temp2;
                    
                    if(lowAlarm > highAlarm){
                        clearTextScreen();
                        clearPixelScreen();
                        writeNewRowDisp(nCharDisp);
                        writeNewRowDisp(nCharDisp);
                        writeNewRowDisp(nCharDisp);
                        writeNewRowDisp(nCharDisp);
                        writeNewRowDisp(nCharDisp);
                        writeNewRowDisp(nCharDisp);
                        writeWordsDisp("-------Error: min bigger than max-------");
                        Delay(10000000);
                        clearTextScreen();
                        clearPixelScreen();
                        alarmMenu();
                    }
                    
                    clearTextScreen();
                    clearPixelScreen();
                    writeNewRowDisp(nCharDisp);
                    writeNewRowDisp(nCharDisp);
                    writeWordsDisp("Max Alarm:");
                    writeNumberDisp(highAlarm);
                    writeNewRowDisp(nCharDisp);
                    writeWordsDisp("Min Alarm:");
                    writeNumberDisp(lowAlarm);
                    writeNewRowDisp(nCharDisp);
                    writeNewRowDisp(nCharDisp);
                    writeWordsDisp("0. Back");
                    while(1){
                        clearTextRows(1);
                        tempMeasure();
                        Delay(391667);
                        writeNumberDisp(nTempC);
                        if(nTempC >= highAlarm){
                            clearTextScreen();
                            clearPixelScreen();
                            writeNewRowDisp(nCharDisp);
                            writeNewRowDisp(nCharDisp);
                            writeNewRowDisp(nCharDisp);
                            writeNewRowDisp(nCharDisp);
                            writeNewRowDisp(nCharDisp);
                            writeNewRowDisp(nCharDisp);
                            writeWordsDisp("------Error: Temprature is too high-----");
                            writeNewRowDisp(nCharDisp);
                            writeWordsDisp("------0. Back to alarm menu-------------");    
                            while(1){
                                func();
                                if(button == 11){
                                    clearTextScreen();
                                    clearPixelScreen();
                                    alarmMenu();  
                                }
                            }
                        }
                        else if(nTempC <= lowAlarm){
                            clearTextScreen();
                            clearPixelScreen();
                            writeNewRowDisp(nCharDisp);
                            writeNewRowDisp(nCharDisp);
                            writeNewRowDisp(nCharDisp);
                            writeNewRowDisp(nCharDisp);
                            writeNewRowDisp(nCharDisp);
                            writeNewRowDisp(nCharDisp);
                            writeWordsDisp("------Error: Temprature is too low------");
                            writeNewRowDisp(nCharDisp);
                            writeWordsDisp("------0. Back to alarm menu-------------");
                            while(1){
                                func();
                                if(button == 11){
                                    clearTextScreen();
                                    clearPixelScreen();
                                    alarmMenu();  
                                }
                            }
                            
                        }
                        Delay(2500000);
                        func();
                        if(button == 11){
                            clearTextScreen();
                            clearPixelScreen();
                            alarmMenu();
                        }
                    }
                }     
                writeNumberDisp(temp1);
                writeWordsDisp("            ");
                writeNumberDisp(temp2);
                Delay(1500000);
                clearTextRows(1);
                
                
            }
        }
        
        else if(button == 11){
            clearTextScreen();
            clearPixelScreen();
            startMenu();   
        }
        
    }
}

void ldrMenu(void){
    writeWordsDisp("Light Menu");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("1. Find Light Source");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("0. Back");
    while(1){
        func();
        if(button == 1){
            clearTextScreen();
            clearPixelScreen();
            writeWordsDisp("Finding Light Source");
            servoScan();
            clearTextScreen();
            clearPixelScreen();
            ldrMenu(); 
        }
        else if(button == 11){
            clearTextScreen();
            clearPixelScreen();
            startMenu();   
        }
    }
}

void loggingMenu(void){
    writeWordsDisp("Logging Menu");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("1. Slow Mode (One week)");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("2. Fast Mode (Seven Minutes)");
    writeNewRowDisp(nCharDisp);
    writeWordsDisp("0. Back");
    writeNewRowDisp(nCharDisp);
    while(1){
        func();
        if(button == 1){
            logFunction(0);
        }
        else if(button == 2){
            logFunction(1);
        }
        else if(button == 11){
            clearTextScreen();
            clearPixelScreen();
            startMenu();
        }
    }
}

void logFunction(int mode) {
    clearTextScreen();
    clearPixelScreen();
    while(1){
        float minTemp = FLT_MAX;
        float maxTemp = FLT_MIN;
        float temp = 0;
        for(int j = 0; j <12; j++){
            writeNewRowDisp(nCharDisp);
        } 
        writeWordsDisp("0. Back");
        clearTextRows(10);
        for(int i = 0; i < 10080; i++){
            if(mode == 0){
                for(int k = 0; k < 1439; k++){
                    Delay(391667);
                    func();
                    if(button == 11){
                        clearTextScreen();
                        clearPixelScreen();
                        loggingMenu(); 
                    }
                }
            }
            tempMeasure();
            Delay(391667);
            nLogTemp[i] = nTempC;
            func();
            if(button == 11){
                clearTextScreen();
                clearPixelScreen();
                loggingMenu(); 
            }
            if(i < 1440){
                if(i != 0){
                    if(nLogTemp[i] < minTemp){
                        minTemp = nLogTemp[i];
                    }
                    if(nLogTemp[i] > maxTemp){
                        maxTemp = nLogTemp[i]; 
                    }
                    temp = temp + nLogTemp[i];
                }
                else{
                    tempMeasure();
                    tempMeasure();
                    nLogTemp[i] = nTempC;
                    if(nLogTemp[i] < minTemp){
                        minTemp = nLogTemp[i];
                    }
                    if(nLogTemp[i] > maxTemp){
                        maxTemp = nLogTemp[i]; 
                    }
                    temp = temp + nLogTemp[i];
                }
                if(i == 1439){
                    writeWordsDisp("Min:");
                    writeNumberDisp(minTemp);
                    writeWordsDisp(" Max:");
                    writeNumberDisp(maxTemp);
                    writeWordsDisp(" Av:");
                    float a = temp / 1439;
                    writeNumberDisp(a);
                    //writeNewRowDisp(nCharDisp);
                }
            }
            else if(i >= 1440 && i < 2880 ){
              if(i == 1440){
                  minTemp = FLT_MAX;
                  maxTemp = FLT_MIN;
                  temp = 0;
              }
                //if(nTempC > 0 && nTempC< 100){
                    if(nLogTemp[i] < minTemp){
                        minTemp = nLogTemp[i];
                    }
                    if(nLogTemp[i] > maxTemp){
                        maxTemp = nLogTemp[i]; 
                    }
                    temp = temp + nLogTemp[i];
                //}
                //else{
                    //i--; 
                //}
                if(i == 2879){
                    writeWordsDisp("Min:");
                    writeNumberDisp(minTemp);
                    writeWordsDisp(" Max:");
                    writeNumberDisp(maxTemp);
                    writeWordsDisp(" Av:");
                    float a = temp / 1439;
                    writeNumberDisp(a);
                    //writeNewRowDisp(nCharDisp);
                }
            }
            else if(i >= 2880 && i < 4320){
                if(i == 2880){
                    minTemp = FLT_MAX;
                    maxTemp = FLT_MIN;
                    temp = 0;
                } 
                //if(nTempC > 0 && nTempC< 100){
                    if(nLogTemp[i] < minTemp){
                        minTemp = nLogTemp[i];
                    }
                    if(nLogTemp[i] > maxTemp){
                        maxTemp = nLogTemp[i]; 
                    }
                    temp = temp + nLogTemp[i];
                //}
                //else{
                  //  i--; 
                //}
                if(i == 4319){
                    writeWordsDisp("Min:");
                    writeNumberDisp(minTemp);
                    writeWordsDisp(" Max:");
                    writeNumberDisp(maxTemp);
                    writeWordsDisp(" Av:");
                    float a = temp / 1439;
                    writeNumberDisp(a);
                    //writeNewRowDisp(nCharDisp);
                }
            }
            else if(i >= 4320 && i < 5760){
                if(i == 4320){
                    minTemp = FLT_MAX;
                    maxTemp = FLT_MIN;
                    temp = 0;
                }
                //if(nTempC > 0 && nTempC< 100){
                    if(nLogTemp[i] < minTemp){
                        minTemp = nLogTemp[i];
                    }
                    if(nLogTemp[i] > maxTemp){
                        maxTemp = nLogTemp[i]; 
                    }
                    temp = temp + nLogTemp[i];
                //}
                //else{
                 //   i--; 
                //}
                if(i == 5759){
                    writeWordsDisp("Min:");
                    writeNumberDisp(minTemp);
                    writeWordsDisp(" Max:");
                    writeNumberDisp(maxTemp);
                    writeWordsDisp(" Av:");
                    float a = temp / 1439;
                    writeNumberDisp(a);
                    //writeNewRowDisp(nCharDisp);
                }
            }
            else if(i >= 5760 && i < 7200){
                if(i == 5760){
                    minTemp = FLT_MAX;
                    maxTemp = FLT_MIN;
                    temp = 0;
                }
                //if(nTempC > 0 && nTempC< 100){
                    if(nLogTemp[i] < minTemp){
                        minTemp = nLogTemp[i];
                    }
                    if(nLogTemp[i] > maxTemp){
                        maxTemp = nLogTemp[i]; 
                    }
                    temp = temp + nLogTemp[i];
                //}
                //else{
                  //  i--; 
                //}
                if(i == 7199){
                    writeWordsDisp("Min:");
                    writeNumberDisp(minTemp);
                    writeWordsDisp(" Max:");
                    writeNumberDisp(maxTemp);
                    writeWordsDisp(" Av:");
                    float a = temp / 1439;
                    writeNumberDisp(a);
                    //writeNewRowDisp(nCharDisp);
                }
            }
            else if(i >= 7200 && i < 8640){
                if(i == 7200){
                    minTemp = FLT_MAX;
                    maxTemp = FLT_MIN;
                    temp = 0;
                }
                //if(nTempC > 0 && nTempC< 100){
                    if(nLogTemp[i] < minTemp){
                        minTemp = nLogTemp[i];
                    }
                    if(nLogTemp[i] > maxTemp){
                        maxTemp = nLogTemp[i]; 
                    }
                    temp = temp + nLogTemp[i];
                //}
                //else{
                  //  i--; 
                //}
                if(i == 8639){
                    writeWordsDisp("Min:");
                    writeNumberDisp(minTemp);
                    writeWordsDisp(" Max:");
                    writeNumberDisp(maxTemp);
                    writeWordsDisp(" Av:");
                    float a = temp / 1439;
                    writeNumberDisp(a);
                    //writeNewRowDisp(nCharDisp);
                }
            }
            else if(i >= 8640 && i < 10080){
                if(i == 8640){
                    minTemp = FLT_MAX;
                    maxTemp = FLT_MIN;
                    temp = 0;
                }
                //if(nTempC > 0 && nTempC< 100){
                    if(nLogTemp[i] < minTemp){
                        minTemp = nLogTemp[i];
                    }
                    if(nLogTemp[i] > maxTemp){
                        maxTemp = nLogTemp[i]; 
                    }
                    temp = temp + nLogTemp[i];
                //}
                //else{
                 //   i--; 
                //}
                if(i == 10079){
                    writeWordsDisp("Min:");
                    writeNumberDisp(minTemp);
                    writeWordsDisp(" Max:");
                    writeNumberDisp(maxTemp);
                    writeWordsDisp(" Av:");
                    float a = temp / 1439;
                    writeNumberDisp(a);
                    Delay(50000000);
                    clearTextScreen();
                    clearPixelScreen();
                }
            }   
        }
    }
}