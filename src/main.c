#include "AppInfo.h"
#include "HopfieldContext.h"
#include "HopfieldCalc.h"
#include "HopfieldIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define MAXFILENAME_SIZE 100

static void usage(int argc);
static void clearInput(void);
static void handle_error(HopfieldError err);

int main(int argc, char *argv[])
{
   int noise = 0;
   int indexPattern;
   int menu = 0;
   bool firstRun = true;
   bool useStorkey = false;
   const char *ruleName = "Hebbian";
   char fileName[MAXFILENAME_SIZE] = {0};
   const char menuChars[] = "ELNelnRr";

   /* Local pattern buffers */
   double inputPattern[NMAX_NEURONS] = {0};
   double inputPatternWithNoise[NMAX_NEURONS] = {0};
   double outputPattern[NMAX_NEURONS] = {0};

   usage(argc);
   srand((unsigned int)time(NULL));

   /* Create context */
   HopfieldContext *ctx = hopfield_context_create();
   if (ctx == NULL) {
      fprintf(stderr, "Error: Failed to allocate context\n");
      return EXIT_FAILURE;
   }

   printf("Hopfield's ANN associative memory: " APPNAME_VERSION
          "\n\n- Input patterns file name: %s, loading .... ",
          argv[1]);

   HopfieldError err = readFile(ctx, argv[1]);
   if (err != HOPFIELD_OK) {
      handle_error(err);
      hopfield_context_destroy(ctx);
      return EXIT_FAILURE;
   }
   printf(
      "ready\n"
      "- Number of neurons: %d * %d = %d, number of patterns = %d\n",
      ctx->nRows, ctx->nColumns, ctx->patternSize, ctx->nPatterns);

   int stCapacity = storageCapacity(ctx->patternSize);
   if (stCapacity < ctx->nPatterns) {
      fprintf(stderr,
              "- Warning: associative storage capacity %d exceeded\n",
              stCapacity);
   }

   int ruleChoice = 0;
   printf("- Learning rule: (H)ebbian or (S)torkey? [H]: ");
   ruleChoice = getchar();
    if (ruleChoice == 'S' || ruleChoice == 's') {
       useStorkey = true;
       ruleName = "Storkey";
    }
    if (ruleChoice != '\n') {
       clearInput();
    }
   puts("");

   printf("- Learning patterns by %s learning rule, "
          "training starts .... ", ruleName);
   bool learned = useStorkey ? learnStorkey(ctx) : learnHebbian(ctx);
   if (!learned) {
      fprintf(stderr, "Error: Failed to learn patterns\n");
      hopfield_context_destroy(ctx);
      return EXIT_FAILURE;
   }
   printf("ready\n");
   printf("- Learning result: 1 connection matrix, size %d x %d\n\n",
          ctx->nRows * ctx->nColumns, ctx->nRows * ctx->nColumns);

   if (argc == 3) {
      printf("- Noisy patterns file name: %s\n\n", argv[2]);
      printf("- Loading noisy patterns data .... ");
      err = readNoisyFile(ctx, argv[2]);
      if (err != HOPFIELD_OK) {
         handle_error(err);
         hopfield_context_destroy(ctx);
         return EXIT_FAILURE;
      }
      printf("ready\n\n");
      printf("- Number of noisy patterns: %d\n\n", ctx->nNoisyPatterns);
   }

   while (menu != 'E' && menu != 'e') {
      bool enterPressed = (menu == '\n');
      if (enterPressed) {
         menu = 'R';
      }
      if (argc == 2 && (menu == 'L' || menu == 'l')) {
         clearInput(); /* remove \n previous input */
         printf("- Input patterns file name: ");
          fgets(fileName, MAXFILENAME_SIZE, stdin);
          size_t fileNameLen = strlen(fileName);
          if (fileNameLen > 0 && fileName[fileNameLen - 1] == '\n') {
             fileName[fileNameLen - 1] = '\0'; /* remove /n input */
         }
         printf("\n- Loading input patterns data .... ");
         err = readFile(ctx, fileName);
         if (err != HOPFIELD_OK) {
            handle_error(err);
            hopfield_context_destroy(ctx);
            return EXIT_FAILURE;
         }
         printf(
            "ready\n\n"
            "- Number of neurons: %d * %d = %d, number of patterns: "
            "%d\n\n",
            ctx->nRows, ctx->nColumns, ctx->patternSize, ctx->nPatterns);

         stCapacity = storageCapacity(ctx->patternSize);
         if (stCapacity < ctx->nPatterns) {
            fprintf(
               stderr,
               "- Warning: associative storage capacity %d exceeded\n",
               stCapacity);
         }

          printf("- Learning patterns by %s learning rule .... ", ruleName);
          if (!(useStorkey ? learnStorkey(ctx) : learnHebbian(ctx))) {
            fprintf(stderr, "Error: Failed to learn patterns\n");
            hopfield_context_destroy(ctx);
            return EXIT_FAILURE;
         }
         printf("ready\n\n");
         printf("- Learning result: 1 connection matrix, size %d x %d\n\n",
                ctx->nRows * ctx->nColumns, ctx->nRows * ctx->nColumns);
      }

      if (firstRun || strchr(menuChars, menu) != NULL) {
         firstRun = false;
         bool repeat = (menu == 'R' || menu == 'r');
         switch (argc) {
            case 2:
               if (!repeat) {
                  printf(
                     "- Choose pattern to disturb by noise, index "
                     "(1..%d): ",
                     ctx->nPatterns);
                  if (scanf(" %d", &indexPattern) != 1) {
                      fprintf(stderr, "\n\tERROR: invalid input\n\n");
                      hopfield_context_destroy(ctx);
                      return EXIT_FAILURE;
                   }
                   puts("");
                   if (indexPattern < 1 || indexPattern > ctx->nPatterns) {
                      fprintf(stderr, "\n\tERROR: index %d out of range\n\n",
                              indexPattern);
                      hopfield_context_destroy(ctx);
                      return EXIT_FAILURE;
                   }
                  indexPattern--;
                  showIndexedPattern(ctx, indexPattern);
                  puts("");
                  copyPattern(ctx->patternSize, ctx->patterns[indexPattern],
                              inputPattern);
                  copyPattern(ctx->patternSize, ctx->patterns[indexPattern],
                              inputPatternWithNoise);
                  printf("- Pattern %d noise level [%%]: ",
                         indexPattern + 1);

                   if (scanf(" %d", &noise) != 1) {
                      fprintf(stderr, "\n\tERROR: invalid input\n\n");
                      hopfield_context_destroy(ctx);
                      return EXIT_FAILURE;
                   }
               }

               addNoiseToPattern(ctx, indexPattern, noise);
               /* Copy noisy pattern to local buffer */
               copyPattern(ctx->patternSize, ctx->noisyPatterns[indexPattern],
                          inputPatternWithNoise);
               printf(
                  "\n\n- Pattern %d as 2D image and %d%% noisy "
                  "pixels:\n\n",
                  indexPattern + 1, noise);
               showAssociatedPattern(ctx, inputPattern,
                                     inputPatternWithNoise, outputPattern);
               puts("");
               break;
            case 3:
               if (!repeat) {
                  printf("\n- Choose noisy pattern, index (1..%d): ",
                         ctx->nNoisyPatterns);
                  if (scanf(" %d", &indexPattern) != 1) {
                      fprintf(stderr, "\n\tERROR: invalid input\n\n");
                      hopfield_context_destroy(ctx);
                      return EXIT_FAILURE;
                   }
                   puts("");
                   if (indexPattern < 1 ||
                       indexPattern > ctx->nNoisyPatterns) {
                      fprintf(stderr, "\n\tERROR: index %d out of range\n\n",
                              indexPattern);
                      hopfield_context_destroy(ctx);
                      return EXIT_FAILURE;
                   }
                  indexPattern--;
                  puts("");
                  showIndexedNoisyPattern(ctx, indexPattern);
                  puts("");
                  copyPattern(ctx->patternSize,
                              ctx->noisyPatterns[indexPattern], inputPattern);
               }
               printf("\n\n- Pattern %d as 2D image:\n\n",
                      indexPattern + 1);
               showAssociatedPattern(ctx, inputPattern,
                                     inputPattern, outputPattern);
               puts("");
               break;
             default:
                fprintf(stderr,
                        "\n\tSYSTEM ERROR: this should never happen!\n\n");
                hopfield_context_destroy(ctx);
                return EXIT_FAILURE;
         }
      }
       if (argc == 2) {
          printf(
             "- E(xit), L(oad new patterns data file), N(ext simulation),\n"
             "  R(un again), <Enter>(repeat) .... ");
       }
       else {
          printf("- E(xit), N(ext simulation), R(un again), <Enter>(repeat)"
                 " .... ");
       }
       if (!enterPressed) {
          clearInput();
       }
       menu = getchar();
   }

   hopfield_context_destroy(ctx);
   return 0;
}

static void usage(int argc)
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

static void clearInput(void)
{
   int c;
   while ((c = getchar()) != '\n' && c != EOF) {
   }
}

static void handle_error(HopfieldError err)
{
   switch (err) {
      case HOPFIELD_ERR_FILE_NOT_FOUND:
         fprintf(stderr, "Error: File not found\n");
         break;
      case HOPFIELD_ERR_INVALID_FORMAT:
         fprintf(stderr, "Error: Invalid file format\n");
         break;
      case HOPFIELD_ERR_INDEX_OUT_OF_RANGE:
         fprintf(stderr, "Error: Index out of range\n");
         break;
      case HOPFIELD_ERR_SIZE_EXCEEDED:
         fprintf(stderr, "Error: Pattern size exceeded maximum\n");
         break;
      default:
         fprintf(stderr, "Error: Unknown error (%d)\n", err);
   }
}
