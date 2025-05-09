struct petersonlock {
  int used;           // Is this lock active
  int flag[2];        // Intent flags for the two processes
  int turn;           // Whose turn is it
};