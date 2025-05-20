struct petersonlock {
  int used;
  int flag[2];
  int turn;
};