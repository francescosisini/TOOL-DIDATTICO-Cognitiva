

/*
    Cognitrone: sistema di pattern recongnition
    Copyright (C) 2018  Francesco Sisini (francescomichelesisini@gmail.com)
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CGN_TESTATE_H_
#define CGN_TESTATE_H_



/*** Definizioni ***/
/*******************/
#define NCOLS 12
#define NEURONS NCOLS*NCOLS
#define LAYERS 4

/*
  ATTENZIONE: quando si modificano le  seguenti tre macro
  si ricordi di modificare appropriatamente anche il metodo
  inizializza_aree
 */
#define S_CA 25 //area di connessione
#define CA_L 2 //lato mezzi area di connessione
#define O_VA 13 //area di vicinato
#define O_L 2 //Lato mezzi di vicinato
#define INIB_L 3 //Lato mezzi di inibizione
#define H_HA (INIB_L*2+1)*(INIB_L*2+1) //area laterale

#define Q0 2
#define Q1 16

#define INIT_A 0.0 //conn. ecc.
#define INIT_B 0.0 //conn. al n. inib.
#define INIT_C 1./(double)S_CA //conn. inib.
#define INIT_H 1./((double)(H_HA-1)) //conn. inib. lat.

/*Opzioni di elaborazione*/
#define INIBIZIONE_LATERALE 1
#define COMPENSAZIONE_BORDI 1

/*** Tipi astratti ***/
/*********************/

/* aree intorno */
typedef struct{
  /*
    Sono definiti le aree di connessione,
    inibizione e vicinanza per ogni 
    neurone di idice 'i' in un dato strato.
    Le aree sono definite in termini relativi
    cioè specificando le variazioni positve e 
    negative dell'indice i.
    Per esempio dato il neurone di indice 'i',
    l'elemento a_i[0]=-2 indica che il neurone
    di indice 'i-2' appartiene alla 
    connectable-area del neurone 'i'.
   */
  
  /* area di connessione */
  int a_i[S_CA];
  int a_j[S_CA];

  /* area di inibizione */
  int g_i[H_HA];
  int g_j[H_HA];
  
  
  /* area di vicinanza */
  int o_i[O_VA];
  int o_j[O_VA];
} AREA;

/* neurone eccitatore */
typedef struct
{

  /* ingresso/input eccitatorio */
  double ingresso_ecc;

  /* ingresso inibitorio */
  double ingresso_inib;

  /* potenziale */
  double pot;
  
  /* segnale/output */
  double segnale;
  double segnale_tmp;

  /* peso delle connessioni dell' 
     area di connessione */
  double a[S_CA];

  /* peso dell connessioni 
     dell'area di inibizione laterale */
  double g[H_HA];
  
  /* peso della connessione
     al neurone inibitore
     del livello precedente
  */
  double b;
  
  /* Nota: nel modello del
     cognitrone il segnale
     o è 0 o coincide con
     il potenziale
  */
} NEURONE_ECCITATORE;

/* neurone inibitore */
typedef struct
{
   /* potenziale */
  double pot;
  
  /* segnale/output */
  double segnale;

  /* peso delle connessioni dell' 
     area di connessione 
     sullo stesso livello
  */
  double c[S_CA-1];
  
} NEURONE_INIBITORE;

/* strato di neuroni */
typedef struct
{
  NEURONE_ECCITATORE u[NEURONS];
  NEURONE_INIBITORE  v[NEURONS];
  
}STRATO;

/*** Prototipi pubblici ***/
/**************************/
/* Inizializza gli array che specificano gli indice dei neuroni
   di: area di connessione, vicinanto e laterali
*/
void inizializza_aree(AREA *ar);

/* inizializza i valori delle connessioni del
   cognitrone con un valore predefinito
*/
void inizializza_connessioni(STRATO *l, AREA *a, double conn_ecc,double conn_inib);

/*
  Applica l'inibizione laterale al segnale  dei neuroni del
  livello l
 */
void inibizione_laterale(STRATO *l, AREA *a);


/* Assegna agli ingrssi ecc. e inib.  dei neuroni u 
   dello strato 'to' il valore del segnale 
   dei neuroni ecc. e inib.  dello strato 'from'
*/
void trasmetti(STRATO *from, STRATO *to);

/* Assegna agli ingrssi ecc. e inib.  dei neuroni u 
   dello strato 'to' il valore del segnale 
   dei neuroni ecc. e inib.  dello strato 'from'
   pesato
*/
void trasmetti_indietro(STRATO *from, STRATO *to,AREA *ar);

/* Calcola il potenziale 
   dei neuroni di uno strato
   integrando il segnale sull'area di connessione
*/
void calcola_potenziale(STRATO *l, AREA *a);

/* Calcola il valore di output dei neuroni dello
   strato 'l' che hanno potenziale 'pot'.
 */
void calcola_segnale_eccitatore(STRATO *l);

/* 
   Filtra lo strato lasciando solo il
   neurone più eccitato
 */
void filtra_max(STRATO *l);

/* Calcola il segnale di output dei neuroni
   inibitori
*/
void calcola_segnale_inibitore(STRATO *l,AREA *a);

/*
  Calcola il rinforzo delle connessioni sinaptiche
  dei neuroni inibitori ed eccitatori dello strato 'l'
 */
void calcola_rinforzo(STRATO *l, AREA *a);

/*** Prototipi di servizio ***/
/*****************************/

/*
  E' un metodo per il test che assegna i valori
  di ingresso eccitatorio e inibitorio dei neuroni
  dello strato 'l' con dei valori prestabiliti.
  Si possono scegliere diverse configurazioni 
  cambiando l'indice 'i'.
  Chiamando più volte di seguito il metodo, si ottiene
  che le configurazioni vengono sommate.
  Usarwe l'indice -1 per inizializzare a 0.
 */
void carica_segnale(STRATO *l, int indice, AREA *ar);

/*
  Visualizza sul monitor una matrice di 'r' righe e
  'c' colonne a partire dalla cella R e C con il
  contenuto dell'array 'x'
 */
void print_object(double x[],int r, int c,int R,int C,char * descrizione);

/*
  Copia i neuroni dello strato 'l' nell'array
  'dest'
  Il paramtero selettore serve a indicare quale array estrarre
  tra:
  0-segnale ecc
  1-segnale inib
  2-potenziale
  3-ingresso ecc
 */
double * estrai_array(double *dest,STRATO *l,int selettore);

/*** Prototipi privati ***/
/*************************/
double phi(double x);
int delta(STRATO *l, AREA *ar,int neurone);

#endif
