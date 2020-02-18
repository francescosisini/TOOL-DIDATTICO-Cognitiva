#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bow_lib.h"


// bow aggiungi  <testo.txt> <classe>

int main(int argc, char *argv[])
{

  enum
  {
    AGGIUNGI,
    CLASSIFICA
  } AZIONE;
  
  

  int azione;
  
  if(argc<3)
    {
      printf("Uso: cognitiva_bow AZIONE FILE [CLASSE]\n");
      printf("AZIONE = {aggiungi, classifica }\n");
      exit(1);
    }
  
  int t = strcmp("aggiungi",argv[1]);
  if(t == 0)
    {
      azione = AGGIUNGI;
    }
  else
    {
      t = strcmp("classifica",argv[1]);
      if(t == 0)
        {
          azione = CLASSIFICA;
        }
      else
        {
          printf("Azione %s non supportata:\n",argv[1]);
          printf("Uso: cognitiva_bow AZIONE FILE [CLASSE]\n");
          printf("AZIONE = {aggiungi, classifica }\n");
          exit(1);
        }
      
    }
  
  //La BOW con tutte le classi
  Bow b;
  inizia_bow(&b);
  leggi_bow(&b,"Bow.txt");

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

  if(azione == AGGIUNGI)
    {
  
      int cl = atoi(argv[3]);//La classe del documento
      f = fopen(file_pulito,"rt");
      char lemma[MAX];
      while(fscanf(f,"%s",lemma) == 1)
        {
          aggiorna_blocco(&b,lemma,cl);
        }
      
  
      scrivi_bow(b,"Bow.txt");
      for(int i=0;i<NCLASSI;i++)
        printf("Classe %d, parole totali:%d\n",i,conta_parole(b,i));
    }

  if(azione == CLASSIFICA)
    {
      Bow testo;
      inizia_bow(&testo);
      f = fopen(file_pulito,"rt");
      char lemma[MAX];
      while(fscanf(f,"%s",lemma) == 1)
        {
          aggiorna_blocco(&testo,lemma,0);
        }

      double classi[NCLASSI];
      classifica_testo(testo,b, classi);
      printf("CLASSIFICAZIONE:\n");
      for(int i=0;i<NCLASSI;i++)
        printf("Class %d, p=%f\n",i,classi[i]);
      
    }

  
  
  
}
