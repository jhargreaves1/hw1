//cmps335 HW1
//John Hargreaves
//
//cs335 
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
extern "C" {
	#include "fonts.h"
}

typedef float Flt;

#define MakeVector(x,y,z,v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecNegate(a) (a)[0]=(-(a)[0]); (a)[1]=(-(a)[1]); (a)[2]=(-(a)[2]);
#define VecDot(a,b) ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecCopy(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 10000
#define GRAVITY 0.1
#define NUM_BOXES 5
//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

typedef struct t_Sphere {
	Vec pos;
	Vec vel;
	float radius;
	float mass;
} Sphere;


struct Game {
  Shape boxs[NUM_BOXES];
  Shape bubbler;
  Shape bucket;
  Sphere sphere1;
  Particle particle[MAX_PARTICLES];
  int n;
  int b;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);
void initSphere(Game *game);

int main(void){
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	
	//declare game object
	Game game;
	game.n=0;
    game.b=0;
    
    initSphere(&game);
	//declare box shapes
	for (int i=0; i < NUM_BOXES; i++){
	game.boxs[i].width = 100;
	game.boxs[i].height = 10;
	game.boxs[i].center.x = 120 + i * 90;
	game.boxs[i].center.y = 500 - i * 100;
	}
	//declare bubbler shape
	game.bubbler.width = 10;
	game.bubbler.height = 20;
	game.bubbler.center.x = 100;
	game.bubbler.center.y = 510;
	//declare bucket shape
	game.bucket.width = 20;
	game.bucket.height = 20;
	game.bucket.center.x = 620;
	game.bucket.center.y = 60;
	//start animation
	while(!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void){
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 hw1   Waterfall process");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}


void drawSphere(Flt rad)
{
	int i;
	static int firsttime=1;
	static Flt verts[32][2];
	static int n=32;
	if (firsttime) {
		Flt ang=0.0;
		Flt inc = 3.14159 * 2.0 / (Flt)n;
		for (i=0; i<n; i++) {
			verts[i][0] = sin(ang);
			verts[i][1] = cos(ang);
			ang += inc;
		}
		firsttime=0;
	}
	glBegin(GL_TRIANGLE_FAN);
		for (i=0; i<n; i++) {
			glVertex2f(verts[i][0]*rad, verts[i][1]*rad);
		}
	glEnd();
}

void initSphere(Game *g)
{
	Sphere *s1 = &g->sphere1;
	s1->pos.x = 140;
	s1->pos.y = 570;
	s1->vel.x = 1.6;
	s1->vel.y = 0.0;
	s1->radius = 10.0;
	s1->mass = 1.0;
	
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
	  std::cout << "\n\tcannot connect to X server\n" << std::endl;
	  exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
	  std::cout << "\n\tno appropriate visual found\n" << std::endl;
	  exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
	  ButtonPress | ButtonReleaseMask |
	  PointerMotionMask |
	  StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
			    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void){
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	//Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void makeParticle(Game *game, int x, int y) {
	if (game->n >= MAX_PARTICLES)
		return;
//	std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->s.width=2;
	p->s.height=2;
	p->velocity.y =  2.0;
	p->velocity.x =  0.01;
	game->n++;

}




void check_mouse(XEvent *e, Game *game){
	static int savex = 0;
	static int savey = 0;
	static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			makeParticle(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
	  savex = e->xbutton.x;
	  savey = e->xbutton.y;
	  
	  if (++n < 10)
	    return;
	  //int y = WINDOW_HEIGHT - e->xbutton.y;
	}
}

int check_keys(XEvent *e, Game *game){
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
			return 1;
		}
		//You may check other keys here.

	}
	return 0;
}

void VecNormalize(Vec v)
{
	Flt dist = v.x*v.x + v.y*v.y + v.z*v.z;
	if (dist==0.0)
		return;
	Flt len = 1.0 / sqrt(dist);
	v.x *= len;
	v.y *= len;
	v.z *= len;
}


void movement(Game *game)
{
	Particle *p;
	Sphere *sp;
	
	//bubbler
	if(game->n < MAX_PARTICLES && game->b <3000){
	  int x=100,y=530;
	  for(int i=0;i<4;i++)
	   makeParticle(game,x, y);
	}
	
	//sphere
	sp= &game->sphere1;
	sp->pos.x += sp->vel.x;
	sp->pos.y += sp->vel.y;
	if(sp->vel.y > -2)
	sp->vel.y -= GRAVITY;
	if(sp->vel.y>0)
	sp->radius = sp->radius+ sp->vel.y/4;
	if(sp->pos.y < 0){
	sp->pos.y = 620;
	sp->pos.x = 0;
	if(sp->radius > 50)
		sp->radius = 10;
	}
	//particles
	if (game->n <= 0)
		return;
	for (int i=0;i<game->n;i++){
	p = &game->particle[i];
	p->s.center.x += p->velocity.x;
	p->s.center.y += p->velocity.y;
	p->velocity.y -= GRAVITY;

	//check for collision with shapes...
	Shape *s;
	for(int b =0;b<NUM_BOXES;b++){
	s = &game->boxs[b];
	// collision sphere with box
	Flt distance;
		distance = sp->pos.y - sp->radius;
	//std::cout << "distance= " << distance << " y velocity= " << sp->vel.y << std::endl;
	if(	distance < s->center.y + s->height && 
		distance > s->center.y - s->height &&
		sp->pos.x < s->center.x + s->width &&
		sp->pos.x > s->center.x - s->width){
		//std::cout << "Collision" <<  std::endl;
		sp->vel.y = -(sp->vel.y)*1;
		sp->pos.x++;
		sp->pos.y++;
	}
	//
	//check for sphere collision with bucket
        s = &game->bucket;
        distance = sp->pos.y - sp->radius;
        if(	distance < s->center.y + s->height && 
		distance > s->center.y - s->height &&
		sp->pos.x < s->center.x + s->width &&
		sp->pos.x > s->center.x - s->width){
		sp->vel.y = -(sp->vel.y)*1;
		sp->pos.x++;
		sp->pos.y++;
		}
	//collision particle with box
	s = &game->boxs[b];
	if( (p->s.center.y <= s->center.y + s->height ) &&   
	    (p->s.center.y >= s->center.y - s->height ) &&
	    (p->s.center.x <= s->center.x + s->width  ) &&   
	    (p->s.center.x >= s->center.x - s->width) )	{
		  
	//	std::cout << "y velocity= " << p->velocity.y << " x velocity= " << p->velocity.x << std::endl;
		p->s.center.y = s->center.y + s->height + 0.1;
		p->velocity.y = (-(p->velocity.y/2));    // -0.5;
		if(p->velocity.y >= 0)		
		p->velocity.y -= GRAVITY*3;
		p->s.center.y = s->center.y + s->height + p->s.height;
		if( ( (p->velocity.x) >= 0 && (p->velocity.x) < 0.5 ) &&
		    ( (p->velocity.x) <  0 && (p->velocity.x) > -0.5) )
		  p->velocity.x = p->velocity.x*1.01;
		
	//	std::cout << "y velocity= " << p->velocity.y << " x velocity = " << p->velocity.x << std::endl;
	//	std::cout << "number of particles " << game->n << std::endl;
		}
	}
       
	//collision with other particles
	Particle *op;
	for(int j=0;j < game->n; j++) {
		op = &game->particle[j];
		if(p->s.center.y <= op->s.center.y+op->s.height && 
		   p->s.center.y >= op->s.center.y-op->s.height &&
		   p->s.center.x <= op->s.center.x+op->s.width && 
		   p->s.center.x >= op->s.center.x-op->s.width && 
		   i!=j)  {
		  int x = p->velocity.x;
		  int y = p->velocity.y;
		  if(y>0){y=y-GRAVITY;}else y=y+GRAVITY;
		  
		  if(p->velocity.x > 0 && p->velocity.x <1)
			p->velocity.x = p->velocity.x + op->velocity.x/16;
		else
			p->velocity.x = p->velocity.x - op->velocity.x/16;
		  		   
		  if(op->velocity.x > 0 && op->velocity.x < 2)
		  op->velocity.x = op->velocity.x + x/16;
		else
		  op->velocity.x = op->velocity.x - x/16;
		
		  p->s.center.y = p->s.center.y - 0.2;
		  p->s.center.x = p->s.center.x + 0.4;
		  op->s.center.x = op->s.center.x - 0.4;
		  op->s.center.y = op->s.center.y -0.1;
		  }
	  }
		//check for bucket collision	
        s = &game->bucket;
        if ( (p->s.center.y <= s->center.y + s->height ) &&   
	    (p->s.center.y >= s->center.y - s->height -5 ) &&
	    (p->s.center.x <= s->center.x + s->width +5 ) &&   
	    (p->s.center.x >= s->center.x - s->width -5 ))	{
               p->velocity.y = 0;
               p->velocity.x = 0;
	       game->b++;
              
                if (p->s.center.x < s->center.x - (s->width/2)){
                p->s.center.x = p->s.center.x+15;
                p->s.center.y =p->s.center.y+3;
                        }
                if (p->s.center.x > s->center.x + (s->width/2)){
                        p->s.center.x = p->s.center.x-5;
                        p->s.center.y = p->s.center.y+3;
                         }
                if (p->s.center.y < s->center.y ){
                p->s.center.y = p->s.center.y+20;
                p->velocity.y =2;
                }
           }

        //check for off-screen
	if (p->s.center.y < 1)  {
        //std::cout << "y = " << p->s.center.y << " x = " << p->s.center.x << std::endl;
	    game->particle[i]=game->particle[game->n-1];
	    game->n--;
	  }
	}
        // is bucket full?
	Shape *s =  &game->bucket;
        
        if(game->b > 0 && s->center.y>-15 && game->n > 0)
        s->center.y = s->center.y -0.5;
		//std::cout << "bucket at " << s->center.y << std::endl;
		//std::cout << "game b= " << game->b << std::endl;
		
		if(s->center.y<=60 && game->b==0){
			s->center.y = s->center.y + 0.5;
	}
	//std::cout << "number of particles " << game->n << std::endl;
	if(s->center.y < -10 ){
	  //game->n = game->n-1;
	    if(game->n <= 15) {
	      s->center.y = s->center.y + 0.5;
	      game->n = 0;
	      game->b = 0; 
	    }
	}
	
}

void render(Game *game){
	float w, h;
	
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...
	
	//draw Sphere
	glColor3ub(250,250,0);
	glPushMatrix();
	glTranslatef(game->sphere1.pos.x, game->sphere1.pos.y, game->sphere1.pos.z);
	drawSphere(game->sphere1.radius);
	glPopMatrix();
	
	//draw boxs
	Shape *s;
	glColor3ub(90,140,90);
	for(int b=0;b<NUM_BOXES;b++){
	s = &game->boxs[b];
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
	
	glEnd();
	glPopMatrix();
	}
	//draw bubbler
	glColor3ub(200,140,90);
	s = &game->bubbler;
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
	glVertex2i(-w,-h);
	glVertex2i(-w, h);
	glVertex2i( w, h);
	glVertex2i( w,-h);
	glEnd();
	glPopMatrix();
	
	//draw bucket
	s = &game->bucket;
	glColor3ub(200, 140, 90 +s->center.y *2);
	
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
	glVertex2i(-w,-h);
	glVertex2i(-w, h);
	glVertex2i( w, h);
	glVertex2i( w,-h);
	glEnd();
	glPopMatrix();

	//draw all particles here
	glPushMatrix();
	int cl1=150,cl2=160,cl3=220;
	for (int i=0;i<game->n;i++) {
	  glColor3ub(cl1,cl2++,cl3++);
	  if(cl3>250) cl3=120;
	  // if(cl1>250) cl1=100;
	  if(cl2>200) cl2=140;
	Vec *c = &game->particle[i].s.center;
	w = 2;
	h = 2;
	glBegin(GL_QUADS);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);
	}
	
	Rect r;
	//
	r.bot = WINDOW_HEIGHT - 50;
	ggprint16(&r, 16, 0x00ff0000, "");
	
	r.left = 300;
	ggprint16(&r, 16, 0x00ff0000, "CS335 Waterfall Model");
	r.left = 150;
	r.bot = WINDOW_HEIGHT -110;
	ggprint16(&r, 16, 0x00ffff00,"Requirements " );
	r.left = 200;
	r.bot = WINDOW_HEIGHT -210;
	ggprint16(&r, 16, 0x00ffff00,"Design " );
	r.left = 300;
	r.bot = WINDOW_HEIGHT -310;
	ggprint16(&r, 16, 0x00ffff00,"Coding " );
	r.left = 400;
	r.bot = WINDOW_HEIGHT -410;
	ggprint16(&r, 16, 0x00ffff00,"Testing" );
	r.left = 500;
	r.bot = WINDOW_HEIGHT -510;
	ggprint16(&r, 16, 0x00ffff00,"Maintenance " );
	
		glEnd();
		glPopMatrix();
	
}




