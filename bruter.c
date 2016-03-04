#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "proxmark3.h"
#include "ui.h"
#include "util.h"
#include "cmdparser.h"
#include "cmddata.h"
#include "cmdlf.h"
#include "cmdlfem4x.h"
#include "bruter.h"
#include "sleep.h"

float compareDifference = 0.96; //Level of difference for image compare. 1 = exact

char simCMD[20];
char cmd[128];
char id[12] = {0x00};
char readBuffer[128];
uint32_t hi=0;
uint64_t lo=0;

int readEM410xTag(const char *Cmd)
{
	if(!AskEm410xDemod("", &hi, &lo, false)) return 0;
	PrintAndLog("##Bruter - EM410x pattern found: ");
	printEM410x(hi, lo);
	if (hi){
		PrintAndLog ("##Bruter - EM410x XL pattern found");
		return 0;
	}
	sprintf(id, "%010"PRIx64,lo);
	return 1;
}
int waitForEM410xTag(const char *Cmd)
{
	do {
		if (ukbhit()) {
			printf("\n##Bruter - aborted via keyboard!\n");
			break;
		}
		CmdLFRead("s");
		getSamples("8201",true); //capture enough to get 2 complete preambles (4096*2+9)	
	} while (!readEM410xTag(""));
	return 0;
}
int executeCommand(char *execCmd)
{
	FILE* pipe = popen(execCmd, "r");
	if (pipe){
		while(!feof(pipe)){
			if(fgets(readBuffer, 128, pipe) != NULL){}
		}
		pclose(pipe);
		readBuffer[strlen(readBuffer)-1] = '\0';
	}
	return 1;
}
int runBruterEM410X(int *mode, int *bruteSize, char *filePath, bool useCam, char *customTag)
{
	if(strcmp(customTag, "N") == 0)
	{
		printf("##Bruter - Started. Waiting for valid tag.\n");
		waitForEM410xTag("");
		printf("##Bruter - Captured ID: %s\n", id);
	}
	else
	{
		printf("##Bruter - Started. Custom tag provided: %s\n", customTag);
		sprintf(id, "%s",customTag);
		lo = strtoul(id, 0, 16);
	}
	uint64_t id_as_int = lo;
	uint64_t id_as_intPlus = id_as_int;
	uint64_t id_as_intMin = id_as_int;
	float imgDiff = 0;
	if(useCam){
		msleep(1000);
		sprintf(simCMD, "%s","0101010101"); 
		CmdEM410xSim(simCMD);
		sprintf(cmd, "fswebcam --no-banner -q --png 5 %sfalseImage.png",filePath);
                executeCommand(cmd);

		msleep(2000);
		sprintf(cmd, "convert %sfalseImage.png -set colorspace RGB -equalize %sfalseImage.png",filePath,filePath);
		executeCommand(cmd);
	}
	for(int j = 5; j > 0; j--)
	{
		printf("##Bruter - Brute force starts in %d seconds.\n", j);
		msleep(1000);
	}
	bool up = true;
	for(int i = 0; i < *bruteSize; i++)
	{
		printf("##Bruter - Testing %d of %d\n",(i+1),*bruteSize);
		if(*mode==3)
			id_as_int++;
		if(*mode==2)
			id_as_int--;
		sprintf(id, "%010"PRIx64,id_as_int);
		if(*mode==1)
		{
			if(up){
				id_as_intPlus++;
				sprintf(id, "%010"PRIx64,id_as_intPlus);
			}
			if(!up){
				id_as_intMin--;
				sprintf(id, "%010"PRIx64,id_as_intMin);
			}
			up = !up;
		}
		sprintf(simCMD, "%s",id);
		CmdEM410xSim(simCMD);
		if(useCam){
			sprintf(cmd, "fswebcam --no-banner -q --png 5 %s%s.png",filePath,id);
			executeCommand(cmd);
		}
		msleep(2000);//delay between attempts
		if(useCam){
			sprintf(cmd, "convert %s%s.png -set colorspace RGB -equalize %s%s.png",filePath,id,filePath,id);
			executeCommand(cmd);
			sprintf(cmd, "compare -metric ncc -alpha off  %sfalseImage.png %s%s.png null: 2>&1",filePath,filePath,id);
			executeCommand(cmd);
			sscanf(readBuffer, "%f", &imgDiff);
			printf("\n\n##Bruter - compareVal: %f\n\n", imgDiff);
			if(imgDiff < compareDifference)
			{
				printf("\n##Bruter - We might have found a valid tag: %sPOSITIVE_%s.png\n",filePath,id);
				sprintf(cmd, "mv %s%s.png %sPOSITIVE_%s.png",filePath,id,filePath,id);
				executeCommand(cmd);
				imgDiff = 0;
			}
		}
	}
	printf("\nBruter run finished!\n\n\n");
	return 0;
}
