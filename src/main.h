#ifndef MAIN_H
#define MAIN_H

int main(int argc, char** argv);

typedef struct
{
  char** argv;
  int argc;
} ProgramArguments_t;

extern ProgramArguments_t ProgramArguments;

#endif // MAIN_H

