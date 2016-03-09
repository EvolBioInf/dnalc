#include "minunit.h"
#include <inttypes.h>
#include <string.h>
#include <time.h>

#include "esa.h"
#include "lempelziv.h"
#include "stringUtil.h"

// hotspot paper example
static char *seq = "GCACGCACGCACACACACACACACACACACACACACACACACACACACACACACACACACACACACACA"
                   "CACATATGCTAACTCTCAGTCTGTGTGTGCA$";

// correct factorization
static char *factors[] = {
    "G",   "C",        "A",
    "C",   "GCACGCAC", "ACACACACACACACACACACACACACACACACACACACACACACACACACACACACACACA",
    "T",   "AT",       "GC",
    "TA",  "AC",       "T",
    "CTC", "A",        "G",
    "TCT", "GT",       "GTGTG",
    "CA",  "$"};

// prevOcc example from Crochemore/Ilie paper
static char *str = "abbaabbbaaabab";
/* static int64_t poResult[] = {3,3,0,10,0,-1,0,7,2,1,2,1,-1,1}; //prevOcc[sa[i]] */

char *test_LempelZiv() {
  size_t n = strlen(seq);
  Esa *esa = getEsa(seq, n); // calculate esa, including $
  Fact *lzf = computeLZFact(esa, false);
  mu_assert(lzf->n == 20, "wrong number of LZ factors");

  for (size_t i = 0; i < lzf->n; i++) {
    mu_assert(!strncmp(lzf->str + lzf->fact[i], factors[i], factLen(lzf, i)),
              "wrong factor");
  }

  freeFact(lzf);
  freeEsa(esa);
  return NULL;
}

char *test_prevOcc() {
  size_t n = strlen(str);
  Esa *esa = getEsa(str, n);
  Fact *lzf = computeLZFact(esa, false);
  Fact *lzfRef = computeLZFact(esa, true);

  // should be same as poResult[sa[i]]
  for (size_t i = 0; i < n; i++)
    mu_assert(lzf->prevOcc[i] == lzfRef->prevOcc[i], "wrong prevOcc array");

  freeFact(lzf);
  freeFact(lzfRef);
  freeEsa(esa);
  return NULL;
}

char *test_randomSequence() {
  size_t n = 1000000;
  char *s = randSeq(n);
  fprintnf(stdout, s, 80);
  printf("\n");
  Esa *esa = getEsa(s, n + 1);
  Fact *lzf = computeLZFact(esa, false);
  Fact *lzfRef = computeLZFact(esa, true); // using different algorithm

  for (size_t i = 0; i < esa->n; i++)
    mu_assert(lzf->prevOcc[i] == lzfRef->prevOcc[i], "incorrect prevOcc array");
  for (size_t i = 0; i < esa->n; i++)
    mu_assert(lzf->lpf[i] == lzfRef->lpf[i], "incorrect lpf array");
  mu_assert(lzf->n == lzfRef->n, "different number of factors");
  for (size_t i = 0; i < lzf->n; i++)
    mu_assert(lzf->fact[i] == lzfRef->fact[i], "incorrect factor positions");

  for (size_t i = 0; i < lzf->n; i++) {
    int64_t po = lzf->prevOcc[lzf->fact[i]];
    bool tmp = po <= (int64_t)lzf->fact[i];
    if (!tmp)
      printf("prev: %ld curr: %ld\n", po, lzf->fact[i]);
    mu_assert(tmp, "prev can not be after");
    if (po >= 0)
      mu_assert(!strncmp(esa->str + lzf->fact[i], esa->str + po, factLen(lzf, i)),
                "prev is not a match");
  }

  freeFact(lzfRef);
  freeFact(lzf);
  freeEsa(esa);
  free(s);
  return NULL;
}

char *all_tests() {
  srand(time(NULL));
  mu_suite_start();
  mu_run_test(test_LempelZiv);
  mu_run_test(test_prevOcc);
  for (size_t i = 0; i < 5; i++)
    mu_run_test(test_randomSequence);
  return NULL;
}
RUN_TESTS(all_tests)
