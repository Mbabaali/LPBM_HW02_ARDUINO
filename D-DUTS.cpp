  /**
 * Main_Program_TestBench_Arduino
 * 
 * Ce programme est destiné à être utilisé sur le banc de test "LOW Power Bench Measurement".
 * Le code a été développé en 2016, puis retravaillé en 2018.
 * 
 */
// Ajout des librairies pour le SPI
#include <Arduino.h>
#include "SPI.h"

// Déclaration variables en fonction d'interruption
volatile bool TC3_flag = false;
void configureTC3a();
  
// Déclaration des pins de sélection de Channel Mux et ADC
#define CSADC1 2 //chip select
#define CSMUX1 3 //mux
#define CSADC2 A4
#define CSMUX2 A5
#define CSADC3 A2
#define CSMUX3 A3
#define CSADC4 A0
#define CSMUX4 A1

// Déclaration des pins de veille/éveil des produits
#define CMD_ACC_DUT1 4
#define CMD_ACC_DUT2 6
#define CMD_ACC_DUT3 8
#define CMD_ACC_DUT4 10

// Déclaration des pins d'état de channel (1: µa, sinon mA/A)
#define UI_PROT_STATE_DUT1 5
#define UI_PROT_STATE_DUT2 7
#define UI_PROT_STATE_DUT3 9
#define UI_PROT_STATE_DUT4 11

// Déclaration des variables globales
long compteur = 0; // Compteur de seconde pour fonction d'interruption
int uploadconfig = 0; // Etat de la configuration

int statut = 0; // appareil testé en mode sleep ou awake (dépend de la tension ACC)
int etat_mA_uA = 0; // 0 on est en mA, si 1 on est en µA //à vérifier, mais doit etre supprimé
int ok_pour_mA = 0; // Validation d'envoi ou non mA/A //à Supprimer
String inputString=""; //string de stockage du bus UART (Pi->Arduino)
bool finReception = false;  // whether the string is complete

enum courantUnit
{
  uA=0,
  mA=1,
  A=2,
  unknown=3
}; //Creation d'un type de variable "courant unit" représentant l'unité dans laquelle doit être affiché la puissance et le courant sur l'IHM

/**
 * Création d'une structure ADC. 
 * Cette structure comprend : 
 * des variables "channel", dans lesquels seront stocké les valeur envoyé par les différents ADC par 
 * le bus SPI avant conversion de ses données
 * des variables de puissance et d'intensité fourni par chaque ADC
 * l'unité à priviliégié sur l'affichage 
 */

struct adc {
  double channel_uA;
  double channel_mA;
  double channel_A;
  
  double Vin;

  double uA;
  double mA;
  double A;

  double uW;
  double mW;
  double W;

  courantUnit unite;

}; 

 struct cycle{
  long time_awake;
  long time_sleep;
  String time_awake_str;
  String time_sleep_str;
  int nb_rep;
  String nb_rep_str;
  };

 struct cycle cycle[3]={{0,0,"","",0,""},{0,0,"","",0,""},{0,0,"","",0,""}};
 

 short int nb_cycle=1;
 bool etat_start=1;
 int f_acquisition=1000;
 String conteneur="";
 short int cycle_en_cours=0;
 short int flag_cycle=0;
 short int rep_en_cours=1;
  





/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////INITIALISATION PROGRAMME/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  // Ouverture du port Serial pour l'affichage console du résultat de la conversion
  delay(1000);
  SerialUSB.begin(115200);

  // Initialisation du SPI
  SPI.begin();
  SPI.setClockDivider(48);    // Fréquence d'horloge de 1MHz
  SPI.setBitOrder(MSBFIRST);  // Most Significant Bit First (Arduino.org pour plus de détails)
  SPI.setDataMode(SPI_MODE0); // SPI Mode_0d

  // Initialisation des sorties Chip Select
  digitalWrite(CSADC1, HIGH);
  digitalWrite(CSMUX1, HIGH);
  digitalWrite(CSADC2, HIGH);
  digitalWrite(CSMUX2, HIGH);
  digitalWrite(CSADC3, HIGH);
  digitalWrite(CSMUX3, HIGH);
  digitalWrite(CSADC4, HIGH);
  digitalWrite(CSMUX4, HIGH);

  pinMode(CSADC1, OUTPUT);
  pinMode(CSMUX1, OUTPUT);
  pinMode(CSADC2, OUTPUT);
  pinMode(CSMUX2, OUTPUT);
  pinMode(CSADC3, OUTPUT);
  pinMode(CSMUX3, OUTPUT);
  pinMode(CSADC4, OUTPUT);
  pinMode(CSMUX4, OUTPUT);

  // Initialisation des sorties de veille/éveil des produits CMD_ADC_DUT
 changerEtatACC(LOW);

  pinMode(CMD_ACC_DUT1, OUTPUT);
  pinMode(CMD_ACC_DUT2, OUTPUT);
  pinMode(CMD_ACC_DUT3, OUTPUT);
  pinMode(CMD_ACC_DUT4, OUTPUT);

  // Initialisation des entrées d'états de conversion(µa ou mA/A ?)
  pinMode(UI_PROT_STATE_DUT1, INPUT);
  pinMode(UI_PROT_STATE_DUT2, INPUT);
  pinMode(UI_PROT_STATE_DUT3, INPUT);
  pinMode(UI_PROT_STATE_DUT4, INPUT);

  // Attente d'ouverture du port Serial
//   while (!SerialUSB) {
//     ; // wait for serial port to connect. Needed for native USB
//   }

  pinMode(LED_BUILTIN, OUTPUT);

  // Fonction d'initialisation de la phase d'interruption
  configureTC3a();
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////FONCTION PRINCIPALE/////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
    Serial.println("Début loop");

  int start=millis();
  int duree=0;
  
  struct adc Adc1={0,0,0,0,0,0,0,0,0,0,unknown};
  struct adc Adc2={0,0,0,0,0,0,0,0,0,0,unknown};
  struct adc Adc3={0,0,0,0,0,0,0,0,0,0,unknown};
  struct adc Adc4={0,0,0,0,0,0,0,0,0,0,unknown};
  double Vin = 0;
  double channel_Vin=0;
  boolean start_mesure=false;


//   while(SerialUSB.available()&&!finReception)
//   {
//     /**
//      * Cette boucle s'active si l'arduino commence à recevoir des données sur son port uart (données envoyé par la pi). on receptionne les données sous forme 
//      * de string (inputString), puis lorsque la reception est fini, on analyse la chaine de caractère. selon le format suivant :
//      * ->la première lettre est un s : l'user a "start" l'acquisition, on reçoit donc les paramètres de time_awake et time_sleep
//      * ->la première lettre est un "p" : l'user a mis en pause l'acquisition : il faut donc....
//      * 
//      */
//     // get the new byte:
//     char inChar = (char)SerialUSB.read();
//     // add it to the inputString:
//     inputString += inChar;
//     // if the incoming character is a newline, set a flag so the main loop can
//     // do something about it:
//     if (inChar == '\n') {
//       finReception = true;
//     }
//     if(finReception)
//     {
//       switch (inputString[0])
//       {
//         /**
//          * On regarde la première lettre des données reçues
//          */
//         case 's':
//           cycle[0].time_awake_str=inputString.substring(1,7);
//           cycle[0].time_sleep_str=inputString.substring(7,13);
//           cycle[0].time_awake=cycle[0].time_awake_str.toInt();
//           cycle[0].time_sleep=cycle[0].time_sleep_str.toInt();
          
//           cycle[1].time_awake_str=inputString.substring(13,19);
//           cycle[1].time_sleep_str=inputString.substring(19,25);
//           cycle[1].time_awake=cycle[1].time_awake_str.toInt();
//           cycle[1].time_sleep=cycle[1].time_sleep_str.toInt();
          
//           cycle[2].time_awake_str=inputString.substring(25,31);
//           cycle[2].time_sleep_str=inputString.substring(31,37);
//           cycle[2].time_awake=cycle[2].time_awake_str.toInt();
//           cycle[2].time_sleep=cycle[2].time_sleep_str.toInt();

//           /*conteneur=inputString.substring(37,40);
//           f_acquisition=conteneur.toInt();
//           f_acquisition=f_acquisition*1000;
//           f_acquisition-=50;
//           conteneur="";*/

//           conteneur=inputString.substring(37,38);
//           etat_start=conteneur.toInt();
//           cycle_en_cours=0;
//           flag_cycle=0;
//           rep_en_cours=1;
//           cycle[0].nb_rep_str=inputString.substring(38,40);
//           cycle[1].nb_rep_str=inputString.substring(40, 42);
//           cycle[2].nb_rep_str=inputString.substring(42,44);
//           cycle[0].nb_rep=cycle[0].nb_rep_str.toInt();
//           cycle[1].nb_rep=cycle[1].nb_rep_str.toInt();
//           cycle[2].nb_rep=cycle[2].nb_rep_str.toInt();

//           //DEBUG//
//           /*
//           SerialUSB.print("awake 1 : ");
//           SerialUSB.println(cycle[0].time_awake);
//           SerialUSB.print("sleep 1 : ");
//           SerialUSB.println(cycle[0].time_sleep);
          
//           SerialUSB.print("awake 2 : ");
//           SerialUSB.println(cycle[1].time_awake);
//           SerialUSB.print("sleep 2 : ");
//           SerialUSB.println(cycle[1].time_sleep);

//           SerialUSB.print("awake 3 : ");
//           SerialUSB.println(cycle[2].time_awake);
//           SerialUSB.print("sleep 3 : ");
//           SerialUSB.println(cycle[2].time_sleep);

//           SerialUSB.print("frequence : ");
//           SerialUSB.println(f_acquisition);

//           SerialUSB.print("start : ");
//           SerialUSB.println(etat_start);

//           SerialUSB.print("repetition 1er cycle : ");
//           SerialUSB.println(cycle[0].nb_rep);
//           SerialUSB.print("repetition 2eme cycle : ");
//           SerialUSB.println(cycle[1].nb_rep);
//           SerialUSB.print("repetition 3eme cycle : ");
//           SerialUSB.println(cycle[2].nb_rep);
//           */
          
          

          

//           if(etat_start==1)
//           {
//             changerEtatACC(HIGH);
//           }
//           if(etat_start==0)
//           {
//             changerEtatACC(LOW);
//           }

//           nb_cycle=verif_nb_cycle();
//           /*SerialUSB.print("nb cycle : ");
//           SerialUSB.println(nb_cycle);*/
//           uploadconfig=true;
//           SerialUSB.print("ok\n");
//         break;

//         case 'p':
//           uploadconfig=false;
//           changerEtatACC(LOW);
//           cycle_en_cours=0;
//           flag_cycle=0;
          
//           SerialUSB.print("ok\n");
//         break;

//         case'd':
//           SerialUSB.print("d");
//           SerialUSB.print("test debug");
//           SerialUSB.print("\n");
        
//         default:
//           SerialUSB.print("problème : l'arduino ne reconnait pas la donnée reçue : ");
//           SerialUSB.println(inputString);
//         break;
//       }
//       inputString="";
//       finReception=false;
//     }
//   }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Acquistion SPI de tous les Duts
    // Sélection du channel de tension d'entrée des équipements testés
    /**
     * La tension d'entrée est la même pour chaque ADC, mais pour une raison que j'ignore encore il est important d'appeler SPIread channel pour chaque adc en channel (3)
     * en revanche, seul la variable channel_Vin est réellement utilisé. 
     */
    SelectChannel(3);
    delay(150);  // Delay de conversion
    // Lecture de chaque ADC pour le channel correspondant
    Adc1.Vin = SpiReadChannelADC1();

    Serial.println(Adc1.Vin);
    // Adc2.Vin = SpiReadChannelADC2();
    // Adc3.Vin = SpiReadChannelADC3();
    // channel_Vin = SpiReadChannelADC4();
    
    // // Sélection du channel de consommation en µA des équipements testés
    // SelectChannel(0);
    // delay(150);  // Delay de conversion
    // // Lecture de chaque ADC pour le channel correspondant
    // Adc1.channel_uA = SpiReadChannelADC1();
    // Adc2.channel_uA = SpiReadChannelADC2();
    // Adc3.channel_uA = SpiReadChannelADC3();
    // Adc4.channel_uA = SpiReadChannelADC4();
    
    // // Sélection du channel de consommation en mA des équipements testés
    // SelectChannel(1);
    // delay(150);  // Delay de conversion
    // // Lecture de chaque ADC pour le channel correspondant
    // Adc1.channel_mA = SpiReadChannelADC1();
    // Adc2.channel_mA = SpiReadChannelADC2();
    // Adc3.channel_mA = SpiReadChannelADC3();
    // Adc4.channel_mA = SpiReadChannelADC4();
    
    // // Sélection du channel de consommation en A des équipements testés
    // SelectChannel(2);
    // delay(150);  // Delay de conversion
    // // Lecture de chaque ADC pour le channel correspondant
    // Adc1.channel_A = SpiReadChannelADC1();
    // Adc2.channel_A = SpiReadChannelADC2();
    // Adc3.channel_A = SpiReadChannelADC3();
    // Adc4.channel_A = SpiReadChannelADC4();

    // // Conversion des résultats de chaque Duts
    // // Conversion des réceptions SPI du Channel 1
    // Adc1.uA = conversion_channel_microA(Adc1.channel_uA);
    // Adc1.mA = conversion_channel_mA(Adc1.channel_mA);
    // Adc1.A = conversion_channel_A(Adc1.channel_A);
    // // Conversion des réceptions SPI du Channel 2
    // Adc2.uA = conversion_channel_microA(Adc2.channel_uA);
    // Adc2.mA = conversion_channel_mA(Adc2.channel_mA);
    // Adc2.A = conversion_channel_A(Adc2.channel_A);
    // // Conversion des réceptions SPI du Channel 3
    // Adc3.uA = conversion_channel_microA(Adc3.channel_uA);
    // Adc3.mA = conversion_channel_mA(Adc3.channel_mA);
    // Adc3.A = conversion_channel_A(Adc3.channel_A);
    // // Conversion des réceptions SPI du Channel 4
    // Adc4.uA = conversion_channel_microA(Adc4.channel_uA);
    // Adc4.mA = conversion_channel_mA(Adc4.channel_mA);
    // Adc4.A = conversion_channel_A(Adc4.channel_A);
    // // Conversion de la tension d'entrée des équipements testés
    // Vin = conversion_channel_power_in(channel_Vin);

    // // Calcul de la puissance en mW à partir de la conso µA
    // Adc1.uW = Adc1.uA * Vin * 0.001;
    // Adc2.uW = Adc2.uA * Vin * 0.001;
    // Adc3.uW = Adc3.uA * Vin * 0.001;
    // Adc4.uW = Adc4.uA * Vin * 0.001;


    // // Calcul de la puissance en mW à partir de la conso en mA
    // Adc1.mW = Adc1.mA * Vin;
    // Adc2.mW = Adc2.mA * Vin;
    // Adc3.mW = Adc3.mA * Vin;
    // Adc4.mW = Adc4.mA * Vin; ///Puissance fausse ??? non ?

    // // Calcul de la puissance en W
    // Adc1.W = Adc1.A * Vin;
    // Adc2.W = Adc2.A * Vin;
    // Adc3.W = Adc3.A * Vin;
    // Adc4.W = Adc4.A * Vin;

    // /////////////////////////////////////////////////////////////////////////////////////////
    // /////////////////////////////ENVOI DE LA TRAME NEW VERSION///////////////////////////////
    // /////////////////////////////////////////////////////////////////////////////////////////

    // /**
    //  * DEBUG : AFFICHAGE DE LA TRAME UART DES VARIABLES
    //  * 
    //  */
    //  CorrectionValeur(&Adc1);//On applique l'offset de correction des valeurs
    //  Adc1.unite=checkUniteAdc(Adc1); //On vérifie quelle unité choisir

    //  CorrectionValeur(&Adc2);
    //  Adc2.unite=checkUniteAdc(Adc2);

    //  CorrectionValeur(&Adc3);
    //  Adc3.unite=checkUniteAdc(Adc3);

    //  CorrectionValeur(&Adc4);
    //  Adc4.unite=checkUniteAdc(Adc4);



     
    //  EnvoiTrame(Adc1, Adc2, Adc3, Adc4); //puis on envoie la trame via le port UART sur la pi.



    //  /*SerialUSB.println("##########ADC2###########");
    //  printAdc(Adc2);
    //  SerialUSB.println("##########ADC3###########");
    //  printAdc(Adc3);
    //  SerialUSB.println("##########ADC4###########");
    //  printAdc(Adc4);*/
     
    
    delay(50);
  
}


/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////CONVERSION CHANNEL A///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
float conversion_channel_A(long result)
 /**
   * Au niveau de la conversion, on sait que 4,096 V = 1 048 575 bit
   * On sait aussi qu'on a un span (quantum) de valeur : q = 0.000003906253725 V/bit    et q = 0.000777245 pA
   * Or au niveau de du "ACS713" admet en entrée une difference de courant (I+ qui est la valeur du courant de l'interface toujours égale à 1.5 A et I-), une résistance interne q'on nommera R1 de valeur 1.2mOhm, 
   *   une sortie représentée par un courant (entre 0 et 4,096 V) qu'on nommera Us  : 
   *                                                                                Us = R1 . [(I+) - (I-)]
   *                                          Par conséquent, on devra determiner  [(I+) - (I-)]
   *                                          [(I-) - (I+)] = Us / R1
   *                                          
   *                                          on auras : 
   *                                          (I-) = [(Valeur mesurée - 1.5)*0.000777245]     // (la valeur mesurée peut etre en BIN/HEX/DEC sa dépend) et le 0.000777245 = q (span)
   *             "SI BESOIN DE PLUS D'INFOS, PLONGER DANS LES DOCS DE L'AC713 ET DANS LES RAPPORTS :) "
   *                                          
   */
{
  // Déclaration de la variable pour la tension
  double courant_A;

  result = result - 384000;
  courant_A =  (result * 0.000007);

 
  
  
  // Retour de la valeur convertie
  return courant_A;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////CONVERSION CHANNEL mA//////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
float conversion_channel_mA(long result)
/*
   * |||||||||||||||||||||||||| EXPLICATION DU CALCUL ||||||||||||||||||||||
   * Ici nous souhaitons convertir notre tension en sortie de l'ADC en mA
   * tout d'abord, le span (ou quantum) q = 0.0000039062533725 V/bit
   * Etapes de la conversion :
   *    1ère etape : conversion de la valeur lue (par arduino en sortie de l'ADC) en V 
   *                Us = Us . q 
   *    2ème étape : On convertira cette valeur en mV (car trop petite)
   *                Us = Us . 1000
   *    3ème étape : On convertie notre en A avec la lois d'ohm (Is = Us/R) avec R = 21.5 mOhm (à voir dans le schéma de la carte au niveau du MAX232)
   *    4ème étape: On convertie notre valeur en Us en Ue puis on retrance 250 (ne pas oublier que le MAX232 a un signal en sortie qui est égal 250* le signal d'entrée ) 
   *    
   *    Bonne Lecture :)
   */
   {
  // Déclaration de la variable pour la tension
  double courant_mA;

  courant_mA = (result * 0.00000390625);

  courant_mA = (courant_mA * 1000) / (0.0215 * 250);

  // Retour de la valeur convertie
  return courant_mA;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////CONVERSION CHANNEL µA//////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
float conversion_channel_microA(long result)
{
  /*
   * |||||||||||||||||||||||||| EXPLICATION DU CALCUL ||||||||||||||||||||||
   * Ici nous souhaitons convertir notre tension en sortie de l'ADC en mA
   * tout d'abord, le span (ou quantum) q = 0.0000039062533725 A/bit
   * Etapes de la conversion :
   *    1ère etape : conversion de la valeur lue (par arduino en sortie de l'ADC) en V 
   *                Us = Us . q 
   *    2ème étape : On convertira cette valeur en mV (car trop petite)
   *                Us = Us . 1,000,0000
   *    3ème étape : On convertie notre en A avec la lois d'ohm (Is = Us/R) avec R = 20 Ohm (à voir dans le schéma de la carte au niveau du MAX232)
   *    4ème étape: On convertie notre valeur en Us en Ue puis on retrance 250 (ne pas oublier que le MAX232 a un signal en sortie qui est égal 250* le signal d'entrée ) 
   *    
   *    Bonne Lecture :)
   */
  // Déclaration de la variable pour la tension
  double courant_microA;

  courant_microA =  (result * 0.00000390625);
  //courant_microA =  (result * 0.000777245);
  courant_microA = (courant_microA * 1000000) /(10 * 250);

  // Retour de la valeur convertie
  return courant_microA;
  
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////CONVERSION CHANNEL Power In////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
float conversion_channel_power_in(long result)
{
  // Déclaration de la variable pour la tension
  double power_in;

  power_in =  (result * 0.00003047);

  // Retour de la valeur convertie
  return power_in;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////CONVERSION CHANNEL Power In////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
float conversion_channel_power_out(long result)
{
  // Déclaration de la variable pour la tension
  double power_out;

  power_out =  (result * 0.00003047);

  // Retour de la valeur convertie
  return power_out;
}


/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////FONCTION SELECTION CHANNEL ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void SelectChannel(int n)
/**
 * Les 4 ADC du banc ont un multiplexeur integré, cette fonction nous permet de le set pour récuperer un channel en particulier de l'ADC
 * (les 4 channels à récuperer sont : tension d'entrée, courant en uA, mA, et A)
 */
{
  // Tableau des Codes Hexa sélection de channel
  int channel[8] = {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};//

  // Chip Select à l'état haut pour la sélection du channel
  digitalWrite(CSADC1, HIGH); // CSADC à 1, pas de sortie
  digitalWrite(CSADC2, HIGH);
  digitalWrite(CSADC3, HIGH);
  digitalWrite(CSADC4, HIGH);

  digitalWrite(CSMUX1, HIGH); // Uniquement le CSMUX à 1 de l'ADC dont on souhaite mofidier le channel
  digitalWrite(CSMUX2, HIGH);
  digitalWrite(CSMUX3, HIGH);
  digitalWrite(CSMUX4, HIGH);

  // Envoie SPI de l'adresse MUX pour le channel souhaité
  SPI.transfer(channel[n]);

  // On remet tous les CSMUX à 0 pour les connecter en interne à l'ADC
  digitalWrite(CSADC1, LOW);
  digitalWrite(CSADC2, LOW);
  digitalWrite(CSADC3, LOW);
  digitalWrite(CSADC4, LOW);
  /**
   * Pourquoi on remet les CSADC à High ? qu'est ce que ça donne si on le met à low ? 
   */

  digitalWrite(CSMUX1, LOW);
  digitalWrite(CSMUX2, LOW);
  digitalWrite(CSMUX3, LOW);
  digitalWrite(CSMUX4, LOW);
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////FONCTION LECTURE SPI ADC1//////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
long SpiReadChannelADC1(void)
{
  /**
   * fonction pour lire le bus SPI de l'ADC 1 
   */
  // Variable pour sauvegarder le résultat de la conversion
  long result = 0;

  digitalWrite(CSADC1, LOW);
  digitalWrite(CSMUX1, LOW);
  
  digitalWrite(CSADC2, HIGH);
  digitalWrite(CSMUX2, HIGH);
  
  digitalWrite(CSADC3, HIGH);
  digitalWrite(CSMUX3, HIGH);
  
  digitalWrite(CSADC4, HIGH);
  digitalWrite(CSMUX4, HIGH);

  // Attente de la fin de conversion
  // Observation du passage de MISO à zéro
  while (MISO == HIGH);//tourne dans le vide tant que MISO n'est pas égale à 0 

  // Récupération des trois octets du résultat
  // Récupération de l'octet B1
  result = SPI.transfer(0xff);            //SerialUSB.println(result,BIN);
  result = result << 8;                   //SerialUSB.println(result,BIN);
  // Récupération de l'octet B2
  result = result | SPI.transfer(0xff);   //SerialUSB.println(result,BIN);
  result = result << 8;                   //SerialUSB.println(result,BIN);
  // Récupération de l'octet B3
  result = result | SPI.transfer(0xff);   //SerialUSB.println(result,BIN);
  // Supression des 4bits de poids forts, conservation des 20bits de données
  result = 0x0fffff & result;             //SerialUSB.println(result,BIN);

  // On termine la conversion en remettant le chip select à l'état haut
  digitalWrite(CSADC1, HIGH);
  digitalWrite(CSADC2, HIGH);
  digitalWrite(CSADC3, HIGH);
  digitalWrite(CSADC4, HIGH);
  
  digitalWrite(CSMUX1, HIGH); // Uniquement le CSMUX à 1 de l'ADC dont on souhaite mofidier le channel
  digitalWrite(CSMUX2, HIGH);
  digitalWrite(CSMUX3, HIGH);
  digitalWrite(CSMUX4, HIGH);

  // Renvoie du résultat
  return (result);
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////FONCTION LECTURE SPI ADC2//////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
long SpiReadChannelADC2(void)
{
  // Variable pour sauvegarder le résultat de la conversion
  long result = 0;

  digitalWrite(CSADC1, HIGH);
  digitalWrite(CSADC2, LOW);
  digitalWrite(CSMUX2, LOW);
  digitalWrite(CSADC3, HIGH);
  digitalWrite(CSADC4, HIGH);

  // Attente de la fin de conversion
  // Observation du passage de MISO à zéro
  while (MISO == HIGH);

  // Récupération des trois octets du résultat
  // Récupération de l'octet B1
  result = SPI.transfer(0xff);            //SerialUSB.println(result,BIN);
  result = result << 8;                   //SerialUSB.println(result,BIN);
  // Récupération de l'octet B2
  result = result | SPI.transfer(0xff);   //SerialUSB.println(result,BIN);
  result = result << 8;                   //SerialUSB.println(result,BIN);
  // Récupération de l'octet B3
  result = result | SPI.transfer(0xff);   //SerialUSB.println(result,BIN);

  // Supression des 4bits de poids forts, conservation des 20bits de données
  result = 0x0fffff & result;             //SerialUSB.println(result,BIN);

  // On termine la conversion en remettant le chip select à l'état haut
  digitalWrite(CSADC1, HIGH);
  digitalWrite(CSADC2, HIGH);
  digitalWrite(CSADC3, HIGH);
  digitalWrite(CSADC4, HIGH);
  
  digitalWrite(CSMUX1, HIGH); // Uniquement le CSMUX à 1 de l'ADC dont on souhaite mofidier le channel
  digitalWrite(CSMUX2, HIGH);
  digitalWrite(CSMUX3, HIGH);
  digitalWrite(CSMUX4, HIGH);

  // Renvoie du résultat
  return (result);
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////FONCTION LECTURE SPI ADC3//////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
long SpiReadChannelADC3(void)
{
  // Variable pour sauvegarder le résultat de la conversion
  long result = 0;

  digitalWrite(CSADC1, HIGH);
  digitalWrite(CSADC2, HIGH);
  digitalWrite(CSADC3, LOW);
  digitalWrite(CSMUX3, LOW);
  digitalWrite(CSADC4, HIGH);

  // Attente de la fin de conversion
  // Observation du passage de MISO à zéro
  while (MISO == HIGH);

  // Récupération des trois octets du résultat
  // Récupération de l'octet B1
  result = SPI.transfer(0xff);            //SerialUSB.println(result,BIN);
  result = result << 8;                   //SerialUSB.println(result,BIN);
  // Récupération de l'octet B2
  result = result | SPI.transfer(0xff);   //SerialUSB.println(result,BIN);
  result = result << 8;                   //SerialUSB.println(result,BIN);
  // Récupération de l'octet B3
  result = result | SPI.transfer(0xff);   //SerialUSB.println(result,BIN);

  // Supression des 4bits de poids forts, conservation des 20bits de données
  result = 0x0fffff & result;             //SerialUSB.println(result,BIN);

  // On termine la conversion en remettant le chip select à l'état haut
  digitalWrite(CSADC1, HIGH);
  digitalWrite(CSADC2, HIGH);
  digitalWrite(CSADC3, HIGH);
  digitalWrite(CSADC4, HIGH);
  
  digitalWrite(CSMUX1, HIGH); // Uniquement le CSMUX à 1 de l'ADC dont on souhaite mofidier le channel
  digitalWrite(CSMUX2, HIGH);
  digitalWrite(CSMUX3, HIGH);
  digitalWrite(CSMUX4, HIGH);

  // Renvoie du résultat
  return (result);
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////FONCTION LECTURE SPI ADC4//////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
long SpiReadChannelADC4(void)
{
  /**
   * Permet de lire les signaux envoyé par SPI par les ADC : 
   * 
   */
  // Variable pour sauvegarder le résultat de la conversion
  long result = 0;

  digitalWrite(CSADC1, HIGH);
  digitalWrite(CSADC2, HIGH);
  digitalWrite(CSADC3, HIGH);
  digitalWrite(CSADC4, LOW);
  digitalWrite(CSMUX4, LOW);

  // Attente de la fin de conversion
  // Observation du passage de MISO à zéro
  while (MISO == HIGH);

  // Récupération des trois octets du résultat
  // Récupération de l'octet B1
  result = SPI.transfer(0xff);            //SerialUSB.println(result,BIN);
  result = result << 8;                   //SerialUSB.println(result,BIN);
  // Récupération de l'octet B2
  result = result | SPI.transfer(0xff);   //SerialUSB.println(result,BIN);
  result = result << 8;                   //SerialUSB.println(result,BIN);
  // Récupération de l'octet B3
  result = result | SPI.transfer(0xff);   //SerialUSB.println(result,BIN);

  // Supression des 4bits de poids forts, conservation des 20bits de données
  result = 0x0fffff & result;             //SerialUSB.println(result,BIN);

  // On termine la conversion en remettant le chip select à l'état haut
  digitalWrite(CSADC1, HIGH);
  digitalWrite(CSADC2, HIGH);
  digitalWrite(CSADC3, HIGH);
  digitalWrite(CSADC4, HIGH);

  digitalWrite(CSMUX1, HIGH); // Uniquement le CSMUX à 1 de l'ADC dont on souhaite mofidier le channel
  digitalWrite(CSMUX2, HIGH);
  digitalWrite(CSMUX3, HIGH);
  digitalWrite(CSMUX4, HIGH);
  // Renvoie du résultat
  return (result);
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////FONCTIONS DEBUG////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void printAdc(struct adc adc)
/**
 * focntion de debug permettant d'afficher les principales valeurs de notre structure ADC
 */
{
  SerialUSB.print("Valeur de uA : ");
  SerialUSB.println(adc.uA);
  
  SerialUSB.print("Valeur de mA : ");
  SerialUSB.println(adc.mA);
  
  SerialUSB.print("Valeur de A : ");
  SerialUSB.println(adc.A);
  
  SerialUSB.print("Valeur de uW : ");
  SerialUSB.println(adc.uW);
  
  SerialUSB.print("Valeur de mW : ");
  SerialUSB.println(adc.mW);

  SerialUSB.print("Valeur de W : ");
  SerialUSB.println(adc.W);

  SerialUSB.print("unité à selectionner : ");
  SerialUSB.println(adc.unite);

  SerialUSB.println("\n");

}

courantUnit checkUniteAdc(adc adc)
/**
 * fonction permettant de choisir la meilleur unité à utiliser en fonctiond du courant reçu
 */
{
  courantUnit unite=unknown;
  
  if(adc.A>0.5)
  {
    unite=A;
    //SerialUSB.println("adc.A>0.5");
  }
  else if(adc.mA>=1.4)
  {
    unite=mA;
    //SerialUSB.println("unite : mA");
    //SerialUSB.println(adc.mA);
  }
  else if(adc.mA<1.4)
  {
    unite=uA;
    //SerialUSB.println("unite : uA");
    
  }
  else
  {
    unite=unknown;
    SerialUSB.println("unknown   (checkUniteAdc)");
  }
  return unite;
}

void CorrectionValeur(adc* adc)
/**
 * Dans la version 2016 du code, des offset était appliqué au code pour modifier des imprécisions de mesure vérifiées par experimentation.
 * Ceux ci sont appliqué dans cette fonction
 */
{

    adc->mA=adc->mA-0.35;
    adc->A=adc->A-0.06;
}

void EnvoiTrame(adc Adc1, adc Adc2, adc Adc3, adc Adc4)
/**
 * Ici, on reçoit nos 4 structures ADC en paramètres, et on envoie la trame UART de mesure vers la pi.
 */
{
      float P[4]={0};
      float A[4]={0};

      switch (Adc1.unite)
      {
        case 0:
          P[0]=Adc1.uW;
          A[0]=Adc1.uA;
        break;
        
        case 1 : 
          P[0]=Adc1.mW;
          A[0]=Adc1.mA;
        break;
        
        case 2 : 
          P[0]=Adc1.W;
          A[0]=Adc1.A;
        break;
        
        default:
          SerialUSB.println("Arduino: Envoi Trame: aucune unité favorisé");
        break;
      }

      switch (Adc2.unite)
      {
        case 0:
          P[1]=Adc2.uW;
          A[1]=Adc2.uA;
        break;
        
        case 1 : 
          P[1]=Adc2.mW;
          A[1]=Adc2.mA;
        break;
        
        case 2 : 
          P[1]=Adc2.W;
          A[1]=Adc2.A;
        break;
        
        default:
          SerialUSB.println("Arduino: Envoi Trame: aucune unité favorisé");
        break;
      }

      switch (Adc3.unite)
      {
        case 0:
          P[2]=Adc3.uW;
          A[2]=Adc3.uA;
        break;
        
        case 1 : 
          P[2]=Adc3.mW;
          A[2]=Adc3.mA;
        break;
        
        case 2 : 
          P[2]=Adc3.W;
          A[2]=Adc3.A;
        break;
        
        default:
          SerialUSB.println("Arduino: Envoi Trame: aucune unité favorisé");
        break;
      }

      switch (Adc4.unite)
      {
        case 0:
          P[3]=Adc4.uW;
          A[3]=Adc4.uA;
        break;
        
        case 1 : 
          P[3]=Adc4.mW;
          A[3]=Adc4.mA;
        break;
        
        case 2 : 
          P[3]=Adc4.W;
          A[3]=Adc4.A;
        break;
        
        default:
          SerialUSB.println("Arduino: Envoi Trame: aucune unité favorisé");
        break;
      }

      SerialUSB.print(statut);
      SerialUSB.print(":");
      SerialUSB.print(Adc1.unite);
      SerialUSB.print(":");
      SerialUSB.print(P[0]);
      SerialUSB.print(":");
      SerialUSB.print(A[0]);
      SerialUSB.print(":");
      SerialUSB.print(Adc2.unite);
      SerialUSB.print(":");
      SerialUSB.print(P[1]);
      SerialUSB.print(":");
      SerialUSB.print(A[1]);
      SerialUSB.print(":");
      SerialUSB.print(Adc3.unite);
      SerialUSB.print(":");
      SerialUSB.print(P[2]);
      SerialUSB.print(":");
      SerialUSB.print(A[2]);
      SerialUSB.print(":");
      SerialUSB.print(Adc4.unite);
      SerialUSB.print(":");
      SerialUSB.print(P[3]);
      SerialUSB.print(":");
      SerialUSB.print(A[3]);
      SerialUSB.print(":\r\n");
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////FONCTIONS DE MODIFICATION DE L ETAT DES ENTREES////////////
/////////////////////////////////////////////////////////////////////////////////////////

void changerEtatADCMUX(int etat)
{
  
}
short int verif_nb_cycle()
{
  nb_cycle=1;
  if((cycle[1].time_awake!=0)||(cycle[1].time_sleep!=0))
  {
    nb_cycle++;
  }

  if((cycle[2].time_awake!=0)||(cycle[2].time_sleep!=0))
  {
    nb_cycle++;
  }
  return nb_cycle;
}
void changerEtatACC(int etat)
{
  /**
   * Permet de change l'état de l'ACC
   */
  digitalWrite(CMD_ACC_DUT1, etat);
  digitalWrite(CMD_ACC_DUT2, etat);
  digitalWrite(CMD_ACC_DUT3, etat);
  digitalWrite(CMD_ACC_DUT4, etat);

  statut=etat;
  compteur=0;
}

void flashLED()
{
  static bool led;

  led = !led;                         //flip status led
  if (led)
  {
    digitalWrite(LED_BUILTIN, LOW);
    SerialUSB.println("OFF\r\n");
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
    SerialUSB.println("ON\r\n");
  }
}

void verifCycle()
{
  if(flag_cycle==2)
  {
    if(rep_en_cours==cycle[cycle_en_cours].nb_rep)
    {
      if(cycle_en_cours+1==nb_cycle)
      {
        cycle_en_cours=0;
      }
      else
      {
        cycle_en_cours++;
      }
      rep_en_cours=0;
    }
    
    flag_cycle=0;
    rep_en_cours++;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////INTERRUPTION (COMPTEUR DE SECONDES)////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void configureTC3a()
{
  const uint8_t GCLK_SRC = 4;

  SYSCTRL->XOSC32K.bit.RUNSTDBY = 1;

  GCLK->GENDIV.reg = GCLK_GENDIV_ID(GCLK_SRC) | GCLK_GENDIV_DIV(2);
  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {
    /* Wait for synchronization */
  }

  GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN |
                      GCLK_GENCTRL_SRC_XOSC32K |
                      GCLK_GENCTRL_ID(GCLK_SRC) |
                      GCLK_GENCTRL_RUNSTDBY;
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {
    /* Wait for synchronization */
  }

  // Turn the power to the TC3 module on
  PM->APBCMASK.reg |= PM_APBCMASK_TC3;

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |
                      GCLK_CLKCTRL_GEN(GCLK_SRC) |
                      GCLK_CLKCTRL_ID(GCM_TCC2_TC3);
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {
    /* Wait for synchronization */
  }


  TC3->COUNT8.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC3->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY) {
    /* Wait for synchronization */
  }

  TC3->COUNT8.CTRLA.reg = TC_CTRLA_MODE_COUNT8 |
                          TC_CTRLA_RUNSTDBY |
                          TC_CTRLA_PRESCALER_DIV64;
  while (TC3->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY) {
    /* Wait for synchronization */
  }

  // Enable interrupt on overflow
  // TC_INTENSET_OVF, enable an interrupt on overflow
  TC3->COUNT8.INTENSET.reg = TC_INTENSET_OVF;
  while (TC3->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY) {
    /* Wait for synchronization */
  }

  // Enable TC3
  TC3->COUNT8.CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC3->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY) {
    /* Wait for synchronization */
  }

  // Enable the TC3 interrupt vector
  // Set the priority to max
  NVIC_EnableIRQ(TC3_IRQn);
  NVIC_SetPriority(TC3_IRQn, 0x00);
}

void TC3_Handler()
{
  /*
  
  */
  if (TC3->COUNT8.INTFLAG.bit.OVF)
  {

    // Reset interrupt flag
    TC3->COUNT8.INTFLAG.bit.OVF = 1;

    // Set compare match flag for CC0
    compteur++;

    if(uploadconfig)
    {
       if(nb_cycle==1)
       {
         if((statut==0)&&(compteur >= cycle[0].time_sleep))
          {   
            changerEtatACC(HIGH);
          }
          if((statut==1)&&(compteur >= cycle[0].time_awake))
          {
            changerEtatACC(LOW);
          }
       }
       else
       {
        if(uploadconfig)
        {
          if((statut==0)&&(compteur>=cycle[cycle_en_cours].time_sleep))
          {
            changerEtatACC(HIGH);
            flag_cycle++;
            verifCycle();
          }
          if((statut==1)&&(compteur>=cycle[cycle_en_cours].time_awake))
          {
            changerEtatACC(LOW);
            flag_cycle++;
            verifCycle();
          }
        }
       }
    }
  }
}