#include <GL/glut.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/vec3.hpp>
#include <random>
#include <iostream>
using namespace std;

double ballx, bally, ballz;  //The position of the ball - you can set this in your code
double boxxl, boxxh, boxyl, boxyh, boxzl, boxzh;  // The low and high x, y, z values for the box sides

//simulation variables
int n;
float t, h, E, M, particleBlue, d, m, deltaV,dn1; //d = strength of air resistance   m = mass of ball
glm::vec3 v0, p0, vnew, pnew, a, g, vn, vt, pnewTest, an, at;

glm::vec3 A, B, C, D; //four corners/points of the plane
glm::vec3 AD, DC, CB, BA, AX, BX, CX, DX; //the four vectors from one corner to another, plus holders (the x ones) for the vectors that are from the corners to the point in question
bool insidePlane;

//define plane with a point P and a normal N
glm::vec3 p, p2, normal; //p2 is a plane right under the original plane P, for the extra cushion for particles to stop falling through

//define all the particle arrays to hold the state
const int maxParticles = 10000;
bool particles[maxParticles]; //says whether the particle is active or inactive (true or false)
bool hasCollided[maxParticles]; //says whether the particle has had a collision before or not
int inactiveStack[maxParticles]; //a list of indices of all the inactive (false) particles in the array particles.
int inactiveCount; //number of how many inactive particles there are; also the index to the top of InactiveStack
glm::vec3 particlePos[maxParticles]; //xyz position of each particle
glm::vec3 particleVel[maxParticles]; //xyz velocity of each particle
glm::vec3 pColor[maxParticles]; //color of particles
int pLifetime[maxParticles]; //lifetime of each particle
int counter;
GLfloat color[] = { 0.0, 0.0, 0 };

//create random number generators
std::default_random_engine generator;
std::normal_distribution<double> distributionPos(0, 2);
std::normal_distribution<double> distributionVel(0, 1);
std::normal_distribution<double> distributionLife(250, 50);
std::normal_distribution<double> distributionColor(0, .2);

//variables that allow generator to move
int xshift, yshift, zshift;

int rotateon;

double xmin, xmax, ymin, ymax, zmin, zmax;
double maxdiff;

int lastx, lasty;
int xchange, ychange;
float spin = 90.0;
float spinup = -15.0;

void checkRest(int particleIndex) //is the particle at rest or is it still moving?
{
	if (length(v0) < deltaV) //is velocity small?
	{
		if (dn1 < .005) //is object near a surface?
		{
			particles[particleIndex] = false;
			/*if (dot(a, normal) < deltaV)
			{
				an = dot(a, normal)*normal;
				at = a - an;
				if (length(at) < M*length(an))
				{
					particles[particleIndex] = false;
				}
			}*/
		}
	}
}

void checkCollision(int particleIndex)
{
	//float dn = dot((p0 - p), normal);
	dn1 = dot((pnew - p), normal);
	if (dot((p0 - p), normal) * dn1 < 0) //can make second term with pnew (dn1) a global variable. so it's only declared/allocated once but you overwrite it here each time so you don't have to do the dot product calculation twice
	{
		hasCollided[particleIndex] = true;
		particleBlue = 1;
		//collision occurred so adjust speed and pos. accordingly
		pnew = pnew + (1 + E)*dn1; //is this supposed to be a - or a + ??  //DOING DOT PROD TWICE (calculating dn1 again)

		vn = dot(vnew, normal) * normal;
		vt = vnew - vn;
		vnew = -vn + (1-M)*vt; //is M supposed to be in here? (1-M)vt ??
		//vnew.y *= 1.5;
	}

	else if (dot((p0 - p2), normal) * dot((pnew - p2), normal) <= 0) //can make second term with pnew (dn1) a global variable. so it's only declared/allocated once but you overwrite it here each time so you don't have to do the dot product calculation twice
	{
		particleBlue = 1;
		//collision occurred so adjust speed and pos. accordingly
		pnew = pnew + (1 + E)*dot((pnew - p2), normal); //is this supposed to be a - or a + ??  //DOING DOT PROD TWICE (calculating dn1 again)

		vn = dot(vnew, normal) * normal;
		vt = vnew - vn;
		vnew = -vn + (1-M)*vt; //is M supposed to be in here? (1-M)vt ??
		//vnew.y *= 1.5;
	}
	
}

void checkInsidePlane()
{
	AX = cross(AD, (pnew - A));
	DX = cross(DC, (pnew - D));
	CX = cross(CB, (pnew - C));
	BX = cross(BA, (pnew - B));

	if(AX.x*BX.x*CX.x*DX.x < 0 || AX.y*BX.y*CX.y*DX.y < 0 || AX.z*BX.z*CX.z*DX.z < 0)
	{
		insidePlane = false;
	}
	else
	{
		insidePlane = true;
	}
}

void spawnParticles()
{
	//cout << inactiveCount << endl;
	//activate/ spawn five new particles every frame
	if (n % 1 == 0)
	{
		for (int k = 0; k < 100; k++)
		{
			if (inactiveCount > 0)
			{
				//cout << inactiveCount << endl;
				//make particle active
				int currentParticle = inactiveStack[inactiveCount];
				particles[currentParticle] = true;
				inactiveCount--;
			}
		}
	}
	
	for (int i = 0; i < maxParticles; i++) //number of particles generated per frame ??
	{
		//cout << "particle activity: i = " << i << "    " << particles[i] << endl;
		if (particles[i] == true)
		{
			
			//calculate acceleration
				//a = g; //a = g - (d*v0) / m; //acceleration only changes if there is air resistance so until i implement that this is more efficient. one less calculation
			pLifetime[i]--;
			if (pLifetime[i] <= 0)
			{
				particles[i] = false;
				inactiveCount++;
				inactiveStack[inactiveCount] = i;
				//probably reset lifetime here? also need new velocity and position
				hasCollided[i] = false;
				//pLifetime[i] = 300;
				//particlePos[i] = {0,100,0};
				//particleVel[i] = {5,2,0};

				particlePos[i] = { distributionPos(generator) +xshift, distributionPos(generator) + 100 +yshift , distributionPos(generator)+zshift };
				particleVel[i] = { distributionVel(generator) * 1, distributionVel(generator) * 5,distributionVel(generator) };//{ distribution(generator),distribution(generator),distribution(generator) };
				pLifetime[i] = distributionLife(generator);
				pColor[i] = { 241 / 256 ,171 / 256 + distributionColor(generator),.7 + distributionColor(generator) };// { distributionColor(generator), distributionColor(generator), distributionColor(generator) };

			}
			//set initial pos and vel
			v0 = particleVel[i];
			p0 = particlePos[i];
			//euler integration to find Vnew and Xnew
			vnew = v0 + a * h;
			pnew = p0 + (vnew + v0)*(h / 2);
			pnewTest = p0 + (vnew + a * h + v0)*(h / 2);
			//checkCollision
			//THIS NEEDS TIMER TO RESET hasCollided, so it can collide more than once(bc it'll bounce up then back down later bc of gravity) 
			//or do a rest condition! 
			//if (p0.x > -100 && p0.x < 100 && p0.z > -100 && p0.z < 100) //see if particle is within the plane's boundaries (not infinite plane!)
			checkInsidePlane();
			if(insidePlane)
			{
				if (hasCollided[i] == false)
				{
					//cout << "preCollision pos: " << pnew.x << "  " << pnew.y << "   " << pnew.z << endl;
					checkCollision(i);
					//cout << "post Collision pos: " << pnew.x << "  " << pnew.y << "   " << pnew.z << endl;
				}
				/*else
				{
					if (n % 2 == 0)
					{
						hasCollided[i] = false;
					}
				}*/
			}
		
			checkRest(i);

			//v0 = vnew;
			//p0 = pnew;
			particleVel[i] = vnew;
			particlePos[i] = pnew;
			//update display if necessary

			//draw the particle
			//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ball_ambient);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ball_diffuse);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ball_specular);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, ball_shininess);
			glPushMatrix();
			pColor[i].x += .002;
			color[0] = pColor[i].x;
			color[1] = pColor[i].y;
			color[2] = pColor[i].z;
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
			glTranslatef(pnew.x, pnew.y, pnew.z);
			glutSolidSphere(3, 5, 5);
			glPopMatrix();
		}
	}
	n++;
}

void display(void)
{
	GLfloat box_ambient[] = { 0.1, 0.1, 0.1 };
	GLfloat smallr00[] = { 0.1, 0.0, 0.0 };
	GLfloat small0g0[] = { 0.0, 0.1, 0.0 };
	GLfloat small00b[] = { 0.0, 0.0, 0.1 };
	GLfloat smallrg0[] = { 0.1, 0.1, 0.0 };
	GLfloat smallr0b[] = { 0.1, 0.0, 0.1 };
	GLfloat small0gb[] = { 0.0, 0.1, 0.1 };
	GLfloat smallrgb[] = { 0.1, 0.1, 0.1 };

	GLfloat box_diffuse[] = { 0.7, 0.7, 0.7 };
	GLfloat box_specular[] = { 0.1, 0.1, 0.1 };
	GLfloat box_shininess[] = { 1.0 };
	GLfloat ball_ambient[] = { 0.4, 0.0, 0.0 };
	GLfloat ball_diffuse[] = { 0.3, 0.0, particleBlue };
	GLfloat ball_specular[] = { 0.3, 0.3, 0.3 };
	GLfloat ball_shininess[] = { 10.0 };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	//rotate the view
	glRotatef(spinup, 1.0, 0.0, 0.0);
	glRotatef(spin, 0.0, 1.0, 0.0);
	
	/*
	8 vertices:
	glVertex3f(boxxl, boxyl, boxzl);
	glVertex3f(boxxh, boxyl, boxzl);
	glVertex3f(boxxh, boxyh, boxzl);
	glVertex3f(boxxl, boxyh, boxzl);
	glVertex3f(boxxl, boxyl, boxzh);
	glVertex3f(boxxh, boxyl, boxzh);
	glVertex3f(boxxh, boxyh, boxzh);
	glVertex3f(boxxl, boxyh, boxzh);
	*/

	//Draw the box
	//set material parameters
	glMaterialfv(GL_FRONT, GL_AMBIENT, box_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, box_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, box_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, box_shininess);

	glBegin(GL_QUADS);
	//back face
	/*glMaterialfv(GL_FRONT, GL_AMBIENT, smallrgb);
	glVertex3f(boxxl, boxyl, boxzl);
	glVertex3f(boxxh, boxyl, boxzl);
	glVertex3f(boxxh, boxyh, boxzl);
	glVertex3f(boxxl, boxyh, boxzl);

	//left face
	glMaterialfv(GL_FRONT, GL_AMBIENT, small0g0);
	glVertex3f(boxxl, boxyl, boxzh);
	glVertex3f(boxxl, boxyl, boxzl);
	glVertex3f(boxxl, boxyh, boxzl);
	glVertex3f(boxxl, boxyh, boxzh);

	//right face
	glMaterialfv(GL_FRONT, GL_AMBIENT, small00b);
	glVertex3f(boxxh, boxyl, boxzh);
	glVertex3f(boxxh, boxyh, boxzh);
	glVertex3f(boxxh, boxyh, boxzl);
	glVertex3f(boxxh, boxyl, boxzl);

	

	//top face
	glMaterialfv(GL_FRONT, GL_AMBIENT, smallr0b);
	glVertex3f(boxxh, boxyh, boxzh);
	glVertex3f(boxxl, boxyh, boxzh);
	glVertex3f(boxxl, boxyh, boxzl);
	glVertex3f(boxxh, boxyh, boxzl);

	//front face
	glMaterialfv(GL_FRONT, GL_AMBIENT, small0gb);
	glVertex3f(boxxh, boxyl, boxzh);
	glVertex3f(boxxl, boxyl, boxzh);
	glVertex3f(boxxl, boxyh, boxzh);
	glVertex3f(boxxh, boxyh, boxzh);*/

	//bottom face
	glMaterialfv(GL_FRONT, GL_AMBIENT, smallrg0);
	glVertex3f(A.x, A.y, A.z);// (boxxh, boxyh, boxzh);
	glVertex3f(B.x, B.y, B.z);//(boxxh, boxyl, boxzl);
	glVertex3f(C.x, C.y, C.z);//(boxxl, boxyl, boxzl);
	glVertex3f(D.x, D.y, D.z);//(boxxl, boxyh, boxzh);

	glEnd();

	

	//draw the ball
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ball_ambient);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ball_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ball_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, ball_shininess);
	//glPushMatrix();
	//glTranslatef(ballx, bally, ballz);
	//draw particles
	spawnParticles();
	//glutSolidSphere(5, 10, 10);
	//glPopMatrix();

	glPopMatrix();
	glutSwapBuffers();
}

void initializeValues() {
	
	//inactiveCount = maxParticles-1;
	g = { 0,-5,0 }; a = g; p = { 0,-100,0 }; p2 = { 0,-101,0 }; vn = { 0,0,0 }; vt = { 0,0,0 }; pnewTest = { 0,0,0 }; an = { 0,0,0 }; at = { 0,0,0 };
	h = .05; E = 3.8; n = 0; M = .5; dn1 = 0; deltaV = .005;
	counter = 0; inactiveCount = 0;
	insidePlane = false;
	
	//A B C D are the four points of the plane
	A = { 30,-100,100 };
	B = { 100,100,-100 };
	C = { -100,100,-100 };
	D = { -30,-100,100 };

	//these are the vectors from one corner of the plane to another (used for checking if a point is within the plane)
	AD = D - A;
	DC = C - D;
	CB = B - C;
	BA = A - B;

	//initialize all of these to 0, will be used in checkInsidePlane()
	AX = { 0,0,0 }; BX = { 0,0,0 }; CX = { 0,0,0 }; DX = { 0,0,0 };
	//calculate normal of the plane
	p = { 0,0,0 }; p2={ 0,-1,0 }; //{ boxxh, boxyh, boxzh }; p2 = { boxxh, boxyh-1, boxzh };
	normal = { 0,1,0 };

	normal = cross((A - B), (C - B));
	normal = normalize(normal);

	//create random number generator
	/* std::default_random_engine generator;
	std::normal_distribution<double> distributionPos(0, 2);
	std::normal_distribution<double> distributionVel(0, 1);
	std::normal_distribution<double> distributionLife(300, 5);
	std::normal_distribution<double> distributionColor(0, 1); */

	xshift = 0; yshift = 0; zshift = 0;

	//initialize all the particle values
	for (int i = 0; i < maxParticles; i++)
	{
		particles[i] = false;
		hasCollided[i] = false;
		//inactiveCount++;
		inactiveStack[i] = inactiveCount;
		//cout << i << "    " << inactiveStack[i] <<endl;
		inactiveCount++;
		particlePos[i] = { distributionPos(generator)+xshift, distributionPos(generator)+100+yshift , distributionPos(generator)+zshift };
		particleVel[i] = { distributionVel(generator)*1, distributionVel(generator)*5,distributionVel(generator) };//{ distribution(generator),distribution(generator),distribution(generator) };
		pLifetime[i] = distributionLife(generator);
		pColor[i] = { 241 / 256 ,171 / 256 + distributionColor(generator),.7 + distributionColor(generator) };// { distributionColor(generator), distributionColor(generator), distributionColor(generator) };
	}

	inactiveCount--; //only decrement once, because in the for loop it increments inactive count one extra time, which we don't want.
}

void init(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	// Enable Z-buffering, backface culling, and lighting
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 1, 600);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set eye point and lookat point
	gluLookAt(0, 225, 300, 0, 0, 0, 0, 1, 0);

	// Set up lights
	GLfloat light0color[] = { 1.0, 1.0, 1.0 };
	GLfloat light0pos[] = { 0, 500, 300 };
	GLfloat light1color[] = { 1.0, 1.0, 1.0 };
	GLfloat light1pos[] = { 300, 300, 300 };
	glLightfv(GL_LIGHT0, GL_POSITION, light0pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0color);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0color);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0color);
	glLightfv(GL_LIGHT1, GL_POSITION, light1pos);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1color);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1color);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1color);

	//Initialize ball position
	ballx = 0.0; bally = 0.0; ballz = 0.0;

	//Initialize box boundaries
	boxxl = -90;
	boxxh = 90;
	boxyl = 100;
	boxyh = -100;
	boxzl = -100;
	boxzh = 100;
}

void reshapeFunc(GLint newWidth, GLint newHeight)
{
	if (newWidth > newHeight) // Keep a square viewport
		glViewport((newWidth - newHeight) / 2, 0, newHeight, newHeight);
	else
		glViewport(0, (newHeight - newWidth) / 2, newWidth, newWidth);
	init();
	glutPostRedisplay();
}

void rotateview(void)
{
	if (rotateon) {
		spin += xchange / 250.0;
		if (spin >= 360.0) spin -= 360.0;
		if (spin < 0.0) spin += 360.0;
		spinup -= ychange / 250.0;
		if (spinup > 89.0) spinup = 89.0;
		if (spinup < -89.0) spinup = -89.0;
	}
	glutPostRedisplay();
}

void keyPress(unsigned char key, int x, int y)
{
	switch (key) {
	case 'a':
		xshift--;
		cout << xshift << endl;
		break;

	case 'd':
		xshift++;
		cout << xshift << endl;
		break;
	case 'w':
		yshift++;
		cout << yshift << endl;
		break;
	case 's':
		yshift--;
		cout << yshift << endl;
		break;
	case 'q':
		zshift--;
		cout << zshift << endl;
		break;
	case 'e':
		zshift++;
		cout << zshift << endl;
		break;
	default:
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
			lastx = x;
			lasty = y;
			xchange = 0;
			ychange = 0;
			rotateon = 1;
		}
		else if (state == GLUT_UP) {
			xchange = 0;
			ychange = 0;
			rotateon = 0;
		}
		break;

	default:
		break;
	}
}

void motion(int x, int y)
{
	xchange = x - lastx;
	ychange = y - lasty;
}

int main(int argc, char** argv)
{
	GLint SubMenu1, SubMenu2, SubMenu3, SubMenu4;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(700, 700);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Ball in Cube Demo");
	init();
	initializeValues();
	rotateon = 0;
	glutDisplayFunc(display);
	glutKeyboardFunc(keyPress);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutIdleFunc(rotateview);
	glutReshapeFunc(reshapeFunc);

	glutMainLoop();
	return 0;
}
