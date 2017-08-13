char outfile[256]="./out.gsim";
FILE *outfp;
int fps;
int numframes;
int framewidth;
int frameheight;

int numphysthreads;
int iterations;
float dt;

int seed;
int nparts;
int physdeactivationradius;
int speedlimit;
float partmass;
float initxv,inityv;
pthread_t *physthreads;
struct phys_stw *phys_stws;
struct part *parts;
