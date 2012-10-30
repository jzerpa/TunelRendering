#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <GL/glut.h>

#include "SOIL.h"
#include "Splines.h"

//cam variables
float xpos = 0, ypos = 0, zpos = 0, xrot = 0, yrot = 0, angle=0.0, zoom=0.0;
float lastx, lasty;

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 640
#define RADIUS 5;
#define NUMPOINTS 200
#define NUMSPLINES1 2
#define CIRCLEPOINTS 200
#define LENGTH NUMPOINTS*NUMSPLINES1
#define PI 3.1416

/* - Menu State Identifier - */
int g_iMenuId;

/* - Mouse State Variables - */
int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

/* - SplineList Variable for the Aplication - */
SplineList g_Splines;
Vector circle[360];   //store the precalculated x, y positions of the circle
Vector points[LENGTH];  //store the precalculated points of the list of splines
Vector tnarray[LENGTH]; //store the precalculated vector T of the list of splines
Vector bnarray[LENGTH]; //store the precalculated vector B of the list of splines
Vector Nnarray[LENGTH]; //store the precalculated vector N of the list of splines
Vector globulos[200]; //store the position of the red cells

unsigned char *ht_map;
int width, height, channels;
int camPos = 0;
float camStep = 0;;

/* Posible visualization modes */
bool travelMode =  false;
bool polygon = true;
bool wireframe = false;
bool pointsMode = false;

//Helper function
double toRadians(double angdeg) 
{
	return angdeg / 180.0 * PI;
}
//Helper function
double toDegree(double angrad) 
{
	return (angrad * 180.0) / PI;
}

/*
	myinit - Initialize circle points
	-Initialize red cells position
	-initialize cam position variables
*/
void myinit()
{
	float factor = (360.0/(CIRCLEPOINTS));
	for(int i = 0; i<CIRCLEPOINTS; i++)
	{
		float angle = (float)i;
		angle = angle* factor;
		circle[i].x = (float)sin (toRadians ((double)angle));
		circle[i].y = (float)cos (toRadians ((double)angle));
	}

	//initialize globulos
	float x, y, r;
	srand(time(NULL));

	//200 number of red cells
	for(int i = 0; i<200; i++)
	{
		//Random displacement d = [0.2 ...  RADIUS]
		int r = RADIUS;
		float d = 0.5+rand() % r;

		//random angle of the circle (angle = index) 
		int circleIndex = rand() % CIRCLEPOINTS;

		x = circle[circleIndex].x * (d);
		y = circle[circleIndex].y * (d);
		
		//Random index reference (position)
		int index = rand() % (LENGTH);

		Vector despUp;
		despUp.x = bnarray[index].x*y;
		despUp.y = bnarray[index].y*y;
		despUp.z = bnarray[index].z*y;

		Vector despRight;
		despRight.x = Nnarray[index].x*x;
		despRight.y = Nnarray[index].y*x;
		despRight.z = Nnarray[index].z*x;

		//Final position of the red cells
		globulos[i].x = points[index].x + despRight.x + despUp.x;
		globulos[i].y = points[index].y + despRight.y + despUp.y;
		globulos[i].z = points[index].z + despRight.z + despUp.z;
	}

	//Cam vars initialization
	zoom = 1;
	xrot = 0;
	yrot = 0;
	xpos = 0;
	ypos = 0;
	zpos = 0;


	/* setup gl view here */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);  //Z buffer
}

/*
     Calculate points, T, B, N of the spline list
	 store all this data in global arrays
*/
void calcPoints()
{
	Vector P[4];
	Vector spPoint;
	Vector Tn, Bn, Nn;
	float t2 = 0;
	float t3 = 0;
	int index = 0;
	float t=0;

	//Loop all the splines
	for (int spIndex = 0; spIndex < NUMSPLINES1; spIndex++)
	{
		index = spIndex*NUMPOINTS;
		t = 0;
		t2 = t*t;
		t3 = t*t*t;

		//F'' = (6-6*t)*P1 + (-12+18*t)*P2 + (6-18*t)*P3 + 6*t*P4
		//We use the second derivate for the initial T, B, N
		//Then we proceed with the iterative procedure
		//Calculate initial t,b,n when t=0
		g_Splines.GetCurrent(P);

		Vector fn1d;
		Vector fn2d;
		
		//Calculate spline point at moment t=0
		spPoint.x = ((1-3*t + 3*t2 - t3) * P[0].x) + ((3*t - 6*t2 + 3*t3)*P[1].x) + ((3*t2 - 3*t3)*P[2].x) + (t3*P[3].x);
		spPoint.y = ((1-3*t + 3*t2 - t3) * P[0].y) + ((3*t - 6*t2 + 3*t3)*P[1].y) + ((3*t2 - 3*t3)*P[2].y) + (t3*P[3].y);
		spPoint.z = ((1-3*t + 3*t2 - t3) * P[0].z) + ((3*t - 6*t2 + 3*t3)*P[1].z) + ((3*t2 - 3*t3)*P[2].z) + (t3*P[3].z);
	
		fn1d.x = ((-3 + 6*t - 3*t2)*P[0].x) + ((3 - 12*t + 9*t2)*P[1].x) + ((6*t - 9*t2)*P[2].x) + 3*t2*P[3].x;
		fn1d.y = ((-3 + 6*t - 3*t2)*P[0].y) + ((3 - 12*t + 9*t2)*P[1].y) + ((6*t - 9*t2)*P[2].y) + 3*t2*P[3].y;
		fn1d.z = ((-3 + 6*t - 3*t2)*P[0].z) + ((3 - 12*t + 9*t2)*P[1].z) + ((6*t - 9*t2)*P[2].z) + 3*t2*P[3].z;

		fn2d.x = 6*P[0].x - 12*P[1].x + 6*P[3].x;
		fn2d.y = 6*P[0].y - 12*P[1].y + 6*P[3].y;
		fn2d.z = 6*P[0].z - 12*P[1].z + 6*P[3].z;

		//Calculate Tn
		Tn.x = fn1d.x;	Tn.y = fn1d.y;	Tn.z = fn1d.z;
		Tn.Normalize();

		//Calculate Bn or UP in our case
		Bn = fn1d.Cross(fn2d);
		Bn.Normalize();

		//Çalculate N or RIGHT in our case
		Nn = Bn.Cross(Tn);
		Nn.Normalize();

		//Save the points in the array structure
		points[index] = spPoint;
		tnarray[index] = Tn;
		bnarray[index] = Bn;
		Nnarray[index] = Nn;

		//Each element calculated represents a ring in the artery
		for(int r = 1; r < NUMPOINTS; r++)
		{
			index = r + (spIndex*NUMPOINTS);
			
			//t = [0...1]
			t = (float) r / (float)(NUMPOINTS - 1);
			t2 = t*t;
			t3 = t*t*t;

			//Calculate spline point at moment t
			spPoint.x = ((1-3*t + 3*t2 - t3) * P[0].x) + ((3*t - 6*t2 + 3*t3)*P[1].x) + ((3*t2 - 3*t3)*P[2].x) + (t3*P[3].x);
			spPoint.y = ((1-3*t + 3*t2 - t3) * P[0].y) + ((3*t - 6*t2 + 3*t3)*P[1].y) + ((3*t2 - 3*t3)*P[2].y) + (t3*P[3].y);
			spPoint.z = ((1-3*t + 3*t2 - t3) * P[0].z) + ((3*t - 6*t2 + 3*t3)*P[1].z) + ((3*t2 - 3*t3)*P[2].z) + (t3*P[3].z);

			//Calculate T
			Tn.x = ((-3 + 6*t - 3*t2)*P[0].x) + ((3 - 12*t + 9*t2)*P[1].x) + ((6*t - 9*t2)*P[2].x) + 3*t2*P[3].x;
			Tn.y = ((-3 + 6*t - 3*t2)*P[0].y) + ((3 - 12*t + 9*t2)*P[1].y) + ((6*t - 9*t2)*P[2].y) + 3*t2*P[3].y;
			Tn.z = ((-3 + 6*t - 3*t2)*P[0].z) + ((3 - 12*t + 9*t2)*P[1].z) + ((6*t - 9*t2)*P[2].z) + 3*t2*P[3].z;
			Tn.Normalize();

			//Çalculate N or RIGHT in our case
			Nn = Bn.Cross(Tn);
			Nn.Normalize();

			//Calculate Bn or UP in our case
			Bn = Tn.Cross(Nn);
			Bn.Normalize();

			//Save the points in the array structure
			points[index] = spPoint;
			tnarray[index] = Tn;
			bnarray[index] = Bn;
			Nnarray[index] = Nn;
		}
		//Move to the next spline in the spline list (This function moves 4 x 4 points)
		g_Splines.MoveToNextModif();
	}
}

/*
    Dibuja un trozo de la arteria desde un punto de inicio hasta el fin del array de puntos.
	La forma de dibujarlos en con GL_TRIANGLE_STRIP
*/

void drawTrozo(int inicio, int fin)
{
	float x, y;
	int index = 0;
	float altura1 = 0;
	float altura2 = 0;
	Vector pfinal;
	Vector pfinal2;

	Vector pfinal0;
	Vector pfinal20;
	for(int i = inicio; i < fin; i++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		index = i % (LENGTH);  //Module so it doesn't exceed the array size

		//Iterate in each circle point creating the vertices and connecting them 
		//with the next ring
		for(int alpha = 0; alpha<CIRCLEPOINTS; alpha++)
		{
			//altura1 will store the data for the heightmap
			//ht_map contains all data in a single row mode
			altura1 = ht_map[(index%NUMPOINTS)*width + alpha];
			altura1 =(3*altura1 / 255.0) + 0.4;
			float r = RADIUS;
			x = circle[alpha].x * (r+altura1);
			y = circle[alpha].y * (r+altura1);
			
			Vector despUp;
			despUp.x = bnarray[index].x*y;
			despUp.y = bnarray[index].y*y;
			despUp.z = bnarray[index].z*y;

			Vector despRight;
			despRight.x = Nnarray[index].x*x;
			despRight.y = Nnarray[index].y*x;
			despRight.z = Nnarray[index].z*x;

			
			pfinal.x = points[index].x + despRight.x + despUp.x;
			pfinal.y = points[index].y + despRight.y + despUp.y;
			pfinal.z = points[index].z + despRight.z + despUp.z;

			//  ---- Ring in front  -----
			//altura1 will store the data for the heightmap
			//ht_map contains all data in a single row mode
			int index2 = (index + 1) % (LENGTH);
			altura2 = ht_map[(index2%NUMPOINTS)*width + alpha];
			altura2 = (3*altura2 / 255.0) + 0.4;

			x = circle[alpha].x * (r+altura2);
			y = circle[alpha].y * (r+altura2);
			
			//Project the y position in the direction of UP
			despUp.x = bnarray[index2].x*y;
			despUp.y = bnarray[index2].y*y;
			despUp.z = bnarray[index2].z*y;

			//Project the x position in the direction of RIGHT
			despRight.x = Nnarray[index2].x*x;
			despRight.y = Nnarray[index2].y*x;
			despRight.z = Nnarray[index2].z*x;

			//Calculate final position
			pfinal2.x = points[index2].x + despRight.x + despUp.x;
			pfinal2.y = points[index2].y + despRight.y + despUp.y;
			pfinal2.z = points[index2].z + despRight.z + despUp.z;

			//Color the first vertex using the heightmap info to adjust the red intensity
			glColor3f(abs(altura1/10), 0.0, 0.0);
			glVertex3d(pfinal.x, pfinal.y, pfinal.z);

			//Same as first vertex -- Ring in front
			glColor3f(abs(altura2/10), 0.0, 0.0);
			glVertex3d(pfinal2.x, pfinal2.y, pfinal2.z);
			
			//save the first points to connect the circle at the end
			if(alpha==0) { pfinal0 = pfinal; pfinal20 = pfinal2; }
		}
		
		//connect the circle
		glColor3f(abs(altura1/10), 0.0, 0.0);
		glVertex3d(pfinal0.x, pfinal0.y, pfinal0.z);

		glColor3f(abs(altura2/10), 0.0, 0.0);
		glVertex3d(pfinal20.x, pfinal20.y, pfinal20.z);
		glEnd();
	}

	//Create the red cells
	for (int i=0; i<200; i++){
		glPushMatrix();
		//Adjust their position according to the precalculated pos
		glTranslatef(globulos[i].x, globulos[i].y, globulos[i].z); 
		glColor3f(0.50, 0.0, 0.0);
		glutSolidTorus(0.1,0.1,10,10);
		glPopMatrix();
	}
}

/*
	display - Function to modify with your heightfield rendering code (Currently displays a simple cube)
*/

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	if(wireframe) glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);
	if (polygon) glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);
	if (pointsMode) glPolygonMode(GL_FRONT_AND_BACK , GL_POINT);
	
	if(travelMode){
		//Aim the camera to the next point in the array
		//bnarray[campos] ->  up vector
		gluLookAt(points[camPos].x,points[camPos].y,points[camPos].z, points[camPos+1].x,points[camPos+1].y,points[camPos+1].z,bnarray[camPos].x,bnarray[camPos].y,bnarray[camPos].z);
		drawTrozo(camPos, camPos+50);
		glutSwapBuffers ();
	}
	else
	{
		//Let the user choose his position and apply the corresponding transformation
		gluLookAt(points[0].x,points[0].y,points[0].z-zoom, points[0+1].x,points[0+1].y,points[0+1].z,bnarray[0].x,bnarray[0].y,bnarray[0].z);
		glRotatef(-xrot, 0.0, 1.0, 0.0);
		glRotatef(-yrot, 1.0, 0.0, 0.0);
		glTranslatef(-xpos, -ypos, -zpos);

		drawTrozo(0, LENGTH);
		glutSwapBuffers ();
	}
}

/*
	menufunc - Menu Event-handler
*/
void menufunc(int value)
{
	switch (value)
	{
	case 0: //on exit liberate resources
		SOIL_free_image_data( ht_map );
		exit(0);
		break;
	}
}

/*
	doIdle - The idle-function used to update the screen
*/
void doIdle()
{
	if(travelMode){
		/* make the screen update */
		camStep+= 0.2;
		camPos = (camStep);
		if(camPos > (LENGTH))
		{camPos = 0; camStep = 0;}
	}
	glutPostRedisplay();
}



/*
	mousebutton - Sets the global mouse states according to the actions
*/
void mousebutton(int button, int state, int x, int y)
{
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		g_iLeftMouseButton = (state==GLUT_DOWN);
		break;
	case GLUT_MIDDLE_BUTTON:
		g_iMiddleMouseButton = (state==GLUT_DOWN);
		break;
	case GLUT_RIGHT_BUTTON:
		g_iRightMouseButton = (state==GLUT_DOWN);
		break;
	}

	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

//KeyBoard function - Here we define all the keys

void keyboard (unsigned char key, int x, int y)
{
   switch (key) {
      case 'e':
         yrot += 4.5;
         glutPostRedisplay();
         break;
      case 'r':
         yrot -= 4.5;
         glutPostRedisplay();
         break;
     case 'a':
         xrot += 4.5;
         glutPostRedisplay();
         break;
      case 'd':
         xrot -= 4.5;
         glutPostRedisplay();
         break;
	  case 's':
         zoom -= 4;
         glutPostRedisplay();
		 break;
	case 'w':
         zoom += 4;
         glutPostRedisplay();
		 break;
	case 'v':
         travelMode = !travelMode;
         glutPostRedisplay();
		 break;
	case 'b':
		polygon = false;
		pointsMode = false;
		//wireframe mode
		wireframe = true;
		break;
	case 'n':
		wireframe = false;
		pointsMode = false;
		//polygon mode
		polygon = true;
		break;
	case 'm':
		wireframe = false;
		polygon = false;
		//points mode
		pointsMode = true;
		break;
      default:
         break;
   }
}


//Reshape function
void reshape(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)height / (GLfloat)width, 0.01, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


/*
	main - The Main Function
*/
int main (int argc, char ** argv)
{
	if (argc < 2)
	{
		printf ("usage: %s trackfile\n", argv[0]);
		exit(1);
	}

	if (!g_Splines.LoadSplines (argv[1]))
	{
		printf ("failed to load splines\n");
		exit(1);
	}
	
	glutInit(&argc,argv);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	glutCreateWindow("Un Viaje Alucinante - José Zerpa y Nelson Guarin");

	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);

	/* allow the user to quit using the right mouse button menu */
	g_iMenuId = glutCreateMenu(menufunc);
	glutSetMenu(g_iMenuId);
	glutAddMenuEntry("Quit",0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	/* replace with any animate code */
	glutIdleFunc(doIdle);

	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);

	glutReshapeFunc(reshape);

   glutKeyboardFunc(keyboard);

   calcPoints();

	/* Initialize circles, globulos, screen, buffers */
	myinit();

	//Here we load our height map
	ht_map = SOIL_load_image
	(
		"hflab4.jpg",
		&width, &height, &channels,
		SOIL_LOAD_L
	);

	glutMainLoop();
	return 0;
}
