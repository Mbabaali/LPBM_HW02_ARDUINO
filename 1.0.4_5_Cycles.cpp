/****************************************************************************
   Programme : TEST_0.0.1_Insertion_des_DUT_et_Lecture
   Date de création : Jeudi 24 Octobre 2019 à 14h41
   Créateur: BARWANE Aslam
   Fonction du programme test: Insertion des signaux de DUT1 à DUT6
*/

// Ajout des librairies pour le SPI
#include <Arduino.h>
#include "SPI.h"
#include <DueTimer.h>

// Déclaration des pins de sélection de Channel ADC et ADC
#define CSADC1 2
#define CSADC2 A4
#define CSADC3 A2
#define CSADC4 A0
#define CSADC5 3
#define CSADC6 A5

// Déclaration des pins de veille/éveil des produits
#define CMD_ACC_DUT1 4
#define CMD_ACC_DUT2 6
#define CMD_ACC_DUT3 8
#define CMD_ACC_DUT4 10
#define CMD_ACC_DUT5 A11
#define CMD_ACC_DUT6 A9

// Déclaration des pins d'état de channel (1: µa, sinon mA/A)
#define UI_PROT_STATE_DUT1 5
#define UI_PROT_STATE_DUT2 7
#define UI_PROT_STATE_DUT3 9
#define UI_PROT_STATE_DUT4 11
#define UI_PROT_STATE_DUT5 A10
#define UI_PROT_STATE_DUT6 A8

// Déclaration des pins de On/Off des produits
#define CMD_PWR_DUT1 25
#define CMD_PWR_DUT2 27
#define CMD_PWR_DUT3 31
#define CMD_PWR_DUT4 33
#define CMD_PWR_DUT5 47
#define CMD_PWR_DUT6 49

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////DECLARATION DES CLASSES ET ENUMERATION//////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
struct cycle
{
    long time_awake;
    long time_sleep;
    String time_awake_str;
    String time_sleep_str;

    int nb_rep;
    String nb_rep_str;
};

/*Structure pour stocker la valeur de l'amperage Maximala ne pas dépasser,
  valeur qui sera saisie par l'utilisateur dans l'IHM*/
struct amperageMax
{
    double amax;
    String amax_str;
};

/*Enumération pour l'unité du courant:
  0 => µA
  1 => mA
  2 => A
  3 => Unknown (valeur inconnue qui apparait sur la trame à régler plus tard)*/
enum courantUnit
{
    cu_uA = 0,
    cu_mA = 1,
    cu_A = 2,
    unknown = 3
};

class Dut
{
private:
    String nom;
    double Vin = 0;

    double uA = 0;
    double mA = 0;
    double A = 0;

    double SECURITY_MAX_CURRENT = 0;

    int CMD_ACC_DUT;
    int CMD_PWR_DUT;
    int UI_PROT_STATE_DUT;

    volatile int flag_state_power;

    bool boutonAliementationDut = false;

    double signal_UI;
    double signal_MI;
    double signal_I;
    double signal_PWR_DUT;

    long channel_UI = 0;
    long channel_MI = 0;
    long channel_I = 0;
    long channel_NO = 0;
    long channel_PWR_DUT = 0;

    double uW = 0;
    double mW = 0;
    double W = 0;

    courantUnit unite;

    bool BOUTON_ON_DUT = false;

public:
    /**************************************************************************
                               CONSTRUCTEUR DE LA CLASSE DUT
       * ************************************************************************/
    Dut(String nomDut);

    /**************************************************************************
                                   GETTER DE LA CLASSE DUT
       * ************************************************************************/

    double get_A();
    double get_mA();
    double get_uA();
    double get_Vin();

    double get_signal_UI();
    double get_signal_MI();
    double get_signal_I();
    double get_signal_PWR_DUT();

    long get_channel_UI();
    long get_channel_MI();
    long get_channel_I();
    long get_channel_NO();
    long get_channel_PWR_DUT();

    double get_uW();
    double get_mW();
    double get_W();

    String get_name();

    volatile int get_flag_state_power();

    courantUnit get_unite();
    bool get_BOUTON_ON_DUT();

    /**************************************************************************
                                   SETTER DE LA CLASSE DUT
       * ************************************************************************/
    void set_A(double setter_A);
    void set_mA(double setter_mA);
    void set_uA(double setter_uA);
    void set_Vin(double setter_Vin);

    void set_signal_UI(double setter_signal_UI);
    void set_signal_MI(double setter_signal_MI);
    void set_signal_I(double setter_signal_I);
    void set_signal_PWR_DUT(double setter_signal_PWR_DUT);

    void set_channel_UI(long setter_channel_UI);
    void set_channel_MI(long setter_channel_MI);
    void set_channel_I(long setter_channel_I);
    void set_channel_NO(long setter_channel_NO);
    void set_channel_PWR_DUT(long setter_channel_PWR_DUT);

    void set_uW(double setter_uW);
    void set_mW(double setter_mW);
    void set_W(double setter_W);
    void set_unite(courantUnit setter_unite);

    void set_SECURITY_MAX_CURRENT(double courant_max);
    void set_courant_max();
    void set_ACC_PWR_PROT(int CMD_ACC, int CMD_PWR, int UI_PROT_STATE);
    void set_flag_state_power(volatile int flag_max_current_name_DUT);
    void set_state_power(volatile int flag_max_current_name_DUT);
    void set_boutonAliementationDut();

    /**************************************************************************
                           METHODE DE LA CLASSE DUT
       * ************************************************************************/

    // float conversion_channel_A();
    // double conversion_channel_microA();
    // float conversion_channel_mA();
    // float conversion_channel_PWR_DUT();

    // Methode pour faire la conversion des valeurs lues dans l'ADC
    void assignation_valeurs_converties();
    void test_assignation_valeurs_converties();

    // Metdode de Test des valeurs lues dans les channel
    void test_channel();

    void reassemblage_valeurs_lue();

    void correctionValeur();

    void checkUniteAdc();

    double get_SECURITY_MAX_CURRENT();
    double get_courant_max;
    volatile int get_state_power();

    void CorrectionValeur();

    void changerEtatAlimDut(int etatAlimDut);

    void test_dut();

    courantUnit checkUnitsAdc();
};

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////DECLARATION DES FONCTIONS////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// void Affichage(float volts, int i);
// float Conversion(long result);

// SPI Fonctions pour lecture ADC
// Fonction pour la selection de canaux au niveau de l'ADC (LTC2428)
void SelectChannel(int n);
// Fonction de lecuture de l'ADC1 (U101 dans les Schémas Electrique)
long SpiReadChannelADC1();
// Fonction de lecuture de l'ADC2 (U201 dans les Schémas Electrique)
long SpiReadChannelADC2();
// Fonction de lecuture de l'ADC3 (U301 dans les Schémas Electrique)
long SpiReadChannelADC3();
// Fonction de lecuture de l'ADC4 (U401 dans les Schémas Electrique)
long SpiReadChannelADC4();
// Fonction de lecuture de l'ADC4 (U401 dans les Schémas Electrique)
long SpiReadChannelADC5();
// Fonction de lecuture de l'ADC4 (U401 dans le s Schémas Electrique)
long SpiReadChannelADC6();

// Fonctions de conversion
float conversion_channel_A(long result);
float conversion_channel_mA(long result);
float conversion_channel_microA(long result);
float conversion_channel_power_in(long result);
float conversion_channel_power_out(long result);

short int verif_nb_cycle();
void changerEtatACC(int etat);

void EnvoiTrame(Dut Adc1, Dut Adc2, Dut Adc3, Dut Adc4, Dut Adc5, Dut Adc6);

void HANDLER_CURRENT_MAX();

void HANDLER_ACC();

void state_CSADC(int etat);

// Declaration des variables  gloobales
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////DECLARATION DES VARIABLES GLOBALE////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

//double COURANT_MAX = 5.0;
double COURANT_MAX_DUT1 = 12.0;
volatile boolean compteur_bouton = false;

long compteur = 0;    // Compteur de seconde pour fonction d'interruption
int uploadconfig = 0; // Etat de la configuration

int statut = 0;            // appareil testé en mode sleep ou awake (dépend de la tension ACC)
int etat_mA_uA = 0;        // 0 on est en mA, si 1 on est en µA //à vérifier, mais doit etre supprimé
int ok_pour_mA = 0;        // Validation d'envoi ou non mA/A //à Supprimer
String inputString = "";   //string de stockage du bus UART (Pi->Arduino)
bool finReception = false; // whether the string is complete

short int nb_cycle = 1;
bool etat_start = 1;
int f_acquisition = 1000;
String conteneur = "";
short int cycle_en_cours = 0;
short int flag_cycle = 0;
short int rep_en_cours = 1;

// Drapeau de l'Etat des DUTs (false car le courant Max n'est pas dépassé)
volatile int FLAG_CURRENT_MAX_DUT1 = false;
volatile int FLAG_CURRENT_MAX_DUT2 = false;
volatile int FLAG_CURRENT_MAX_DUT3 = false;
volatile int FLAG_CURRENT_MAX_DUT4 = false;
volatile int FLAG_CURRENT_MAX_DUT5 = false;
volatile int FLAG_CURRENT_MAX_DUT6 = false;

struct cycle cycle[5] = {{0, 0, "", ""}, {0, 0, "", ""}, {0, 0, "", ""}, {0, 0, "", ""}, {0, 0, "", ""}};
struct amperageMax amperageMaxUser;

amperageMax COURANT_MAX;

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////INITIALISATION PROGRAMME/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void setup()
{

    // Ouverture du port Serial pour l'affichage console du résultat de la conversion
    Serial.begin(115200);
    SerialUSB.begin(115200);

    SPI.begin();

    // Clock DUE = 84 MHz
    SPI.setClockDivider(48); //  Fréquence d'horloge arduino 84MHz Fréquence d'horloge de 1.31MHz  SPI_CLOCK_DIV64

    SPI.setBitOrder(MSBFIRST);  // Most Significant Bit First (Arduino.org pour plus de détails)
    SPI.setDataMode(SPI_MODE0); // SPI Mode_0

    // Initialisation des sorties Chip Select
    digitalWrite(CSADC1, HIGH);
    digitalWrite(CSADC2, HIGH);
    digitalWrite(CSADC3, HIGH);
    digitalWrite(CSADC4, HIGH);
    digitalWrite(CSADC5, HIGH);
    digitalWrite(CSADC6, HIGH);

    pinMode(CSADC1, OUTPUT);
    pinMode(CSADC2, OUTPUT);
    pinMode(CSADC3, OUTPUT);
    pinMode(CSADC4, OUTPUT);
    pinMode(CSADC5, OUTPUT);
    pinMode(CSADC6, OUTPUT);

    // Initialisation des sorties de veille/éveil des produits CMD_ADC_DUT
    digitalWrite(CMD_ACC_DUT1, HIGH);
    digitalWrite(CMD_ACC_DUT2, HIGH);
    digitalWrite(CMD_ACC_DUT3, HIGH);
    digitalWrite(CMD_ACC_DUT4, HIGH);
    digitalWrite(CMD_ACC_DUT5, HIGH);
    digitalWrite(CMD_ACC_DUT6, HIGH);

    pinMode(CMD_ACC_DUT1, OUTPUT);
    pinMode(CMD_ACC_DUT2, OUTPUT);
    pinMode(CMD_ACC_DUT3, OUTPUT);
    pinMode(CMD_ACC_DUT4, OUTPUT);
    pinMode(CMD_ACC_DUT5, OUTPUT);
    pinMode(CMD_ACC_DUT6, OUTPUT);

    // Initialisation des entrées d'états de conversion(µa ou mA/A ?)
    pinMode(UI_PROT_STATE_DUT1, INPUT);
    pinMode(UI_PROT_STATE_DUT2, INPUT);
    pinMode(UI_PROT_STATE_DUT3, INPUT);
    pinMode(UI_PROT_STATE_DUT4, INPUT);
    pinMode(UI_PROT_STATE_DUT5, INPUT);
    pinMode(UI_PROT_STATE_DUT6, INPUT);

    pinMode(CMD_PWR_DUT1, OUTPUT);
    pinMode(CMD_PWR_DUT2, OUTPUT);
    pinMode(CMD_PWR_DUT3, OUTPUT);
    pinMode(CMD_PWR_DUT4, OUTPUT);
    pinMode(CMD_PWR_DUT5, OUTPUT);
    pinMode(CMD_PWR_DUT6, OUTPUT);

    //Serial.println("Low Power Bench Measurement VBRC-K8400");
    // SerialUSB.println("Low Power Bench Measurement VBRC-K8400");

    COURANT_MAX.amax = 5;

    Timer3.getAvailable().attachInterrupt(HANDLER_ACC).setPeriod(1000000).start();
    Timer2.getAvailable().attachInterrupt(HANDLER_CURRENT_MAX).setFrequency(10).start();

    while (!SerialUSB)
    {
        // SerialUSB.println("wait for serial port to connect. Needed for native USB"); //
        Serial.println("wait for serial port to connect. Needed for native USB"); //
    }

    //Serial.println("Designed and developed by Robin Carriou & Vincent Bougouin");
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////FONCTION PRINCIPALE/////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
    Serial.println("\n\n\t*** DEBUT BOUCLE LOOP ***");
    // SerialUSB.println("\n\n\t*** DEBUT BOUCLE LOOP USB***");

    Dut dut1("DUT 1");
    Dut dut2("DUT 2");
    Dut dut3("DUT 3");
    Dut dut4("DUT 4");
    Dut dut5("DUT 5");
    Dut dut6("DUT 6");
    Dut dutt("Dut de test");

    // On initialise les entrées et sorties des Duts
    dut1.set_ACC_PWR_PROT(CMD_ACC_DUT1, CMD_PWR_DUT1, UI_PROT_STATE_DUT1);
    dut2.set_ACC_PWR_PROT(CMD_ACC_DUT2, CMD_PWR_DUT2, UI_PROT_STATE_DUT2);
    dut3.set_ACC_PWR_PROT(CMD_ACC_DUT3, CMD_PWR_DUT3, UI_PROT_STATE_DUT3);
    dut4.set_ACC_PWR_PROT(CMD_ACC_DUT4, CMD_PWR_DUT4, UI_PROT_STATE_DUT4);
    dut5.set_ACC_PWR_PROT(CMD_ACC_DUT5, CMD_PWR_DUT5, UI_PROT_STATE_DUT5);
    dut6.set_ACC_PWR_PROT(CMD_ACC_DUT6, CMD_PWR_DUT6, UI_PROT_STATE_DUT6);
    dutt.set_ACC_PWR_PROT(CMD_ACC_DUT1, CMD_PWR_DUT1, UI_PROT_STATE_DUT1);

    // Initialisation des flag state POWER dut
    dut1.set_flag_state_power(FLAG_CURRENT_MAX_DUT1);
    dut2.set_flag_state_power(FLAG_CURRENT_MAX_DUT2);
    dut3.set_flag_state_power(FLAG_CURRENT_MAX_DUT3);
    dut4.set_flag_state_power(FLAG_CURRENT_MAX_DUT4);
    dut5.set_flag_state_power(FLAG_CURRENT_MAX_DUT5);
    dut6.set_flag_state_power(FLAG_CURRENT_MAX_DUT6);
    dutt.set_flag_state_power(FLAG_CURRENT_MAX_DUT6);

    int start = millis();
    int duree = 0;
    double channel_Vin = 0;
    boolean start_mesure = false;

    boolean quitter = false;

       while (SerialUSB.available() && !finReception)
    {
        /**
     * Cette boucle s'active si l'arduino commence à recevoir des données sur son port uart (données envoyé par la pi). on receptionne les données sous forme 
     * de string (inputString), puis lorsque la reception est fini, on analyse la chaine de caractère. selon le format suivant :
     * ->la première lettre est un s : l'user a "start" l'acquisition, on reçoit donc les paramètres de time_awake et time_sleep
     * ->la première lettre est un "p" : l'user a mis en pause l'acquisition : il faut donc....
     * 
     */
        // get the new byte:
        char inChar = (char)SerialUSB.read();
        // add it to the inputString:
        inputString += inChar;

        Serial.println(inputString);
        // if the incoming character is a newline, set a flag so the main loop can
        // do something about it:
        if (inChar == '\n')
        {
            finReception = true;
        }
        if (finReception)
        {
            Serial.println(inputString);
            switch (inputString[0])
            {
            /**
         * On regarde la première lettre des données reçues
         */
            case 's':

                Serial.print("inputString: ");
                Serial.println(inputString);

                cycle[0].time_awake_str = inputString.substring(1, 7);
                cycle[0].time_sleep_str = inputString.substring(7, 13);
                cycle[0].time_awake = cycle[0].time_awake_str.toInt();
                cycle[0].time_sleep = cycle[0].time_sleep_str.toInt();

                Serial.print("cycle[0].time_awake_str: ");
                Serial.println(cycle[0].time_awake_str);
                Serial.print("cycle[0].time_sleep_str: ");
                Serial.println(cycle[0].time_sleep_str);

                cycle[1].time_awake_str = inputString.substring(13, 19);
                cycle[1].time_sleep_str = inputString.substring(19, 25);
                cycle[1].time_awake = cycle[1].time_awake_str.toInt();
                cycle[1].time_sleep = cycle[1].time_sleep_str.toInt();

                Serial.print("cycle[1].time_awake_str: ");
                Serial.println(cycle[1].time_awake_str);
                Serial.print("cycle[1].time_sleep_str: ");
                Serial.println(cycle[1].time_sleep_str);

                cycle[2].time_awake_str = inputString.substring(25, 31);
                cycle[2].time_sleep_str = inputString.substring(31, 37);
                cycle[2].time_awake = cycle[2].time_awake_str.toInt();
                cycle[2].time_sleep = cycle[2].time_sleep_str.toInt();

                Serial.print("cycle[2].time_awake_str: ");
                Serial.println(cycle[2].time_awake_str);
                Serial.print("cycle[2].time_sleep_str: ");
                Serial.println(cycle[2].time_sleep_str);

                cycle[3].time_awake_str = inputString.substring(37, 43);
                cycle[3].time_sleep_str = inputString.substring(43, 49);
                cycle[3].time_awake = cycle[3].time_awake_str.toInt();
                cycle[3].time_sleep = cycle[3].time_sleep_str.toInt();

                Serial.print("cycle[3].time_awake_str: ");
                Serial.println(cycle[3].time_awake_str);
                Serial.print("cycle[3].time_sleep_str: ");
                Serial.println(cycle[3].time_sleep_str);

                cycle[4].time_awake_str = inputString.substring(49, 55);
                cycle[4].time_sleep_str = inputString.substring(55, 61);
                cycle[4].time_awake = cycle[4].time_awake_str.toInt();
                cycle[4].time_sleep = cycle[4].time_sleep_str.toInt();

                Serial.print("cycle[4].time_awake_str: ");
                Serial.println(cycle[4].time_awake_str);
                Serial.print("cycle[4].time_sleep_str: ");
                Serial.println(cycle[4].time_sleep_str);

                /*conteneur=inputString.substring(37,40);
          f_acquisition=conteneur.toInt();
          f_acquisition=f_acquisition*1000;
          f_acquisition-=50;
          conteneur="";*/

                conteneur = inputString.substring(61, 62);
                etat_start = conteneur.toInt();
                cycle_en_cours = 0;
                flag_cycle = 0;
                rep_en_cours = 1;

                Serial.print("etat_start: ");
                Serial.println(etat_start);

                cycle[0].nb_rep_str = inputString.substring(62, 64);
                cycle[1].nb_rep_str = inputString.substring(64, 66);
                cycle[2].nb_rep_str = inputString.substring(66, 68);
                cycle[3].nb_rep_str = inputString.substring(68, 70);
                cycle[4].nb_rep_str = inputString.substring(70, 72);

                cycle[0].nb_rep = cycle[0].nb_rep_str.toInt();
                cycle[1].nb_rep = cycle[1].nb_rep_str.toInt();
                cycle[2].nb_rep = cycle[2].nb_rep_str.toInt();
                cycle[3].nb_rep = cycle[1].nb_rep_str.toInt();
                cycle[4].nb_rep = cycle[2].nb_rep_str.toInt();

                COURANT_MAX.amax_str = inputString.substring(72, 75);
                COURANT_MAX.amax = COURANT_MAX.amax_str.toFloat();

                Serial.print("cycle[0].nb_rep_str: ");
                Serial.println(cycle[0].nb_rep_str);
                Serial.print("cycle[1].nb_rep_str ");
                Serial.println(cycle[1].nb_rep_str);
                Serial.print("cycle[2].nb_rep_str ");
                Serial.println(cycle[2].nb_rep_str);
                Serial.print("cycle[3].nb_rep_str ");
                Serial.println(cycle[3].nb_rep_str);
                Serial.print("cycle[4].nb_rep_str ");
                Serial.println(cycle[4].nb_rep_str);

                Serial.print("\n\nLa valeur lu du courant max vaut: ");
                Serial.println(COURANT_MAX.amax_str);
                Serial.print("Amperage max vaut: ");
                Serial.println(COURANT_MAX.amax);

                //DEBUG//
                /*
          SerialUSB.print("awake 1 : ");
          SerialUSB.println(cycle[0].time_awake);
          SerialUSB.print("sleep 1 : ");
          SerialUSB.println(cycle[0].time_sleep);
          
          SerialUSB.print("awake 2 : ");
          SerialUSB.println(cycle[1].time_awake);
          SerialUSB.print("sleep 2 : ");
          SerialUSB.println(cycle[1].time_sleep);

          SerialUSB.print("awake 3 : ");
          SerialUSB.println(cycle[2].time_awake);
          SerialUSB.print("sleep 3 : ");
          SerialUSB.println(cycle[2].time_sleep);

          SerialUSB.print("frequence : ");
          SerialUSB.println(f_acquisition);

          SerialUSB.print("start : ");
          SerialUSB.println(etat_start);

          SerialUSB.print("repetition 1er cycle : ");
          SerialUSB.println(cycle[0].nb_rep);
          SerialUSB.print("repetition 2eme cycle : ");
          SerialUSB.println(cycle[1].nb_rep);
          SerialUSB.print("repetition 3eme cycle : ");
          SerialUSB.println(cycle[2].nb_rep);
          */

                if (etat_start == 1)
                {
                    changerEtatACC(HIGH);
                }
                if (etat_start == 0)
                {
                    changerEtatACC(LOW);
                }

                nb_cycle = verif_nb_cycle();
                /*SerialUSB.print("nb cycle : ");
                 SerialUSB.println(nb_cycle);*/
                uploadconfig = true;
                SerialUSB.print("ok\n");
                Serial.println("OK TRANSMISSION");
                break;

            case 'p':
                uploadconfig = false;
                changerEtatACC(LOW);
                pause_DUT(dut1, dut2, dut3, dut4, dut5, dut6);
                cycle_en_cours = 0;
                flag_cycle = 0;

                SerialUSB.print("ok\n");
                break;

            case 'd':
                SerialUSB.print("d");
                SerialUSB.print("test debug");
                SerialUSB.print("\n");

            default:
                SerialUSB.print("problème : l'arduino ne reconnait pas la donnée reçue : ");
                SerialUSB.println(inputString);
                break;
            }
            inputString = "";
            finReception = false;
            quitter = true;
        }
    }


    dut1.set_channel_UI(SpiRead(0, 0));
    dut2.set_channel_UI(SpiRead(1, 0));
    dut3.set_channel_UI(SpiRead(2, 0));
    dut4.set_channel_UI(SpiRead(3, 0));
    dut5.set_channel_UI(SpiRead(4, 0));
    dut6.set_channel_UI(SpiRead(5, 0));

    dut1.set_channel_MI(SpiRead(0, 1));
    dut2.set_channel_MI(SpiRead(1, 1));
    dut3.set_channel_MI(SpiRead(2, 1));
    dut4.set_channel_MI(SpiRead(3, 1));
    dut5.set_channel_MI(SpiRead(4, 1));
    dut6.set_channel_MI(SpiRead(5, 1));

    dut1.set_channel_I(SpiRead(0, 2));
    dut2.set_channel_I(SpiRead(1, 2));
    dut3.set_channel_I(SpiRead(2, 2));
    dut4.set_channel_I(SpiRead(3, 2));
    dut5.set_channel_I(SpiRead(4, 2));
    dut6.set_channel_I(SpiRead(5, 2));
    
    dut1.set_channel_PWR_DUT(SpiRead(0, 3));
    dut2.set_channel_PWR_DUT(SpiRead(1, 3));
    dut3.set_channel_PWR_DUT(SpiRead(2, 3));
    dut4.set_channel_PWR_DUT(SpiRead(3, 3));
    dut5.set_channel_PWR_DUT(SpiRead(4, 3));
    dut6.set_channel_PWR_DUT(SpiRead(5, 3));

    dut1.set_channel_NO(SpiRead(0, 4));
    dut2.set_channel_NO(SpiRead(1, 4));
    dut3.set_channel_NO(SpiRead(2, 4));
    dut4.set_channel_NO(SpiRead(3, 4));
    dut5.set_channel_NO(SpiRead(4, 4));
    dut6.set_channel_NO(SpiRead(5, 4));


    dut1.reassemblage_valeurs_lue();
    dut2.reassemblage_valeurs_lue();
    dut3.reassemblage_valeurs_lue();
    dut4.reassemblage_valeurs_lue();
    dut5.reassemblage_valeurs_lue();
    dut6.reassemblage_valeurs_lue();

    // dut1.test_channel();
    // dut2.test_channel();
    // dut3.test_channel();
    // dut4.test_channel();
    // dut5.test_channel();
    // dut6.test_channel();
    // dutt.test_channel();

    dut1.assignation_valeurs_converties();
    dut2.assignation_valeurs_converties();
    dut3.assignation_valeurs_converties();
    dut4.assignation_valeurs_converties();
    dut5.assignation_valeurs_converties();
    dut6.assignation_valeurs_converties();

    // dut1.test_assignation_valeurs_converties();
    // dut2.test_assignation_valeurs_converties();
    // dut3.test_assignation_valeurs_converties();
    // dut4.test_assignation_valeurs_converties();
    // dut5.test_assignation_valeurs_converties();
    // dut6.test_assignation_valeurs_converties();

    EnvoiTrame(dut1, dut2, dut3, dut4, dut5, dut6);

    Serial.println("\nFin LOOP \n\n\n");

    delay(50);
}

void HANDLER_CURRENT_MAX()
{

    //Serial.println("\t*******  Handler  1*********");
    if (FLAG_CURRENT_MAX_DUT1 == true || FLAG_CURRENT_MAX_DUT2 == true || FLAG_CURRENT_MAX_DUT3 == true ||
        FLAG_CURRENT_MAX_DUT4 == true || FLAG_CURRENT_MAX_DUT5 == true || FLAG_CURRENT_MAX_DUT6 == true)
    {
        digitalWrite(CMD_PWR_DUT1, LOW);
        digitalWrite(CMD_PWR_DUT2, LOW);
        digitalWrite(CMD_PWR_DUT3, LOW);
        digitalWrite(CMD_PWR_DUT4, LOW);
        digitalWrite(CMD_PWR_DUT5, LOW);
        digitalWrite(CMD_PWR_DUT6, LOW);
    }
    else if (FLAG_CURRENT_MAX_DUT1 == false || FLAG_CURRENT_MAX_DUT2 == false || FLAG_CURRENT_MAX_DUT3 == false ||
             FLAG_CURRENT_MAX_DUT4 == false || FLAG_CURRENT_MAX_DUT5 == false || FLAG_CURRENT_MAX_DUT6 == false)
    {
        digitalWrite(CMD_PWR_DUT1, HIGH);
        digitalWrite(CMD_PWR_DUT2, HIGH);
        digitalWrite(CMD_PWR_DUT3, HIGH);
        digitalWrite(CMD_PWR_DUT4, HIGH);
        digitalWrite(CMD_PWR_DUT5, HIGH);
        digitalWrite(CMD_PWR_DUT6, HIGH);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////CONVERSION CHANNEL A///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
float conversion_channel_A(long result)
/**
 * Le composant utilisé pour la mesure de courant en ampère est l'ACS713
 * Il a une sensibilité en sortie de 185 mV/A
 * Tension en sortie à vide: Vs_vide = 0.505 V
 * Valeur de la tension à vide en bits: 320927
 * Amplification: gain = 3
 * 
 * Pour avoir la bonne tension en mesure, il faut retrancher au Vs le Vs_vide
 */ 
{
    double resolution = 1048575;
    double uMax = 4.098;
    double gain = 3;
    double Vs_vide = 0.505;
    double sensibility = 185;

    long Vs_vide_bit = ( (Vs_vide * gain) * resolution) / uMax;

    result = result - Vs_vide_bit;
    courant_A = result / gain;
    courant_A = (result * (uMax/resolution));

    // Retour de la valeur convertie
    return courant_A;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////CONVERSION CHANNEL mA//////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
float conversion_channel_mA(long result)
{
    double courant_mA = 0;

    double resolution = 1048576;
    double resistanceShunt = 0.020;
    double uMax = 4.098;
    double span = 0.00000390625; //(uMax/resolution);
    int gain = 250 * 3;

    courant_mA = (result * (uMax/resolution));

    courant_mA = (courant_mA * 1000) / (resistanceShunt * gain);
    // Retour de la valeur convertie
    return courant_mA;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////CONVERSION CHANNEL µA//////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
float conversion_channel_microA(long result)
{
    double courant_microA;

    double resolution = 1048576;
    double resistanceShunt = 10;
    double uMax = 4.098;
    double span = 0.00000390625; //(uMax/resolution);
    int gain = 250 * 3;
 

    courant_microA = (result * (uMax/resolution));
    courant_microA = (courant_microA * 1000000) / (resistanceShunt * gain);

    // Retour de la valeur convertie     0.00000
    return courant_microA;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////CONVERSION CHANNEL Power In////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
float conversion_channel_power_in(long result)
{
    // Déclaration de la variable pour la tension
    double power_in;
    double resolution = 1048576;
    double uMax = 4.098;

    power_in = (result * (uMax/resolution));

    // Retour de la valeur convertie
    return power_in;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////CONVERSION CHANNEL Power out////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
float conversion_channel_power_out(long result)
{
    // Déclaration de la variable pour la tension
    double power_out;
    double resolution = 1048576;
    double uMax = 4.098;

    power_out = (result * (uMax/resolution));

    // Retour de la valeur convertie
    return power_out;
}


/**************************************************************************
                        Fonction pour vérifier le nombre de Cycle
  Dernière modification 28/10/2019 à 14h07 par Aslam BARWANE
*************************************************************************/
short int verif_nb_cycle()
{
    nb_cycle = 1;
    // if ((cycle[1].time_awake != 0) || (cycle[1].time_sleep != 0))
    // {
    //     nb_cycle++;
    // }

    // if ((cycle[2].time_awake != 0) || (cycle[2].time_sleep != 0))
    // {
    //     nb_cycle++;
    // }

    for (int i = 1; i < 5; i++)
    {
        if ((cycle[i].time_awake != 0) || (cycle[i].time_sleep != 0))
        {
            nb_cycle++;
        }
    }
    return nb_cycle;
}

/**************************************************************************
                        Fonction pour Changer l'état de l'ACC
  Dernière modification 28/10/2019 à 12h07 par Aslam BARWANE
*************************************************************************/
void changerEtatACC(int etat)
{
    /**
     Permet de change l'état de l'ACC
  */
    digitalWrite(CMD_ACC_DUT1, etat);
    digitalWrite(CMD_ACC_DUT2, etat);
    digitalWrite(CMD_ACC_DUT3, etat);
    digitalWrite(CMD_ACC_DUT4, etat);
    digitalWrite(CMD_ACC_DUT5, etat);
    digitalWrite(CMD_ACC_DUT6, etat);

    statut = etat;
    compteur = 0;
}

void pause_DUT(Dut dut1, Dut dut2, Dut dut3, Dut dut4, Dut dut5, Dut dut6)
{
    Dut tableauDeDut[6] = {dut1, dut2, dut3, dut4, dut5, dut6};
    for (int n = 0; n < 6; n++)
    {
        tableauDeDut[n].set_flag_state_power(true);
    }
}

/**************************************************************************
                        Fonction pour l'envoi de La trame
  Dernière modification 28/10/2019 à 12h07 par Aslam BARWANE
*************************************************************************/
void EnvoiTrame(Dut Adc1, Dut Adc2, Dut Adc3, Dut Adc4, Dut Adc5, Dut Adc6)
/**
   Ici, on reçoit nos 4 structures ADC en paramètres, et on envoie la trame UART de mesure vers la pi.
*/
{
    float P[6] = {0};
    float A[6] = {0};
    bool etat_Alimentation_duts[6];

    switch (Adc1.get_unite())
    {
    case 0:
        P[0] = Adc1.get_uW();
        A[0] = Adc1.get_uA();
        break;

    case 1:
        P[0] = Adc1.get_mW();
        A[0] = Adc1.get_mA();
        break;

    case 2:
        P[0] = Adc1.get_W();
        A[0] = Adc1.get_A();
        break;

    default:
        //SerialUSB.println("Arduino: Envoi Trame: aucune unité favorisé pour DUT1");
        P[0] = 0;
        A[0] = 0;
        break;
    }

    switch (Adc2.get_unite())
    {
    case 0:
        P[1] = Adc2.get_uW();
        A[1] = Adc2.get_uA();
        break;

    case 1:
        P[1] = Adc2.get_mW();
        A[1] = Adc2.get_mA();
        break;

    case 2:
        P[1] = Adc2.get_W();
        A[1] = Adc2.get_A();
        break;

    default:
        //SerialUSB.println("Arduino: Envoi Trame: aucune unité favorisé  pour DUT2");
        P[1] = 0;
        A[1] = 0;
        break;
    }

    switch (Adc3.get_unite())
    {
    case 0:
        P[2] = Adc3.get_uW();
        A[2] = Adc3.get_uA();
        break;

    case 1:
        P[2] = Adc3.get_mW();
        A[2] = Adc3.get_mA();
        break;

    case 2:
        P[2] = Adc3.get_W();
        A[2] = Adc3.get_A();
        break;

    default:
        //SerialUSB.println("Arduino: Envoi Trame: aucune unité favorisé pour DUT3");
        P[2] = 0;
        A[2] = 0;
        break;
    }

    switch (Adc4.get_unite())
    {
    case 0:
        P[3] = Adc4.get_uW();
        A[3] = Adc4.get_uA();
        break;

    case 1:
        P[3] = Adc4.get_mW();
        A[3] = Adc4.get_mA();
        break;

    case 2:
        P[3] = Adc4.get_W();
        A[3] = Adc4.get_A();
        break;

    default:
        //SerialUSB.println("Arduino: Envoi Trame: aucune unité favorisé  pour DUT4");
        P[3] = 0;
        A[3] = 0;
        break;
    }

    switch (Adc5.get_unite())
    {
    case 0:
        P[4] = Adc5.get_uW();
        A[4] = Adc5.get_uA();
        break;

    case 1:
        P[4] = Adc5.get_mW();
        A[4] = Adc5.get_mA();
        break;

    case 2:
        P[4] = Adc5.get_W();
        A[4] = Adc5.get_A();
        break;

    default:
        //SerialUSB.println("Arduino: Envoi Trame: aucune unité favorisé pour DUT5");
        P[4] = 0;
        A[4] = 0;
        break;
    }
    switch (Adc6.get_unite())
    {
    case 0:
        P[5] = Adc6.get_uW();
        A[5] = Adc6.get_uA();
        break;

    case 1:
        P[5] = Adc6.get_mW();
        A[5] = Adc6.get_mA();
        break;

    case 2:
        P[5] = Adc6.get_W();
        A[5] = Adc6.get_A();
        break;

    default:
        //SerialUSB.println("Arduino: Envoi Trame: aucune unité favorisé  pour DUT6");
        P[5] = 0;
        A[5] = 0;
        break;
    }

    Serial.println("J'ai casé les valeurs dans le tableau\n\nJ'envois les valeurs dans le port Série");

    SerialUSB.print(statut);
    SerialUSB.print(":");

    SerialUSB.print(Adc1.get_unite());
    SerialUSB.print(":");
    SerialUSB.print(P[0]);
    SerialUSB.print(":");
    SerialUSB.print(A[0]);
    SerialUSB.print(":");

    SerialUSB.print(Adc2.get_unite());
    SerialUSB.print(":");
    SerialUSB.print(P[1]);
    SerialUSB.print(":");
    SerialUSB.print(A[1]);
    SerialUSB.print(":");

    SerialUSB.print(Adc3.get_unite());
    SerialUSB.print(":");
    SerialUSB.print(P[2]);
    SerialUSB.print(":");
    SerialUSB.print(A[2]);
    SerialUSB.print(":");

    SerialUSB.print(Adc4.get_unite());
    SerialUSB.print(":");
    SerialUSB.print(P[3]);
    SerialUSB.print(":");
    SerialUSB.print(A[3]);
    SerialUSB.print(":");

    SerialUSB.print(Adc5.get_unite());
    SerialUSB.print(":");
    SerialUSB.print(P[4]);
    SerialUSB.print(":");
    SerialUSB.print(A[4]);
    SerialUSB.print(":");

    SerialUSB.print(Adc6.get_unite());
    SerialUSB.print(":");
    SerialUSB.print(P[5]);
    SerialUSB.print(":");
    SerialUSB.print(A[5]);

    SerialUSB.print(":\r\n");

    // Serial.println("J'ai envoyé les valeurs via le port série");

    // // Debug trame
    // Serial.println("Debug Trame\n");
    // Serial.print(statut);
    // Serial.print(":");

    // Serial.print(Adc1.get_unite());
    // Serial.print(":");
    // Serial.print(P[0]);
    // Serial.print(":");
    // Serial.print(A[0]);
    // Serial.print(":");

    // Serial.print(Adc2.get_unite());
    // Serial.print(":");
    // Serial.print(P[1]);
    // Serial.print(":");
    // Serial.print(A[1]);
    // Serial.print(":");

    // Serial.print(Adc3.get_unite());
    // Serial.print(":");
    // Serial.print(P[2]);
    // Serial.print(":");
    // Serial.print(A[2]);
    // Serial.print(":");

    // Serial.print(Adc4.get_unite());
    // Serial.print(":");
    // Serial.print(P[3]);
    // Serial.print(":");
    // Serial.print(A[3]);
    // Serial.print(":");

    // Serial.print(Adc5.get_unite());
    // Serial.print(":");
    // Serial.print(P[4]);
    // Serial.print(":");
    // Serial.print(A[4]);
    // Serial.print(":");

    // Serial.print(Adc6.get_unite());
    // Serial.print(":");
    // Serial.print(P[5]);
    // Serial.print(":");
    // Serial.print(A[5]);

    Serial.print("\r\n");
}

/**************************************************************************
                        CONSTRUCTEUR DE LA CLASSE DUT
  Dernière modification 24/10/2019 à 13h15 par Aslam BARWANE
*************************************************************************/
Dut::Dut(String nomDut) : nom(nomDut), uA(0), mA(0), A(0), Vin(0),
                          CMD_ACC_DUT(0), CMD_PWR_DUT(0), UI_PROT_STATE_DUT(0), flag_state_power(0),
                          channel_UI(0), channel_MI(0), channel_I(0), channel_NO(0), channel_PWR_DUT(0),
                          uW(0), mW(0), W(0)
{
}

/**************************************************************************
                            GETTER DE LA CLASSE DUT
  Dernière modification 24/10/2019 à 13h15 par Aslam BARWANE
* ************************************************************************/
String Dut::get_name()
{
    return nom;
}

double Dut::get_A()
{
    return A;
}

double Dut::get_mA()
{
    return mA;
}

double Dut::get_uA()
{
    return uA;
}

double Dut::get_Vin()
{
    return Vin;
}

double Dut::get_signal_UI()
{
    return signal_UI;
}

double Dut::get_signal_MI()
{
    return signal_MI;
}

double Dut::get_signal_I()
{
    return signal_I;
}

double Dut::get_signal_PWR_DUT()
{
    return signal_PWR_DUT;
}

long Dut::get_channel_UI()
{
    return channel_UI;
}

long Dut::get_channel_MI()
{
    return channel_MI;
}

long Dut::get_channel_I()
{
    return channel_I;
}

long Dut::get_channel_NO()
{
    return channel_NO;
}

long Dut::get_channel_PWR_DUT()
{
    return channel_PWR_DUT;
}

double Dut::get_uW()
{
    return uW;
}

double Dut::get_mW()
{
    return mW;
}

double Dut::get_W()
{
    return W;
}

courantUnit Dut::get_unite()
{
    return unite;
}

volatile int Dut::get_flag_state_power()
{
    return flag_state_power;
}

/**************************************************************************
                      METHODES DE LA CLASSE DUT
  Dernière modification 24/10/2019 à 13h15 par Aslam BARWANE
* ************************************************************************/
void Dut::set_A(double setter_A)
{
    A = setter_A;
}

void Dut::set_mA(double setter_mA)
{
    mA = setter_mA;
}

void Dut::set_uA(double setter_uA)
{
    uA = setter_uA;
}

void Dut::set_Vin(double setter_Vin)
{
    Vin = setter_Vin;
}

void Dut::set_signal_UI(double setter_signal_UI)
{
    signal_UI = setter_signal_UI;
}

void Dut::set_signal_MI(double setter_signal_MI)
{
    signal_MI = setter_signal_MI;
}

void Dut::set_signal_I(double setter_signal_I)
{
    signal_I = setter_signal_I;
}

void Dut::set_signal_PWR_DUT(double setter_signal_PWR_DUT)
{
    signal_PWR_DUT = setter_signal_PWR_DUT;
}

void Dut::set_channel_UI(long setter_channel_UI)
{
    channel_UI = setter_channel_UI;

    // Serial.print("\n\t\tChannel UI interne du ");
    // Serial.print(nom);
    // Serial.print("  ");
    // Serial.print(channel_UI);
    // Serial.println("");
}

void Dut::set_channel_MI(long setter_channel_MI)
{
    channel_MI = setter_channel_MI;

    // Serial.print("\n\t\tChannel MI interne du ");
    // Serial.print(nom);
    // Serial.print("  ");
    // Serial.print(channel_MI);
    // Serial.println("");
}

void Dut::set_channel_I(long setter_channel_I)
{
    channel_I = setter_channel_I;

    // Serial.print("\n\t\tChannel I interne du ");
    // Serial.print(nom);
    // Serial.print("  ");
    // Serial.print(channel_I);
    // Serial.println("");
}

void Dut::set_channel_NO(long setter_channel_NO)
{
    channel_NO = setter_channel_NO;

    // Serial.print("\n\t\tChannel NO interne du ");
    // Serial.print(nom);
    // Serial.print("  ");
    // Serial.print(channel_NO);
    // Serial.println("");
}

void Dut::set_channel_PWR_DUT(long setter_channel_PWR_DUT)
{
    channel_PWR_DUT = setter_channel_PWR_DUT;

    // Serial.print("\n\t\tChannel PWR_DUT interne du ");
    // Serial.print(nom);
    // Serial.print("  ");
    // Serial.print(channel_PWR_DUT);
    // Serial.println("");
}

void Dut::set_uW(double setter_uW)
{
    uW = setter_uW;
}

void Dut::set_mW(double setter_mW)
{
    mW = setter_mW;
}

void Dut::set_W(double setter_W)
{
    W = setter_W;
}

void Dut::set_unite(courantUnit setter_unite)
{
    unite = setter_unite;
}

void Dut::set_ACC_PWR_PROT(int CMD_ACC, int CMD_PWR, int UI_PROT_STATE)
{
    CMD_ACC_DUT = CMD_ACC;
    CMD_PWR_DUT = CMD_PWR;
    UI_PROT_STATE_DUT = UI_PROT_STATE;
}

void Dut::set_flag_state_power(volatile int flag_max_current_name_DUT)
{
    flag_state_power = flag_max_current_name_DUT;
}

void Dut::set_SECURITY_MAX_CURRENT(double courant_max)
{
}

void Dut::set_boutonAliementationDut()
{
}
void Dut::set_courant_max()
{
}

/**************************************************************************
                      METHODES DE LA CLASSE DUT
  Dernière modification 24/10/2019 à 13h15 par Aslam BARWANE
* ************************************************************************/
/*Methodes de conversion des valeurs lues dans l'ADC*/

void Dut::assignation_valeurs_converties()
{
    // Dernière modification 24/10/2019 à 13h15 par Aslam BARWANE
    // set_A(conversion_channel_A(get_channel_I()));
    // set_mA(conversion_channel_mA(get_channel_MI()));
    // set_uA(conversion_channel_microA(get_channel_UI()));
    // set_Vin(conversion_channel_power_in(get_channel_PWR_DUT()));

    A = conversion_channel_A(get_channel_I());
    mA = conversion_channel_mA(get_channel_MI());
    uA = conversion_channel_microA(get_channel_UI());
    Vin = conversion_channel_power_in(get_channel_PWR_DUT());

    // if(uA<3){
    //     uA = 0;
    // }

    // On applique le coefficient de correction des valeur
    // Dernière modification 24/10/2019 à 14h24 par Aslam BARWANE
    correctionValeur();

    checkUniteAdc();

    if (A > COURANT_MAX.amax)
    {
        flag_state_power = true;
    }
    else
    {
        flag_state_power = false;
    }
}

/*Test de la methodes de conversion des valeurs lues dans l'ADC*/
void Dut::test_assignation_valeurs_converties()
{
    // Dernière modification 24/10/2019 à 13h15 par Aslam BARWANE
    Serial.print("\n\n****** TEST VALEURS LUES ET CONVERTIES dans le ");

    Serial.print(get_name());
    Serial.println(" ******");

    Serial.print("valeur dans uA : ");
    Serial.println(get_uA());

    Serial.print("valeur dans mA : ");
    Serial.println(get_mA());

    Serial.print("valeur dans A : ");
    Serial.println(get_A());

    Serial.print("valeur dans Vin : ");
    Serial.println(get_Vin());

    Serial.println("\tFin de lecture des assignastion des valeur du ");
    Serial.print(get_name());

    Serial.print("valeur dans UNITE : ");
    Serial.println(get_unite());
}

void Dut::test_channel()
{
    //Dernière modification 24/10/2019 à 13h15 par Aslam BARWANE
    Serial.print("\n\n****** TEST DES VALEURS LUES DANS LES CHANNEL DU ");
    Serial.print(nom);
    Serial.println(" ******");

    Serial.print("channel_UI :");
    Serial.println(get_channel_UI());

    Serial.print("channel_MI : ");
    Serial.println(get_channel_MI());

    Serial.print("channel_I : ");
    Serial.println(get_channel_I());

    Serial.print("channel_NO : ");
    Serial.println(get_channel_NO());

    Serial.print("channel_PWR_DUT : ");
    Serial.println(get_channel_PWR_DUT());

    Serial.print("\tFIN TEST DES VALEURS LUES DANS LES CHANNEL du ");
    Serial.print(get_name());
}

/*Methode pour la mise en place d'un coefficient correcteur du mA et de l'A
  Dans la version 2016 du code, des offset était appliqué au code pour modifier des imprécisions de mesure vérifiées par experimentation.
  Ceux ci sont appliqué dans cette fonction
  Dernière modification 24/10/2019 à 14h20 par Aslam BARWANE
*/
void Dut::correctionValeur()
{
    mA = mA - 0.35;
    A = A - 0.35;
}

/**
  fonction permettant de choisir la meilleur unité à utiliser en fonctiond du courant reçu
*/
void Dut::checkUniteAdc()
{
    courantUnit check = unknown;
    set_unite(check);

    // SerialUSB.println("*************** CheckUnitAdc **************** ");
    // SerialUSB.println(" ");
    // SerialUSB.println(" On vas prendre les get ");
    // SerialUSB.print("get_A(): ");
    // SerialUSB.println(get_A());

    // SerialUSB.print("get_mA(): ");
    // SerialUSB.println(get_mA());

    // SerialUSB.print("get_uA(): ");
    // SerialUSB.println(get_uA());

    if (get_A() > 0.5)
    {
        unite = cu_A;
        //SerialUSB.println("adc.A>0.5");
    }
    else if (get_mA() >= 1.4)
    {
        unite = cu_mA;
        //SerialUSB.println("unite : mA");
        //SerialUSB.println(adc.mA);
    }
    else if (get_mA() < 1.4)
    {
        unite = cu_uA;
        //SerialUSB.println("unite : µA");
        //SerialUSB.println("unite : uA");
    }
    else
    {
        unite = unknown;
        //SerialUSB.println("unknown   (checkUniteAdc)");
    }
    // SerialUSB.print("Unite: ");
    // SerialUSB.println(unite);
    //return unite;
}

void Dut::reassemblage_valeurs_lue()
{
    long channelUI, channelMI, channelI, channelPwrDut;
    channelUI = channel_MI;
    channelMI = channel_I;
    channelI = channel_PWR_DUT;
    channelPwrDut = channel_PWR_DUT;

    channel_UI = channelUI;
    channel_MI = channelMI;
    channel_I = channelI;
    channel_PWR_DUT = channelPwrDut;

}

void state_CSADC(int etat)
/**
   Les 4 ADC du banc ont un multiplexeur integré, cette fonction nous permet de le set pour récuperer un channel en particulier de l'ADC
   (les 4 channels à récuperer sont : tension d'entrée, courant en uA, mA, et A)
*/
{
    // Tableau des Codes Hexa sélection de channel
    int channel[8] = {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}; //
    int CSADC_channel[6] = {CSADC1, CSADC2, CSADC3, CSADC4, CSADC5, CSADC6};

    for (unsigned char i = 0; i < 4; i++)
    {
        // Chip Select à l'état haut pour la sélection du channel
        digitalWrite(CSADC_channel[i], etat); // CSADC à 1, pas de sortie

        digitalWrite(CSADC_channel[i], etat); // Uniquement le CSADC à 1 de l'ADC dont on souhaite mofidier le channel
    }
}

void verifCycle()
{
    if (flag_cycle == 2)
    {
        if (rep_en_cours == cycle[cycle_en_cours].nb_rep)
        {
            if (cycle_en_cours + 1 == nb_cycle)
            {
                cycle_en_cours = 0;
            }
            else
            {
                cycle_en_cours++;
            }
            rep_en_cours = 0;
        }

        flag_cycle = 0;
        rep_en_cours++;
    }
}

void HANDLER_ACC()
{
    compteur++;

    if (uploadconfig)
    {

        if (nb_cycle == 1)
        {
            if ((statut == 0) && (compteur >= cycle[0].time_sleep))
            {
                changerEtatACC(HIGH);
            }
            if ((statut == 1) && (compteur >= cycle[0].time_awake))
            {
                changerEtatACC(LOW);
            }
        }
        else
        {
            if (uploadconfig)
            {
                if ((statut == 0) && (compteur >= cycle[cycle_en_cours].time_sleep))
                {
                    changerEtatACC(HIGH);
                    flag_cycle++;
                    verifCycle();
                }
                if ((statut == 1) && (compteur >= cycle[cycle_en_cours].time_awake))
                {
                    changerEtatACC(LOW);
                    flag_cycle++;
                    verifCycle();
                }
            }
        }
    }
}



/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////FONCTION SELECTION CHANNEL ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/*Typically, CSADC and CSADC are tied together
    SCK and CLK are tied together and driven with a common clock signal.

    During channel selection, CSADC is HIGH. Data is shifted into the DIN pin on
    the rising edge of CLK, see Figure 4. Table 3 shows the bit combinations for
    channel selection.

    In order to enable the multiplexer output, CSADC must be pulled LOW.
    The multiplexer should be programmed after the previous conversion is complete.

    In order to guarantee the conversion is complete, the multiplexer addressing should
    be delayed a minimum tCONV (approximately 133ms for a 60Hz notch) after the data out is read.
    While the multiplexer is being programmed, the ADC is in the sleep state.
    Once the ADC addressing is complete, the data from the preceding conversion can be read.
    A new conversion cycle is initiated following the data read cycle with the analog
    input tied to the newly selected channel.
*/

/**
     * A single CS signal drives both the multiplexer CSMUX and converter CSADC inputs. 
     * This common signal is used to monitor and control the state of the conversion as well as enable the channel selection.
     * 
     * The serial clock mode is selected on the falling edge of CSADC. To select the external serial clock mode, 
     * the serial clock pin (SCK) must be LOW during each CSADC falling edge.
     * 
     * The serial data output pin (SDO) is Hi-Z as long as CSADC is HIGH. At any time during the conversion cycle, 
     * CSADC may be pulled LOW in order to monitor the state of the converter. While CSADC is LOW, EOC is output to the SDO pin. 
     * EOC = 1 while a conversion is in progress and EOC = 0 if the device is in the sleep state. Independent of CSADC, the device automatically enters 
     * the sleep state once the conversion is complete. While the device is in the sleep state and CSADC is HIGH, the power consumption is reduced an order of magnitude.
     * 
     * While the device is in the sleep state, prior to entering the data output state, the user may program the multiplexer. 
     * As shown in Figure 13, the multiplexer channel is selected by serial shifting a 4-bit word into the DIN pin on the rising edge of CLK (CLK is tied to SCK). 
     * The first bit is an enable bit that must be HIGH in order to program a channel. The next three bits determine which channel is selected, see Table 3. 
     * On the falling edge of CSMUX, the new channel is selected and will be valid for the first conversion performed following the data output state. 
     * Clock signals applied to the CLK pin while CSMUX is LOW (during the data output state) will have no effect on the channel selection. Furthermore, 
     * if DIN is held LOW or CLK is held LOW during the sleep state, the channel selection is unchanged.
     */
long SpiRead(int CS_ADC, int channel_adc)
{
    int channel[8] = {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}; //
    int CSADC_channel[6] = {CSADC1, CSADC2, CSADC3, CSADC4, CSADC5, CSADC6};

    digitalWrite(CSADC_channel[CS_ADC], LOW);
    digitalWrite(CSADC_channel[CS_ADC], HIGH);
    SPI.transfer(channel[channel_adc]);

    for (int j = 0; j < 24; j++)
        ;
    digitalWrite(CSADC_channel[CS_ADC], HIGH);

    delay(133);
    digitalWrite(CSADC_channel[CS_ADC], LOW);

    long result = 0;

    result = SPI.transfer(0x00);
    result = result << 8;
    // Récupération de l'octet B2
    result = result | SPI.transfer(0x00);
    result = result << 8;

    // Récupération de l'octet B3
    result = result | SPI.transfer(0x00);

    // Supression des 4bits de poids forts, conservation des 20bits de données
    result = 0x0fffff & result;
    digitalWrite(CSADC_channel[CS_ADC], HIGH);

    return result;
}