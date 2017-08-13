#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#define PARTSIZE 3
#include "inc/structs.c"
#include "inc/global_vars.c"
#include "inc/readconf.c"
#include "inc/misc.c"
#include "inc/simulation.c"
void init() {
	phys_init();
}
int main(int argc, char *argv[]) {
	system("clear");
	fps=60;
	if(argc==1) {
		readsettings("./gsim.conf");
	}
	else if(argc==3) {
		readsettings(argv[1]);
		strcpy(outfile,argv[2]);
	}
	outfp=fopen(outfile,"w");
	fwrite(&numframes,sizeof(int),1,outfp);
	fwrite(&framewidth,sizeof(int),1,outfp);
	fwrite(&frameheight,sizeof(int),1,outfp);
	fwrite(&nparts,sizeof(int),1,outfp);
	srand(seed);
	init();
	/*int i;
	for(i=0;i<numphysthreads;++i) {
		printf("%d, %d\n",phys_stws[i].start,phys_stws[i].end);
	}*/
	//phys_place_galaxy(framewidth/2,frameheight/2,0);
	printf("Simulating...\n");
	dt=(float)fps*(float)iterations;
	int start=time(NULL);
	phys_simulate();
	printf("Took %d seconds\n",(int)time(NULL)-start);
	fclose(outfp);
	return 0;
}
