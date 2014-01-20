/*
Proyecto sencillo para representar en un display con la Arduino Yún el tiempo restante para que pasen autobuses de Madrid por una determinada parada.

Jorge Gomez / Sindormir.net 
 jorge@sindormir.net 2014
 
El proyecto esta preparado para funcionar con el LCD Keypad Shield.

Licencia GPLv3

*/

#include <Process.h>
#include <LiquidCrystal.h>

byte sindormir[8] = {
  B00000,
  B01111,
  B10001,
  B10101,
  B10001,
  B10001,  
  B11111,
};

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//Estimaciones de paradas para la parada 634 (Illescas con Tembleque, buses 25 y 121)
const String url_emt = "http://www.emtmadrid.es/PMVVisor/pmv.aspx?stopnum=";
String parada = "634";
unsigned int tiempo_refresco = 60; // 1 minuto de tiempo de refreso

void setup() {
  pinMode(13, OUTPUT);

  Bridge.begin();
  lcd.createChar(0, sindormir);
  lcd.begin(16, 2);
  
  //Logotipo inicial
  lcd.print(" Sindormir.net ");
  lcd.setCursor(7,2);
  lcd.write(byte(0));
  lcd.write(byte(0));
  delay(2000);
}

void loop() {
  bus_data();
  delay(tiempo_refresco*1000);
}

void bus_data() {
  Process p;
  char c;
  byte conta_long = 0;
  byte estado = 0;

  digitalWrite(13, HIGH); //Avisamos de que estamos estableciendo comunicacion
  lcd.clear();
  lcd.print("Recibiendo datos");
  lcd.setCursor(7,2);
  lcd.write(byte(0));
  lcd.write(byte(0));

  
  p.runShellCommand("curl -k --silent "  + url_emt + parada + " | grep -A 11 ctl00_MainContent_Estimacion | grep -E \"style10|center\" | cut -d \">\" -f 2 | cut -d \"<\" -f 1 | sed -e \"s/\\&gt;/\\>/g\" | sed -e \"s/\\&\\#39;/\\'/g\" | sed -e \"s/ //g\" | tr \"\\n\" \" \" ");
  while (p.running()); // Esperar hasta que termine

  digitalWrite(13, LOW);

  while (p.available() > 0) {
    c = p.read();
    
    if (c == ' ') {  //Utilizamos como separador de cada una de las 6 informaciones un espacio (Ver codigo de parseo de la URL)
      estado++;
      c = p.read();
    }

    switch (estado) {
      case 0: //Cabecera linea 1
        lcd.clear();
        //lcd.print("L ");
        estado++;
      case 1: //Numero de linea
        Console.print(c);
        lcd.write(c);
        conta_long++;
        break;
      case 2: // Padding
        if (conta_long==1) lcd.print("   -> ");
        else if (conta_long==2) lcd.print("  -> ");
        else lcd.print(" -> ");
        conta_long=0;
        estado++;
      case 3: // Tiempo primer bus
        Console.print(c);
        lcd.write(c);
        break;
      case 4: // Padding
        lcd.print("  ");
        estado++;        
      case 5: // Tiempo segundo bus
        Console.print(c);
        lcd.write(c);
        break;
      case 6: // Deteccion de siguiente linea
        lcd.setCursor(0, 2);
        Console.print("\n");
        estado++;
      case 7: //Cabecera linea 2
        //lcd.print("L ");
        estado++;        
      case 8:  //Numero de linea
        Console.print(c);
        lcd.write(c);
        conta_long++;
        break;
      case 9: // Padding
        if (conta_long==1) lcd.print("   -> ");
        else if (conta_long==2) lcd.print("  -> ");
        else lcd.print(" -> ");
        estado++;     
      case 10: // Tiempo primer bus
        Console.print(c);
        lcd.write(c);
        break;
      case 11: // Padding
        lcd.print("  ");
        estado++;           
      case 12: // Tiempo segundo bus
        Console.print(c);
        lcd.write(c);
        break;
    }
  }
  p.close();
}


