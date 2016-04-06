//cmps335 HW1
//John Hargreaves
//
//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 1000
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

struct Game {
  Shape boxs[NUM_BOXES];
  Shape bubbler;
  Particle particle[MAX_PARTICLES];
  int n;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void){
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=0;

	//declare box shapes
	for (int i=0; i < NUM_BOXES; i++){
	game.boxs[i].width = 100;
	game.boxs[i].height = 10;
	game.boxs[i].center.x = 120 + i * 90;
	game.boxs[i].center.y = 500 - i * 70;
	}
	//declare bubbler shape
	game.bubbler.width = 10;
	game.bubbler.height = 20;
	game.bubbler.center.x = 100;
	game.bubbler.center.y = 510;
	
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
	XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
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
}

void makeParticle(Game *game, int x, int y) {
	if (game->n >= MAX_PARTICLES)
		return;
	std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->s.width=5;
	p->s.height=5;
	p->velocity.y =  3.0;
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

void movement(Game *game)
{
	Particle *p;

	//bubler
	if(game->n < MAX_PARTICLES-20){
	  int x=100,y=530;
	   makeParticle(game,x, y);
	}
	
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
	if( (p->s.center.y <= s->center.y + s->height ) &&   
	    (p->s.center.y >= s->center.y - s->height ) &&
	    (p->s.center.x <= s->center.x + s->width  ) &&   
	    (p->s.center.x >= s->center.x - s->width) )	{
		  //collision with box	
	std::cout << "y velocity= " << p->velocity.y << " x velocity= " << p->velocity.x << std::endl;
		p->s.center.y = s->center.y + s->height + 0.1;
		p->velocity.y = (-(p->velocity.y/2));    // -0.5;
		if(p->velocity.y >= 0)		
		p->velocity.y -= GRAVITY*3;
		p->s.center.y = s->center.y + s->height + p->s.height;
		if( ( (p->velocity.x) >= 0 && (p->velocity.x) < 0.5 ) &&
		    ( (p->velocity.x) <  0 && (p->velocity.x) > -0.5) )
		  p->velocity.x = p->velocity.x*1.01;
		
	std::cout << "y velocity= " << p->velocity.y << " x velocity = " << p->velocity.x << std::endl;
	std::cout << "number of particles " << game->n << std::endl;
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
		  
		  //if(p->velocity.y > 0 )
		  //p->velocity.y = (p->velocity.y) - op->velocity.y/16;
		  
		  if(op->velocity.x > 0 && op->velocity.x < 2)
		  op->velocity.x = op->velocity.x + x/16;
		else
		  op->velocity.x = op->velocity.x - x/16;
		
		  //if(op->velocity.y > 0 && op->velocity.y < 2)
		  //op->velocity.y = (op->velocity.y) - y/16;
		  p->s.center.y = p->s.center.y - 0.2;
		  p->s.center.x = p->s.center.x + 0.4;
		  op->s.center.x = op->s.center.x - 0.4;
		  op->s.center.y = op->s.center.y -0.1;
		  }
	  }
	//check for off-screen
	if (p->s.center.y < 0.0)  {
	    game->particle[i]=game->particle[game->n-1];
	    game->n--;
	  }
	}
}

void render(Game *game){
	float w, h;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...

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
	
	//draw all particles here
	glPushMatrix();
	int cl1=150,cl2=160,cl3=220;
	for (int i=0;i<game->n;i++) {
	  glColor3ub(cl1,cl2++,cl3++);
	  if(cl3>250) cl3=120;
	  // if(cl1>250) cl1=100;
	  if(cl2>200) cl2=140;

	Vec *c = &game->particle[i].s.center;
	w = 3;
	h = 3;
	
	glBegin(GL_QUADS);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);
	}
		glEnd();
		glPopMatrix();
	
}




