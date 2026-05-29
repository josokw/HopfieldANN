#include "AppInfo.h"
#include "Hopfield.h"
#include "HopfieldCalc.h"
#include "HopfieldIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXFILENAME_SIZE 100

void usage(int argc);
void clearInput(void);

int main(int argc, char *argv[])
{
   double inputPattern[NMAX_NEURONS] = {0};
   double inputPatternWithNoise[NMAX_NEURONS] = {0};
   double outputPattern[NMAX_NEURONS] = {0};

   int noise = 0;
   int indexPattern;
   int menu = 0;
   char fileName[MAXFILENAME_SIZE] = {0};
   const char menuChars[] = "ELNeln";

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

   while (menu != 'E' && menu != 'e') {
      if (argc == 2 && (menu == 'L' || menu == 'l')) {
         fgetc(stdin); /* remove /n previous input */
         printf("- Input patterns file name: ");
         fgets(fileName, MAXFILENAME_SIZE, stdin);
         if (fileName[strlen(fileName) - 1] == '\n') {
            fileName[strlen(fileName) - 1] = '\0'; /* remove /n input */
         }
         printf("\n- Loading input patterns data .... ");
         readFile(fileName);
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

      if (strchr(menuChars, menu) != NULL) {
         switch (argc) {
            case 2:
               printf(
                  "\n- Choose pattern to disturb by noise, index "
                  "(1..%d): ",
                  nPatterns);
               if (scanf(" %d", &indexPattern) != 1) {
                  fprintf(stderr, "\n\tERROR: invalid input\n\n");
                  exit(EXIT_FAILURE);
               }
               puts("");
               if (indexPattern < 1 || indexPattern > nPatterns) {
                  fprintf(stderr, "\n\tERROR: index %d out of range\n\n",
                          indexPattern);
                  getchar();
                  exit(EXIT_FAILURE);
               }
               indexPattern--;
               showIndexedPattern(indexPattern);
               puts("");
               copyPattern(patternSize, patterns[indexPattern],
                           inputPattern);
               copyPattern(patternSize, patterns[indexPattern],
                           inputPatternWithNoise);
               printf("- Noise level [%%]: ");
               if (scanf(" %d", &noise) != 1) {
                  fprintf(stderr, "\n\tERROR: invalid input\n\n");
                  exit(EXIT_FAILURE);
               }

               addNoiseToPattern(patternSize, indexPattern,
                                 inputPatternWithNoise, noise);
               printf(
                  "\n\n- Pattern %d as 2D image and %d%% noisy "
                  "pixels:\n\n",
                  indexPattern + 1, noise);
               showAssociatedPattern(patternSize, W, inputPattern,
                                     inputPatternWithNoise, outputPattern);
               puts("");
               break;
            case 3:
               printf("\n- Choose noisy pattern, index (1..%d): ",
                      nNoisyPatterns);
               if (scanf(" %d", &indexPattern) != 1) {
                  fprintf(stderr, "\n\tERROR: invalid input\n\n");
                  exit(EXIT_FAILURE);
               }
               puts("");
               if (indexPattern < 1 || indexPattern > nNoisyPatterns) {
                  fprintf(stderr, "\n\tERROR: index %d out of range\n\n",
                          indexPattern);
                  getchar();
                  exit(EXIT_FAILURE);
               }
               indexPattern--;
               puts("");
               showIndexedNoisyPattern(indexPattern);
               puts("");
               copyPattern(patternSize, noisyPatterns[indexPattern],
                           inputPattern);
               /* printf("- Pattern as vector:\n\n"); */
               /* showPatternAsVector(InputPattern); */
               printf("\n\n- Pattern as 2D image:\n\n");
               showAssociatedPattern(patternSize, W, inputPattern,
                                     inputPattern, outputPattern);
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
      menu = getchar();
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
   int c;
   while ((c = getchar()) != '\n' && c != EOF) {
   }
}
