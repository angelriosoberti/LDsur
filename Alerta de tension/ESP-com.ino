#include <ESP8266WiFi.h>
#include "Gsender.h"
#pragma region Globals

#define D0 16 //GPIO16 - WAKE UP
#define D1 5  //GPIO5
#define D2 4  //GPIO4
#define D3 0  //GPIO0
#define D4 2  //GPIO2 - TXD1
#define D5 14 //GPIO14 - HSCLK
#define D6 12 //GPIO12 - HMISO
#define D7 13 //GPIO13 - HMOSI - RXD2
#define D8 15 //GPIO15 - HCS   - TXD2
#define RX 3  //GPIO3 - RXD0
#define TX 1  //GPIO1 - TXD0

const int LedVerificacion = D4;
const int SensorMagnetico = D1;
int EstadoSenMagnetico = 0;

const char *ssid = "MOVISTAR_C162";            //MyRedSegura
const char *password = "T5U47we8N386eSb394V8"; //123456789
// cambiar a boolean  no un byte
uint8_t connection_state = 0;        // Determina el estado de la conexion
uint16_t reconnect_interval = 10000; // Si no se conecta espera este tiempo para volver a intentar
#pragma endregion Globals
String TramaMensajeGmail = "";

String Cerreos[4] = {"lvillarr@luzdelsur.com.pe", "villarrealquinto@hotmail.com", "angelica_sanchez75@hotmail.com", "francevillarreal2000@outlook.es"};
byte CorreosParaEnviar = 4;

byte Var;
//String Cerreo2 = "alexdiaz_1493@hotmail.com";

boolean Estado = true;
// cambiar funcion
uint8_t WiFiConnect(const char *nSSID = nullptr, const char *nPassword = nullptr)
{
  static uint16_t attempt = 0;
  Serial.print("Conectando a ");
  if (nSSID)
  {
    WiFi.begin(nSSID, nPassword);
    Serial.println(nSSID);
  }
  else
  {
    WiFi.begin(ssid, password);
    Serial.println(ssid);
  }

  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 50)
  {
    delay(200);
    Serial.print(".");
  }
  ++attempt;
  Serial.println("");
  if (i == 51)
  {
    Serial.print("Conexion: supero el limite de tiempo TIMEOUT: ");
    Serial.println(attempt);
    if (attempt % 2 == 0)
      Serial.println("Verifique si el Access Point esta disponible or verifique el SSID y el Password\r\n");
    return false;
  }

  Serial.println("Conexion: ESTABLECIDA");
  Serial.print("Direccion IP Leida: ");
  Serial.println(WiFi.localIP());
  return true;
}

void Awaits()
{
  // mide tiempo 
  uint32_t ts = millis();
  // mientras que no haya conexion  
  while (!connection_state)
  {
    delay(50);
    // si el tiempo de espera supera los 10 segundos  + ts  y no hay conexion 
    if (millis() > (ts + reconnect_interval) && !connection_state)
    {
      connection_state = WiFiConnect();
      ts = millis();
    }
  }
}

void setup()
{
  pinMode(SensorMagnetico, INPUT_PULLUP);
  pinMode(LedVerificacion, OUTPUT);
  digitalWrite(LedVerificacion, LOW);

  Serial.begin(9600);

  connection_state = WiFiConnect();
  delay(1000);
}

void loop()
{
  EstadoSenMagnetico = digitalRead(SensorMagnetico);

  if (EstadoSenMagnetico == LOW)
  {
    if (Estado == true)
    {
      Estado = false;
      digitalWrite(LedVerificacion, HIGH);
      EnviarMensajeGMAIL();
      delay(10000);
      digitalWrite(LedVerificacion, LOW);
    }
  }
  else
  {
    if (Estado == false)
    {
      Estado = true;
      digitalWrite(LedVerificacion, HIGH);
      EnviarMensajeGMAIL();
      digitalWrite(LedVerificacion, LOW);
      delay(10000);
    }
  }
}

void EnviarMensajeGMAIL(void)
{
  for (Var = 0; Var < CorreosParaEnviar; Var++)
  {
    TramaMensajeGmail = "";
    if (!connection_state) // if not connected to WIFI
      Awaits();            // constantly trying to connect

    Gsender *gsender = Gsender::Instance(); // Getting pointer to class instance

    String subject = "EQUIPO REGISTRADOR DE TENSIONES";

    TramaMensajeGmail += "<html>";
    TramaMensajeGmail += "<body>";

    TramaMensajeGmail += "<b>ESTIMADOS</b>";
    TramaMensajeGmail += "<br>";
    TramaMensajeGmail += "<p>";
    if (Estado == false)
    {
      TramaMensajeGmail += "<b>Se informa que el suministro 1337531 Cliente LUZ DEL SUR S.A.A. Dirección PEDRO MIOTTA 400 SAN JUAN DE MIRAFLORES está SIN SERVICIO</b>.";
    }
    else
    {
      TramaMensajeGmail += "<b>Se informa que el suministro 1337531 Cliente LUZ DEL SUR S.A.A. Dirección PEDRO MIOTTA 400 SAN JUAN DE MIRAFLORES está CON SERVICIO</b>.";
    }

    TramaMensajeGmail += "</p>";
    TramaMensajeGmail += "</body>";
    TramaMensajeGmail += "</html>";

    if (gsender->Subject(subject)->Send(Cerreos[Var], TramaMensajeGmail))
    {
      Serial.println("MENSAJE ENVIADO EXITOSAMENTE");
      delay(5000);
    }
    else
    {
      digitalWrite(LedVerificacion, LOW);
      Serial.print("ERROR AL ENVIAR EL MENSAJE: ");
      Serial.println(gsender->getError());
      delay(5000);
    }
  }
}
