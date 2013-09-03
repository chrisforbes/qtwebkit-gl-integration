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
QWebPage *page;
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

int wnd_w;
int wnd_h;

void reshape(int w, int h) {
    printf("reshape %dx%d\n", w, h);
    glViewport(0, 0, w, h);
    wnd_w = w;
    wnd_h = h;
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

int buttons = 0;

Qt::MouseButton buttons_to_qt(int buttons) {
    int r = Qt::NoButton;
    if (buttons & (1<<GLUT_LEFT_BUTTON))
        r |= Qt::LeftButton;
    if (buttons & (1<<GLUT_RIGHT_BUTTON))
        r |= Qt::RightButton;
    if (buttons & (1<<GLUT_MIDDLE_BUTTON))
        r |= Qt::MiddleButton;
    return (Qt::MouseButton) r;
}


void mouse(int button, int state, int x, int y) {
    // first, get the coords converted into the space
    // expected by the browser

    float wx = ((float)x - wnd_w/4) / (wnd_w/2);
    float wy = ((float)y - wnd_h/4) / (wnd_h/2);

    QPoint pt(WIDTH * wx, HEIGHT * wy);

    printf("ev %d %d %d %d [%d %d]\n",
            button, state, x, y, pt.x(), pt.y());

    if (button == 3 || button == 4)
        return; /* ignore wheel events */

    QMouseEvent ev(
        state == GLUT_DOWN ? QEvent::MouseButtonPress : QEvent::MouseButtonRelease,
        pt, pt /* global */,
        buttons_to_qt(1<<button),
        QFlags<Qt::MouseButton>(buttons_to_qt(buttons)),
        Qt::NoModifier);

    page->event(&ev);

    if (state == GLUT_DOWN)
        buttons |= (1<<button);
    else
        buttons &= ~(1<<button);
}

void motion(int x, int y) {
    float wx = ((float)x - wnd_w/4) / (wnd_w/2);
    float wy = ((float)y - wnd_h/4) / (wnd_h/2);

    QPoint pt(WIDTH * wx, HEIGHT * wy);

    printf("mo - - %d %d [%d %d]\n",
            x, y, pt.x(), pt.y());
    QMouseEvent ev(
        QEvent::MouseMove,
        pt, pt /* global */,
        Qt::NoButton,
        QFlags<Qt::MouseButton>(buttons_to_qt(buttons)),
        Qt::NoModifier);

    page->event(&ev);
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
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(motion);

    QSize size(WIDTH, HEIGHT);
    page = new QWebPage;
    page->setViewportSize(size);
    EventGlue glue(page);

    page->mainFrame()->load(QUrl("http://www.google.co.nz"));

    glutMainLoop();

    delete page;
    delete app;

    return 0;
}
