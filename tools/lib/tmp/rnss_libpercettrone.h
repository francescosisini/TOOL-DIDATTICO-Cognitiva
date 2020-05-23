/*______________________________________________
 | rnss_libpercettrone
 | 
 | Francesco e Valentina Sisini (c) 2020
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
  int N_strati_computazionali;

  /* Strato 1: dall'ingresso 0 all'uscita 1 */
  double * v_x0;
  double * v_t;
  double * v_Dt;
  double * v_s1;
  double * v_y1;

  /* Strato 2: dall'ingresso 1 all'uscita 2 */
  double * v_x1;
  double * v_u;
  double * v_u_tmp;
  double * v_Du;
  double * v_s2;
  double * v_y2;

  /* Strato 3: dall'ingresso 2 all'uscita 3 */
  double * v_x2;
  double * v_v;
  double * v_v_tmp;
  double * v_Dv;
  double * v_s3;
  double * v_y3;
} rna_rete;


typedef struct
{
  double fattore_apprendimento;
  double max_per_normalizzazione;
  int seme_pseudocasuale;
  int epoche;
  int campioni;
}rna_parametri;


/* crea una rete neurale che può essere addestrata.
   Inizializza i pesi dendritici (pesi delle connessioni)
   tra -0.1 e 0.1
 */
rna_rete * rna_Crea_rete(
			   int N_neuroni_ingresso,
			   int N_neuroni_uscita,
			   int N_neuroni_primo_strato_interno,
			   int N_neuroni_secondo_strato_interno);

/* libera le risorse della rete */
void  rna_Libera_rete(rna_rete * rete);

/* 
   addestra una rete neurale usando i dati e le classi passate
   ogni chiamata viene eseguita una singola iterazione
*/
rna_rete *  rna_Addestra(rna_rete * rn,
			   rna_parametri * par,
			   double * dati,
			   double * classi);
/* 
   Usa una rete già addestrata per classificare i dati 
   il risultato è puntato da rete->strato_uscita
*/
void  rna_Classifica(rna_rete * rete, double * dati);
