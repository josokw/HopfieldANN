#include "AppInfo.h"
#include "Hopfield.h"
#include "HopfieldCalc.h"
#include "HopfieldIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXFILENAME 100

void usage(int argc);
void clearInput(void);

int main(int argc, char *argv[])
{
   double InputPattern[NMAX_NEURONS] = {0};
   double InputPatternWithNoise[NMAX_NEURONS] = {0};
   double OutputPattern[NMAX_NEURONS] = {0};

   int Noise = 0;
   int indexPattern;
   char Menu = '\0';
   char FileName[MAXFILENAME];
   const char MenuChars[] = "ELNeln";

   usage(argc);
   srand((unsigned int)time(NULL));

   printf("Hopfield's ANN associative memory: " APPNAME_VERSION
          "\n\n- Input patterns file name: %s, loading .... ",
          argv[1]);

   readFile(argv[1]);
   printf(
      "ready\n"
      "- Number of neurons: %d * %d = %d, number of patterns = %d\n",
      nRows, nColumns, patternSize, nPatterns);

   int stCapacity = storageCapacity(patternSize);
   if (stCapacity < nPatterns) {
      fprintf(stderr,
              "- Warning: associative storage capacity %d exceeded\n",
              stCapacity);
   }

   printf(
      "- Learning patterns by Hebbian learning rule, "
      "training starts .... ");
   learnHebbian(nPatterns, patternSize, W);
   printf("ready\n");
   printf("- Learning result: 1 connection matrix, size %d x %d\n\n",
          nRows * nColumns, nRows * nColumns);

   if (argc == 3) {
      printf("- Noisy patterns file name: %s\n\n", argv[2]);
      printf("- Loading noisy patterns data .... ");
      readNoisyFile(argv[2]);
      printf("ready\n\n");
      printf("- Number of noisy patterns: %d\n\n", nNoisyPatterns);
   }

   while (Menu != 'E' && Menu != 'e') {
      if (argc == 2 && (Menu == 'L' || Menu == 'l')) {
         fgetc(stdin); /* remove /n previous input */
         printf("- Input patterns file name: ");
         fgets(FileName, MAXFILENAME, stdin);
         if (FileName[strlen(FileName) - 1] == '\n') {
            FileName[strlen(FileName) - 1] = '\0'; /* remove /n input */
         }
         printf("\n- Loading input patterns data .... ");
         readFile(FileName);
         printf(
            "ready\n\n"
            "- Number of neurons: %d * %d = %d, number of patterns: "
            "%d\n\n",
            nRows, nColumns, patternSize, nPatterns);

         int stCapacity = storageCapacity(patternSize);
         if (stCapacity < nPatterns) {
            fprintf(
               stderr,
               "- Warning: associative storage capacity %d exceeded\n",
               stCapacity);
         }

         printf("- Learning patterns by Hebbian learning rule .... ");
         learnHebbian(nPatterns, patternSize, W);
         printf("ready\n\n");
         printf("- Learning result: 1 connection matrix, size %d x %d\n\n",
                nRows * nColumns, nRows * nColumns);
      }

      if (strchr(MenuChars, Menu) != NULL) {
         switch (argc) {
            case 2:
               printf(
                  "\n- Choose pattern to disturb by noise, index "
                  "(1..%d): ",
                  nPatterns);
               scanf(" %d", &indexPattern);
               puts("");
               if (indexPattern < 1 || indexPattern > nPatterns + 1) {
                  fprintf(stderr, "\n\tERROR: index %d out of range\n\n",
                          indexPattern);
                  getchar();
                  exit(EXIT_FAILURE);
               }
               indexPattern--;
               showIndexedPattern(indexPattern);
               puts("");
               copyPattern(patternSize, Patterns[indexPattern],
                           InputPattern);
               copyPattern(patternSize, Patterns[indexPattern],
                           InputPatternWithNoise);
               printf("- Noise level [%%]: ");
               scanf(" %d", &Noise);

               addNoiseToPattern(patternSize, indexPattern,
                                 InputPatternWithNoise, Noise);
               /* printf("- Pattern as vector:\n\n"); */
               /* showPatternAsVector(InputPatternWithNoise); */
               printf(
                  "\n\n- Pattern %d as 2D image and %d%% noisy "
                  "pixels:\n\n",
                  indexPattern + 1, Noise);
               showAssociatedPattern(patternSize, W, InputPattern,
                                     InputPatternWithNoise, OutputPattern);
               puts("");
               break;
            case 3:
               printf("\n- Choose noisy pattern, index (1..%d): ",
                      nNoisyPatterns);
               scanf(" %d", &indexPattern);
               puts("");
               if (indexPattern < 1 || indexPattern > nNoisyPatterns + 1) {
                  fprintf(stderr, "\n\tERROR: index %d out of range\n\n",
                          indexPattern);
                  getchar();
                  exit(EXIT_FAILURE);
               }
               indexPattern--;
               puts("");
               showIndexedNoisyPattern(indexPattern);
               puts("");
               copyPattern(patternSize, NoisyPatterns[indexPattern],
                           InputPattern);
               /* printf("- Pattern as vector:\n\n"); */
               /* showPatternAsVector(InputPattern); */
               printf("\n\n- Pattern as 2D image:\n\n");
               showAssociatedPattern(patternSize, W, InputPattern,
                                     InputPattern, OutputPattern);
               puts("");
               break;
            default:
               fprintf(stderr,
                       "\n\tSYSTEM ERROR: this should never happen!\n\n");
               getchar();
               exit(EXIT_FAILURE);
         }
      }
      clearInput();
      if (argc == 2) {
         printf(
            "- E(xit), L(oad new patterns data file), N(ext simulation) "
            ".... ");
      }
      else {
         printf("- E(xit), N(ext simulation) .... ");
      }
      Menu = getchar();
   }

   return 0;
}

void usage(int argc)
{
   if (!((argc == 2) || (argc == 3))) {
      fprintf(stderr,
              "\n\tUSAGE: hopfieldann <input patterns filename>\n");
      fprintf(stderr,
              "\n\tUSAGE: hopfieldann <input patterns filename> "
              "<noisy patterns filename>\n\n");
      exit(EXIT_FAILURE);
   }
}

void clearInput(void)
{
   while (getchar() != '\n') {
      // empty loop
   }
}
