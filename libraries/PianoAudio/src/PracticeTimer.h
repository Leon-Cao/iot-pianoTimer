#pragma once
#include <stdint.h>
// All intervals are half-open and expressed in monotonic microseconds.
class PracticeTimer {
 public:
  uint64_t totalUs=0;
  void reset() { totalUs=0; lastEnd=lastPianoEnd=0; haveInput=havePiano=false; }
  void discontinuity() { havePiano=false; }
  void add(bool piano, uint64_t start, uint64_t end) {
    if (end<=start) return;
    if (haveInput && start<lastEnd) start=lastEnd; // never double count
    if (end<=start) return;
    if (haveInput && start>lastEnd) havePiano=false; // missing audio is not silence
    if (piano) {
      if (havePiano && start>=lastPianoEnd && start-lastPianoEnd<=1000000ULL)
        totalUs+=start-lastPianoEnd;
      totalUs+=end-start;
      lastPianoEnd=end; havePiano=true;
    } else if (havePiano && end-lastPianoEnd>1000000ULL) havePiano=false;
    lastEnd=end; haveInput=true;
  }
 private:
  uint64_t lastEnd=0,lastPianoEnd=0;
  bool haveInput=false,havePiano=false;
};
