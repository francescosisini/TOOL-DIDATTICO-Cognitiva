/*_________________________________________________________________________
  |  Rete neurale shallow (rns)
  |  Copyright (C) 2020  Francesco Sisini (francescomichelesisini@gmail.com)
  |
  |  This program is free software: you can redistribute it and/or modify
  |  it under the terms of the GNU General Public License as published by
  |  the Free Software Foundation, either version 3 of the License, or
  |  (at your option) any later version.
  |
  |  This program is distributed in the hope that it will be useful,
  |  but WITHOUT ANY WARRANTY; without even the implied warranty of
  |  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  |  GNU General Public License for more details.
  |
  |  You should have received a copy of the GNU General Public License
  |  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libcogni.h"

/*___________________________
 *| suite:  cognitiva
 *| programma: rns
 *| Francesco Sisini (c) 2019
 */


/* Strato (layer) 1*/
#define L1_ND 784
#define L1_NP 100

/* Strato (layer) 2*/
#define L2_ND 100
#define L2_NP 10

/* Valore massimo sinapsi */
#define SINAPSI 0.001

/* Velocità apprendimento*/
#define RATE 0.2

int main(int argc,char *argv[])
{
    
  /* __________________________________________________
     |impostazione dei parametri di default della rete
  */
    
  /* Velocita' apprendimento*/
  double rate = RATE;

  /* Estremo superiore del valore iniziale delle sinapsi (0 to ...)*/
  double sinapsi = SINAPSI;

  /*Estremo superiore del valore iniziale delle sinapsi (0 to ...)*/
  int seed=1;
  
  /* Epoche e campioni*/
  int epoche=1;
  int campioni=1;

  /* Valore di normalizzazione */
  int norm = 1;
  
  /* Nodi nei layer */
  int l1_nd = L1_ND;
  int l1_np = L1_NP;
  int l2_nd = L2_ND; 
  int l2_np = L2_NP;
  

  /* _____________________________________________
     |Lettura dei parametri dalla linea di comando
  */

  char cls = 0;// 0->uso del programma per addestramento rete, 1->classuficazione
  char help = 0;
  char verbose = 0;
  char layer_files = 0;
  int c;
  char * f_name; //il file con i dati
  char * l1_name; // i dati del  layer 1
  char * l2_name; // del layer 2
  int argmin = 5; //Numero minimo di argomenti 
  for(int i=1;i<argc;i++)
    {

      //opzione uso classificaziozne "-c"
      if(strcmp(argv[i],"-c") == 0)
        {
          cls = 1;
        }
      
      
      //opzione valore di normalizzazione "-n"
      if(strcmp(argv[i],"-n") == 0)
        {
          argmin += 2;
          if(sscanf(argv[i+1],"%d",&norm)!=1)
            {
              printf("Attenzione <normalizzazione> deve essere un intero. %s non e' un intero\n",argv[i]);
              exit(1);
            }
        }
      
      //opzione valore massimo di inizializzazione "-m"
      if(strcmp(argv[i],"-m") == 0)
        {
          argmin += 2;
          if(sscanf(argv[i+1],"%lf",&sinapsi)!=1)
            {
              printf("Attenzione <max_sinapsi> deve essere un decimale. %s non e' un decimale\n",argv[i]);
              exit(1);
            }
        }
      
      //opzione forzatura seed x random "-s"
      if(strcmp(argv[i],"-s") == 0)
        {
          argmin += 2;
          if(sscanf(argv[i+1],"%i",&seed)!=1)
            {
              printf("Attenzione <rnd_seed> deve essere un decimale. %s non e' un decimale\n",argv[i+1]);
              exit(1);
            }
        }

      //opzione forzatura learning rate "-l"
       if(strcmp(argv[i],"-l") == 0)
        {
          argmin += 2;
          if(sscanf(argv[i+1],"%lf",&rate)!=1)
            {
              printf("Attenzione <learning_rate> deve essere un decimale. %s non e' un decimale\n",argv[i]);
              exit(1);
            }
        }

       //opzione forzatura epoche  "-e"
       if(strcmp(argv[i],"-e") == 0)
        {
          argmin += 2;
          if(sscanf(argv[i+1],"%d",&epoche)!=1)
            {
              printf("Attenzione <numero_epoche> deve essere un decimale. %s non e' un decimale\n",argv[i]);
              exit(1);
            }
        }

       //opzione help  "-h"
       if(strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"--help") == 0)
        {
          help = 1;
        }

       //opzione help  "-v"
       if(strcmp(argv[i],"-v") == 0)
        {
          verbose = 1;
        }

       /*_____________________________________________
         | Parametri obbligatori
       */
       //obbligatorio numero neuroni  "-d"
       if(strcmp(argv[i],"-d") == 0)
         {
           if(sscanf(argv[i+1],"%i",&l1_nd)!=1)
             {
               printf("Attenzione: <neuroni input> deve essere intero. %s non e' un valore intero\n",argv[i+1]);
               exit(1);
             }
           
           if(sscanf(argv[i+2],"%i",&l1_np)!=1)
             {
               printf("Attenzione: <neuroni intercalari> deve essere intero. %s non e' un valore intero\n",argv[i+2]);
               exit(1);
             }
           else
             {
               //NOTA BENE
               l2_nd = l1_np;
             }
           
           if(sscanf(argv[i+3],"%i",&l2_np)!=1)
             {
               printf("Attenzione: <neuroni output> deve essere intero. %s non e' un valore intero\n",argv[i+3]);
               exit(1);
             }
           
         }
         //obbligatorio nime file  "-f"
       if(strcmp(argv[i],"-f") == 0)
         {

           if(i == argc-1)
             {
               printf("Errore: specificare un nome per il file.\n");
               exit(1);
             }
           
           int l=strlen(argv[i+1]);
           f_name=malloc(l+10);
           if(f_name==0)
             {
               printf("Errore: memoria insufficiente.\n");
               exit(1);
             }
           strcpy(f_name,argv[i+1]);
         }
        //obbligatorio nime file  "-l1"
       if(strcmp(argv[i],"-l1") == 0)
         {
           int l=strlen(argv[i+1]);
           l1_name=malloc(l+10);
           if(l1_name==0)
             {
               printf("\Errore: memoria insufficiente.\n");
             }
           strcpy(l1_name,argv[i+1]);
           layer_files++;
         }

        //obbligatorio nime file  "-l2"
       if(strcmp(argv[i],"-l2") == 0)
         {
           int l=strlen(argv[i+1]);
           l2_name=malloc(l+10);
           if(l2_name==0)
             {
               printf("\nErrore: memoria insufficiente.\n");
               exit(1);
             }
           strcpy(l2_name,argv[i+1]);
           layer_files++;
         }
       
    }
  
       if(help)
    {
      printf("\nUSO: rns OPZIONE [PARAMETRI]");
      printf("\nrns di Scuola_Sisini addestra due layer di una rete neurale shallow oppure li usa per la classificazione\n");
      printf("Esempi:");
      printf("\n\t rn -a -f FILE<dati> -d INT<neuroni input> INT<neuroni intrecalari> INT<neuroni output>  [-e INT<numero_epoche>] [-l DECIMAL<lerning_rate>] [-m DECIMAL<max_sinapsi>] [-s INT<rnd_seed>] [-n INT<normalizzazione] [-v]\n");
      printf("\n\t rn -c -f FILE<dati> -l1 FILE<layer1> -l2FILE<layer2> -d INT<neuroni input> INT<neuroni intrecalari> INT<neuroni output>\n");
      
      printf("\n");
      printf("\n\t-a addestra la rete e produce i file layer1.w layer2.w");
      printf("\n\t-c classifica l'input");
      printf("\t<normalizzazione> se presente è il divisore di tutti i valori di input.\n");
      printf("\t<max_sinapsi> è il valore massimo che viene assegnato alle connessioni durante la inizializzazione random\n");
      printf("\t<rnd_seed> è il seme usato per la inizializzazione random.\n");
      printf("\tFILE è in formato csv. Ogni riga è un dato. Il primo campo è la classe, seguono un numero di valori interi pari a <neuroni_input>.\n");
      printf("\t-v (verbose) visualizza output durante l'elaborazione");
      printf("Help %d\n",help);
            exit(1);
    }

  

  

  /*__________________________________________
    |Allocazione memoria
  */
  
  /* Immagine da file CSV */
  int * img=malloc(l1_nd*sizeof(int));
  /* Label da file CSV (desiderd output) */
  int label;
  
  /*Strato 1*/
  
  double * v_x0 = malloc((l1_nd+1)*sizeof(double));/* input dei percettroni del layer 1*/
 
  double * v_t = malloc((l1_nd+1)*l1_np*sizeof(double));/* NP vettori di peso dendritico*/
  
  double * v_Dt = malloc((l1_nd+1)*l1_np*sizeof(double));/* Variazione v_t */

  double * v_s1 = malloc(l1_np*sizeof(double)); /*NP valori input*/

  double * v_y1 = malloc(l1_np*sizeof(double));/* NP output uno per percettrone*/

  /*Strato 2*/
  double * v_x1 = malloc((l2_nd+1)*sizeof(double));/* input dei percettroni del layer 2*/

  double * v_u = malloc((l2_nd+1)*l2_np*sizeof(double));/* NP vettori di peso dendritico*/

  double * v_Du = malloc((l2_nd+1)*l2_np*sizeof(double));/* Variazione v_u*/

  double * v_s2 = malloc(l2_np*sizeof(double)); /*NP valori input*/

  double * v_y2 = malloc(l2_np*sizeof(double));/* NP output uno per percettrone*/

  /*Output desiderato*/
  double * v_d = malloc(l2_np*sizeof(double));/* NP output desiderato uno per percettrone*/
  
  if(img ==0 || v_x0==0 ||v_t==0 ||v_Dt==0 ||v_s1==0 ||v_y1==0 ||v_x1==0 ||v_u==0 ||v_Du==0 ||v_s2==0 ||v_y2==0 ||v_d == 0)
    {
      printf("\nErrore: memoria insufficiente. Verificare le dimensioni della rete\n");
      printf(" %p, %p\n",v_x1,v_Du);
      //exit(1);
    }

  
  if(cls)
    {
      /* controlli preliminari */
      if(layer_files != 2)
        {
           printf("\nErrore: non sono stati specificati i due file <layer1> e <layer2>. Sricivi rns -h per vedere l'uso del programma.\n");
           exit (1);
        }
      
      /* carica i pesi v_t e v_u dai file */
      FILE* w=fopen(l1_name,"rb");
      if(w==0)
        {
          printf("\nFile %s non trovato\n",l1_name);
          exit (1);
        }
      
      layer_read(w,v_t, l1_nd, l1_np);
      fclose(w);
      
      w=fopen(l2_name,"rb");
      if(w==0)
        {
          printf("\nFile %s non trovato\n",l2_name);
          exit (1);
        }
      
      layer_read(w,v_u, l2_nd, l2_np);
      fclose(w);
      
      w=fopen(f_name,"r");
      if(w==0)
        {
          printf("\nFile %s non trovato\n",f_name);
          exit (1);
        }
      leggi_dato(img,w);
      if(verbose)
        {
          printf("\nDATO  : ");
          for(int i=0;i<l1_nd;i++)
            printf("%d ",img[i]);

          printf("\n");
        } 
      fclose(w);
      
      /* conversione immagine da int a double */
      v_x0[0]=1;
      for(int i=0;i<l1_nd;i++)
        {
          v_x0[i+1]=((double)img[i])/(double)norm;
        }      
      
      /*** PROPAGAZIONE AVANTI ***/
      
      /** Feed Forward: Input->L1->output to L2*/
      layer_feed_forward(v_s1,v_y1,v_t,v_x0,l1_np,l1_nd);
      
      /** Mappa y1 in x1 aggiungendo l'elemento x1_0=1*/
      layer_map_out_in(v_x1, v_y1,l2_nd);
      
      /** Feed Forward: L1->L2->output*/
      layer_feed_forward(v_s2,v_y2,v_u,v_x1,l2_np,l2_nd);
  
      /* Valutazione output */
      int imax;
      double fmax=0;
      
      for(int i=0; i<l2_np;i++)
        {
          if(v_y2[i]>fmax)
            {
              fmax=v_y2[i];
              imax=i;
            }
          if(verbose)
            printf("\nClasse: %d\t%0.lf%%",i,v_y2[i]*100);
        }

            
      
      printf("%d",imax);
      
      fflush(stdout);
    }
  else
    {      
      /* Imposta random la configurazione iniziale della rete */
      
      srand(seed);
      /*2) bias+pesi strato 1*/
      for(int i=0;i<(l1_nd+1)*l1_np;i++)
        v_t[i]=sinapsi*(double)rand()/(double)RAND_MAX;
            
      /*3) bias+pesi strato 2*/
      for(int i=0;i<(l2_nd+1)*l2_np;i++)
        v_u[i]=sinapsi*(double)rand()/(double)RAND_MAX;

      printf("\n*****************************");
      printf("\nNeuroni input: %d",l1_nd);
      printf("\nNeuroni intercalari: %d",l2_nd);
      printf("\nNeuroni output: %d",l2_np);
      printf("\nLearning rate: %f",rate);
      printf("\nRandom seed: %d",seed);
      printf("\nMassimo sinapsi: %f",sinapsi);
      printf("\nNormalizzazione: %d",norm);
      printf("\n*****************************\n");
            
      for(int ii=0;ii<epoche;ii++)
        {
          /* comunicazioni: epoche */
          if(verbose)
            printf("\n\n\tEpoca %d di %d\n",ii+1,epoche);
          
          FILE* stream = fopen(f_name, "r");
          if(stream==0)
            {
              printf("\n\nErrore: il file %s non e' presente\n",f_name);
              exit(1);
            }

          int dati = 0;

          /* Carica i dati di training ed esegue il training*/
          while(leggi_dato_classe(img,&label,stream)>0)
            {
              dati++;

              /* Comunicazioni: visualizzazione del dato */
              if(verbose)
                {
                  printf("Label %d\t",label);
                  for(int z=0;z<l1_nd;z++)
                    {
                      printf("%d\t",img[z]);
                    }
                  printf("\n");

                }
              
              /* conversione dati da int a double */
              v_x0[0]=1;
              for(int i=0;i<l1_nd;i++)
                {
                  v_x0[i+1]=((double)img[i])/(double)norm;
                }                 
              /*4 output desiderato*/
              for(int i=0;i<l2_np;i++)
                {
                  if(i == label)
                    v_d[i]=1;
                  else
                    v_d[i]=0;
                }
              
              /*** PROPAGAZIONE AVANTI ***/
              
              /** Feed Forward: Input->L1->output to L2*/
              layer_feed_forward(v_s1,v_y1,v_t,v_x0,l1_np,l1_nd);
              
              /** Mappa y1 in x1 aggiungendo l'elemento x1_0=1*/
              layer_map_out_in(v_x1, v_y1,l2_nd);
              
              /** Feed Forward: L1->L2->output*/
              layer_feed_forward(v_s2,v_y2,v_u,v_x1,l2_np,l2_nd);
              
              
              /*** CORREZIONE PER PROPAGAZIONE INVERSA ***/
              
              /** Propagazione inversa dell'errore in L2  (v_u  <- v_y2) */
              for(int i=0;i<l2_np;i++)
                {
                  /* correzione dei pesi (v_u) del percettrone i-esimo */
                  perc_correzione( v_u+i*(l2_nd+1),v_x1,v_s2[i],v_d[i]-v_y2[i], rate,l2_nd);
                }
              
              /** Propagazione inversa dell'errore in L1  (v_t <- v_y2)*/
              for(int i=0;i<l1_np;i++)
                {
                  double dd=0;
                  for(int j=0;j<l2_np;j++)
                    {
                      /* w: peso del i-esimo dendrite del j-esimo percettrone dello strato più esterno */
                      double w=v_u[j*(l2_nd+1)+i];
                      /* correzione   */
                      dd=dd+w*(v_d[j]-v_y2[j])*Dactiv_function(v_s2[j]);
                    }
                  
                  /* correzione del percettrone i-esimo*/
                  perc_correzione( v_t+i*(l1_nd+1),v_x0,v_s1[i],dd, rate,l1_nd);
                }
            }
          if(verbose)
            printf("\n\tLetti %d dati",dati);
          fclose(stream);
    }
      
      /*_____________________________________
        | Salva la rete
      */
      FILE* sw = fopen("layer1.w", "w");
      layer_writedown(sw,v_t, l1_nd, l1_np);
      fclose(sw);
      
      sw = fopen("layer2.w", "w");
      layer_writedown(sw,v_u,l2_nd,l2_np);
      fclose(sw);
      
      printf("\nProdotti: layer1.w e layer2.w\n");
    }
}

/************************************
 * ATTENZIONE: il codice non libera
 * esplicitamente la memoria perché
 * pensato solo per l'uso one-shot
 * eseguire le free nel caso lo si 
 * riutilizzi in un contesto diverso
 */
