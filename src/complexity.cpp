#include <cinttypes>
#include <vector>
#include <algorithm>
using namespace std;

#include "matchlength.h"
#include "periodicity.h"
#include "shulen.h"

// input: prefix-sum array, left and right bound (inclusive)
#define sumFromTo(a, l, r) ((a)[(r)] - ((l) ? (a)[(l)-1] : 0))

// calculate match length complexity for sliding windows
// input: sane w and k, allocated array for results, match length factors, gc content
void mlComplexity(size_t w, size_t k, vector<double> &y, Fact const &mlf, double gc) {
  size_t n = mlf.strLen/2; // mlf was calculated on both strands, we look on first only

  // compute observed number of match factors for every prefix
  vector<size_t> ps(n);
  size_t nextfact = 1;
  ps[0] = 1;
  for (size_t i = 1; i < n; i++) {
    ps[i] = ps[i - 1];
    if (nextfact < mlf.fact.size() && i == mlf.fact[nextfact]) {
      ps[i]++;
      nextfact++;
    }
  }

  // calculations (per nucleotide)
  double cMin = 2.0 / n; // at least 2 factors an any sequence, like AAAAAA.A
  // some wildly advanced estimation for avg. shulen length,
  // 2n because matches are from both strands
  double esl = expShulen(gc, 2 * n);
  double cAvg = 1.0 / (esl - 1.0); // expected # of match length factors / nucleotide

  // calc. for each window
  size_t entries = (n - w) / k + 1;
  for (size_t j = 0; j < entries; j++) {
    size_t l = j * k;
    size_t r = min(n, l + w) - 1;
    double cObs = (double)sumFromTo(ps, l, r) / (double)w;
    y[j] = (cObs /* - cMin */) / (cAvg - cMin);
  }
}

void runComplexity(size_t w, size_t k, vector<double> &y, size_t n,
                   vector<list<Periodicity>> const &ls) {
  vector<int64_t> ps(n, 0);
  for (size_t i = 0; i < n; i++) {
    for (auto it = ls[i].begin(); it != ls[i].end(); it++) {
      Periodicity p = *it;
      if (perLen(p) < 8)
        continue;
      // for (size_t j = p.b; j <= p.e; j += p.l) // increment for each starting "atom"
      //   ps[j]++;
      for (size_t j = p.b; j <= p.e; j++) // mark nucleotides inside runs
        ps[j] |= 1;
    }
  }

  // prefix sum (for fast range sum retrieval)
  for (size_t i = 1; i < n; i++)
    ps[i] += ps[i - 1];

  // double pMax = w; // no more runs than window size
  size_t entries = (n - w) / k + 1;
  for (size_t j = 0; j < entries; j++) {
    size_t l = j * k;
    size_t r = min(n, l + w) - 1;
    // double pObs = sumFromTo(ps, l, r);
    double pObs = (double)sumFromTo(ps, l, r) / (double)w;
    // y[j] = pObs / pMax;
    y[j] = 1 - pObs;
  }
}
