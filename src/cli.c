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
   RULE_PSEUDO_INVERSE,
   RULE_DAYDREAMING,
   RULE_MODERN
} LearningRule;

/* Scratch buffers and recall parameters carried across the interactive
   session. Owned by run_cli and released in its cleanup path. */
typedef struct {
   double *inputPattern;
   double *inputPatternWithNoise;
   double *outputPattern;
   int indexPattern;
   int noise;
} SimState;

static bool usage(int argc);
static void clearInput(void);
static void handle_error(HopfieldError err);
static bool learnPatterns(HopfieldContext *ctx, LearningRule rule);
static const char *ruleNameOf(LearningRule rule);
static bool ensurePatternBuffers(HopfieldContext *ctx, double **inputPattern,
                                 double **inputPatternWithNoise,
                                 double **outputPattern);
static void print_neurons(const HopfieldContext *ctx);
static void prompt_learning_rule(LearningRule *rule);
static bool prepare_network(HopfieldContext *ctx, LearningRule rule,
                            SimState *state);
static bool read_and_prepare(HopfieldContext *ctx, const char fileName[],
                             LearningRule rule, SimState *state);
static bool prompt_reload(HopfieldContext *ctx, LearningRule rule,
                          SimState *state);
static bool run_simulation(HopfieldContext *ctx, int argc, bool repeat,
                           SimState *state);

int run_cli(HopfieldContext *ctx, int argc, char *argv[])
{
   int menu = 0;
   bool firstRun = true;
   LearningRule rule = RULE_HEBBIAN;
   const char menuChars[] = "ELNelnRr";
   SimState state = {0};
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
   printf("ready\n\n");
   print_neurons(ctx);

   prompt_learning_rule(&rule);

   if (!prepare_network(ctx, rule, &state)) {
      result = 1;
      goto cleanup;
   }

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
         if (!prompt_reload(ctx, rule, &state)) {
            result = 1;
            goto cleanup;
         }
      }

      if (firstRun || strchr(menuChars, menu) != NULL) {
         firstRun = false;
         bool repeat = (menu == 'R' || menu == 'r');
         if (!run_simulation(ctx, argc, repeat, &state)) {
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
   free(state.inputPattern);
   free(state.inputPatternWithNoise);
   free(state.outputPattern);
   return result;
}

/* Print the network dimensions derived from the loaded pattern file. */
static void print_neurons(const HopfieldContext *ctx)
{
   printf("- Number of neurons: %d * %d = %d, number of patterns = %d\n",
          ctx->nRows, ctx->nColumns, ctx->patternSize, ctx->nPatterns);
}

/* Ask for the learning rule and update *rule. A blank answer selects the
   default (Hebbian). */
static void prompt_learning_rule(LearningRule *rule)
{
   printf("- Learning rule: (H)ebbian, (S)torkey, (P)seudo-inverse, "
          "(D)aydreaming or (M)odern? [H]: ");
   int ruleChoice = getchar();
   if (ruleChoice == EOF) {
      printf("\n- No input, using Hebbian learning rule\n");
   }
   else if (ruleChoice == 'S' || ruleChoice == 's') {
      *rule = RULE_STORKEY;
   }
   else if (ruleChoice == 'P' || ruleChoice == 'p') {
      *rule = RULE_PSEUDO_INVERSE;
   }
   else if (ruleChoice == 'D' || ruleChoice == 'd') {
      *rule = RULE_DAYDREAMING;
   }
   else if (ruleChoice == 'M' || ruleChoice == 'm') {
      *rule = RULE_MODERN;
   }
   if (ruleChoice != '\n' && ruleChoice != EOF) {
      clearInput();
   }
   puts("");
}

/* (Re)allocate the session buffers, warn about the storage-capacity limit,
   then learn the loaded patterns with the given rule. Common to startup and
   file reload. */
static bool prepare_network(HopfieldContext *ctx, LearningRule rule,
                            SimState *state)
{
   if (!ensurePatternBuffers(ctx, &state->inputPattern,
                             &state->inputPatternWithNoise,
                             &state->outputPattern)) {
      fprintf(stderr, "Error: Out of memory\n");
      return false;
   }

   int stCapacity = storageCapacity(ctx->patternSize);
   if (stCapacity < ctx->nPatterns) {
      fprintf(stderr,
              "- Warning: associative storage capacity %d exceeded\n",
              stCapacity);
   }

   printf("- Learning patterns by %s learning rule, training starts .... ",
          ruleNameOf(rule));
   if (!learnPatterns(ctx, rule)) {
      fprintf(stderr, "Error: Failed to learn patterns\n");
      return false;
   }
   printf("ready\n");
   printf("- Learning result: 1 connection matrix, size %d x %d\n\n",
          ctx->nRows * ctx->nColumns, ctx->nRows * ctx->nColumns);
   return true;
}

/* Load a pattern file, report its dimensions, then learn (shared by the
   startup path's reload and the interactive L command). */
static bool read_and_prepare(HopfieldContext *ctx, const char fileName[],
                             LearningRule rule, SimState *state)
{
   printf("\n- Loading input patterns data .... ");
   HopfieldError err = readFile(ctx, fileName);
   if (err != HOPFIELD_OK) {
      handle_error(err);
      return false;
   }
   printf("ready\n\n");
   print_neurons(ctx);
   return prepare_network(ctx, rule, state);
}

/* Interactive file reload: read a file name (with overlong-name detection),
   load it and learn with the current rule. */
static bool prompt_reload(HopfieldContext *ctx, LearningRule rule,
                          SimState *state)
{
   char fileName[MAXFILENAME_SIZE] = {0};
   clearInput();
   printf("- Input patterns file name: ");
   if (fgets(fileName, MAXFILENAME_SIZE, stdin) == NULL) {
      fprintf(stderr, "\n\tERROR: could not read file name\n\n");
      return false;
   }
   size_t fileNameLen = strlen(fileName);
   if (fileNameLen > 0 && fileName[fileNameLen - 1] == '\n') {
      fileName[fileNameLen - 1] = '\0';
   }
   else if (fileNameLen == MAXFILENAME_SIZE - 1) {
      /* Buffer filled without a newline: the name is either exactly
         MAXFILENAME_SIZE - 1 characters (newline still pending) or
         longer. Peek at the next character to tell the two apart. */
      int c = getchar();
      if (c != '\n' && c != EOF) {
         clearInput();
         fprintf(stderr, "\n\tERROR: file name too long "
                         "(max %d characters)\n\n",
                         MAXFILENAME_SIZE - 1);
         return false;
      }
   }
   return read_and_prepare(ctx, fileName, rule, state);
}

/* Run one recall simulation. In mode 1 (argc == 2) the stored pattern is
   corrupted on the fly; in mode 2 (argc == 3) a pre-made noisy pattern is
   recalled. On repeat the previous index/noise parameters are reused. */
static bool run_simulation(HopfieldContext *ctx, int argc, bool repeat,
                           SimState *state)
{
   if (argc == 2) {
      if (!repeat) {
         printf("- Choose pattern to disturb by noise, index (1..%d): ",
                ctx->nPatterns);
         if (scanf(" %d", &state->indexPattern) != 1) {
            fprintf(stderr, "\n\tERROR: invalid input\n\n");
            return false;
         }
         puts("");
         if (state->indexPattern < 1 || state->indexPattern > ctx->nPatterns) {
            fprintf(stderr, "\n\tERROR: index %d out of range\n\n",
                   state->indexPattern);
            return false;
         }
         state->indexPattern--;
         showIndexedPattern(ctx, state->indexPattern);
         puts("");
         copyPattern(ctx->patternSize, ctx->patterns[state->indexPattern],
                     state->inputPattern);
         copyPattern(ctx->patternSize, ctx->patterns[state->indexPattern],
                     state->inputPatternWithNoise);
         printf("- Pattern %d noise level [%%]: ", state->indexPattern + 1);
         if (scanf(" %d", &state->noise) != 1) {
            fprintf(stderr, "\n\tERROR: invalid input\n\n");
            return false;
         }
         if (state->noise < 0 || state->noise > MAX_NOISE_PERCENT) {
            fprintf(stderr,
                    "\n\tERROR: noise level %d out of range (0..%d)\n\n",
                    state->noise, MAX_NOISE_PERCENT);
            return false;
         }
         if (state->noise > MAX_INFORMATIVE_NOISE_PERCENT) {
            printf("- Note: above %d%% noise the input is anti-correlated "
                   "with the stored pattern; the network converges to the "
                   "inverted pattern.\n",
                   MAX_INFORMATIVE_NOISE_PERCENT);
         }
      }

      addNoiseToPattern(ctx, state->indexPattern, state->noise);
      copyPattern(ctx->patternSize, ctx->noisyPatterns[state->indexPattern],
                  state->inputPatternWithNoise);
      printf("\n\n- Pattern %d as 2D image and %d%% noisy pixels:\n\n",
             state->indexPattern + 1, state->noise);
      showAssociatedPattern(ctx, state->inputPattern,
                           state->inputPatternWithNoise, state->outputPattern);
      puts("");
      return true;
   }

   if (argc == 3) {
      if (!repeat) {
         printf("\n- Choose noisy pattern, index (1..%d): ",
                ctx->nNoisyPatterns);
         if (scanf(" %d", &state->indexPattern) != 1) {
            fprintf(stderr, "\n\tERROR: invalid input\n\n");
            return false;
         }
         puts("");
         if (state->indexPattern < 1 ||
             state->indexPattern > ctx->nNoisyPatterns) {
            fprintf(stderr, "\n\tERROR: index %d out of range\n\n",
                   state->indexPattern);
            return false;
         }
         state->indexPattern--;
         puts("");
         showIndexedNoisyPattern(ctx, state->indexPattern);
         puts("");
         copyPattern(ctx->patternSize, ctx->noisyPatterns[state->indexPattern],
                     state->inputPattern);
      }
      printf("\n\n- Pattern %d as 2D image:\n\n", state->indexPattern + 1);
      showAssociatedPattern(ctx, state->inputPattern, state->inputPattern,
                           state->outputPattern);
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
   case RULE_DAYDREAMING:
      return "Daydreaming";
   case RULE_MODERN:
      return "modern Hopfield (softmax)";
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
   case RULE_DAYDREAMING:
      return learnDaydreaming(ctx);
   case RULE_MODERN:
      return learnModernHopfield(ctx);
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