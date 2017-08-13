struct part {
	float xp,yp;
	float xv,yv;
	float mass;
};
struct scalar {
	float xp,yp;
};
struct phys_stw {
	struct scalar *dest;
	int start,end;
};
