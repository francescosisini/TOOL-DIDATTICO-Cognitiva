/*______________________________________________
 | ss_snn_lib
 | Scuola Sisini Shallow Neural Network Library
 | Francesco Sisini (c) 2019
 */




typedef struct
{
  double * strato_ingresso;
  double * strato_uscita;
  
  /* Numero di elementi dell'array che saranno allocati */ 
  int N_neuroni_ingresso;
  int N_neuroni_uscita;
  int N_neuroni_primo_strato_interno;
  int N_neuroni_secondo_strato_interno;

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
  

}rnss_dati_classificati;

typedef struct
{
  

}rnss_dati_non_classificati;

rnss_rete * rnss_Crea_rete(
			   int N_neuroni_ingresso,
			   int N_neuroni_uscita,
			   int N_neuroni_primo_strato_interno,
			   int N_neuroni_secondo_strato_interno);

double * rnss_Strato_output(rnss_rete * rete);

rnss_rete *  rnss_Addestra(rnss_rete * rete, rnss_dati_classificati * dc);

rnss_rete *  rnss_Classifica(rnss_rete * rete, rnss_dati_non_classificati * d);


rnss_rete * rnss_Crea_rete(
			   int N_neuroni_ingresso,
			   int N_neuroni_uscita,
			   int N_neuroni_primo_strato_interno,
			   int N_neuroni_secondo_strato_interno);
{
  rnss_rete * rn = malloc(sizeof(rnss_rete));
  if(rn == 0) exit (1);
  
  /* neuroni del primo strato interno */
  int l1_np =  N_neuroni_primo_strato_interno;
  if(l1_np == 0)
    l1_np = N_neuroni_uscita; //la rete ha un solo strato
  /* dendriti del primo strato iinterno */
  int l1_nd = int N_neuroni_ingresso;
  
    
  rn->N_strati_interni =  N_strati_interni;

  double * v_x0 = malloc((l1_nd+1)*sizeof(double));/* input dei percettroni del layer 1*/
  
  double * v_t = malloc((l1_nd+1)*l1_np*sizeof(double));/* NP vettori di peso dendritico*/
  
  double * v_Dt = malloc((l1_nd+1)*l1_np*sizeof(double));/* Variazione v_t */
  
  double * v_s1 = malloc(l1_np*sizeof(double)); /*NP valori input*/
  
  double * v_y1 = malloc(l1_np*sizeof(double));/* NP output uno per percettrone*/

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
