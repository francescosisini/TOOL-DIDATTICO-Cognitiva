/**
  Il FILE cognitiva_acgn.c è parte del
  Cognitiva: sistema di pattern recongnition
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
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "cgn_testate.h"

/*______________________________________________
  
   riga e colonna dell'angolo superiore sinistro 
   del pannello di selezione nueroni 
   e distanza tra i pannelli
*/
#define R_PAN 15
#define C_PAN 2
#define DIST 30

int dimensioni_finestra(int *rows, int *cols) 
{
  struct winsize ws;  
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
      return -1;
    } else
    {
      *cols = ws.ws_col;
      *rows = ws.ws_row;
      return 0;
    }
}


int main(int argc,char * argv[])
{
  AREA area;
  STRATO U[LAYERS];
  double x0[NEURONS];
  double x1[NEURONS];
  double x2[NEURONS];
  double x3[NEURONS];


  /* presentazione */
  printf("\x1b[2J\x1b[1;1H\ncognitiva_: auto organizzazione cognitrone (acgn) \n");
  printf("\n(C)Scuola_Sisini 2019 https://pumar.it/cognitiva\n\n\n");


  /* controllo dimensione della finestra */
  int mr,mc;
  dimensioni_finestra(&mr, &mc);
  if(mc<DIST*4+1 || mr<DIST)
    {
      printf("Allarga un po' il terminale..., diciamo %dx%d\n",DIST*4+1,DIST);
      exit(1);
    }
  

  /* Passare al programma la radice del nome dei 4 file di pesi */
  if(argc<3)
    {
      printf("\nUSO: cognitiva_acgn <radice> <epoche>\n");
      printf("\nPARAMETRI:\n");
      printf("<radice>         il programma scrive 4 file <radice>_0.w, <radice>_1.w, <radice>_2.w, <radice>_3.w\n");
      printf("<epoche>         epoche di addestramento (consigliato <50)\n");
      printf("<attesa>  (ms)   intervallo tra i cicli (opzionale<1000)\n");
      exit(1);
    }

  int epoche;
  if(sscanf(argv[2],"%i",&epoche)!=1)
    {
      printf("\n Non hai inserito un valore intero per le epoche\n");
      exit(1);
    }
  
  
  int attesa = 100;
  if(argc>3)
    {
      if(sscanf(argv[3],"%i",&attesa)!=1)
        {
          printf("\n Non hai inserito un valore intero per l'attesa\n");
          exit(1);
        }
      if(attesa>999) attesa=999;
    }
    


  
  inizializza_aree(&area);

  inizializza_connessioni(&U[0], &area,1,INIT_B);

  inizializza_connessioni(&U[1], &area,INIT_A,INIT_B);

  inizializza_connessioni(&U[2], &area,INIT_A,INIT_B);

  inizializza_connessioni(&U[3], &area,INIT_A,INIT_B);
    
  for(int iter=0;iter<(epoche*4);iter++)
    {
      // printf("\x1b[2J");
      int c=iter%5;
      printf("\x1b[%d;%dH%s %d/%d ",R_PAN-4,C_PAN,"Epoca",iter/4,epoche);
  
      if(c==0)
        {
          carica_segnale(&U[0],-1,&area);
          carica_segnale(&U[0],0,&area);
        }
      
      if(c==1)
        {
          carica_segnale(&U[0],-1,&area);
          carica_segnale(&U[0],1,&area);
        }
      if(c==2)
        {
          carica_segnale(&U[0],-1,&area);
          carica_segnale(&U[0],2,&area);
        }
      if(c==3)
        {
          carica_segnale(&U[0],-1,&area);
          carica_segnale(&U[0],3,&area);
        }
      if(c==4)
        {
          carica_segnale(&U[0],-1,&area);
          carica_segnale(&U[0],4,&area);
        }
  

      calcola_segnale_eccitatore(&U[0]);

      inibizione_laterale(&U[0],&area);
  
      calcola_segnale_inibitore(&U[0],&area);

      print_object(estrai_array(x1,&U[0],0),12, 12,R_PAN,C_PAN,"U_0");
       
      trasmetti(&U[0],&U[1]);
      
      //Strato 1
      
      calcola_potenziale(&U[1],&area);
      
      calcola_segnale_eccitatore(&U[1]);

      inibizione_laterale(&U[1],&area);
    
      calcola_segnale_inibitore(&U[1],&area);
      
      calcola_rinforzo(&U[1],&area);
      
      print_object(estrai_array(x2,&U[1],0),12, 12,R_PAN,C_PAN+DIST,"U_1");

      trasmetti(&U[1],&U[2]);

      //Strato 2

      calcola_potenziale(&U[2],&area);
      
      calcola_segnale_eccitatore(&U[2]);

      inibizione_laterale(&U[2],&area);

      calcola_segnale_inibitore(&U[2],&area);
      
      calcola_rinforzo(&U[2],&area);

      print_object(estrai_array(x3,&U[2],0),12, 12,R_PAN,C_PAN+DIST*2,"U_2");

      trasmetti(&U[2],&U[3]);

      //Strato 3

      calcola_potenziale(&U[3],&area);
      
      calcola_segnale_eccitatore(&U[3]);

      inibizione_laterale(&U[3],&area);

      calcola_segnale_inibitore(&U[3],&area);
      
      calcola_rinforzo(&U[3],&area);
      
      print_object(estrai_array(x3,&U[3],0),12, 12,R_PAN,C_PAN+DIST*3,"U_3");

      struct timespec tc;
      tc.tv_sec = attesa/1000;
      tc.tv_nsec = attesa*1000000;
      nanosleep(&tc, 0);
      fflush(stdout);
    }
  
  FILE * f;
  char  fname[300];
  printf("\n\n");
  for(int i = 0; i<4; i++)
    {
      sprintf(fname,"%s_%d.w",argv[1],i);
      f = fopen(fname,"wb");
      if( f == 0) exit(1);
      int s = fwrite(U+i,sizeof(U[i]),1,f);
      if(s != 1)
        {
          printf("\nIl file %s non è stato salvato correttamente\n",fname);
          fclose(f);
          exit(1);
        }
      printf("\nIl file %s è stato salvato correttamente\n",fname);
      fclose(f);
    }                    
  
  
}

