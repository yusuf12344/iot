#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define ONE_WIRE_BUS 11
#define FIS_TYPE float
#define FIS_RESOLUSION 101
#define FIS_MIN -3.4028235E+38
#define FIS_MAX 3.4028235E+38
typedef FIS_TYPE(*_FIS_MF)(FIS_TYPE, FIS_TYPE*);
typedef FIS_TYPE(*_FIS_ARR_OP)(FIS_TYPE, FIS_TYPE);
typedef FIS_TYPE(*_FIS_ARR)(FIS_TYPE*, int, _FIS_ARR_OP);

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

//deklare variabel output fuzzy
float ys;

//deklare relay
int rlyPm = 2;
int rlyPk = 3;
int rlyHt = 4;
int rlyKp = 5;
int rlyPu = 6;
int rlyPd = 7;

//deklare ultrasonik
int echo = 10;
int trig = 9;
long durasi, jarak;

//deklarasi persentase tinggi air
  int percentage;

//deklare kekeruhan
int sensorPin = A1;
float volt;
float ntu;
float teg,kekeruhan;
double kekeruhanFix;

//deklare ph
const int ph_Pin  = A0;
float Po = 0;
float PH_step;
int nilai_analog_PH;
double TeganganPh;

//untuk kalibrasi
float PH4 = 3.226;
float PH7 = 2.691;
float suhu;

// Number of inputs to the fuzzy inference system
const int fis_gcI = 2;
// Number of outputs to the fuzzy inference system
const int fis_gcO = 1;
// Number of rules to the fuzzy inference system
const int fis_gcR = 9;

FIS_TYPE g_fisInput[fis_gcI];
FIS_TYPE g_fisOutput[fis_gcO];

// Setup routine runs once when you press reset:
void setup()
{   
    Serial.begin(9600);
    lcd.begin();
    
    //triger&echo
    pinMode(trig, OUTPUT);    
    pinMode(echo, INPUT);
    
    // initialize the Analog pins for input.
    // Pin mode for Input: pH
    pinMode(0 , INPUT);
    // Pin mode for Input: kekeruhan
    pinMode(1 , INPUT);

    // initialize the Analog pins for output.
    // Pin mode for Output: pengurasan
    pinMode(rlyPm , OUTPUT);  
    pinMode(rlyPk , OUTPUT);
    pinMode(rlyKp , OUTPUT);
    pinMode(rlyHt , OUTPUT);
    pinMode(rlyPu , OUTPUT);
    pinMode(rlyPd , OUTPUT);
}

// Loop routine runs over and over again forever:
void loop()
{
//relay start off  
    digitalWrite(rlyPm , HIGH);  
    digitalWrite(rlyPk , HIGH);
    digitalWrite(rlyKp , HIGH);
    digitalWrite(rlyHt , HIGH);
    digitalWrite(rlyPu , HIGH);
    digitalWrite(rlyPd , HIGH);
  
//suhu
    suhu_ukur();
//    Serial.println(" Nilai suhu : "+ String(suhu)); 
//    Serial.println("==============");
    
        
//ph
    nilai_analog_PH = analogRead(ph_Pin);
  //  Serial.print("Nilai ADC Ph: ");
  //  Serial.println(nilai_analog_PH);
      TeganganPh = 3.3 / 1024.0 * nilai_analog_PH;
  //  Serial.print("TeganganPh: ");
  //  Serial.println(TeganganPh, 3);
  
    PH_step = (PH4 - PH7) / 3;
    Po = 7.00 + ((PH7 - TeganganPh) / PH_step);     //Po = 7.00 + ((teganganPh7 - TeganganPh) / PhStep);
//    Serial.print("Nilai PH cairan: ");
//    Serial.println(Po, 2);
//    Serial.println("==============");

 //kekeruhan
    double kekeruhan = 0;
    double kekeruhanTotal = 0;
  
    for (int i = 0; i <= 120; i++) {
      int val = analogRead(A1);
      teg = val*(5.0/1024);
      kekeruhan = 500.00-(teg/3.99)*500.00;
      kekeruhanTotal = kekeruhanTotal + kekeruhan;
     }
     kekeruhanFix = kekeruhanTotal/120; 

//sensor ultrasonik
    ultrasonik();

//    demo testing
//    double Po1 = 7;
//    double kekeruhanFix1 = 50;
//    double suhu1 = 24;
//    long jarak1 = 2;
//
    
    // Read Input: pH
    g_fisInput[0] = Po ;
    // Read Input: kekeruhan
    g_fisInput[1] = kekeruhanFix ;

    g_fisOutput[0] = 0;

    fis_evaluate();

    // Set output vlaue: pengurasan
    ys = g_fisOutput[0];

//     Serial.println("==================");
//     Serial.println(ys);
      Serial.print("ph=");
      Serial.print(String(Po));
      Serial.print("&");
      Serial.print("suhu=");
      Serial.print(String(suhu));
      Serial.print("&");
      Serial.print("kekeruhan=");
      Serial.print(String(kekeruhanFix));
      Serial.print("&");
      Serial.print("tinggi=");
      Serial.print(String(percentage));

      tampilan();
      
//logic fuzzy kekeruhan & pH
    if (ys >= 0 && ys < 30) { // if either x or y is greater than zero (0-29)
//      Serial.println("pengurasan 0%");
      lcd.setCursor(0,0);
      lcd.print("Kuras:0%");
      delay(10000);
    }
    else if (ys >= 30 && ys < 50) { // if either x or y is greater than zero (30-49)
//      Serial.println("pengurasan 30%");

      while(jarak < 8){
        digitalWrite(rlyPk, LOW); 
        lcd.setCursor(0,0);
        lcd.print("Kuras:30%");
        lcd.setCursor(0,1);
        lcd.print("T.Air:" + String(percentage) +"%");
        Serial.print("ph=");
        Serial.print(String(Po));
        Serial.print("&");
        Serial.print("suhu=");
        Serial.print(String(suhu));
        Serial.print("&");
        Serial.print("kekeruhan=");
        Serial.print(String(kekeruhanFix));
        Serial.print("&");
        Serial.print("tinggi=");
        Serial.print(String(percentage));
        ultrasonik();
        delay(2000);
       }
                
      if (jarak == 8) {
        lcd.setCursor(0,0);
        lcd.print("Kuras:30%"); 
  
        digitalWrite(rlyPk, HIGH);
        delay(5000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Kuras sukses");

        delay(5000);
        digitalWrite(rlyPm, LOW);
        lcd.setCursor(0,1);
        lcd.print("T.Air:" + String(percentage) +"%");
       }
        
        
      while(jarak > 2){
        ultrasonik();
        lcd.setCursor(0,1);
        lcd.print("T.Air:" + String(percentage) +"%");
        Serial.print("ph=");
        Serial.print(String(Po));
        Serial.print("&");
        Serial.print("suhu=");
        Serial.print(String(suhu));
        Serial.print("&");
        Serial.print("kekeruhan=");
        Serial.print(String(kekeruhanFix));
        Serial.print("&");
        Serial.print("tinggi=");
        Serial.print(String(percentage));
        delay(2000);
      }
      if (jarak == 2) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Pengisian sukses");
//        Serial.println("air penuh");
        digitalWrite(rlyPm, HIGH);
        delay(10000);
      }
    }
    else { // if either x or y is greater than zero

      while(jarak < 12){
        digitalWrite(rlyPk, LOW); 
        lcd.setCursor(0,0);
        lcd.print("Kuras:50%");
        lcd.setCursor(0,1);
        lcd.print("T.Air:" + String(percentage) +"%");
        Serial.print("ph=");
        Serial.print(String(Po));
        Serial.print("&");
        Serial.print("suhu=");
        Serial.print(String(suhu));
        Serial.print("&");
        Serial.print("kekeruhan=");
        Serial.print(String(kekeruhanFix));
        Serial.print("&");
        Serial.print("tinggi=");
        Serial.print(String(percentage));
        ultrasonik();
        delay(2000);
        }
                
        if (jarak == 12) {
        lcd.setCursor(0,0);
        lcd.print("Kuras:50%"); 
//        Serial.println("pengurasan 50% selesai");
        digitalWrite(rlyPk, HIGH);
        delay(5000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Kuras sukses");
        delay(5000);
        digitalWrite(rlyPm, LOW);
        lcd.setCursor(0,1);
        lcd.print("T.Air:" + String(percentage) +"%");
       }
        
        
      while(jarak > 2){
        ultrasonik();
        lcd.setCursor(0,1);
        lcd.print("T.Air:" + String(percentage) +"%");
        Serial.print("ph=");
        Serial.print(String(Po));
        Serial.print("&");
        Serial.print("suhu=");
        Serial.print(String(suhu));
        Serial.print("&");
        Serial.print("kekeruhan=");
        Serial.print(String(kekeruhanFix));
        Serial.print("&");
        Serial.print("tinggi=");
        Serial.print(String(percentage));
        delay(2000);
      }
      if (jarak == 2) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Pengisian sukses");
//        Serial.println("air penuh");
        digitalWrite(rlyPm, HIGH);
        delay(10000);
      }
    }

      
//logic suhu
    if(suhu >= 0 && suhu < 23) { // if either x or y is greater than zero
//      Serial.println("dingin heater nyala");
      while(suhu < 23) {
        lcd.setCursor(0,0);
        lcd.print("Heater:On");
        lcd.setCursor(0,1);
        lcd.print("Suhu:"+String(suhu)+" C");
        Serial.print("ph=");
        Serial.print(String(Po));
        Serial.print("&");
        Serial.print("suhu=");
        Serial.print(String(suhu));
        Serial.print("&");
        Serial.print("kekeruhan=");
        Serial.print(String(kekeruhanFix));
        Serial.print("&");
        Serial.print("tinggi=");
        Serial.print(String(percentage));
        digitalWrite(rlyHt, LOW);
        delay (5000);
        tampilan();
        suhu_ukur();
         }
    }
    else if(suhu >= 23 && suhu < 27) { // if either x or y is greater than zero
//      Serial.println("optimal mati semua");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Suhu optimal");
      lcd.setCursor(0,1);
      lcd.print("Suhu:"+String(suhu)+" C");
      delay (10000);
      tampilan();
    }
    else { // if either x or y is greater than zero
//      Serial.println("panas kipas nyala");
      while(suhu >= 27) {
        lcd.setCursor(0,0);
        lcd.print("Kipas:On");
        lcd.setCursor(0,1);
        lcd.print("Suhu:"+String(suhu)+" C");
        Serial.print("ph=");
        Serial.print(String(Po));
        Serial.print("&");
        Serial.print("suhu=");
        Serial.print(String(suhu));
        Serial.print("&");
        Serial.print("kekeruhan=");
        Serial.print(String(kekeruhanFix));
        Serial.print("&");
        Serial.print("tinggi=");
        Serial.print(String(percentage));
        digitalWrite(rlyKp, LOW);
        delay(10000);
        tampilan(); 
        suhu_ukur();
      
        }
        
    }
  
}

// class ultrasonik
void ultrasonik(){
   // ultrasonik
     digitalWrite(trig, LOW);
     delayMicroseconds(7);
     digitalWrite(trig, HIGH);
     delayMicroseconds(7);
     digitalWrite(trig, LOW);
     delayMicroseconds(7);
     durasi = pulseIn(echo, HIGH); 
     jarak = (durasi / 2) / 29.1;
     percentage=map(jarak, 23, 2, 0, 100);
     if(percentage<0){
        percentage=0;
      }
     else if(percentage>100){
         percentage=100;
      }
  }

//class suhu
void suhu_ukur(){
  sensors.requestTemperatures(); 
  suhu = sensors.getTempCByIndex(0);
  } 

//class tampilan
void tampilan(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Suhu:"+String(suhu)+" C");
  lcd.setCursor(0,1);
  lcd.print("NTU:"+String(kekeruhanFix));
  delay (10000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T.Air:" + String(percentage) +"%");
  lcd.setCursor(0,1);
  lcd.print("pH:" + String(Po));
  delay(10000);
  lcd.clear();
  }

//***********************************************************************
// Support functions for Fuzzy Inference System                          
//***********************************************************************
// Triangular Member Function
FIS_TYPE fis_trimf(FIS_TYPE x, FIS_TYPE* p)
{
    FIS_TYPE a = p[0], b = p[1], c = p[2];
    FIS_TYPE t1 = (x - a) / (b - a);
    FIS_TYPE t2 = (c - x) / (c - b);
    if ((a == b) && (b == c)) return (FIS_TYPE) (x == a);
    if (a == b) return (FIS_TYPE) (t2*(b <= x)*(x <= c));
    if (b == c) return (FIS_TYPE) (t1*(a <= x)*(x <= b));
    t1 = min(t1, t2);
    return (FIS_TYPE) max(t1, 0);
}

// Trapezoidal Member Function
FIS_TYPE fis_trapmf(FIS_TYPE x, FIS_TYPE* p)
{
    FIS_TYPE a = p[0], b = p[1], c = p[2], d = p[3];
    FIS_TYPE t1 = ((x <= c) ? 1 : ((d < x) ? 0 : ((c != d) ? ((d - x) / (d - c)) : 0)));
    FIS_TYPE t2 = ((b <= x) ? 1 : ((x < a) ? 0 : ((a != b) ? ((x - a) / (b - a)) : 0)));
    return (FIS_TYPE) min(t1, t2);
}

FIS_TYPE fis_min(FIS_TYPE a, FIS_TYPE b)
{
    return min(a, b);
}

FIS_TYPE fis_max(FIS_TYPE a, FIS_TYPE b)
{
    return max(a, b);
}

FIS_TYPE fis_prod(FIS_TYPE a, FIS_TYPE b)
{
    return (a * b);
}

FIS_TYPE fis_sum(FIS_TYPE a, FIS_TYPE b)
{
    return (a + b);
}

FIS_TYPE fis_array_operation(FIS_TYPE *array, int size, _FIS_ARR_OP pfnOp)
{
    int i;
    FIS_TYPE ret = 0;

    if (size == 0) return ret;
    if (size == 1) return array[0];

    ret = array[0]; 
    for (i = 1; i < size; i++)
    {
        ret = (*pfnOp)(ret, array[i]);
    }

    return ret;
}


//***********************************************************************
// Data for Fuzzy Inference System                                       
//***********************************************************************
// Pointers to the implementations of member functions
_FIS_MF fis_gMF[] =
{
    fis_trimf, fis_trapmf
};

// Count of member function for each Input
int fis_gIMFCount[] = { 3, 3 };

// Count of member function for each Output 
int fis_gOMFCount[] = { 3 };

// Coefficients for the Input Member Functions
FIS_TYPE fis_gMFI0Coeff1[] = { 0, 6.5, 7.5 };
FIS_TYPE fis_gMFI0Coeff2[] = { 6.5, 7.5, 8.5 };
FIS_TYPE fis_gMFI0Coeff3[] = { 7.5, 8.5, 14 };
FIS_TYPE* fis_gMFI0Coeff[] = { fis_gMFI0Coeff1, fis_gMFI0Coeff2, fis_gMFI0Coeff3 };
FIS_TYPE fis_gMFI1Coeff1[] = { 0, 0, 200, 250 };
FIS_TYPE fis_gMFI1Coeff2[] = { 200, 250, 400, 450 };
FIS_TYPE fis_gMFI1Coeff3[] = { 400, 450, 650, 650 };
FIS_TYPE* fis_gMFI1Coeff[] = { fis_gMFI1Coeff1, fis_gMFI1Coeff2, fis_gMFI1Coeff3 };
FIS_TYPE** fis_gMFICoeff[] = { fis_gMFI0Coeff, fis_gMFI1Coeff };

// Coefficients for the Output Member Functions
FIS_TYPE fis_gMFO0Coeff1[] = { 0, 0, 0 };
FIS_TYPE fis_gMFO0Coeff2[] = { 0, 0, 30 };
FIS_TYPE fis_gMFO0Coeff3[] = { 0, 0, 50 };
FIS_TYPE* fis_gMFO0Coeff[] = { fis_gMFO0Coeff1, fis_gMFO0Coeff2, fis_gMFO0Coeff3 };
FIS_TYPE** fis_gMFOCoeff[] = { fis_gMFO0Coeff };

// Input membership function set
int fis_gMFI0[] = { 0, 0, 0 };
int fis_gMFI1[] = { 1, 1, 1 };
int* fis_gMFI[] = { fis_gMFI0, fis_gMFI1};

// Output membership function set

int* fis_gMFO[] = {};

// Rule Weights
FIS_TYPE fis_gRWeight[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };

// Rule Type
int fis_gRType[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };

// Rule Inputs
int fis_gRI0[] = { 1, 1 };
int fis_gRI1[] = { 1, 2 };
int fis_gRI2[] = { 1, 3 };
int fis_gRI3[] = { 2, 1 };
int fis_gRI4[] = { 2, 2 };
int fis_gRI5[] = { 2, 3 };
int fis_gRI6[] = { 3, 1 };
int fis_gRI7[] = { 3, 2 };
int fis_gRI8[] = { 3, 3 };
int* fis_gRI[] = { fis_gRI0, fis_gRI1, fis_gRI2, fis_gRI3, fis_gRI4, fis_gRI5, fis_gRI6, fis_gRI7, fis_gRI8 };

// Rule Outputs
int fis_gRO0[] = { 2 };
int fis_gRO1[] = { 2 };
int fis_gRO2[] = { 3 };
int fis_gRO3[] = { 1 };
int fis_gRO4[] = { 1 };
int fis_gRO5[] = { 2 };
int fis_gRO6[] = { 2 };
int fis_gRO7[] = { 2 };
int fis_gRO8[] = { 3 };
int* fis_gRO[] = { fis_gRO0, fis_gRO1, fis_gRO2, fis_gRO3, fis_gRO4, fis_gRO5, fis_gRO6, fis_gRO7, fis_gRO8 };

// Input range Min
FIS_TYPE fis_gIMin[] = { 0, 0 };

// Input range Max
FIS_TYPE fis_gIMax[] = { 14, 650 };

// Output range Min
FIS_TYPE fis_gOMin[] = { 0 };

// Output range Max
FIS_TYPE fis_gOMax[] = { 1 };

//***********************************************************************
// Data dependent support functions for Fuzzy Inference System           
//***********************************************************************
// None for Sugeno

//***********************************************************************
// Fuzzy Inference System                                                
//***********************************************************************
void fis_evaluate()
{
    FIS_TYPE fuzzyInput0[] = { 0, 0, 0 };
    FIS_TYPE fuzzyInput1[] = { 0, 0, 0 };
    FIS_TYPE* fuzzyInput[fis_gcI] = { fuzzyInput0, fuzzyInput1, };
    FIS_TYPE fuzzyOutput0[] = { 0, 0, 0 };
    FIS_TYPE* fuzzyOutput[fis_gcO] = { fuzzyOutput0, };
    FIS_TYPE fuzzyRules[fis_gcR] = { 0 };
    FIS_TYPE fuzzyFires[fis_gcR] = { 0 };
    FIS_TYPE* fuzzyRuleSet[] = { fuzzyRules, fuzzyFires };
    FIS_TYPE sW = 0;

    // Transforming input to fuzzy Input
    int i, j, r, o;
    for (i = 0; i < fis_gcI; ++i)
    {
        for (j = 0; j < fis_gIMFCount[i]; ++j)
        {
            fuzzyInput[i][j] =
                (fis_gMF[fis_gMFI[i][j]])(g_fisInput[i], fis_gMFICoeff[i][j]);
        }
    }

    int index = 0;
    for (r = 0; r < fis_gcR; ++r)
    {
        if (fis_gRType[r] == 1)
        {
            fuzzyFires[r] = FIS_MAX;
            for (i = 0; i < fis_gcI; ++i)
            {
                index = fis_gRI[r][i];
                if (index > 0)
                    fuzzyFires[r] = fis_min(fuzzyFires[r], fuzzyInput[i][index - 1]);
                else if (index < 0)
                    fuzzyFires[r] = fis_min(fuzzyFires[r], 1 - fuzzyInput[i][-index - 1]);
                else
                    fuzzyFires[r] = fis_min(fuzzyFires[r], 1);
            }
        }
        else
        {
            fuzzyFires[r] = FIS_MIN;
            for (i = 0; i < fis_gcI; ++i)
            {
                index = fis_gRI[r][i];
                if (index > 0)
                    fuzzyFires[r] = fis_max(fuzzyFires[r], fuzzyInput[i][index - 1]);
                else if (index < 0)
                    fuzzyFires[r] = fis_max(fuzzyFires[r], 1 - fuzzyInput[i][-index - 1]);
                else
                    fuzzyFires[r] = fis_max(fuzzyFires[r], 0);
            }
        }

        fuzzyFires[r] = fis_gRWeight[r] * fuzzyFires[r];
        sW += fuzzyFires[r];
    }

    if (sW == 0)
    {
        for (o = 0; o < fis_gcO; ++o)
        {
            g_fisOutput[o] = ((fis_gOMax[o] + fis_gOMin[o]) / 2);
        }
    }
    else
    {
        for (o = 0; o < fis_gcO; ++o)
        {
            FIS_TYPE sWI = 0.0;
            for (j = 0; j < fis_gOMFCount[o]; ++j)
            {
                fuzzyOutput[o][j] = fis_gMFOCoeff[o][j][fis_gcI];
                for (i = 0; i < fis_gcI; ++i)
                {
                    fuzzyOutput[o][j] += g_fisInput[i] * fis_gMFOCoeff[o][j][i];
                }
            }

            for (r = 0; r < fis_gcR; ++r)
            {
                index = fis_gRO[r][o] - 1;
                sWI += fuzzyFires[r] * fuzzyOutput[o][index];
            }

            g_fisOutput[o] = sWI / sW;
        }
    }
}
