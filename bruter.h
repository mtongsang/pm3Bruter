int readEM410xTag(const char *Cmd);
int waitForEM410xTag(const char *Cmd);
int executeCommand(char *execCmd);
int runBruterEM410X(int *mode, int *bruteSize, char *filePath, bool useCam, char *customTag);
