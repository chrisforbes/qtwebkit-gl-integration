#include "main.h"

#include <QtCore>
#include <QApplication>
#include <QtWebKit>
#include <QImage>
#include <QPainter>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <cstdio>

bool done = false;
QApplication *app;
GLuint tex;

#define __unused __attribute__((unused))
#define WIDTH 800
#define HEIGHT 600

void EventGlue::repaint(QRect const &dirtyRect __unused) {
    QSize size(WIDTH, HEIGHT);
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);

    this->page->mainFrame()->render(&painter);
    painter.end();

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_BGRA, GL_UNSIGNED_BYTE,
        image.bits());

    printf("web repaint\n");
}

void EventGlue::finished(bool ok __unused) {
    done = true;
}

void init(void) {
    glGenTextures(1, &tex);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void reshape(int w, int h) {
    printf("reshape %dx%d\n", w, h);
    glViewport(0, 0, w, h);
}

void draw(void) {
    glClearColor(0,0,1,0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex2f(-0.5,0.5);

        glTexCoord2f(1,0);
        glVertex2f(0.5,0.5);

        glTexCoord2f(1,1);
        glVertex2f(0.5,-0.5);

        glTexCoord2f(0,1);
        glVertex2f(-0.5,-0.5);
    glEnd();

    glutSwapBuffers();
    glutPostRedisplay();

    bool was_done = done;
    app->processEvents();

    if (!was_done && done) {
        printf("Page load complete.\n");
    }
}


int main(int argc, char **argv) {
    app = new QApplication(argc, argv);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("hello");

    init();
    glutReshapeFunc(reshape);
    glutDisplayFunc(draw);

    QSize size(WIDTH, HEIGHT);
    QWebPage wp;
    wp.setViewportSize(size);
    EventGlue glue(&wp);

    wp.mainFrame()->load(QUrl("http://facebook.com"));

    glutMainLoop();

    delete app;

    return 0;
}
