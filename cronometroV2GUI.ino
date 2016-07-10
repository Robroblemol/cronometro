
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

//**********************************************************************
//DEFINICIONES Y VARIABLES
//**********************************************************************

//Definiciones del display y los pines asociados
#define LCD_RS 10
#define LCD_RW 9
#define LCD_EN 8
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7

//Definiciones del teclado y los pines asociados
#define KEY_ESC A1
#define KEY_ABA A2
#define KEY_ARR A3
#define KEY_ENT A4

//Definiciones de los sensores
#define SEN_O1 2  //Salida del sensor 1   
#define SEN_O2 3  //Salida del sensor 2 
#define SEN_I1 A5  //Entrada del sensor 1
#define SEN_I2 A6  //Entrada del sensor 2

#define LED 13    //LED incorporado del arduino

int MaxNFranjas = 10;     //Máximo número de franjas
int MinAnchoFranjas = 10; //Mínimo ancho de franjas
int MaxAnchoFranjas = 100; //Máximo ancho de franjas

int menu = 1;    //Valor del nivel de menú
int tecla = 0;   //Valor de la tecla oprimida
int midiendo = 0; //Variable que indica si está midiendo o no.

int sensor1Inicio;  //Valor inicial de la entrada del sensor 1
int sensor1Actual;  //Valor actual de la entrada del sensor 1
//int sensor2Inicio;  //Valor inicial de la entrada del sensor 2
//int sensor2Actual;  //Valor actual de la entrada del sensor 2

int nFranjas = 1;     //1 franja equivale a dos eventos, al llegar y al salir.
int nFranjasAux;
int anchoFranjas = MinAnchoFranjas; //10mm

long int tiempos1[21];    //Almacena los tiempos transcurridos hasta cada cambio del sensor 1
long int tiempos1Aux[21]; //Auxiliar para cálculos
long int ti[21];
long int vi[21];
long int dxi[21];
float velocidadAux[21];  //Auxiliar de la velocidad para calculos
int auxTiempos[7];        //Auxiliar para despliegue

int i1 = 0;                //Puntero, indica el tiempo actual del sensor 1
//int i2=0;                  //Puntero, indica el tiempo actual del sensor 2

//**********************************************************************
//RUTINAS DE INICIO Y CONFIGURACIÓN INICIAL
//**********************************************************************
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

//Inicio del puerto serie de comunicaciones, definiendo los pines asociados
SoftwareSerial mySerial(0, 1); // RX, TX

void setup() {
  iniciarLCD();
  iniciaPines();

  //Inicio del puerto de comunicaciones
  Serial.begin(9600);    //Define la velocidad del puerto en 9600bps
  while (!Serial) {      // Espera a que inicien las comunicaciones
  }
  // Serial.println("CRONOMETRO DE LABORATORIO");  //Envía mensaje por el puerto de comunicaciones
  //Serial.println("**** YEIMMY LONDONO *****");  //Envía mensaje por el puerto de comunicaciones
}

//**********************************************************************
//RUTINAS DEL BUCLE
//**********************************************************************

void loop() {

  tecla = SensarTeclas();

  if (midiendo == 1) {
    if (tecla == 1) {                //Si la tecla 1 se presiona, cancela la medición
      midiendo = 0;                //Medición cancelada
    }
    sensor1Actual = digitalRead(SEN_I1); //Lee el estado de entrada del sensor 1
    if (sensor1Actual != sensor1Inicio) { //Compara el estado actual con el anterior para monitorear cambios
      tiempos1[i1] = millis();           //Cuando ocurra un cambio, toma el tiempo actual
      sensor1Inicio = sensor1Actual;     //Actualiza el estado de entrada del sensor para comparación posterior
      if (i1 == 1) {
        mensajeMidiendo(); //Si el puntero está en 1, muestra mensaje de "Midiendo..."
      }
      i1 = i1 + 1;                       //Aumenta el puntero de la matriz de datos, en 1.
      nFranjasAux = nFranjasAux - 1;     //Decrementa el contador de flancos de franja = (2 * franjas)

      if (nFranjasAux == 0) {            //Si termina el conteo, despliega la información
        midiendo = 0;
        ApagarSensor1();
        mensajeTerminado();
        delay(1000);
        calculoDeltaT(tiempos1[0], tiempos1[i1 - 1]); //Calculo del tiempo que paso desde que se activo el sensor
        lcd.clear();                                //Limpia pantalla
        mensajeResultadosTiempoLCD();               //Despliega la información en el LCD
        mensajeCabeceraResultadosUART();
        calculoVoA();
      }
    }
  }
  else {
    if (tecla == 1) {                //Si la tecla 1 se presiona
      if (menu == 1) {                  //Si está en el menú 1
        menu1();
      }
      if (menu == 2) {                  //Si está en el menú 2
        menu1();
      }
      if (menu == 3) {
        ApagarSensor1();
        menu2();
      }
    }

    if (tecla == 2) {                //Si la tecla 2 se presiona
      if (menu == 1) {
        if (nFranjas < MaxNFranjas) {
          nFranjas = nFranjas + 1;
        }
        menu1();
      }
      if (menu == 2) {
        if (anchoFranjas < MaxAnchoFranjas) {
          anchoFranjas = anchoFranjas + 1;
        }
        menu2();
      }
    }

    if (tecla == 3) {                //Si la tecla 3 se presiona
      if (menu == 1) {
        if (nFranjas > 1) {
          nFranjas = nFranjas - 1;
        }
        menu1();
      }
      if (menu == 2) {
        if (anchoFranjas > MinAnchoFranjas) {
          anchoFranjas = anchoFranjas - 1;
        }
        menu2();
      }
    }

    if (tecla == 4) {                //Si la tecla 4 se presiona
      if (menu == 2) {
        menu3();
      }
      if (menu == 1) {
        menu2();
      }
    }
  }
}

//**********************************************************************
//RUTINAS DE CALCULO Y DESPLIEGUE DE RESULTADOS
//**********************************************************************
void calculoVoA(){
  long int tiempo;                     //Variable local para calculo del tiempo transcurrido
  float velocidad = 0;

  if (nFranjas == 1) { //Si es 1 franja, calcula la velocidad
    tiempo = tiempos1[i1 - 1] - tiempos1[0]; //Se toma el valor de la variable global
    //   if (tiempo<=0){
    //     velocidad=0;
    //   }else{
    velocidad = (((float)anchoFranjas) / (float)tiempo);
    //   }
    lcd.setCursor(0, 1);                //
    lcd.print("v :");
    lcd.setCursor(4, 1);                //
    lcd.print(velocidad, 3);
    lcd.print(" m/s");
     Serial.print("V");
    Serial.print(velocidad,3);
    Serial.print("%");//final de trama;
    //Serial.println(" m/s");
  } else {     //Si es un tren de franjas, se calcula la aceleración usando mínimos cuadrados
    int i = 0;  //Indice de las matrices
    int nFranjasCalc = (nFranjas * 2 ) - 1;
    Serial.print("TP");  //Envía mensaje por el puerto de comunicaciones
    for (i = 0; i < nFranjasCalc; i++) {
      tiempos1Aux[i] = tiempos1[i + 1] - tiempos1[i];                //Calculo el dt
      velocidadAux[i] = (((float)anchoFranjas * 1000) / (float)tiempos1Aux[i]); //Calculo el dx/dt
      deltaTnoFormat(tiempos1[i], tiempos1[i + 1],(i+1));  
      //mensajeResultadosTiempoUART() ; //Calculo para desplegar en el puerto serie
      //Serial.print("T");                                             //Envía al puerto serie, la lista de tiempos parciales dt.
      //Serial.print(i+1);
      //Serial.print("-> ");
                                     //Cambia el formato para ser desplegado
    
  }
  sendTiarray();

    float Ex = 0.0;       //Declara las variables para realizar mínimos cuadrados
    float Ev2 = 0.0;      //
    float Exv2 = 0.0;     //
    float Ex2 = 0.0;      //
    float pendiente = 0;  //Almacena el resultado de la pendiente

    nFranjasCalc = (nFranjas * 2 ) - 1;  //Defino el término n de la sumatoria
    for (i = 0; i < nFranjasCalc; i++) { //Calculo las sumatorias para determinar la pendiente
      Ex += anchoFranjas * (i + 1);     //calculo la sumatoria del ancho de las franjas
      Ev2 += (velocidadAux[i]) * (velocidadAux[i]);       //Calculo la sumatoria de la velocidad al cuadrado
      Exv2 += ((velocidadAux[i]) * (velocidadAux[i])) * ((float)anchoFranjas * (i + 1)); //Calculo la sumatoria de la vellocodad al cuadrado por la distancia
      Ex2 += (anchoFranjas * (i + 1)) * (anchoFranjas * (i + 1)); //Calculo de la suma de las distancias al cuadrado
    }
    // Serial.println("__SUMATORIAS REESCALADAS_____");     //Esta seccion se utiliza para depuración
    // Serial.println(Ex);                                  //Envía al puerto serie, los resultados de las sumatorias
    // Serial.println(Ev2,3);                               //Estas sumatorias se encuentran reescaladas
    // Serial.println(Exv2,3);
    // Serial.println(Ex2,3);
    // Serial.println(nFranjasCalc);
    // Serial.println("__SUMATORIAS REESCALADAS_____");

    pendiente = ((nFranjasCalc * Exv2) - (Ex * Ev2)) / ((nFranjasCalc * Ex2) - (Ex * Ex)); //Calcula la pendiente de la recta
    // Serial.println(pendiente/1000.000,3);                                 //Envía el resultado de la pendiente al puerto serie
   // Serial.print("TP");  //Envía mensaje por el puerto de comunicaciones
    Serial.print("A");
    Serial.print((pendiente / 2000.000), 3);               //La pendiente es el doble de la aceleración.
    Serial.print("!");
    //Serial.println(" m/s^2");
    //Serial.println("");

    lcd.setCursor(0, 1);                //Despliega en pantalla el valor calculado de la aceleración
    lcd.print("a :");
    lcd.setCursor(4, 1);                //
    lcd.print((pendiente / 2000.000), 3); //La aceleración es la mitad del valor de la aceleración y se cambia la escala nuevamente (1/(2 * 1000)
    lcd.print(" m/s^2");
  }
}
//i no puede ser 0
int countTi = 0;
void deltaTnoFormat(long int T0,long int Tf, int i){
  long int tiempo;
  if(i==1){
  countTi=0;
  }
  tiempo = (Tf - T0); 
  ti[0] = 0;
  ti[i] = tiempo+ti[i-1]; // caluculo ti
  countTi++;
 // Serial.print("f");
 //Serial.print(tiempo,3);
}
void sendTiarray(){
for(int i =0;i<=countTi;i++){
  Serial.print("f");
  Serial.print(ti[i]);
}
}



void calculoDeltaT(long int T0, long int Tf) {
  int m, mu, md, s, su, l, lu, lc, sd, ld; //Definición de las varables locales
  long int tiempo;                     //Variable local para calculo del tiempo transcurrido

  tiempo = (Tf - T0);                //

  m = (tiempo / 1000) / 60;           //Calculo de los minutos
  mu = m % 10;                        //
  md = (m - mu) / 10;                 //

  s = (tiempo / 1000) % 60;           //Cálculo de los segundos transcurridos
  su = s % 10;                        //
  sd = (s - su) / 10;                 //

  l = (tiempo % 1000);                //Cálculo de las milésimas de segundo
  lu = l % 10;                        //
  ld = ((l - lu) / 10) % 10;          //
  lc = (l - (ld * 10) - lu) / 100;    //

  auxTiempos[0] = md;
  auxTiempos[1] = mu;
  auxTiempos[2] = sd;
  auxTiempos[3] = su;
  auxTiempos[4] = lc;
  auxTiempos[5] = ld;
  auxTiempos[6] = lu;
}

//**********************************************************************
//RUTINAS DE MENÚ POR NIVELES
//**********************************************************************
void menu1() {
  menu = 1;
  lcd.clear();
  lcd.print("Total franjas :");  //lcd.print(data)
  lcd.setCursor(7, 1);          //lcd.setCursor(col, row)
  lcd.print(nFranjas);           //Escribe en la posición seleccionada, el número de franjas actuales
  delay(200);
  midiendo = 0;
}

void menu2() {
  menu = 2;
  lcd.clear();
  lcd.print("Ancho franjas : ");  //lcd.print(data)
  lcd.setCursor(6, 1);           //lcd.setCursor(col, row)
  lcd.print(anchoFranjas);       //Escribe en la posición seleccionada, el ancho de las franjas actuales
  lcd.setCursor(10, 1);
  lcd.print("mm");
  delay(200);
  midiendo = 0;
}

void menu3() {
  EncenderSensor1();                    //Enciende el sensor 1
  //EncenderSensor2();                  //Enciende el sensor 2
  delay(500);                           //Espera para estabilizar los estados lógicos de los sensores, en la entrada
  sensor1Inicio = digitalRead(SEN_I1);  //Lee el estado inicial de la entrada de sensor 1
  //sensor2Inicio=digitalRead(SEN_I2);    //Lee el estado inicial de la entrada de sensor 2
  midiendo = 1;                         //Indicador de medición activo
  i1 = 0;                               //Inicia el puntero de tiempos medidos del sensor 1
  //i2=0;                                 //Inicia el puntero de tiempos medidos del sensor 2
  nFranjasAux = nFranjas * 2;            //Calcula el número de flancos que va a contar durante la medición

  menu = 3;                              //Define menú 3 seleccionado
  lcd.clear();                           //Despliega información en pantalla
  lcd.print("Listo para medir");         //lcd.print(data)
  lcd.setCursor(4, 1);                   //Se ubica el cursor en el caracter 5
  lcd.print("00:00:000");
  delay(200);
}

//**********************************************************************
void mensajeMidiendo() {
  lcd.setCursor(0, 0);            //Se ubica el cursor en la primera fila
  lcd.print("Midiendo........");
}

void mensajeTerminado() {
  ApagarSensor1();
  //ApagarSensor2()
  midiendo = 0;
  lcd.setCursor(0, 0);            //Se ubica el cursor en la primera fila
  lcd.print("Completado.     ");
}

//**********************************************************************
void mensajeResultadosTiempoLCD() {
  lcd.print("t : ");
  lcd.print(auxTiempos[0]);                      //Mostrar los valores en el display
  lcd.print(auxTiempos[1]);
  lcd.print(":");
  lcd.print(auxTiempos[2]);
  lcd.print(auxTiempos[3]);
  lcd.print(":");
  lcd.print(auxTiempos[4]);
  lcd.print(auxTiempos[5]);
  lcd.print(auxTiempos[6]);
}

void mensajeCabeceraResultadosUART() {
  //Serial.println(" ");
  //Serial.println("********** DATOS OBTENIDOS **********");  //Envía mensaje por el puerto de comunicaciones
  Serial.print("@");
  Serial.print("TF");//N fanjas
  if(nFranjas<10){
    Serial.print(0);
    Serial.print(nFranjas);
  }
  else
  Serial.print(nFranjas);
  Serial.print("AF");//Ancho fanjas
  if(anchoFranjas<100){
    Serial.print(0);
    Serial.print(anchoFranjas);
  }
  else 
  Serial.print(anchoFranjas);
  //Serial.println(" mm");
  Serial.print("TT");//tiempo total
  mensajeResultadosTiempoUART();
   
}

void mensajeResultadosTiempoUART() {
  Serial.print(auxTiempos[0]);
  Serial.print(auxTiempos[1]);
  Serial.print(":");
  Serial.print(auxTiempos[2]);
  Serial.print(auxTiempos[3]);
  Serial.print(":");
  Serial.print(auxTiempos[4]);
  Serial.print(auxTiempos[5]);
  Serial.print(auxTiempos[6]);
}
void calculoDeltaA(long int T0, long int Tf){
 long int tiempo;                     //Variable local para calculo del tiempo transcurrido

  tiempo = (Tf - T0);
  Serial.print(tiempo,3);
}

//**********************************************************************
//RUTINAS DE ENCENDIDO, APAGADO DE SENSORES Y LECURA DE TECLAS
//**********************************************************************
void EncenderSensor1() {
  digitalWrite(SEN_O1, LOW);  //Sensor 1 encendido
}
void ApagarSensor1() {
  digitalWrite(SEN_O1, HIGH);  //Sensor 1 apagado
}
void EncenderSensor2() {
  digitalWrite(SEN_O2, LOW);  //Sensor 2 encendido
}
void ApagarSensor2() {
  digitalWrite(SEN_O2, HIGH);  //Sensor 2 apagado
}


int SensarTeclas() {         //Retorna el código de la tecla que se ha presionado, retorna 0 si no hay tecla.
  if (digitalRead(KEY_ESC) == HIGH) {
    return 1;
  }
  if (digitalRead(KEY_ARR) == HIGH) {
    return 2;
  }
  if (digitalRead(KEY_ABA) == HIGH) {
    return 3;
  }
  if (digitalRead(KEY_ENT) == HIGH) {
    return 4;
  }
  return 0;
}
//**********************************************************************
//RUTINAS DE INICIO DE PERIFÉRICOS
//**********************************************************************

void iniciaPines() {
  //Define los pines de las teclas
  pinMode(KEY_ESC, INPUT);   //Tecla escape =tecla 1
  pinMode(KEY_ARR, INPUT);   //Tecla escape =tecla 2
  pinMode(KEY_ABA, INPUT);   //Tecla escape =tecla 3
  pinMode(KEY_ENT, INPUT);   //Tecla escape =tecla 4

  //Define los pines de activación de los sensores
  pinMode(SEN_I1, INPUT);     //Entrada del sensor 1
  pinMode(SEN_I2, INPUT);     //Entrada del sensor 2
  //Define los pines de entrada desde los sensores
  pinMode(SEN_O1, OUTPUT);    //Salida de activación del sensor 1
  pinMode(SEN_O2, OUTPUT);    //Salida de activación del sensor 2
  //Define valores inciales para los sensores (apagados al inicio)
  digitalWrite(SEN_O1, HIGH);  //Sensor 1 apagado
  digitalWrite(SEN_O2, HIGH);  //Sensor 2 apagado
  pinMode(LED, OUTPUT);
}

void iniciarLCD() {  //Inicio del LCD
  pinMode(LCD_RW, OUTPUT);    //Pin de control de lectura/escritura del display.
  digitalWrite(LCD_RW, LOW);  //LCD_RW LOW permanentemente en bajo
  lcd.begin(16, 2);           //Se define el LCD como de 16 x 2
  lcd.print("** CRONOMETRO  **"); //Muestra el mensaje en el LCD
  lcd.setCursor(0, 1);            //Posiciona el cursor en la segunda línea del LCD
  lcd.print("*YEIMMY LONDONO*");  //Despliega mensaje en lcd
  delay(4000);                    //Espera 4s
  lcd.clear();                    //Limpia pantalla
  lcd.print("Iniciando");         //Simula inicio
  delay(400);
  lcd.print(".");
  delay(400);
  lcd.print(".");
  delay(400);
  lcd.print(".");
  delay(700);
  menu1();
}
