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

typedef struct {
	int w, a, d, s;
} ButtonKeys; ButtonKeys userKeys;

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

void movePlayer(float fps) {
	if (userKeys.a == 1) { player_a -= 0.003 * fps; if (player_a < 0) { player_a += 2 * PI; } player_dx = cos(player_a) * 5; player_dy = sin(player_a) * 5; }
	if (userKeys.d == 1) { player_a += 0.003 * fps; if (player_a > 2 * PI) { player_a -= 2 * PI; } player_dx = cos(player_a) * 5; player_dy = sin(player_a) * 5; }

	int xo = 0; if (player_dx < 0) { xo = -20; }
	else { xo = 20; }
	int yo = 0; if (player_dy < 0) { yo = -20; }
	else { yo = 20; }
	int ipx = player_x / 64.0, ipx_add_xo = (player_x + xo) / 64.0, ipx_sub_xo = (player_x - xo) / 64.0;
	int ipy = player_y / 64.0, ipy_add_yo = (player_y + yo) / 64.0, ipy_sub_yo = (player_y - yo) / 64.0;

	if (userKeys.w == 1) {
		if (map[ipy * mapX + ipx_add_xo] == 0) { player_x += player_dx * 0.05 * fps; }
		if (map[ipy_add_yo * mapX + ipx] == 0) { player_y += player_dy * 0.05 * fps; }
	}
	if (userKeys.s == 1) {
		if (map[ipy * mapX + ipx_sub_xo] == 0) { player_x -= player_dx * 0.05 * fps; }
		if (map[ipy_sub_yo * mapX + ipx] == 0) { player_y -= player_dy * 0.05 * fps; }
	}
	glutPostRedisplay();
}

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

float frame1, frame2, gameFPS;

void display() {
	// frames per second
	frame2 = glutGet(GLUT_ELAPSED_TIME); gameFPS = (frame2 - frame1); frame1 = glutGet(GLUT_ELAPSED_TIME);

	movePlayer(gameFPS);

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

void buttonDown(unsigned char key, int x, int y) {
	if (key == 'a') { userKeys.a = 1; }
	if (key == 'd') { userKeys.d = 1; }
	if (key == 'w') { userKeys.w = 1; }
	if (key == 's') { userKeys.s = 1; }
	glutPostRedisplay();
}

void buttonUp(unsigned char key, int x, int y) {
	if (key == 'a') { userKeys.a = 0; }
	if (key == 'd') { userKeys.d = 0; }
	if (key == 'w') { userKeys.w = 0; }
	if (key == 's') { userKeys.s = 0; }
	glutPostRedisplay();
}

void resize(int w, int h) {
	glutReshapeWindow(1024, 512);
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("Raycaster Shooter");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(buttonDown);
	glutKeyboardUpFunc(buttonUp);
	glutMainLoop();
}