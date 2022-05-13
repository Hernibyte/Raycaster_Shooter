#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "freeglut.h"

#define WIDTH 1024
#define HEIGHT 512
#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533 // one degree in radians

float player_x, player_y, player_dx, player_dy, player_a; // player position

void drawPlayer() {
	glColor3f(1, 1, 0);
	glPointSize(8);
	glBegin(GL_POINTS);
	glVertex2i(player_x, player_y);
	glEnd();

	glLineWidth(3);
	glBegin(GL_LINES);
	glVertex2i(player_x, player_y);
	glVertex2i(player_x + player_dx * 5, player_y + player_dy * 5);
	glEnd();
}

void movePlayer(unsigned char key, int x, int y) {
	if (key == 'a') { player_a -= 0.1; if (player_a < 0) { player_a += 2 * PI; } player_dx = cos(player_a) * 5; player_dy = sin(player_a) * 5; }
	if (key == 'd') { player_a += 0.1; if (player_a > 2 * PI) { player_a -= 2 * PI; } player_dx = cos(player_a) * 5; player_dy = sin(player_a) * 5; }
	if (key == 'w') { player_x += player_dx; player_y += player_dy; }
	if (key == 's') { player_x -= player_dx; player_y -= player_dy; }
	glutPostRedisplay();
}

int mapX = 8, mapY = 8, mapS = 64;
int map[] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 1, 0, 0, 0, 0, 1,
	1, 0, 1, 0, 0, 1, 0, 1,
	1, 0, 1, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1
};

void drawMap2D() {
	int x, y, xo, yo;
	for (y = 0; y < mapY; y++) {
		for (x = 0; x < mapX; x++) {
			if (map[y * mapX + x] == 1) { glColor3f(1, 1, 1); }
			else { glColor3f(0, 0, 0); }
			xo = x * mapS; yo = y * mapS;
			glBegin(GL_QUADS);
			glVertex2i(xo + 1, yo + 1);
			glVertex2i(xo + 1, yo + mapS - 1);
			glVertex2i(xo + mapS - 1, yo + mapS - 1);
			glVertex2i(xo + mapS - 1, yo + 1);
			glEnd();
		}
	}
}

float dist(float ax, float ay, float bx, float by, float ang) {
	return (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
}

void drawRays2D() {
	int r, mx, my, mp, dof; float rx, ry, ra, xo, yo, disT;
	ra = player_a - DR * 30; if (ra < 0) { ra += 2 * PI; } if (ra > 2 * PI) { ra -= 2 * PI; }
	for (r = 0; r < 60; r++) {
		// Check Horizontal Lines
		dof = 0;
		float disH = 1000000, hx = player_x, hy = player_y;
		float aTan = -1 / tan(ra);
		if (ra > PI) { ry = (((int)player_y >> 6) << 6) - 0.0001; rx = (player_y - ry) * aTan + player_x; yo = -64; xo = -yo * aTan; } // looking up
		if (ra < PI) { ry = (((int)player_y >> 6) << 6) + 64; rx = (player_y - ry) * aTan + player_x; yo = 64; xo = -yo * aTan; } // looking down
		if (ra == 0 || ra == PI) { rx = player_x; ry = player_y; dof = 8; } // looking straight left or right
		while (dof < 8) {
			mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mapX + mx;
			if (mp > 0 && mp < mapX * mapY && map[mp] == 1) { hx = rx; hy = ry; disH = dist(player_x, player_y, hx, hy, ra); dof = 8; } //hit wall
			else { rx += xo; ry += yo; dof += 1; } // next line
		}

		// Check Vertical Lines
		dof = 0;
		float disV = 1000000, vx = player_x, vy = player_y;
		float nTan = -tan(ra);
		if (ra > P2 && ra < P3) { rx = (((int)player_x >> 6) << 6) - 0.0001; ry = (player_x - rx) * nTan + player_y; xo = -64; yo = -xo * nTan; } // looking left
		if (ra < P2 || ra > P3) { rx = (((int)player_x >> 6) << 6) + 64; ry = (player_x - rx) * nTan + player_y; xo = 64; yo = -xo * nTan; }; // looking right
		if (ra == 0 || ra == PI) { rx = player_x; ry = player_y; dof = 8; } // looking straight up or down
		while (dof < 8) {
			mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mapX + mx;
			if (mp > 0 && mp < mapX * mapY && map[mp] == 1) { vx = rx; vy = ry; disV = dist(player_x, player_y, vx, vy, ra); dof = 8; } //hit wall
			else { rx += xo; ry += yo; dof += 1; } // next line
		}

		if (disV < disH) { rx = vx; ry = vy; disT = disV; glColor3f(0.9, 0, 0); }  // vertical wall hit
		if (disH < disV) { rx = hx; ry = hy; disT = disH; glColor3f(0.7, 0, 0); }  // horizontal wall hit

		glLineWidth(1); glBegin(GL_LINES); glVertex2i(player_x, player_y); glVertex2i(rx, ry); glEnd();

		// Draw 3D Walls
		float ca = player_a - ra; if (ca < 0) { ca += 2 * PI; } if (ca > 2 * PI) { ca -= 2 * PI; } disT = disT * cos(ca);; //fix fisheye
		float lineH = (mapS * 320) / disT; if (lineH > 320) { lineH = 320; }  // line height
		float lineO = 160 - lineH / 2; // line offset
		glLineWidth(8); glBegin(GL_LINES); glVertex2i(r * 8 + 530, lineO); glVertex2i(r * 8 + 530, lineH + lineO); glEnd();
		
		ra += DR; if (ra < 0) { ra += 2 * PI; } if (ra > 2 * PI) { ra -= 2 * PI; }
	}
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawMap2D();
	drawRays2D();
	drawPlayer();

	glutSwapBuffers();
}

void init() {
	glClearColor(0.3, 0.3, 0.3, 0);
	gluOrtho2D(0, 1024, 512, 0);

	player_x = 300, player_y = 300; player_dx = cos(player_a) * 5; player_dy = sin(player_a) * 5;
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Raycaster Shooter");
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(movePlayer);
	glutMainLoop();
}