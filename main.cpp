#include "main.h"

using namespace std;

EdgeBundler *bundler;

const int WIDTH = 2000;
const int HEIGHT = 1000;
const double ZOOM_CONST = 1.9;
const char *FILENAME = "test_data/edges.txt";

void drawLine(const Point *p1, const Point *p2, const int weight) {
//    glLineWidth(weight);
    glColor3f(0.5f, 0.5f, 1.0f);
    glBegin(GL_LINES);
        glVertex2d((p1->x - bundler->center.x) / bundler->width * ZOOM_CONST, (p1->y - bundler->center.y) / bundler->height * ZOOM_CONST);
        glVertex2d((p2->x - bundler->center.x) / bundler->width * ZOOM_CONST, (p2->y - bundler->center.y) / bundler->height * ZOOM_CONST);
    glEnd();
}

void drawBezier(const Point *start, const Point *ctrl1, const Point *ctrl2, const Point *end, const int weight) {
    GLdouble controlPoints[4][3] = {{(start->x - bundler->center.x) / bundler->width * ZOOM_CONST, (start->y - bundler->center.y) / bundler->height * ZOOM_CONST, 0},
                                   {(ctrl1->x - bundler->center.x) / bundler->width * ZOOM_CONST, (ctrl1->y - bundler->center.y) / bundler->height * ZOOM_CONST, 0},
                                   {(ctrl2->x - bundler->center.x) / bundler->width * ZOOM_CONST, (ctrl2->y - bundler->center.y) / bundler->height * ZOOM_CONST, 0},
                                   {(end->x - bundler->center.x) / bundler->width * ZOOM_CONST, (end->y - bundler->center.y) / bundler->height * ZOOM_CONST, 0}};
    glColor3f(0.5f, 0.5f, 1.0f);
    glMap1d(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &controlPoints[0][0]);
    glEnable(GL_MAP1_VERTEX_3);
    glLineWidth(weight * 0.1f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 30; i++) {
        glEvalCoord1d((GLdouble) i / 30);
    }
    glEnd();
}

void init() {
    bundler = new EdgeBundler(FILENAME, 10, 0.8f);
    printf("Created Edge Bundler\n");
    bundler->doMingle();
    printf("Finished mingling");
    bundler->setDrawLineFunction(drawLine);
    bundler->setDrawBezierFunction(drawBezier);
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (w <= h)
        glOrtho(-1.0, 1.0, -1.0*(GLfloat)h/(GLfloat)w,
                1.0*(GLfloat)h/(GLfloat)w, -1.0, 1.0);
    else
        glOrtho(-1.0*(GLfloat)w/(GLfloat)h,
                1.0*(GLfloat)w/(GLfloat)h, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void renderBundler() {
    glClear(GL_COLOR_BUFFER_BIT);
    bundler->renderBezier();
    glFlush();
}

int main(int argc, char *argv[]) {
    init();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("fast-mingle");
    glutDisplayFunc(renderBundler);
    glutReshapeFunc(reshape);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glShadeModel(GL_FLAT);
    glutMainLoop();

    return 0;
}
