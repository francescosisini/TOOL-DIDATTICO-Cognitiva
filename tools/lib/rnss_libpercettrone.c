/*______________________________________________
 | rnss_libpercettrone
 | 
 | Francesco e Valentina Sisini (c) 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_PESO 0.1
#define MIN_PESO -0.1

typedef struct
{
  double * strato_ingresso;
  double * strato_uscita;
  
  /* Numero di elementi dell'array che saranno allocati */ 
  int N_neuroni_ingresso;
  int N_neuroni_uscita;
  int N_neuroni_primo_strato_interno;
  int N_neuroni_secondo_strato_interno;
  int N_starti_computazionali;

  /* Strato 1: dall'ingresso 0 all'uscita 1 */
  double * v_x0;
  double * v_t;
  double * v_Dt;
  double * v_s1;
  double * v_y1;

  /* Strato 2: dall'ingresso 1 all'uscita 2 */
  double * v_x1;
  double * v_u;
  double * v_Du;
  double * v_s2;
  double * v_y2;

  /* Strato 3: dall'ingresso 2 all'uscita 3 */
  double * v_x2;
  double * v_v;
  double * v_Dv;
  double * v_s3;
  double * v_y3;
} rnss_rete;


typedef struct
{
  double fattore_apprendimento;
  double max_per_normalizzazione;
  int seme_pseudocasuale;
  int epoche;
  int campioni;
}rnss_parametri;


/* crea una rete neurale che può essere addestrata.
   Inizializza i pesi dendritici (pesi delle connessioni)
   tra -0.1 e 0.1
 */
rnss_rete * rnss_Crea_rete(
			   int N_neuroni_ingresso,
			   int N_neuroni_uscita,
			   int N_neuroni_primo_strato_interno,
			   int N_neuroni_secondo_strato_interno);

/* libera le risorse della rete */
void * rnss_Libera_rete(rnss_rete * rete);


/* 
   addestra una rete neurale usando i dati e le classi passate
   ogni chiamata viene eseguita una singola iterazione
*/
rnss_rete *  rnss_Addestra(rnss_rete * rete, rnss_parametri par, double * dati, double * classi);

rnss_rete *  rnss_Addestra(rnss_rete * rn, rnss_parametri par, double * dati, double * classi)
{

  int l1_nd =  rn->N_neuroni_ingresso;
  /* preparazione degli ingressi e delle label delle classi*/
  rn->v_x0[0] = 1;
  memcpy(rn->v_x0+1,dati,rn->N_neuroni_ingresso);
  if( int N_starti_computazionali == 1)
    {
      memcpy(rn->v_d,classi,rn->N_neuroni_ingresso);
      
    }
}

/* 
   Usa una rete già addestrata per classificare i dati del file f
   e ritorna un'array di puntatori a double. Ogni elemento dell'array
   punta a un buffer di dimensione N_neuroni_di_uscita*sizeof(double)
   L'ultimo elelemnto punta a 0 come una stringa
*/
double **  rnss_Classifica(rnss_rete * rete, FILE *f);


void * rnss_Libera_rete(rnss_rete * rete)
{
  free(rete->v_x0);
  free(rete->v_t);
  free(rete->v_Dt);
  free(rete->v_s1);
  free(rete->v_y1);

  if( rn->N_starti_computazionali >=2)
    {
      free(rete->v_x1);
      free(rete->v_u);
      free(rete->v_Du);
      free(rete->v_s2);
      free(rete->v_y2);
    }

   if( rn->N_starti_computazionali >=3)
    {
      free(rete->v_x2);
      free(rete->v_v);
      free(rete->v_Dv);
      free(rete->v_s3);
      free(rete->v_y3);
    }
  
  free(rete);
}

rnss_rete * rnss_Crea_rete(
			   int N_neuroni_ingresso,
			   int N_neuroni_uscita,
			   int N_neuroni_primo_strato_interno,
			   int N_neuroni_secondo_strato_interno)
{
  rnss_rete * rn = malloc(sizeof(rnss_rete));
  if(rn == 0) exit (1);
   
  /*
    STRATO 1: Questo è il primo strato computazionale della rete
    se si è creata una rete ad un solo strato (percettrone classico)
    questo rappresenta anche l'unico strato della rete
   */
  
  /* neuroni del primo strato (computazionale) interno */
  int l1_np =  N_neuroni_primo_strato_interno;
  if(l1_np == 0)
    {
      /* la rete ha un solo strato, quindi il numero di neuroni di 
       uscita deve essere uguale al numero di neuroni 
      dello strato computazionale
      */
      l1_np = N_neuroni_uscita;
      rn->N_starti_computazionali = 1;
    }
  /* dendriti del primo strato iinterno */
  int l1_nd = N_neuroni_ingresso;
  rn->N_neuroni_ingresso = l1_nd;
  /* input dei percettroni dello strato 1*/
  rn->v_x0 = malloc((l1_nd+1)*sizeof(double));
  /* un vettore di l1_nd dendriti per ogniuno dei l1_np neuron1*/
  rn->v_t = malloc((l1_nd+1)*l1_np*sizeof(double));
  /* variazione v_t */
  rn->v_Dt = malloc((l1_nd+1)*l1_np*sizeof(double));
  /*vettore della somma pesata dei canali dendritici per ogni neurone*/
  rn->v_s1 = malloc(l1_np*sizeof(double));
  /* vettore dell'output di ogni neurone*/
  rn->v_y1 = malloc(l1_np*sizeof(double));

  /* inizializzazione pseudocasuale delle connessioni */
   for(int i=0;i<(l1_nd+1)*l1_np;i++)
      rn->v_t[i]=MAX_PESO*(double)rand()/(double)RAND_MAX-MIN_PESO;
   
  /* RETURN si tratta si un percettrone a singolo strato*/
  if(rn->N_starti_computazionali == 1) return rn;

  /*
    STRATO 2: Questo è il secondo strato computazionale della rete
    Questo strato è necessario se si vogliono classificare dei dati
    che non sono linearmente separabili
   */
  
  /* neuroni del secondo strato (computazionale) interno */
  int l2_np =  N_neuroni_secondo_strato_interno;
  if(l2_np == 0)
    {
       /* la rete ha un solo due strati, quindi il numero di neuroni di 
       uscita deve essere uguale al numero di neuroni 
      del secondo strato computazionale
      */
      l2_np = N_neuroni_uscita;
      rn->N_starti_computazionali = 2;

    }
  /* dendriti del secondo strato iinterno */
  int l2_nd = N_neuroni_ingresso;
  
  /* input dei percettroni dello strato 1*/
  rn->v_x1 = malloc((l2_nd+1)*sizeof(double));
  /* un vettore di l1_nd dendriti per ogniuno dei l1_np neuron1*/
  rn->v_u = malloc((l2_nd+1)*l1_np*sizeof(double));
  /* variazione v_t */
  rn->v_Du = malloc((l2_nd+1)*l1_np*sizeof(double));
  /*vettore della somma pesata dei canali dendritici per ogni neurone*/
  rn->v_s2 = malloc(l2_np*sizeof(double));
  /* vettore dell'output di ogni neurone*/
  rn->v_y2 = malloc(l2_np*sizeof(double));

  /* inizializzazione pseudocasuale delle connessioni */
   for(int i=0;i<(l2_nd+1)*l2_np;i++)
      rn->v_u[i]=MAX_PESO*(double)rand()/(double)RAND_MAX-MIN_PESO;

  /* RETURN si tratta si un percettrone a due strati*/
  if(rn->N_starti_computazionali == 2) return rn;

   /*
    STRATO 3: Questo è il terzo strato computazionale della rete
    Questo strato è necessario se si vogliono classificare dei dati
    distribuiti in regioni geometriche non convesse
   */
  
  /* neuroni del terzo strato (computazionale) interno */
  int l3_np =  N_neuroni_uscita;
  rn->N_starti_computazionali = 3;

  /* dendriti del terzo strato iinterno */
  int l3_nd = l2_np;
  
  /* input dei percettroni dello strato 1*/
  rn->v_x2 = malloc((l3_nd+1)*sizeof(double));
  /* un vettore di l1_nd dendriti per ogniuno dei l1_np neuron1*/
  rn->v_v = malloc((l2_nd+1)*l1_np*sizeof(double));
  /* variazione v_t */
  rn->v_Dv = malloc((l2_nd+1)*l1_np*sizeof(double));
  /*vettore della somma pesata dei canali dendritici per ogni neurone*/
  rn->v_s3 = malloc(l2_np*sizeof(double));
  /* vettore dell'output di ogni neurone*/
  rn->v_y3 = malloc(l2_np*sizeof(double));

  /* inizializzazione pseudocasuale delle connessioni */
   for(int i=0;i<(l3_nd+1)*l3_np;i++)
      rn->v_v[i]=MAX_PESO*(double)rand()/(double)RAND_MAX-MIN_PESO;

  /* RETURN */
  return rn;
  

}



/*** Livello 1 ***/
/* Calcola l'output di uno strato  percettroni */
void layer_feed_forward(double v_s[],double v_y[],double v_w[],double v_x[],int n_perc, int n_dend);

/* Mappa l'output v_y in v_x aggiungendo prima l'elemento v_x[0]=1*/
void layer_map_out_in(double v_x[],double v_y[], int n_dend);

/*** Livello 2 ***/
/* Calcola la risposta del  percettrone */
double perc_calc_output(double v_w[],double v_x[],int n_dend);

/* Corregge i pesi del percettrone */
void perc_correzione(double v_w[],double v_x[],double z,double d,double rate,int n_dend);

/*** Livello 3 ***/

/* Calcola il valore della risposta del percettrone*/
double activ_function(double summed_input);

/* Calcola il valore della derivata della risposta del percettrone*/
double Dactiv_function(double summed_input);


/* Legge un dato/record in formato csv (d1,d2,...,dn) */
int leggi_dato(int * o,FILE * in_stream);

/* Legge una classe e un  dato/record in formato csv (c,d1,d2,...,dn) */
int leggi_dato_classe(int * o,int * out_label,FILE * in_stream);


/* Stampa a video una matrice r x c in R,C */
void print_object(double x[],int r, int c,int R,int C);

/*_____________________________________________________
 | stream: il file su cui scrivere 
 | v_w: l'array sequenziale con tutti i pesi del layer
 | n_dend: numero di dendriti per percettrone
 | n_perc: numero di percettroni nel layer
 */
void layer_writedown(FILE * stream,double *v_w, int n_dend, int n_perc);

/*______________________________________________________
 | stream: il file da cui legge i pesi delle connessioni
 | v_w: l'array sequenziale con tutti i pesi del layer
 | n_dend: numero di dendriti per percettrone
 | n_perc: numero di percettroni nel layer
 */
void layer_read(FILE * stream,double *v_w, int n_dend, int n_perc);

/*_______________________________
 *
 * Misura righe e colonne del
 * terminale
 */
int dimensioni_finestra(int *rows, int *cols);
