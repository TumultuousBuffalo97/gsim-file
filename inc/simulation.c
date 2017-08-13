#include <math.h>
long unsigned int phys_numcomparsfunc(int numparts) {
	return 0.5*((float)numparts*(float)numparts)-0.5*(float)numparts;
}
long unsigned int phys_numcomparsfuncinverse(int ncomps) {
	return (sqrt(8*ncomps+1)+1)/2.0;
}
void phys_schedule_threads() {
	
	int i;
	int numcompsperthread=phys_numcomparsfunc(nparts)/numphysthreads;
	phys_stws[0].start=0;
	phys_stws[0].end=phys_numcomparsfuncinverse((0+1)*numcompsperthread);
	for(i=1;i<numphysthreads;++i) {
		phys_stws[i].start=phys_stws[i-1].end;
		phys_stws[i].end=phys_numcomparsfuncinverse((i+1)*numcompsperthread);
	}
	phys_stws[i-1].end=nparts-1;
}
void phys_schedule_threads_slow() {
	unsigned long int a,b,counter=0,numcalcsperthread=0;
	int tcounter=0;
	if(numphysthreads==1) {
		phys_stws[0].start=0;
		phys_stws[0].end=nparts;
		return ;
	}
	for(a=0;a<nparts;++a) {
		for(b=a+1;b<nparts;++b) {
			numcalcsperthread++;
			
		}
	}
	numcalcsperthread/=numphysthreads;
	for(a=0;a<nparts;++a) {
		for(b=a+1;b<nparts;++b) {
			counter++;
			if(counter>=numcalcsperthread*(tcounter+1)) {
				break;
			}
		}
		if(counter>=numcalcsperthread*(tcounter+1)) {
			phys_stws[tcounter].start=phys_stws[tcounter-1].end-1;
			phys_stws[tcounter].end=a;
			tcounter++;
		}
	}
	phys_stws[0].start=0;
	phys_stws[tcounter].start=phys_stws[tcounter-1].end;
	phys_stws[tcounter].end=nparts;
}
void phys_init_parts() {
	int i;
	for(i=0;i<nparts;++i) {
		parts[i].xp=(float)randint(framewidth-PARTSIZE-2)+(float)1+(float)randint(8000)/(float)100000;
		parts[i].yp=(float)randint(frameheight-PARTSIZE-2)+(float)1+(float)randint(8000)/(float)100000;
		parts[i].xv=inityv;
		parts[i].yv=initxv;
		parts[i].mass=partmass;
	}
}
void phys_add_part(int x, int y, int xv, int yv, float mass) {
	parts[nparts].xp=x;
	parts[nparts].yp=y;
	parts[nparts].xv=xv;
	parts[nparts].yv=yv;
	parts[nparts].mass=mass;
	nparts++;
}
void phys_place_galaxy(int x, int y, int radius) {
	nparts=0;
	phys_add_part(x,y,0,0,100);
	int i;
	for(i=1;i<101;++i) {
		
	}
}
void (*phys_calc_gravity)(struct part *p1, struct part *p2);
void phys_calc_gravity_deactive(struct part *p1, struct part *p2) {
	float dx,dy;
	float dist;
	dx=p2->xp-p1->xp;
	dy=p2->yp-p1->yp;
	dist=dx*dx+dy*dy;
	if(sqrt(dist)<=physdeactivationradius) {
		p1->xv+=p2->mass*dx/dist/(float)fps/(float)iterations;
		p1->yv+=p2->mass*dy/dist/(float)fps/(float)iterations;
		p2->xv-=p1->mass*dx/dist/(float)fps/(float)iterations;
		p2->yv-=p1->mass*dy/dist/(float)fps/(float)iterations;
	}
}
void phys_calc_gravity_mass(struct part *p1, struct part *p2) {//18 flops?
	float dx,dy;
	float f;
	dx=p2->xp-p1->xp;
	dy=p2->yp-p1->yp;
	f=(dx*dx+dy*dy)*dt;
	p1->xv+=p2->mass*dx/f;
	p1->yv+=p2->mass*dy/f;
	p2->xv-=p1->mass*dx/f;
	p2->yv-=p1->mass*dy/f;
}
void phys_calc_gravity_minimal(struct part *p1, struct part *p2) {
	float dx,dy,xf,yf;
	float dist;
	dx=p2->xp-p1->xp;
	dy=p2->yp-p1->yp;
	dist=dx*dx+dy*dy;
	xf=dx/dist/(float)fps/(float)iterations;
	yf=dy/dist/(float)fps/(float)iterations;
	p1->xv+=xf;
	p1->yv+=yf;
	p2->xv-=xf;
	p2->yv-=yf;
}
void *(*phys_step_thread)(struct phys_stw *wrapper);
void *phys_step_thread_minimal(struct phys_stw *wrapper) {
	static __thread int i,a,b;
	static __thread float nextx,nexty;
	for(i=0;i<iterations;++i) {
		for(a=wrapper->start;a<wrapper->end;++a) {
			for(b=a+1;b<nparts;++b) {
				phys_calc_gravity(&parts[a],&parts[b]);
			}
			nextx=parts[a].xp+parts[a].xv;
			if((nextx>framewidth-PARTSIZE)|(nextx<=0)) {
				parts[a].xv*=-.5;
			}
			nexty=parts[a].yp+parts[a].yv;
			if((nexty>frameheight-PARTSIZE)|(nexty<=0)) {
				parts[a].yv*=-.5;
			}
		}
	}
	return 0;
}
void *phys_step_thread_speedlimit(struct phys_stw *wrapper) {
	static __thread int i,a,b;
	static __thread float nextx,nexty,velo;
	for(i=0;i<iterations;++i) {
		for(a=wrapper->start;a<wrapper->end;++a) {
			for(b=a+1;b<nparts;++b) {
				phys_calc_gravity(&parts[a],&parts[b]);
			}
			velo=sqrt((parts[a].xv*parts[a].xv)+(parts[a].yv*parts[a].yv));
			if(velo>speedlimit) {
				velo/=(float)speedlimit;
				parts[a].xv/=velo;
				parts[a].yv/=velo;
			}
			nextx=parts[a].xp+parts[a].xv;
			if((nextx>framewidth-PARTSIZE)|(nextx<=0)) {
				parts[a].xv*=-.5;
			}
			nexty=parts[a].yp+parts[a].yv;
			if((nexty>frameheight-PARTSIZE)|(nexty<=0)) {
				parts[a].yv*=-.5;
			}
		}
	}
	return 0;
}
void phys_init() {
	//allocate memory for physics threads
	physthreads=calloc(numphysthreads,sizeof(pthread_t));
	//allocate memory for physics step thread argument wrappers
	phys_stws=calloc(numphysthreads,sizeof(struct phys_stw));
	//allocate memory for particles
	parts=calloc(nparts,sizeof(struct part));
	if(physdeactivationradius==0) {
		if(partmass==1) {
			phys_calc_gravity=&phys_calc_gravity_minimal;
		}
		else {
			phys_calc_gravity=&phys_calc_gravity_mass;
		}
	}
	else {
		phys_calc_gravity=&phys_calc_gravity_deactive;
	}
	if(speedlimit==0) {
		phys_step_thread=&phys_step_thread_minimal;
	}
	else {
		phys_step_thread=&phys_step_thread_speedlimit;
	}
	phys_schedule_threads_slow();
	phys_init_parts();
}
void *phys_displayprogress(int *fnum) {
	int seconds=0;
	int minutes=0;
	int hours=0;
	int start=time(NULL);
	int now=start;
	int then;
	int sincestart;
	float avgfps=0;
	then=time(NULL);
	sincestart=now-start;
	while(1) {
		now=time(NULL);
		if(now-then>=1) {
			sincestart=now-start;
			seconds=sincestart%60;
			sincestart/=60;
			minutes=sincestart%60;
			sincestart/=60;
			hours=sincestart%60;
			avgfps=(float)*fnum/(float)(now-start);
		}
		printf("\033[2;0H%f%%, %d/%d, elapsed time: %d:%02d:%02d, avg fps: %f\n",100*(*fnum)*((float)1/numframes),*fnum,numframes,hours,minutes,seconds,avgfps);
		//printf("\033[2;0H%f%%, %d/%d, elapsed time: %d, fps (s): %d, fps (avg): %f\n",100*(*fnum)*((float)1/numframes),*fnum,numframes,(int)(time(NULL)-start),fps_s,fps_avg);
		then=time(NULL);
		
		usleep(100000);
	}
}
void phys_simulate() {
	int f,t;
	pthread_t dt;
	pthread_create(&dt,NULL,(void *)phys_displayprogress,&f);
	for(f=0;f<numframes;f++) {
		for(t=1;t<numphysthreads;++t) {
			pthread_create(&physthreads[t],NULL,(void *)phys_step_thread,&phys_stws[t]);
		}
		phys_step_thread(&phys_stws[0]);
		for(t=1;t<numphysthreads;++t) {
			pthread_join(physthreads[t],NULL);
		}
		for(t=0;t<nparts;++t) {
			fwrite(&parts[t].xp,sizeof(float),1,outfp);
			fwrite(&parts[t].yp,sizeof(float),1,outfp);
			parts[t].xp+=parts[t].xv;
			parts[t].yp+=parts[t].yv;
		}
					
	}
}
