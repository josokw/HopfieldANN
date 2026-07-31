#include "AppInfo.h"
#include "HopfieldContext.h"
#include "HopfieldCalc.h"
#include "HopfieldIO.h"
#include "HopfieldUtil.h"
#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define MAXFILENAME_SIZE 100

typedef enum {
   RULE_HEBBIAN,
   RULE_STORKEY,
   RULE_PSEUDO_INVERSE
} LearningRule;

static bool usage(int argc);
static void clearInput(void);
static void handle_error(HopfieldError err);
static bool learnPatterns(HopfieldContext *ctx, LearningRule rule);
static const char *ruleNameOf(LearningRule rule);
static bool ensurePatternBuffers(HopfieldContext *ctx, double **inputPattern,
                                 double **inputPatternWithNoise,
                                 double **outputPattern);
static bool run_simulation(HopfieldContext *ctx, int argc, bool repeat,
                           int *indexPattern, int *noise,
                           double *inputPattern,
                           double *inputPatternWithNoise,
                           double *outputPattern);

int run_cli(HopfieldContext *ctx, int argc, char *argv[])
{
   int noise = 0;
   int indexPattern = 0;
   int menu = 0;
   bool firstRun = true;
   LearningRule rule = RULE_HEBBIAN;
   const char *ruleName = ruleNameOf(rule);
   char fileName[MAXFILENAME_SIZE] = {0};
   const char menuChars[] = "ELNelnRr";

   double *inputPattern = NULL;
   double *inputPatternWithNoise = NULL;
   double *outputPattern = NULL;
   int result = 0;

   if (!usage(argc)) {
      return 1;
   }
   srand((unsigned int)time(NULL));

   printf("Hopfield's ANN associative memory: " APPNAME_VERSION
          "\n\n- Input patterns file name: %s, loading .... ",
          argv[1]);

   HopfieldError err = readFile(ctx, argv[1]);
   if (err != HOPFIELD_OK) {
      handle_error(err);
      result = 1;
      goto cleanup;
   }
   printf(
      "ready\n"
      "- Number of neurons: %d * %d = %d, number of patterns = %d\n",
      ctx->nRows, ctx->nColumns, ctx->patternSize, ctx->nPatterns);

   if (!ensurePatternBuffers(ctx, &inputPattern, &inputPatternWithNoise,
                             &outputPattern)) {
      fprintf(stderr, "Error: Out of memory\n");
      result = 1;
      goto cleanup;
   }

   int stCapacity = storageCapacity(ctx->patternSize);
   if (stCapacity < ctx->nPatterns) {
      fprintf(stderr,
              "- Warning: associative storage capacity %d exceeded\n",
              stCapacity);
   }

   int ruleChoice = 0;
   printf("- Learning rule: (H)ebbian, (S)torkey or (P)seudo-inverse? [H]: ");
   ruleChoice = getchar();
   if (ruleChoice == EOF) {
      printf("\n- No input, using Hebbian learning rule\n");
   }
   else if (ruleChoice == 'S' || ruleChoice == 's') {
      rule = RULE_STORKEY;
   }
   else if (ruleChoice == 'P' || ruleChoice == 'p') {
      rule = RULE_PSEUDO_INVERSE;
   }
   ruleName = ruleNameOf(rule);
   if (ruleChoice != '\n' && ruleChoice != EOF) {
      clearInput();
   }
   puts("");

   printf("- Learning patterns by %s learning rule, "
          "training starts .... ", ruleName);
   bool learned = learnPatterns(ctx, rule);
   if (!learned) {
      fprintf(stderr, "Error: Failed to learn patterns\n");
      result = 1;
      goto cleanup;
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
         result = 1;
         goto cleanup;
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
         clearInput();
         printf("- Input patterns file name: ");
          fgets(fileName, MAXFILENAME_SIZE, stdin);
          size_t fileNameLen = strlen(fileName);
          if (fileNameLen > 0 && fileName[fileNameLen - 1] == '\n') {
             fileName[fileNameLen - 1] = '\0';
         }
         printf("\n- Loading input patterns data .... ");
         err = readFile(ctx, fileName);
         if (err != HOPFIELD_OK) {
            handle_error(err);
            result = 1;
            goto cleanup;
         }
         printf(
            "ready\n\n"
            "- Number of neurons: %d * %d = %d, number of patterns: "
            "%d\n\n",
            ctx->nRows, ctx->nColumns, ctx->patternSize, ctx->nPatterns);

         if (!ensurePatternBuffers(ctx, &inputPattern,
                                   &inputPatternWithNoise, &outputPattern)) {
            fprintf(stderr, "Error: Out of memory\n");
            result = 1;
            goto cleanup;
         }

         stCapacity = storageCapacity(ctx->patternSize);
         if (stCapacity < ctx->nPatterns) {
            fprintf(
               stderr,
               "- Warning: associative storage capacity %d exceeded\n",
               stCapacity);
         }

          printf("- Learning patterns by %s learning rule .... ", ruleName);
          if (!learnPatterns(ctx, rule)) {
            fprintf(stderr, "Error: Failed to learn patterns\n");
            result = 1;
            goto cleanup;
         }
         printf("ready\n\n");
         printf("- Learning result: 1 connection matrix, size %d x %d\n\n",
                ctx->nRows * ctx->nColumns, ctx->nRows * ctx->nColumns);
      }

       if (firstRun || strchr(menuChars, menu) != NULL) {
          firstRun = false;
          bool repeat = (menu == 'R' || menu == 'r');
          if (!run_simulation(ctx, argc, repeat, &indexPattern, &noise,
                              inputPattern, inputPatternWithNoise,
                              outputPattern)) {
             result = 1;
             goto cleanup;
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

cleanup:
   free(inputPattern);
   free(inputPatternWithNoise);
   free(outputPattern);
   return result;
}

/* Run one recall simulation. In mode 1 (argc == 2) the stored pattern is
   corrupted on the fly; in mode 2 (argc == 3) a pre-made noisy pattern is
   recalled. On repeat the previous index/noise parameters are reused. */
static bool run_simulation(HopfieldContext *ctx, int argc, bool repeat,
                           int *indexPattern, int *noise,
                           double *inputPattern,
                           double *inputPatternWithNoise,
                           double *outputPattern)
{
   if (argc == 2) {
      if (!repeat) {
         printf("- Choose pattern to disturb by noise, index (1..%d): ",
                ctx->nPatterns);
         if (scanf(" %d", indexPattern) != 1) {
            fprintf(stderr, "\n\tERROR: invalid input\n\n");
            return false;
         }
         puts("");
         if (*indexPattern < 1 || *indexPattern > ctx->nPatterns) {
            fprintf(stderr, "\n\tERROR: index %d out of range\n\n",
                    *indexPattern);
            return false;
         }
         (*indexPattern)--;
         showIndexedPattern(ctx, *indexPattern);
         puts("");
         copyPattern(ctx->patternSize, ctx->patterns[*indexPattern],
                     inputPattern);
         copyPattern(ctx->patternSize, ctx->patterns[*indexPattern],
                     inputPatternWithNoise);
         printf("- Pattern %d noise level [%%]: ", *indexPattern + 1);
         if (scanf(" %d", noise) != 1) {
            fprintf(stderr, "\n\tERROR: invalid input\n\n");
            return false;
         }
         if (*noise < 0 || *noise > MAX_NOISE_PERCENT) {
            fprintf(stderr, "\n\tERROR: noise level %d out of range (0..%d)\n\n",
                    *noise, MAX_NOISE_PERCENT);
            return false;
         }
         if (*noise > MAX_INFORMATIVE_NOISE_PERCENT) {
            printf("- Note: above %d%% noise the input is anti-correlated "
                   "with the stored pattern; the network converges to the "
                   "inverted pattern.\n",
                   MAX_INFORMATIVE_NOISE_PERCENT);
         }
      }

      addNoiseToPattern(ctx, *indexPattern, *noise);
      copyPattern(ctx->patternSize, ctx->noisyPatterns[*indexPattern],
                  inputPatternWithNoise);
      printf("\n\n- Pattern %d as 2D image and %d%% noisy pixels:\n\n",
             *indexPattern + 1, *noise);
      showAssociatedPattern(ctx, inputPattern, inputPatternWithNoise,
                            outputPattern);
      puts("");
      return true;
   }

   if (argc == 3) {
      if (!repeat) {
         printf("\n- Choose noisy pattern, index (1..%d): ",
                ctx->nNoisyPatterns);
         if (scanf(" %d", indexPattern) != 1) {
            fprintf(stderr, "\n\tERROR: invalid input\n\n");
            return false;
         }
         puts("");
         if (*indexPattern < 1 || *indexPattern > ctx->nNoisyPatterns) {
            fprintf(stderr, "\n\tERROR: index %d out of range\n\n",
                    *indexPattern);
            return false;
         }
         (*indexPattern)--;
         puts("");
         showIndexedNoisyPattern(ctx, *indexPattern);
         puts("");
         copyPattern(ctx->patternSize, ctx->noisyPatterns[*indexPattern],
                     inputPattern);
      }
      printf("\n\n- Pattern %d as 2D image:\n\n", *indexPattern + 1);
      showAssociatedPattern(ctx, inputPattern, inputPattern, outputPattern);
      puts("");
      return true;
   }

   fprintf(stderr, "\n\tSYSTEM ERROR: this should never happen!\n\n");
   return false;
}

static bool usage(int argc)
{
   if ((argc == 2) || (argc == 3)) {
      return true;
   }
   fprintf(stderr,
           "\n\tUSAGE: hopfieldann <input patterns filename>\n");
   fprintf(stderr,
           "\n\tUSAGE: hopfieldann <input patterns filename> "
           "<noisy patterns filename>\n\n");
   return false;
}

static void clearInput(void)
{
   int c;
   while ((c = getchar()) != '\n' && c != EOF) {
   }
}

static const char *ruleNameOf(LearningRule rule)
{
   switch (rule) {
      case RULE_STORKEY:
         return "Storkey";
      case RULE_PSEUDO_INVERSE:
         return "pseudo-inverse";
      default:
         return "Hebbian";
   }
}

static bool learnPatterns(HopfieldContext *ctx, LearningRule rule)
{
   switch (rule) {
      case RULE_STORKEY:
         return learnStorkey(ctx);
      case RULE_PSEUDO_INVERSE:
         return learnPseudoInverse(ctx);
      default:
         return learnHebbian(ctx);
   }
}

static bool ensurePatternBuffers(HopfieldContext *ctx, double **inputPattern,
                                 double **inputPatternWithNoise,
                                 double **outputPattern)
{
   if (ctx == NULL || ctx->patternSize <= 0) {
      return false;
   }
   size_t bytes = (size_t)ctx->patternSize * sizeof(double);
   double *ip = (double *)realloc(*inputPattern, bytes);
   if (ip == NULL) {
      return false;
   }
   *inputPattern = ip;
   double *np = (double *)realloc(*inputPatternWithNoise, bytes);
   if (np == NULL) {
      return false;
   }
   *inputPatternWithNoise = np;
   double *op = (double *)realloc(*outputPattern, bytes);
   if (op == NULL) {
      return false;
   }
   *outputPattern = op;
   return true;
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
      case HOPFIELD_ERR_OUT_OF_MEMORY:
         fprintf(stderr, "Error: Out of memory\n");
         break;
      default:
         fprintf(stderr, "Error: Unknown error (%d)\n", err);
   }
}
