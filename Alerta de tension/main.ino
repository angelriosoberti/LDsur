/*Observaciones: 
   - No se indica que linea cae.
   - Hacer una funcion exclusiva para dibujado en la pantalla.
   - Intentar hacer el loop mas ligero.
*/




////////////////////////////////////// Pantalla /////////////////////////////////////////
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

#define _sclk 13
#define _miso 12
#define _mosi 11
#define _cs 10
#define _dc 4
#define _rst 5
Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);
/////////////////////////////////////////////////////////////////////////////////////////

#define Pulsador 6
#define PinESP 3

////////////////////////////////////// GSM /////////////////////////////////////////
#include <SoftwareSerial.h>
SoftwareSerial GSMSerial(7, 8);

#define PinPower 9
#define PinStatus A5 // Pin coneccion a el modulo GSM 


String CLK = "AT+CCLK?";
String CCLK = "+CCLK: \"21/01/29,06:40:41+00\"";
char HoraReporte[] = "16:30";
boolean Reporte = true;
char Numero[] = "944008963";
String Mensaje1 = {"El Sistema a sufrido una caida de tension "};
String Mensaje2 = {"El Sistema se a restablecido "};
String Mensaje3 = {"Suministro electrico Estable"};

String FechaHora = "";
byte Posicion_temporal = 0;
char Temporal[90];
char Caracter;
byte CRLF = 0;
/////////////////////////////////////////////////////////////////////////////////////////

int RS = 220;
int RT = 220;
int ST = 220;

boolean Estado = true;
int ValorMin = 110;

byte TimeTesteo = 3;
byte Var;
byte Var2;

void pantallaInicio(){
  tft.begin();
  tft.fillScreen(ILI9340_BLACK);
  unsigned long start = micros();
  tft.setCursor(20, 18);
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(3);
  tft.println("Registrador");
  tft.setCursor(10, 50);
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(3);
  tft.println("de Tensiones");
  tft.setCursor(10, 100);
  tft.setTextColor(ILI9340_BLUE);
  tft.setTextSize(3);
  tft.print("R-S = ");
  tft.print(RS);
  tft.println(" V");

  tft.setCursor(10, 140);
  tft.setTextColor(ILI9340_YELLOW);
  tft.setTextSize(3);
  tft.print("R-T = ");
  tft.print(RT);
  tft.println(" V");

  tft.setCursor(10, 180);
  tft.setTextColor(ILI9340_RED);
  tft.setTextSize(3);
  tft.print("S-T = ");
  tft.print(ST);
  tft.println(" V");

  tft.setCursor(25, 230);
  tft.setTextColor(ILI9340_MAGENTA);
  tft.setTextSize(3);
  tft.println("Suministro");
  tft.setCursor(10, 262);
  tft.println("con Servicio");

  tft.setCursor(95, 295);
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(3);
  tft.println("DOC");
}

void setup()
{
  Serial.begin(9600);
  // delay(50);
  Serial.flush();
  // delay(50);
  pantallaInicio();
  pinMode(Pulsador, INPUT_PULLUP);
  pinMode(PinPower, OUTPUT);
  pinMode(PinESP, OUTPUT);
  pinMode(PinPower, OUTPUT);

  digitalWrite(PinPower, LOW);
  digitalWrite(PinESP, HIGH);
  digitalWrite(PinPower, LOW);
  //que hace?
  RevisaEstado();

  Serial.println("Listo");
  LimpiaCaptura();
  //comunicacion con modem por comandos AT
  GSMSerial.println("AT");

  CapturaRespuesta();
  Serial.print("La Respuesta es: "); //Serial.println(Posicion_temporal);
  Serial.println(Temporal);
  LimpiaCaptura();
}

void loop()
{ /*
  if (digitalRead(Pulsador) == LOW)
  {
    Serial.println("AT+CCLK?");
    GSMSerial.println("AT+CCLK?");
    CapturaRespuesta();
    Serial.print("La Respuesta es: ");//Serial.println(Posicion_temporal);
    Serial.println(Temporal);  
    ComparaHoraReporte();
    EstraeFechaHora();
    LimpiaCaptura();
    delay(1000);
  }*/

  for (Var = 0; Var < TimeTesteo; Var++)
  {
    for (Var2 = 0; Var2 < 100; Var2++)
    {
      delay(10);
      if (GSMSerial.available() > 0)
      {
        CapturaRespuesta2();
        Serial.print("La Respuesta 2 es: ");
        Serial.println(Temporal);
        //Serial.println(Posicion_temporal);
        ComparaMensaje();
        LimpiaCaptura();
      }
    }
  }

  //delay(1000);
// conexiones para monitoreo de voltaje 
  RS = analogRead(A0);
  RT = analogRead(A1);
  ST = analogRead(A2);
// conversion lineal 
  RS = map(RS, 0, 1023, 0, 220);
  RT = map(RT, 0, 1023, 0, 220);
  ST = map(ST, 0, 1023, 0, 220);

  for (int Y = 115; Y < 170; Y++)
    tft.drawLine(Y, 100, Y, 203, ILI9340_BLACK);

  tft.setTextColor(ILI9340_BLUE);
  tft.setTextSize(3);
  tft.setCursor(115, 100);
  tft.print(RS);

  tft.setTextColor(ILI9340_YELLOW);
  tft.setTextSize(3);
  tft.setCursor(115, 140);
  tft.print(RT);

  tft.setTextColor(ILI9340_RED);
  tft.setTextSize(3);
  tft.setCursor(115, 180);
  tft.print(ST);

  Serial.print("RS:");
  Serial.println(RS);
  Serial.print("RT:");
  Serial.println(RT);
  Serial.print("ST:");
  Serial.println(ST);
  Serial.println();
// pregunta hora 
  GSMSerial.println("AT+CCLK?");
  // posible respuesta del SIM
  //+CCLK:"09/10/15,19:33:42+00" --> 15 de octubre del 2009 a las 7:22 pm
  CapturaRespuesta();
  Serial.print("La Respuesta es: "); //Serial.println(Posicion_temporal);
  Serial.println(Temporal);
  ComparaHoraReporte();
  LimpiaCaptura();
// si el estado es verdadero  
  if (Estado == true)
  {
    // ver si una de las lineas cae del valor minimo entonces
    if (RS < ValorMin || RT < ValorMin || ST < ValorMin)
    {
      for (int X = 262; X < 290; X++)
        tft.drawLine(10, X, 70, X, ILI9340_BLACK);
      tft.setCursor(10, 262);
      tft.setTextColor(ILI9340_MAGENTA);
      tft.println("Sin");
// mandar mensaje indicando  pero no dice q linea!!!
      GSMSerial.println("AT+CCLK?");
      CapturaRespuesta();
      EstraeFechaHora();
      LimpiaCaptura();
      MensajeCaida();
      //mandar señal para que mande el correo
      digitalWrite(PinESP, LOW);
      //analogWrite(PinESP,0);
// pasar el estado a falso 
      Estado = false;
    }
  }
  else // sino  
  {
// ver si una de las lineas esta por debajo del valor minimo     
    if (RS > ValorMin && RT > ValorMin && ST > ValorMin)
    {
      for (int X = 262; X < 290; X++)
        tft.drawLine(10, X, 70, X, ILI9340_BLACK);
      tft.setCursor(10, 262);
      tft.setTextColor(ILI9340_MAGENTA);

      tft.println("con");

      GSMSerial.println("AT+CCLK?");
      
      CapturaRespuesta();
      EstraeFechaHora();
      LimpiaCaptura();
      MensajeRestablecido();
      digitalWrite(PinESP, HIGH);
      // analogWrite(PinESP,16 0);
      Estado = true;
    }
  }
}


// manda mensaje de texto sms a el numero almacenado en la var Numero + la variable FechaHora
void MensajeCaida()
{
  Serial.println("Enviando Mensaje Caida");
  GSMSerial.print("AT+CMGS=");
  GSMSerial.print((char)34);
  GSMSerial.print(Numero);
  GSMSerial.println((char)34);
  delay(500);

  GSMSerial.println(Mensaje1 + FechaHora);
  GSMSerial.println((char)26);
  delay(10000);
}
void MensajeRestablecido()
{
  Serial.println("Enviando Mensaje Restablecido");
  GSMSerial.print("AT+CMGS=");
  GSMSerial.print((char)34);
  GSMSerial.print(Numero);
  GSMSerial.println((char)34);
  delay(500);
  GSMSerial.println(Mensaje2 + FechaHora);
  GSMSerial.println((char)26);
  delay(10000);
}

//Almacena la comunicacion enviada del GSM en el array Temporal[]
void CapturaRespuesta()
{
  //Serial.println("Inicio de Captura");
  Posicion_temporal = 0;
  CRLF = 0;
  //mientras CRLF no sea 4 
  while (CRLF != 4)
  {
  // cuando  GSMSerial.available sea true   
    if (GSMSerial.available() > 0)
    {
  // Guardar lo leido en GSMSerial  en la var caracter ( la cual es un char)    
      Caracter = GSMSerial.read();
  // si caracter toma el valor de  13  OR  10 enteonces CRLF aumenta en 1 
      if (Caracter == 10 || Caracter == 13)
        CRLF++;
  // sino guardar el valor de Caracter en el array Temporal en la posicion  de  Posicion_temporal     
      else
        Temporal[Posicion_temporal++] = Caracter; //Serial.println(Caracter);
    }

    if (RevisaEstado() == true)
      return;
  }

  //Serial.println("Fin de Captura");
}
void CapturaRespuesta2()
{
  Posicion_temporal = 0;
  CRLF = 0;
  while (CRLF != 6)
  {
    if (GSMSerial.available() > 0)
    {
      Caracter = GSMSerial.read();
      if (Caracter == 13 || Caracter == 10)
        CRLF++;
      else
        Temporal[Posicion_temporal++] = Caracter; //Serial.println(Caracter);
    }
    if (RevisaEstado() == true)
      return;
  }
}
// si la hora de reporte coincide con hora guardad en Temporal[] entonces ve 
//si el reporte esta en verdadero y envia sms al var numero y con el mensaje 
//var Mensaje3 , sino cambia el reporte a verdadero
void ComparaHoraReporte()
{

  if (HoraReporte[0] == Temporal[17] && HoraReporte[1] == Temporal[18] && HoraReporte[3] == Temporal[20] && HoraReporte[4] == Temporal[21])
  {
    if (Reporte == true)
    {
      Serial.println("Enviando Mensaje 3");
      GSMSerial.print("AT+CMGS=");
      GSMSerial.print((char)34);
      GSMSerial.print(Numero);
      GSMSerial.println((char)34);
      delay(500);
      GSMSerial.println(Mensaje3);
      GSMSerial.println((char)26);
      delay(10000);
      Reporte = false;
    }
  }
  else
  {
    if (Reporte == false)
      Reporte = true;
  }
}
void EstraeFechaHora()
{
  FechaHora = "";
  for (Posicion_temporal = 8; Posicion_temporal < 25; Posicion_temporal++)
  {
    FechaHora += Temporal[Posicion_temporal];
  }
  //Serial.println(FechaHora);
}
// el for es para borrar toda la memoria de Temporal[], del serial de GSM y reinicia serial GSM
void LimpiaCaptura()
{
  // el for es para borrar toda la memoria de Temporal[]
  for (Posicion_temporal = 0; Posicion_temporal < 50; Posicion_temporal++)
  {
    Temporal[Posicion_temporal] = 0;
  }

  GSMSerial.flush();
  delay(50);
  GSMSerial.end();
  delay(50);
  GSMSerial.begin(9600);
  delay(50);
  GSMSerial.flush();
  Serial.println("Limpio");
}



void ComparaMensaje()
{
  if ('E' == Temporal[46] && 'S' == Temporal[47] && 'T' == Temporal[48] && 'A' == Temporal[49] && 'D' == Temporal[50] && 'O' == Temporal[51])
  {
    Serial.println("Enviando Respuesta");
    delay(5000);
    GSMSerial.print("AT+CMGS=");
    GSMSerial.print((char)34);
    GSMSerial.print(Numero);
    GSMSerial.println((char)34);
    delay(500);
    GSMSerial.print("RS=");
    GSMSerial.print(RS);
    GSMSerial.print("  RT=");
    GSMSerial.print(RT);
    GSMSerial.print("  ST=");
    GSMSerial.println(ST);
    GSMSerial.println((char)26);
    delay(5000);
  }
}
//que estado revisa ? alguna comunicacion del Sim ?=? 
boolean RevisaEstado()
{
  //Mietras señal en Pin A5 es menor a 300
  while (analogRead(PinStatus) < 300)
  {
    // imprimir texto de confirmacion 
    Serial.println("Encendiendo GSM");
    //Encender el GSM por 1 segundo 
    digitalWrite(PinPower, HIGH);
    delay(1000);
    // apagar el GSM por 10 segundos
    digitalWrite(PinPower, LOW);
    delay(10000);
    // limpia el bus de comunicacion con el GSM 
    LimpiaCaptura();
    // retorna verdadero
    return true;
  }
  // si la señal en el  pin a5 es mayor a  300 retorna falso 
  return false;
}
