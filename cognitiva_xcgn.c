/**
  Il FILE cognitiva_xcgn.c è parte del
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
#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "cgn_testate.h"

#define NCOLS 12
#define NEURONS NCOLS*NCOLS
/*______________________________________________
  
   riga e colonna dell'angolo superiore sinistro 
   del pannello di selezione nueroni 
*/
#define R_PAN 20
#define C_PAN 2

#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKey {
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};


struct termios orig_termios;
int rows,cols;

void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}

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

  
void terminale_cucinato()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void terminale_crudo()
{
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) 
    die("tcgetattr");
  
  atexit(terminale_cucinato);
  
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) 
    die("tcsetattr");  
}

int init()
{
  terminale_crudo();
  dimensioni_finestra(&rows, &cols);
  if(rows<40 || cols<70)
    {
      die("\n*** Ridimensionare il terminale ad almeno 70x40\n");
    }
  
  setlocale(LC_CTYPE, "");
}

int leggi_tastiera() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[')
      {
	if (seq[1] >= '0' && seq[1] <= '9')
	  {
	    if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
	    if (seq[2] == '~') {
	      switch (seq[1]) {
	      case '1': return HOME_KEY;
	      case '3': return DEL_KEY;
	      case '4': return END_KEY;
	      case '5': return PAGE_UP;
	      case '6': return PAGE_DOWN;
	      case '7': return HOME_KEY;
	      case '8': return END_KEY;
	      }
	    }
	  } else
	  {
	    switch (seq[1])
	      {
	      case 'A': return ARROW_UP;
	      case 'B': return ARROW_DOWN;
	      case 'C': return ARROW_RIGHT;
	      case 'D': return ARROW_LEFT;
	      case 'H': return HOME_KEY;
	      case 'F': return END_KEY;
	      }
	  }
      }
    
    else if (seq[0] == 'O')
      {
	switch (seq[1]) {
	case 'H': return HOME_KEY;
	case 'F': return END_KEY;
	}
      }
    return '\x1b';
    
    
  } else
    {
      return c;
      
    }
}


 void stampa_cornice()
 {
   int i;
   int r = R_PAN;
   int c = C_PAN;

   /*___________________________________
     
      Eccitazione manuale dei neuroni
    */
   
   // Descrizione
    printf("\x1b[%d;%dHStrato U_3",r-2,c);
   // Angolo sinistra
   printf("\x1b[%d;%dH\u250F\u2501",r,c);
   // Bordo superiore
   for(i=1;i<NCOLS;i++)
     printf("\x1b[%d;%dH\u2501\u2501",r,c+i*2);
   // Angolo destra
   printf("\x1b[%d;%dH\u2501\u2513",r,c+i*2);
   // Bordi laterali
   for(i=1;i<=NCOLS;i++)
      printf("\x1b[%d;%dH\u2503                        \u2503",r+i,c);
   // Bordo inferiore
   for(i=1;i<NCOLS;i++)
      printf("\x1b[%d;%dH\u2501\u2501",r+NCOLS+1,c+i*2);
   // Angolo sisnistra
   printf("\x1b[%d;%dH\u2517\u2501",r+NCOLS+1,c);
   // Angolo destra
   printf("\x1b[%d;%dH\u2501\u251B",r+NCOLS+1,c+i*2);

    /*___________________________________
     
      Strato finale
    */

   c = c + 30;
   
   // Descrizione
    printf("\x1b[%d;%dHStrato U_0",r-2 ,c);
   // Angolo sinistra
   printf("\x1b[%d;%dH\u250F\u2501",r,c);
   // Bordo superiore
   for(i=1;i<NCOLS;i++)
     printf("\x1b[%d;%dH\u2501\u2501",r,c+i*2);
   // Angolo destra
   printf("\x1b[%d;%dH\u2501\u2513",r,c+i*2);
   // Bordi laterali
   for(i=1;i<=NCOLS;i++)
      printf("\x1b[%d;%dH\u2503                        \u2503",r+i,c);
   // Bordo inferiore
   for(i=1;i<NCOLS;i++)
      printf("\x1b[%d;%dH\u2501\u2501",r+NCOLS+1,c+i*2);
   // Angolo sisnistra
   printf("\x1b[%d;%dH\u2517\u2501",r+NCOLS+1,c);
   // Angolo destra
   printf("\x1b[%d;%dH\u2501\u251B",r+NCOLS+1,c+i*2);

   
 }


void attiva_neurone (int * att,int r, int c,int sgn)
{
  int i = r * NCOLS+c;
  int ov = att[i];
  att[i] += sgn;
  if (att[i]<0 || att[i]>255) att[i] = ov;
}


void propagazione_indietro(int * attivazione, char * nome_file)
{
  AREA area;
  STRATO U[LAYERS];
  double x0[NEURONS];
  double x1[NEURONS];
  double x2[NEURONS];
  double x3[NEURONS];
  
  inizializza_aree(&area);

  /* Carica strati da file */
  FILE * f;
  char  fname[320];

  for(int i = 0; i<4; i++)
    {
      sprintf(fname,"%s_%d.w",nome_file,i);
      f = fopen(fname,"rb");
      if( f == 0)
        {
           printf("\nIl file %s non è stato aperto correttamente\n",fname);
          exit(1);
        }
      int s = fread(U+i,sizeof(U[i]),1,f);
      if(s != 1)
        {
          printf("\nIl file %s non è stato letto correttamente\n",fname);
          fclose(f);
          exit(1);
        }
      fclose(f);
    }                    

   
  /* inizializza il layer 3 con l'input utente */
  for(int i=0;i<144;i++)
    {
      (U+3)->u[i].pot =  (double)attivazione[i];
      (U+3)->u[i].segnale =  (double)attivazione[i];
    }
  
  /*Riproduzione inversa*/
    
  //filtra_max(&U[3]);
  
  //print_object(estrai_array(x3,&U[3],0),12, 12,30,110,"U3");

  carica_segnale(&U[2],-1,&area);
  
  trasmetti_indietro(&U[3],&U[2],&area);

  

  //print_object(estrai_array(x3,&U[2],0),12, 12,30,80,"U2");

  carica_segnale(&U[1],-1,&area);
  
  trasmetti_indietro(&U[2],&U[1],&area);

  

  //print_object(estrai_array(x3,&U[1],0),12, 12,30,50,"U1");

  carica_segnale(&U[0],-1,&area);
  
  trasmetti_indietro(&U[1],&U[0],&area);

  

  print_object(estrai_array(x3,&U[0],0),12, 12,R_PAN+1,C_PAN+30+1,"");

  printf("\n");

}



/*___________________________
   *
   * Navigazione del cursore 
   * dentro un riquadro
   * e attivazione dei 
   * neuroni del 4 layer
   */
int main(int argc, char * argv[])
{

  /* Passare al programma la radice del nome dei 4 file di pesi */
  if(argc<2)
    {
      printf("\nUSO: cognitiva_xcgn <radice>\n");
      printf("\nPARAMETRI:\n");
      printf("<radice>         il programma usa 4 file <radice>_0.w, <radice>_1.w, <radice>_2.w, <radice>_3.w\n");
      exit(1);
    }
  
  /* presentazione */
  printf("\x1b[2J\x1b[1;1H\ncognitiva_: attiva i nueroni del cognitrone (xcgn) \n");
  printf("\n(C)Scuola_Sisini 2019 https://pumar.it/cognitiva\n\n\n");
  printf("\nAttiva i neuroni dello strato U_3 del cognitrone"
         "\nUSA:\n -) le frecce (SU, GIU, DESTRA e SINISTRA) per muovere il cursore\n"
         " -) tasti + e - per incrementare/diminuire l'eccitazione del neurone sotto al cursore\n"
         " -) invio per trasmettere l'attivazione allo strato U_0\n"
         " -) CTRL-q per uscire\n");
  fflush(stdout); 
  
  /* matrice di ativazione dei neuroni selezionati dall'utente */
  int attivazione[NEURONS];
  memset(attivazione,0,NEURONS*sizeof(int));
  
  /* coordinate del cursore */
  int r = 0;
  int c = 0;
  
  /*
    0) Abilita terminale crudo
   */
  init();

  /*
    1) stampa la cornice del riquadro
   */
  stampa_cornice();
  printf("\x1b[%d;%dH",R_PAN+1,C_PAN+1),
  fflush(stdout);
  
  /*
    2) ciclo di attesa input
  */
  
  while(1)
    {

      /*
        3) leggi tasiera
       */
       int ch = leggi_tastiera();
       
      
      /*
        4) rispondi al comando
        - freccie: muovi cursore
        - spazio: attiva/disattiva neurone
        - invio: propagazione inversa
        - CTR-q: esci
      */
        int mt,at;
        switch (ch)
          {      
          case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            write(STDOUT_FILENO,"\x1b[48:5:0m",9);
            exit(0);
            break;
            
          case CTRL_KEY('M'):
            //- invio propagazione inversa
            propagazione_indietro(attivazione,argv[1]);
            break;
            
            
          case BACKSPACE:
            //- attiva/disattava nuerone
            printf("\nattiva");
            break;
            
            
          case ARROW_UP:
            // -su
            if(r>0)
              r--;
            break;
          case ARROW_DOWN:
            if(r<NCOLS-1)
              r++;
            break;
          case ARROW_LEFT:
            if(c>0)
            c--;
            break;
          case ARROW_RIGHT:
            if(c<NCOLS-1)
              c++;
            break;
          default:
            if(32<=ch&&ch<=127)
              {

                if(ch == '+')
                  {
                    attiva_neurone(attivazione,r,c,1);
                  }
                 if(ch == '-')
                  {
                    attiva_neurone(attivazione,r,c,-1); 
                  }
                
              }
            
            break;
          }

        /*
          5) Mostra cursore
         */
        int act = attivazione[r*NCOLS+c];
        int rr,gg,bb;
        if(act>0 && act<20)
          {
            rr = 60.;
            gg = 56;
            bb = 15;
          }
        else if(act<56 )
          {
            rr = 4*act;
            gg = 4*act;
            bb = 2*act;
          }
        else if(act<128&&act>56)
          {
            rr = 248.0/128*act;
            gg = 243.0/128*act;
            bb = 43.0/128*act;
          }
        else
          {
            rr = 248.0/256*act;
            gg = 10.0/256*act;
            bb = 0;;
          }
        printf("\x1b[%d;%dHAttivazione: %d ",R_PAN-1,C_PAN,act);
        printf("\x1b[%d;%dH\x1b[48;2;%d;%d;%dm  \x1b[48;2;0;0;0m",
               R_PAN+r+1,C_PAN+c*2+1,rr,gg,bb);
        printf("\x1b[%d;%dH",R_PAN+r+1,C_PAN+c*2+1);    
        fflush(stdout);
    }

}
