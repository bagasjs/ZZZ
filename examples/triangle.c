#include "zzz.h"
#include <stdio.h>

#define GLBIND_IMPLEMENTATION
#include "glbind.h"

int main(void)
{
    ZZZ* zapp = zMemReserve(sizeof(ZZZ));
    ZZZInitInfo zinfo;
    zinfo.name = "Hello, World";
    zinfo.surfaceWidth = 800;
    zinfo.surfaceHeight = 600;

    if(zInit(zapp, &zinfo) != ZERR_NONE) {
        return 1;
    }

    GLBapi gl;
    GLBconfig config = glbConfigInit();
    config.hWnd = zapp->surface.hWnd;
    GLenum result = glbInit(&gl, &config);
    if(GL_NO_ERROR != result) {
        fprintf(stderr, "Failed to initialize glbind");
        return -1;
    }

    glbBindAPI(&gl);
    SetPixelFormat(GetDC(zapp->surface.hWnd), glbGetPixelFormat(), glbGetPFD());
    zSetWindowVisibility(zapp, TRUE);
    gl.wglMakeCurrent(GetDC(zapp->surface.hWnd), glbGetRC());

    ZEvent event;
    b32 quit = FALSE;
    while(!quit) {
        zPollEvents(zapp);
        while(zNextEvent(zapp, &event)) {
            switch(event.type) {
                case ZEVENT_WINDOW_CLOSED:
                    {
                        quit = TRUE;
                    } break;
                case ZEVENT_KEY_PRESSED:
                    {
                        printf("Key Pressed\n");
                    } break;
                case ZEVENT_WINDOW_RESIZED:
                    {
                        glViewport(0, 0, event.window.width, event.window.height);
                    } break;
            }
        }
        glClearColor(0.2f, 0.5, 0.8f, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glBegin(GL_TRIANGLES);
            glColor3f(1, 0, 0);
            glVertex2f(0.0f, +0.5f);
            glColor3f(0, 1, 0);
            glVertex2f(-0.5f, -0.5f);
            glColor3f(0, 0, 1);
            glVertex2f(+0.5f, -0.5f);
        glEnd();
        SwapBuffers(GetDC(zapp->surface.hWnd));
    }

    zMemRelease(zapp);
    glbUninit();
}
