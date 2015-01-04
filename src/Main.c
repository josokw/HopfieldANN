/*---------------------------------------------------------------------------*/
/* Simulation Hopfield ANN                                                   */
/*                                                                           */
/* Developed by: Jos Onokiewicz                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AppInfo.h"
#include "Hopfield.h"
#include "HopfieldCalc.h"
#include "HopfieldIO.h"

#define MAXFILENAME 100

int main(int argc, char *argv[])
{
  double InputPattern[MAXN] = {0};
  double InputPatternWithNoise[MAXN] = {0};
  double OutputPattern[MAXN] = {0};
  //double En = 0.0;
  int Noise = 0;
  int indexPattern;
  char Menu = '\0';
  char FileName[MAXFILENAME];
  const char MenuChars[] = "ELNeln";

  if (!(argc == 2 || argc ==3 ))
  {
    printf("\n\tUSAGE: Hopfield <patterns filename>\n\n");
    printf("\n\tUSAGE: Hopfield <patterns filename> <noisy patterns filename>\n\n");
    exit(EXIT_FAILURE);
  }

  printf("Hopfield's ANN Simulation: Associative Memory " APPNAME_VERSION "\n\n"
    "- Patterns file name: %s   loading .... ", argv[1]);

  readFile(argv[1]);
  printf("ready\n"
    "- Number of neurons: %d * %d = %d, number of patterns: %d\n",
    nRows, nColumns, patSize, nPatterns);

  printf("- Learning patterns by hebbian learning rule .... ");
  LearnJ(nPatterns, patSize, J);
  printf("ready\n");
  printf("- Learning result: connection matrix, size %d x %d\n\n", 
         nRows*nColumns, nRows*nColumns);

  if (argc == 3)
  {
    printf("- Noisy patterns file name: %s\n\n", argv[2]);
    printf("- Loading noisy patterns data .... ");
    readNoisyFile(argv[2]);
    printf("ready\n\n");
    printf("- Number of noisy patterns: %d\n\n", nNoisyPatterns);
  }

  while (Menu != 'E' && Menu != 'e')
  {
    if (argc == 2 && (Menu == 'L' || Menu == 'l'))
    {
      fgetc(stdin); /* remove /n previous input */
      printf("- Patterns file name: ");
      fgets(FileName, MAXFILENAME, stdin);
      if (FileName[strlen(FileName) - 1] == '\n')
      {
        FileName[strlen(FileName) - 1] = '\0'; /* remove /n input */
      }
      printf("\n- Loading patterns data .... ");
      readFile(FileName);
      printf("ready\n\n"
        "- Number of neurons: %d * %d = %d, number of patterns: %d\n\n",
        nRows, nColumns, patSize, nPatterns);
      printf("- Learning patterns by hebbian learning rule .... ");
      LearnJ(nPatterns, patSize, J);
      printf("ready\n\n");
      printf("- Learning result: 1 connection matrix, size %d x %d\n\n", 
             nRows * nColumns, nRows * nColumns);
    }

    if (strchr(MenuChars, Menu) != NULL)
    {
      switch (argc)
      {
      case 2:
        printf("\n- Choose pattern to disturb by noise, index (1..%d): ", nPatterns);
        scanf(" %d", &indexPattern);
        puts("");
        if (indexPattern < 1 || indexPattern > nPatterns+1)
        {
          fprintf(stderr, "\n\tERROR: index %d out of range\n\n", indexPattern);
          getchar();
          exit(EXIT_FAILURE);
        }
        indexPattern--;
        showIndexedPattern(indexPattern);
        puts("");
        CopyPattern(patSize, Patterns[indexPattern], InputPattern);
        CopyPattern(patSize, Patterns[indexPattern], InputPatternWithNoise);
        printf("- Noise [%%]: ");
        scanf(" %d", &Noise);
        
        AddNoise(patSize, indexPattern, InputPatternWithNoise, Noise);
        /* printf("- Pattern as vector:\n\n"); */
        /* showPatternAsVector(InputPatternWithNoise); */
        printf("\n\n- Pattern as 2D image and noisy pixels:\n\n");
        CalcAssociations(patSize, J, InputPattern, InputPatternWithNoise, OutputPattern);
        puts("");
        break;
      case 3:
        printf("\n- Choose noisy pattern, index (1..%d): ", nNoisyPatterns);
        scanf(" %d", &indexPattern);
        puts("");
        if (indexPattern < 1 || indexPattern > nNoisyPatterns+1)
        {
          fprintf(stderr, "\n\tERROR: index %d out of range\n\n", indexPattern);
          getchar();
          exit(EXIT_FAILURE);
        }
        indexPattern--;
        showIndexedNoisyPattern(indexPattern);
        puts("");
        CopyPattern(patSize, NoisyPatterns[indexPattern], InputPattern);
        /* printf("- Pattern as vector:\n\n"); */
        /* showPatternAsVector(InputPattern); */
        printf("\n\n- Pattern as 2D image:\n\n");
        CalcAssociations(patSize, J, InputPattern, InputPattern, OutputPattern);
        puts("");
        break;
      default:
        printf("\n\tSYSTEM ERROR: this should never happen!\n\n");
        getchar();
        exit(EXIT_FAILURE);
      }
    }
    while (getchar() != '\n')
    {
      ;
    }
    fflush(stdin);
    if (argc == 2)
    {
      printf("- E(xit), L(oad new patterns data file), N(ext simulation) ..... ");
    }
    else
    {
      printf("- E(xit), N(ext simulation) ..... ");
    }
    Menu = getchar();
  }

  return 0;
}
