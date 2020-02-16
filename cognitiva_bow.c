#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bow_lib.h"


// bow aggiungi  <testo.txt> <classe>

int main(int argc, char *argv[])
{
  Bow b;
  inizia_bow(&b);
  leggi_bow(&b,"Bow.txt");
  //mostra_bow(b);

  if(argc<4)
    {
      printf("Uso: cognitiva_bow aggiungi <testo.txt> <classe>\n");
      exit(1);
    }


  //Creazione copia pulita del file
  //Mantiene solo alfabeto A-z e trasforma il resto in spazi

  char * file_pulito = malloc(strlen(argv[2]+10));
  strcpy(file_pulito,argv[2]);
  strcat(file_pulito,".clean");
  
  FILE * f = fopen(argv[2],"rb");
  if(f == 0)
    {
      printf("File %s non trovato\n",argv[3]);
      exit(1);
    }

  FILE * fc = fopen(file_pulito,"w");
  char c;
  while(fread(&c,sizeof(char),1,f))
    {
      if(c>='A')
        {
          if(c=='à') c= 'a';
          if(c=='è') c= 'e';
          if(c=='é') c= 'e';
          if(c=='ì') c= 'i';
          if(c=='ò') c= 'o';
          if(c=='ù') c= 'u';
          fwrite(&c,sizeof(char),1,fc);
        }
      else
        {
          c =' ';
          fwrite(&c,sizeof(char),1,fc);
        }
    }
  fclose(fc);
  fclose(f);

  int cl = atoi(argv[3]);//La classe del documento
  f = fopen(file_pulito,"rt");
  char lemma[MAX];
  while(fscanf(f,"%s",lemma) == 1)
    {
      aggiorna_blocco(&b,lemma,cl);
    }
  
  
  //mostra_bow(b);
  
  scrivi_bow(b,"Bow.txt");
  for(int i=0;i<NCLASSI;i++)
    printf("Classe %d, parole totali:%d\n",i,conta_parole(b,i));
  //int r = ricorrenza_parola(b, "ciao");
  
  
}
