// Programa : Teste sensor liquido Arduino
// Autor : LUCAS E LUIZ
#include <SPI.h>
#include <Ethernet.h>
#include <MemoryFree.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define TYPE_CONNECTION EEPROM.read(1) //0 -> CABO IP FIXO / 1 -> CABO DHCP / 2 ->WIFI DHCP
#define SIZE_SSID EEPROM.read(6) //começa no 300 e no 6 esta o tamanho
#define SIZE_PASSW EEPROM.read(7) //começa no 450 e no 7 esta o tamanho
//#define CLOUD_SERVER EEPROM.read(8)


//IP sendo lido (cada byte foi guardado na posição 0,1,2,3 da EEPROM)
byte ip[] = {EEPROM.read(2), EEPROM.read(3), EEPROM.read(4), EEPROM.read(5)}; //ip fixo fornecido pelo pessoal da infraestrutura de rede


//RX pino 2, TX pino 3
SoftwareSerial esp8266(2, 3);
#define DEBUG true

#define HTTP_PORT 80
#define SERVER_POST_COMMAND "lcasys.com"
#define TEMPO_CLOUD 60000
#define DEVICE_ID 6
char server_cloud[] = "lcasys.com";
EthernetClient client_cloud;
long tempo_cloud;
boolean flag_FirstConnectionSuccess = true;
#define TIMEOUT_DHCP 300000 //Tenta se conectar via DHCP por 5 min, antes de usar o ultimo IP valido

//IPAddress ip(192, 168, 25, 4);
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA};



EthernetClient client_interno;
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server_interno(HTTP_PORT);


#define TIMEOUT_CALIBRATE 5000
int nivel_liquido[8];
float nivel_liquido_medio = 0;
float nivel_atual = 0;
long tempo_sensores;
float cmMsec;
long microsec;

#define NIVEL_TOTAL 600 //MEDIDO EM CENTIMETROS
#define CHEIO NIVEL_TOTAL
#define VAZIO 0
#define TOLERANCE 0.05*NIVEL_TOTAL
#define RED_COLOR "#ff0000"
#define GREEN_COLOR "#00cc66"
#define REFRESH_PAGE_TIME 2
#define MAX_DISTANCE NIVEL_TOTAL //agua longe ao máximo do sensor, snedo esse maximo, o nivel maximo admitido pelo sistema no reservatorio
#define MIN_DISTANCE 2  //agua muito perto do sensor
#define TEMPO_SERIAL 1000
#define COMMAND_LEVEL "GET /level"
#define COMMAND_SENSOR_READ "GET /sensor"


//Carrega a biblioteca do sensor ultrassonico
#include <Ultrasonic.h>
//Define os pinos para o trigger e echo
#define pino_trigger 4
#define pino_echo 5
//Inicializa o sensor nos pinos definidos acima
Ultrasonic ultrasonic(pino_trigger, pino_echo);

void setup()
{
  pinMode(3, OUTPUT);
  pinMode(6, INPUT);
  analogWrite(3, 255);


  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    delay(10); // wait for serial port to connect. Needed for native USB port only
  }


  boolean program_mode = false;
  long time_program = 5000;
  String aux_comando_serial = "";
  String comando_serial = "";
  byte aux_serial = 0;
  long time_wait_serial = millis();

  Serial.println(F("DESEJA ENTRAR NO MODO DE CONFIGURACAO? 1 -> SIM / 0 -> NAO"));
  while (millis() - time_wait_serial < time_program) {
    delay(10);
    if (Serial.available()) { //verifica se chegou algum dado na serial
      aux_comando_serial = String(Serial.read() - '0'); //Lê o byte mais recente disponível na serial
      Serial.println(aux_comando_serial);
      if (aux_comando_serial.toInt() != -35) {
        comando_serial += aux_comando_serial;
        Serial.println(aux_comando_serial);
      }
      else {
        if (comando_serial.toInt() == 1) {
          program_mode = true;
        }
        break;
      }
    }
  }

  if (program_mode) {
    Serial.println(F("\n\nINICIO DO MODO DE PROGRAMACAO, TECLE ENTER AO FIM DE CADA COMANDO."));
    aux_comando_serial = "";
    comando_serial = "";
    Serial.println(F("QUAL TIPO DE CONEXAO? 1 -> CABO IP FIXO / 2 -> CABO DHCP / 3-> WIFI DHCP"));
    while (1) {
      delay(10);
      if (Serial.available()) { //verifica se chegou algum dado na serial
        aux_comando_serial = String(Serial.read() - '0'); //Lê o byte mais recente disponível na serial
        if (aux_comando_serial.toInt() != -35) {
          comando_serial += aux_comando_serial;
          Serial.print(aux_comando_serial);
        }
        else {
          EEPROM.write(1, comando_serial.toInt());
          break;
        }
      }
    }

    if (byte(TYPE_CONNECTION) == 1) {
      aux_comando_serial = "";
      comando_serial = "";
      Serial.println(F("\nMODO CABO IP FIXO ESCOLHIDO"));
      Serial.println(F("\nPRIMEIRA PARTE DO IP:"));
      while (1) {
        delay(10);
        if (Serial.available()) { //verifica se chegou algum dado na serial
          aux_comando_serial = String(Serial.read() - '0'); //Lê o byte mais recente disponível na serial
          if (aux_comando_serial.toInt() != -35) {
            comando_serial += aux_comando_serial;
            Serial.print(aux_comando_serial);
          }
          else {
            EEPROM.write(2, comando_serial.toInt());
            break;
          }
        }
      }

      aux_comando_serial = "";
      comando_serial = "";
      Serial.println(F("\nSEGUNDA PARTE DO IP:"));
      while (1) {
        delay(10);
        if (Serial.available()) { //verifica se chegou algum dado na serial
          aux_comando_serial = String(Serial.read() - '0'); //Lê o byte mais recente disponível na serial
          if (aux_comando_serial.toInt() != -35) {
            comando_serial += aux_comando_serial;
            Serial.print(aux_comando_serial);
          }
          else {
            EEPROM.write(3, comando_serial.toInt());
            break;
          }
        }
      }


      aux_comando_serial = "";
      comando_serial = "";
      Serial.println(F("\nTERCEIRA PARTE DO IP:"));
      while (1) {
        delay(10);
        if (Serial.available()) { //verifica se chegou algum dado na serial
          aux_comando_serial = String(Serial.read() - '0'); //Lê o byte mais recente disponível na serial
          if (aux_comando_serial.toInt() != -35) {
            comando_serial += aux_comando_serial;
            Serial.print(aux_comando_serial);
          }
          else {
            EEPROM.write(4, comando_serial.toInt());
            break;
          }
        }
      }

      aux_comando_serial = "";
      comando_serial = "";
      Serial.println(F("\nQUARTA PARTE DO IP:"));
      while (1) {
        delay(10);
        if (Serial.available()) { //verifica se chegou algum dado na serial
          aux_comando_serial = String(Serial.read() - '0'); //Lê o byte mais recente disponível na serial
          if (aux_comando_serial.toInt() != -35) {
            comando_serial += aux_comando_serial;
            Serial.print(aux_comando_serial);
          }
          else {
            EEPROM.write(5, comando_serial.toInt());
            break;
          }
        }
      }
    }


    if (byte(TYPE_CONNECTION) == 2) {
      Serial.println(F("\nMODO CABO DHCP ESCOLHIDO."));
    }

    if (byte(TYPE_CONNECTION) == 3) {
      aux_comando_serial = "";
      comando_serial = "";
      char c;
      Serial.println(F("\nMODO WIFI ESCOLHIDO"));


      Serial.println(F("SSID:"));
      while (1) {
        delay(10);
        if (Serial.available()) { //verifica se chegou algum dado na serial
          c = Serial.read();
          if (isalpha(c)) {
            aux_comando_serial = String(c); //Lê o byte mais recente disponível na serial
          } else if (isdigit(c)) {
            aux_comando_serial = String(c - '0'); //Lê o byte mais recente disponível na serial
          } else {
            aux_comando_serial = String(c); //Lê o byte mais recente disponível na serial
          }
          if (aux_comando_serial != "\r" && aux_comando_serial != "\n") {
            comando_serial += aux_comando_serial;
            Serial.print(aux_comando_serial);
          }
          else {
            EEPROM.write(6, comando_serial.length());

            for (byte i = 0; i < comando_serial.length(); i++) {
              EEPROM.write(300 + i, comando_serial.charAt(i));
            }
            break;
          }
        }
      }




      comando_serial = "";
      aux_serial = 0;
      Serial.println(F("\nPASSWORD:"));
      while (1) {
        delay(10);
        if (Serial.available()) { //verifica se chegou algum dado na serial
          c = Serial.read();
          if (isalpha(c)) {
            aux_comando_serial = String(c); //Lê o byte mais recente disponível na serial
          } else if (isdigit(c)) {
            aux_comando_serial = String(c - '0'); //Lê o byte mais recente disponível na serial
          } else {
            aux_comando_serial = String(c); //Lê o byte mais recente disponível na serial
          }
          if (aux_comando_serial != "\r" && aux_comando_serial != "\n") {
            comando_serial += aux_comando_serial;
            Serial.print(aux_comando_serial);
          }
          else {
            EEPROM.write(7, comando_serial.length());

            for (byte i = 0; i < comando_serial.length(); i++) {
              EEPROM.write(450 + i, comando_serial.charAt(i));
            }
            break;
          }
        }
      }
    }


    Serial.println(F("\nRESET POR FAVOR!"));
    while (1) {
      delay(100);
    }
  }
  else {
    Serial.println(F("MODO DE PROGRAMACAO NAO ATIVADO, CONTINUANDO BOOT NORMALMENTE."));
  }









  //  // start the Ethernet connection and the server:
  //  Ethernet.begin(mac, ip);
  //  server_interno.begin();
  //  Serial.print("server is at ");
  //  Serial.println(Ethernet.localIP());
  switch (TYPE_CONNECTION) {
    case 1: {
        // start the Ethernet connection and the server:
        Ethernet.begin(mac, ip);
        Serial.print(F("server is at "));
        Serial.println(Ethernet.localIP());
        break;
      }
    case 2: {
        // try to start the Ethernet DHCP connection:
        long tempo_DHCP = millis();
        byte connec;
        while ((millis() - tempo_DHCP) < TIMEOUT_DHCP) {
          connec = Ethernet.begin(mac);
          if (connec == 0) {
            Serial.println(F("Failed to configure Ethernet using DHCP, will try again after 5 sec"));
            delay(5000);
          }
          else {
            break;
          }
        }
        if (connec == 0) {
          //Will start the Ethernet connection with the last valid DHCP IP acquired (which connected to cloud successfully):
          Ethernet.begin(mac, ip);
          Serial.print(F("DHCP falhou completamente, iniciando com o ultimo IP que conseguiu se conectar a nuvem: "));
          Serial.println(Ethernet.localIP());
        }
        else {
          // print your local IP address:
          Serial.print("My IP address: ");
          for (byte thisByte = 0; thisByte < 4; thisByte++) {
            // print the value of each byte of the IP address:
            Serial.print(Ethernet.localIP()[thisByte], DEC);
            Serial.print(".");
          }
          Serial.println();
        }
        break;
      }
    case 3: {
        String ssid_wifi = "";
        for (byte i = 0; i < SIZE_SSID; i++) {
          ssid_wifi += String(char(EEPROM.read(300 + i)));
        }
        String password_wifi = "";
        for (byte i = 0; i < SIZE_PASSW; i++) {
          password_wifi += String(char(EEPROM.read(450 + i)));
        }
        // Configure na linha abaixo a velocidade inicial do
        // modulo ESP8266
        esp8266.begin(115200);
        sendData("AT+RST\r\n", 2000, DEBUG);
        delay(1000);
        Serial.println("Versao de firmware");
        delay(3000);
        sendData("AT+GMR\r\n", 2000, DEBUG); // rst
        // Configure na linha abaixo a velocidade desejada para a
        // comunicacao do modulo ESP8266 (9600, 19200, 38400, etc)
        sendData("AT+CIOBAUD=19200\r\n", 2000, DEBUG);
        Serial.println("** Final **");

        esp8266.begin(19200);
        sendData("AT+RST\r\n", 2000, DEBUG); // rst
        // Conecta a rede wireless
        sendData(String("AT+CWJAP=\"" + ssid_wifi + "\",\"" + password_wifi + "\"\r\n"), 2000, DEBUG);
        delay(3000);
        sendData("AT+CWMODE=1\r\n", 1000, DEBUG);
        delay(3000);
        // Mostra o endereco IP
        sendData("AT+CIFSR\r\n", 1000, DEBUG);
        delay(3000);
        // Configura para multiplas conexoes
        sendData("AT+CIPMUX=1\r\n", 1000, DEBUG);
        delay(3000);
        // Inicia o web server na porta 80
        sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG);
        delay(3000);
        break;
      }
  }

  tempo_cloud = millis();
  tempo_sensores = millis();
  nivel_atual = 0;
  for (byte i = 0; i < (sizeof(nivel_liquido) / sizeof(int)); i++) {
    nivel_liquido[i] = 0;
  }
  leitura_sensor();
  long calibrate_time = millis();
  do {
    leitura_sensor();
  }
  while ( (!valida_leituras()) && ((millis() - calibrate_time) < TIMEOUT_CALIBRATE ));
  preenche_nivel_atual();

}


//Verifica se as leituras estão com diferença de no máximo 5%
boolean valida_leituras() {
  boolean teste = true;
  //  for (byte i = 1; i < (sizeof(nivel_liquido) / sizeof(int)); i++) {
  //    if (  ((sqrt(pow(2, ((1.05 * nivel_liquido[i]) - (0.95 * nivel_liquido[i - 1]))))) > (TOLERANCE * NIVEL_TOTAL)) || ((sqrt(pow(2, ((1.05 * nivel_liquido[i - 1]) - (0.95 * nivel_liquido[i]))))) > (TOLERANCE * NIVEL_TOTAL)) ) {
  //      teste = false;
  //    }
  //  }
  return teste;
}

void leitura_sensor() {
  //Le as informacoes do sensor, em cm
  for (byte i = 0; i < (sizeof(nivel_liquido) / sizeof(int)); i++) {
    microsec = ultrasonic.timing();
    nivel_liquido[i] = ultrasonic.convert(microsec, Ultrasonic::CM);
    delay(50);
  }
}

void preenche_nivel_atual() {
  float media = 0;
  for (byte i = 0; i < (sizeof(nivel_liquido) / sizeof(int)); i++) {
    media += float(nivel_liquido[i]);
  }
  media = float(media) / float(sizeof(nivel_liquido) / sizeof(int));
  if (media >= MAX_DISTANCE) {
    media = float(NIVEL_TOTAL);
  }
  if (media <= MIN_DISTANCE) {
    media = float(0);
  }
  nivel_liquido_medio = float(media) / float(100);
  nivel_atual = float(NIVEL_TOTAL) - float(media);
  nivel_atual /= float(100); //para ficar em metros
}

String sendData(String command, const int timeout, boolean debug) {
  // Envio dos comandos AT para o modulo
  String response = "";
  esp8266.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (esp8266.available())
    {
      // The esp has data so display its output to the serial window
      char c = esp8266.read(); // read the next character.
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
  }
  return response;
}

void loop() {

  /*** SENSORES ****************************************************************************************************************************/
  if ((millis() - tempo_sensores) >= TEMPO_SERIAL) {
    Serial.print("Nivel atual:");
    Serial.println(nivel_atual);
    Serial.print("Leitura do sensor: ");
    Serial.println(nivel_liquido_medio);
    //    for (byte i = 0; i < (sizeof(nivel_liquido) / sizeof(int)); i++) {
    //      Serial.println(nivel_liquido[i]);
    //    }
    tempo_sensores = millis();
  }

  leitura_sensor();
  if ( valida_leituras() ) {
    preenche_nivel_atual();
  }
  /*** SENSORES ****************************************************************************************************************************/

  /*** SERVER HTTP-GET / WEB ****************************************************************************************************************************/
  if (TYPE_CONNECTION == 1 || TYPE_CONNECTION == 2) {
    client_interno.flush(); //para liberar o cache do cliente para um possível novo cliente
    // listen for incoming clients
    client_interno = server_interno.available();
    if (client_interno) {
      byte parameter = 0;
      boolean flag = false;
      String is_central = "";
      // Serial.println("new client");
      // an http request ends with a blank line
      boolean currentLineIsBlank = true;
      while (client_interno.connected()) {
        if (client_interno.available()) {
          char c = client_interno.read();

          if (flag == false) {
            is_central += c;
            parameter++;
          }
          if (parameter == sizeof(COMMAND_LEVEL) / sizeof(char) - 1 || parameter == sizeof(COMMAND_SENSOR_READ) / sizeof(char) - 1) {
            if (is_central == COMMAND_LEVEL || is_central == COMMAND_SENSOR_READ) {
              flag = true;
            }
          }

          // if you've gotten to the end of the line (received a newline
          // character) and the line is blank, the http request has ended,
          // so you can send a reply
          if (c == '\n' && currentLineIsBlank) {
            if (is_central != String(COMMAND_LEVEL) && is_central != String(COMMAND_SENSOR_READ)) {
              //            Serial.println(is_central);
              // send a standard http response header
              client_interno.println(F("HTTP/1.1 200 OK"));
              client_interno.println(F("Content-Type: text/html"));
              client_interno.println(F("Connection: close"));  // the connection will be closed after completion of the response
              client_interno.print(F("Refresh: "));  // refresh the page automatically
              client_interno.println(REFRESH_PAGE_TIME);
              client_interno.println();
              client_interno.println(F("<!DOCTYPE html><html><head>"));

              client_interno.println(F("<title>Sua Caixa d'&Aacute;gua</title>"));

              client_interno.println(F("<style>"));

              client_interno.println(F("#customers {"));
              client_interno.println(F("font-family: \"Trebuchet MS\", Arial, Helvetica, sans-serif;"));
              client_interno.println(F("border-collapse: collapse;"));
              client_interno.println(F("width: 80%;"));
              client_interno.println(F("text-align: center;"));
              client_interno.println(F("}"));

              client_interno.println(F("#customers td, #customers th {"));
              client_interno.println(F("border: 1px solid #ddd;"));
              client_interno.println(F("padding: 30px;"));
              client_interno.println(F("}"));

              client_interno.println(F("#customers tr:nth-child(even){background-color: }"));

              client_interno.println(F("#customers tr:hover {background-color: #ddd;}"));

              client_interno.println(F("#customers th {"));
              client_interno.println(F("padding-top: 12px;"));
              client_interno.println(F("padding-bottom: 12px;"));
              client_interno.println(F("text-align: center;"));
              client_interno.println(F("background-color: #4CAF50;"));
              client_interno.println(F("color: white;"));
              client_interno.println(F("}"));

              client_interno.println(F("</style>"));
              client_interno.println(F("</head>"));
              client_interno.println(F("<body>"));


              client_interno.println(F("<h4>Desenvolvido por Lucas e Lu&iacute;z - v3.0</h4>"));
              client_interno.println(F("<h4>Sua Caixa d'&Aacute;gua - Web Server</h4>"));
              client_interno.println(F("<h1 style=text-align:center;>CAIXA D'&Aacute;GUA</h1>"));


              client_interno.println(F("<center>"));

              client_interno.println(F("<table id=\"customers\">"));

              client_interno.println(F("<tr>"));
              client_interno.println(F("<th>ALTURA DA COLUNA DE L&Iacute;QUIDO</th>"));
              client_interno.println(F("</tr>"));
              client_interno.println(F("<tr>"));
              client_interno.print(F("<td>"));
              client_interno.print(nivel_atual);
              client_interno.print(F(" m"));
              client_interno.print(F("</td>"));
              client_interno.print(F("</tr>"));
              client_interno.println(F("</table>"));

              client_interno.println(F("<table id=\"customers\">"));
              client_interno.println(F("<tr>"));
              client_interno.println(F("<th>ALTURA DA COLUNA DE AR</th>"));
              client_interno.println(F("</tr>"));
              client_interno.println(F("<tr>"));
              client_interno.print(F("<td>"));
              client_interno.print(nivel_liquido_medio);
              client_interno.print(F(" m"));
              client_interno.print(F("</td>"));
              client_interno.print(F("</tr>"));
              client_interno.println(F("</table>"));

              if (nivel_atual == VAZIO) {
                client_interno.print(F("<h1 style=\text-align:center;\><font size=\"6\" color=\""));
                client_interno.print(RED_COLOR);
                client_interno.println(F("\"><strong>RESERVAT&Oacute;RIO QUASE VAZIO!</strong></font></h1>"));
              }
              if (nivel_atual == CHEIO) {
                client_interno.print(F("<h1 style=\text-align:center;\><font size=\"6\" color=\""));
                client_interno.print(GREEN_COLOR);
                client_interno.println(F("\"><strong>RESERVAT&Oacute;RIO CHEIO!</strong></font></h1>"));
              }
              client_interno.println(F("</center>"));
              client_interno.println(F("</body>"));
              client_interno.println(F("</html>"));
              break;
            }
            else if (is_central == COMMAND_LEVEL) {
              //Serial.print("!!!!!!!!!!! ");
              client_interno.println(F("HTTP/1.1 200 OK"));
              client_interno.println();
              client_interno.print(nivel_atual);
              // Serial.println(nivel_atual);
              //            client_interno.println();
              break;
            }
            else if (is_central == COMMAND_SENSOR_READ) {
              //Serial.print("!!!!!!!!!!! ");
              client_interno.println(F("HTTP/1.1 200 OK"));
              client_interno.println();
              client_interno.print(nivel_liquido_medio);
              // Serial.println(nivel_atual);
              //            client_interno.println();
              break;
            }
          }

          if (c == '\n') {
            // you're starting a new line
            currentLineIsBlank = true;
          } else if (c != '\r') {
            // you've gotten a character on the current line
            currentLineIsBlank = false;
          }
        }
      }
      // give the web browser time to receive the data
      //delay(1);
      // close the connection:
      //delay(1000);
      client_interno.stop();
      // Serial.println("client disconnected");
    }
    //    Serial.print("freeMemory()=");
    //    Serial.println(freeMemory());
  }
  else if (TYPE_CONNECTION == 3) {
    // Verifica se o ESP8266 esta enviando dados
    if (esp8266.available())
    {
      if (esp8266.find("+IPD,"))
      {
        delay(300);
        int connectionId = esp8266.read() - 48;

        String webpage = "<head><meta http-equiv=""refresh"" content=""3"">";
        webpage += "</head><h1><u>ESP8266 - Web Server</u></h1><h2>Porta";
        webpage += "Digital 8: ";
        int a = digitalRead(8);
        webpage += a;
        webpage += "<h2>Porta Digital 9: ";
        int b = digitalRead(9);
        webpage += b;
        webpage += "</h2>";

        String cipSend = "AT+CIPSEND=";
        cipSend += connectionId;
        cipSend += ",";
        cipSend += webpage.length();
        cipSend += "\r\n";

        sendData(cipSend, 1000, DEBUG);
        sendData(webpage, 1000, DEBUG);

        String closeCommand = "AT+CIPCLOSE=";
        closeCommand += connectionId; // append connection id
        closeCommand += "\r\n";

        sendData(closeCommand, 3000, DEBUG);
      }
    }
  }
  /*** SERVER HTTP-GET / WEB ***********************************************************************************************************************************/




  /*** CLIENT - CLOUD ***********************************************************************************************************************************/

  if ((millis() - tempo_cloud) >= TEMPO_CLOUD) {

    if (TYPE_CONNECTION == 1 || TYPE_CONNECTION == 2) {
      client_cloud.flush(); //para liberar o cache do cliente para um possível novo cliente
      client_cloud.stop();
      //If you get a connection with cloud, report back via serial:
      if (client_cloud.connect(server_cloud, HTTP_PORT)) {
        Serial.println(F("\nConnected to Cloud!\n"));

        //Se a conexao e DHCP e deu certo, guarda o endereco para o caso de ligar e noa conseguir pegar nada pelo DHCP
        if (flag_FirstConnectionSuccess && TYPE_CONNECTION == 2) {
          Serial.println("IP DHCP validado, salvando na EEPROM...");
          for (byte thisByte = 0; thisByte < 4; thisByte++) {
            EEPROM.write(thisByte + 2, Ethernet.localIP()[thisByte]);
            Serial.print(EEPROM.read(thisByte + 2));
            Serial.print(".");
          }
          flag_FirstConnectionSuccess = false;
        }


        // Make a HTTP request:
        String PostCommand = "POST /api/v1/monitoring?level[level]=";
        PostCommand += String(nivel_liquido_medio);
        PostCommand += "&level[device_id]=";
        PostCommand += String(DEVICE_ID);
        PostCommand += " HTTP/1.1";
        client_cloud.println(PostCommand);
        client_cloud.print(F("Host: "));
        client_cloud.println(SERVER_POST_COMMAND);
        client_cloud.println(F("Connection: close"));
        client_cloud.println();

        while (client_cloud.connected()) {
          // if there are incoming bytes available
          // from the server, read them and print them:
          if (client_cloud.available()) {
            Serial.println(F("Return from Cloud Server: "));
            while (client_cloud.available()) {
              char c = client_cloud.read();
              Serial.print(c);
            }
            Serial.println();
            Serial.println();
          }
        }

        //      // if the server's disconnected, stop the client:
        //      if (!client_cloud.connected()) {
        //        Serial.println();
        //        Serial.println(F("Disconnected from Cloud!"));
        //        Serial.println();
        //        client_cloud.stop();
        //      }
      } else {
        // if you didn't get a connection to the server:
        Serial.println(F("\nConnection to Cloud failed!\n"));
      }
    }
    else if (TYPE_CONNECTION == 3) {

    }
    tempo_cloud = millis();
  }
  /*** CLIENT - CLOUD ***********************************************************************************************************************************/

}






