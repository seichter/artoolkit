/*
 *  simpleLite.c
 *
 *  Some code to demonstrate use of gsub_lite's argl*() functions.
 *  Shows the correct GLUT usage to read a video frame (in the idle callback)
 *  and to draw it (in the display callback).
 *
 *  Press '?' while running for help on available key commands.
 *
 *  Copyright (c) 2001-2007 Philip Lamb (PRL) phil@eden.net.nz. All rights reserved.
 *
 *	Rev		Date		Who		Changes
 *	1.0.0	20040302	PRL		Initial version, simple test animation using GLUT.
 *	1.0.1	20040721	PRL		Correctly sets window size; supports arVideoDispOption().
 *
 */
/*
 *
 * This file is part of ARToolKit.
 *
 * ARToolKit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ARToolKit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ARToolKit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


// ============================================================================
//	Includes
// ============================================================================

#include <stdio.h>
#include <stdlib.h>					// malloc(), free()
#include <AR/config.h>
#include <AR/video.h>
#include <AR/param.h>			// arParamDisp()
#include <AR/ar.h>
#include <AR/arGLUtils.h>


#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

char *cparam_name = "Data/camera_para.dat";
char *vconf = "";
char *patt_name  = "Data/patt.hiro";

static ARParam		gARTCparam;





typedef struct {
    SDL_Texture     *background;
    SDL_Window      *window;
    SDL_Renderer    *renderer;

    SDL_GLContext   *context;
} AppState;


Uint32              gsBGTextureFormat = SDL_PIXELFORMAT_RGB24;

// Image acquisition.
static ARUint8		*gsARTImage = 0L;
static int          gsARTImageWidth;
static int          gsARTImageHeight;


// Marker detection.
static int			gARTThreshhold = 100;
static long			gCallCountMarkerDetect = 0;

// Transformation matrix retrieval.
static double		gPatt_width     = 80.0;	// Per-marker, but we are using only 1 marker.
static double		gPatt_centre[2] = {0.0, 0.0}; // Per-marker, but we are using only 1 marker.
static double		gPatt_trans[3][4];		// Per-marker, but we are using only 1 marker.
static int			gPatt_found = 0;	// Per-marker, but we are using only 1 marker.
static int			gPatt_id;				// Per-marker, but we are using only 1 marker.


#define VIEW_SCALEFACTOR		0.025		// 1.0 ARToolKit unit becomes 0.025 of my OpenGL units.
#define VIEW_DISTANCE_MIN		0.1			// Objects closer to the camera than this will not be displayed.
#define VIEW_DISTANCE_MAX		100.0		// Objects further away from the camera than this will not be displayed.


static int createBackgroundImage(AppState* state,int width,int height)
{

    state->background = SDL_CreateTexture(state->renderer,gsBGTextureFormat,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          width, height);

    if (state->background == 0L) {
        fprintf(stderr, "Could not create texture for background\n");
        return 0;
    }

    return 1;
}



// Something to look at, draw a rotating colour cube.
static void DrawCube(void)
{
    // Colour cube data.
    static GLuint polyList = 0;
    float fSize = 0.5f;
    long f, i;
    const GLfloat cube_vertices [8][3] = {
        {1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {-1.0, -1.0, 1.0}, {-1.0, 1.0, 1.0},
        {1.0, 1.0, -1.0}, {1.0, -1.0, -1.0}, {-1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0} };
    const GLfloat cube_vertex_colors [8][3] = {
        {1.0, 1.0, 1.0}, {1.0, 1.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 1.0},
        {1.0, 0.0, 1.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0} };
    GLint cube_num_faces = 6;
    const short cube_faces [6][4] = {
        {3, 2, 1, 0}, {2, 3, 7, 6}, {0, 1, 5, 4}, {3, 0, 4, 7}, {1, 2, 6, 5}, {4, 5, 6, 7} };

    if (!polyList) {

        polyList++;

//        printf("Render!");
//        polyList = glGenLists (1);
//        glNewList(polyList, GL_COMPILE);
        glBegin (GL_QUADS);
        for (f = 0; f < cube_num_faces; f++)
            for (i = 0; i < 4; i++) {
                glColor3f (cube_vertex_colors[cube_faces[f][i]][0], cube_vertex_colors[cube_faces[f][i]][1], cube_vertex_colors[cube_faces[f][i]][2]);
                glVertex3f(cube_vertices[cube_faces[f][i]][0] * fSize, cube_vertices[cube_faces[f][i]][1] * fSize, cube_vertices[cube_faces[f][i]][2] * fSize);
            }
        glEnd ();
        glColor3f (0.0, 0.0, 0.0);
        for (f = 0; f < cube_num_faces; f++) {
            glBegin (GL_LINE_LOOP);
            for (i = 0; i < 4; i++)
                glVertex3f(cube_vertices[cube_faces[f][i]][0] * fSize, cube_vertices[cube_faces[f][i]][1] * fSize, cube_vertices[cube_faces[f][i]][2] * fSize);
            glEnd ();
        }
//        glEndList ();
    }

    glPushMatrix(); // Save world coordinate system.
    glTranslatef(0.0, 0.0, 0.5); // Place base of cube on marker surface.
    //    glRotatef(gDrawRotateAngle, 0.0, 0.0, 1.0); // Rotate about z axis.
//    glDisable(GL_LIGHTING);	// Just use colours.
//    glCallList(polyList);	// Draw the cube.
    glPopMatrix();	// Restore world coordinate system.

//    glCallList(polyList);	// Draw the cube.

}

static int renderScene(AppState* state)
{

    if (gPatt_found) {


        GLdouble projectionMatrix[16];
        GLdouble modelViewMatrix[16];

        // Projection transformation.
        arglCameraFrustumRH(&gARTCparam, VIEW_DISTANCE_MIN, VIEW_DISTANCE_MAX, projectionMatrix);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(projectionMatrix);

        glMatrixMode(GL_MODELVIEW);


        // Calculate the camera position relative to the marker.
        // Replace VIEW_SCALEFACTOR with 1.0 to make one drawing unit equal to 1.0 ARToolKit units (usually millimeters).
        arglCameraViewRH(gPatt_trans, modelViewMatrix, VIEW_SCALEFACTOR);
        glLoadMatrixd(modelViewMatrix);

        // All lighting and geometry to be drawn relative to the marker goes here.
//        DrawCube();


    } // gPatt_found


    {
        GLuint gle = glGetError();
        if (gle) {
            fprintf(stderr,"GL error 0x%x\n");
        }
    }

}


void renderBackgroundImage(AppState* state,Uint8* image)
{
    Uint8 *pixels = 0;
    int pitch1;

    if (0 == SDL_LockTexture(state->background, NULL, (void **)&pixels, &pitch1))
    {
        GLboolean isLightingOn = glIsEnabled(GL_LIGHTING);
        GLboolean isDepthTestOn = glIsEnabled(GL_DEPTH_TEST);


        if (isDepthTestOn) glDisable(GL_DEPTH_TEST);
        if (isLightingOn) glDisable(GL_LIGHTING);

        switch(gsBGTextureFormat) {
        case SDL_PIXELFORMAT_YV12:

            //        memcpy(pixels,             image, size1  );
            //        memcpy(pixels + size1,     pFrame_YUV420P->data[2], size1/4);
            //        memcpy(pixels + size1*5/4, pFrame_YUV420P->data[1], size1/4);
            break;
        case SDL_PIXELFORMAT_RGB24:
            memcpy( pixels, image, gsARTImageHeight*gsARTImageWidth*sizeof(Uint8)*3);
            break;
        }

        SDL_UnlockTexture(state->background);
        SDL_UpdateTexture(state->background, NULL, pixels, pitch1);

        float width,height;

        float posX = 0, posY = 0;


        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0f, gsARTImageWidth, gsARTImageHeight, 0.0f, -1.0f, 1.0f);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        /* this accounts for Texture2D vs TextureRectangle */
        if (0 == SDL_GL_BindTexture(state->background, &width, &height ))
        {
            glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex2f(posX,posY);
                glTexCoord2f(1*width, 0);
                glVertex2f(posX + width, posY);
                glTexCoord2f(1*width, 1*height);
                glVertex2f(posX + width, posY + height);
                glTexCoord2f(0, 1*height);
                glVertex2f(posX, posY + height);
            glEnd();


//            SDL_GL_UnbindTexture(state->background);
        }

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        if (isDepthTestOn) glEnable(GL_DEPTH_TEST);
        if (isLightingOn) glEnable(GL_LIGHTING);

    }
}

static int update(AppState* state)
{

    ARMarkerInfo    *marker_info;					// Pointer to array holding the details of detected markers.
    int             marker_num;						// Count of number of markers detected.
    int             j, k;

    // Grab a video frame.
    if ((gsARTImage = arVideoGetImage()) != NULL) {

        gCallCountMarkerDetect++; // Increment ARToolKit FPS counter.

        // Detect the markers in the video frame.
        if (arDetectMarker(gsARTImage, gARTThreshhold, &marker_info, &marker_num) < 0) {
            exit(-1);
        }

        SDL_RenderClear(state->renderer);

        glDrawBuffer(GL_BACK);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers for new frame.

        renderBackgroundImage(state,gsARTImage);

        // Check through the marker_info array for highest confidence
        // visible marker matching our preferred pattern.
        k = -1;
        for (j = 0; j < marker_num; j++) {
            if (marker_info[j].id == gPatt_id) {
                if (k == -1) k = j; // First marker detected.
                else if(marker_info[j].cf > marker_info[k].cf) k = j; // Higher confidence marker detected.
            }
        }

        if (k != -1) {
            // Get the transformation between the marker and the real camera into gPatt_trans.
            arGetTransMat(&(marker_info[k]), gPatt_centre, gPatt_width, gPatt_trans);
            gPatt_found = 1;
        } else {
            gPatt_found = 0;
        }


        renderScene(state);


//        SDL_RenderPresent(state->renderer);

        SDL_GL_SwapWindow(state->window);
    }

}



static int setupCamera(const char *cparam_name, char *vconf, ARParam *cparam)
{
    ARParam			wparam;

    // Open the video path.
    if (arVideoOpen(vconf) < 0) {
        fprintf(stderr, "setupCamera(): Unable to open connection to camera.\n");
        return 0;
    }

    // Find the size of the window.
    if (arVideoInqSize(&gsARTImageWidth, &gsARTImageHeight) < 0) return 0;
    fprintf(stdout, "Camera image size (x,y) = (%d,%d)\n", gsARTImageWidth, gsARTImageHeight);

    // Load the camera parameters, resize for the window and init.
    if (arParamLoad(cparam_name, 1, &wparam) < 0) {
        fprintf(stderr, "setupCamera(): Error loading parameter file %s for camera.\n", cparam_name);
        return 0;
    }


    arParamChangeSize(&wparam, gsARTImageWidth, gsARTImageHeight, cparam);
    fprintf(stdout, "*** Camera Parameter ***\n");
    arParamDisp(cparam);

    arInitCparam(cparam);


    if (arVideoCapStart() != 0) {
        fprintf(stderr, "setupCamera(): Unable to begin camera data capture.\n");
        return 0;
    }
    return 1;
}

static int setupMarker(const char *patt_name, int *patt_id)
{
    // Loading only 1 pattern in this example.
    if ((*patt_id = arLoadPatt(patt_name)) < 0) {
        fprintf(stderr, "setupMarker(): pattern load error !!\n");
        return 0;
    }

    return 1;
}

/* the main function */
int main(int argc, char *argv[])
{

    AppState state;

    char *vconf = "";

    if (SDL_Init(SDL_INIT_EVERYTHING) == -1){
        fprintf(stderr,"SDL error %s\n",SDL_GetError());
        return 1;
    }

    SDL_RendererInfo displayRendererInfo;


    SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_OPENGL, &state.window, &state.renderer);
    SDL_GetRendererInfo(state.renderer, &displayRendererInfo);


    fprintf(stdout,"SDL Renderer accelerated %d\n",displayRendererInfo.flags & SDL_RENDERER_ACCELERATED);
    fprintf(stdout,"SDL Renderer target texture %d\n",displayRendererInfo.flags & SDL_RENDERER_TARGETTEXTURE);



    /* check support */
    if ((displayRendererInfo.flags & SDL_RENDERER_ACCELERATED) == 0 ||
            (displayRendererInfo.flags & SDL_RENDERER_TARGETTEXTURE) == 0)
    {
        fprintf(stderr,"SDL not fully supported\n");
        return -1;
    }

    if (state.window == 0L){
        fprintf(stderr,"SDL error %s\n",SDL_GetError());
        return 1;
    }


    // turn on double buffering set the depth buffer to 24 bits
    // you may need to change this to 16 or 32 for your system
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

    state.context = SDL_GL_CreateContext(state.window);
    if (state.context == 0L){
        fprintf(stderr,"SDL error %s\n",SDL_GetError());
        return 1;
    }

    if (!setupCamera(cparam_name, vconf, &gARTCparam)) {
        fprintf(stderr, "main(): Unable to set up AR camera.\n");
        return -1;
    }

    // Load marker(s).
    if (!setupMarker(patt_name, &gPatt_id)) {
        fprintf(stderr, "main(): Unable to set up AR marker.\n");
        return -1;
    }

    // Poll for events, and handle the ones we care about.
    SDL_Event event;
    int running = 1;

    SDL_SetRenderDrawColor(state.renderer, 10, 30, 255, 255);

     glEnable(GL_DEPTH_TEST);

    createBackgroundImage(&state,gsARTImageWidth,gsARTImageHeight);

    while (running) {


        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                break;
            case SDL_KEYUP:
                // If escape is pressed, return (and thus, quit)
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    return 0;
                break;
            case SDL_QUIT:
                goto cleanup;
            }
        }

        update(&state);

        SDL_GL_SwapWindow(state.window);

    }

cleanup:

    SDL_DestroyTexture(state.background);
    //    SDL_DestroyTexture(state.scene);

    SDL_GL_DeleteContext(state.context);

    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);

    return 0;
}




#if 0

#if defined(_WIN32)
#include <windows.h>
#endif
#ifndef __APPLE__
#  include <GL/glut.h>
#  ifdef GL_VERSION_1_2
#    include <GL/glext.h>
#  endif
#else
#  include <GLUT/glut.h>
#  include <OpenGL/glext.h>
#endif

// ============================================================================
//	Constants
// ============================================================================

#define VIEW_SCALEFACTOR		0.025		// 1.0 ARToolKit unit becomes 0.025 of my OpenGL units.
#define VIEW_DISTANCE_MIN		0.1			// Objects closer to the camera than this will not be displayed.
#define VIEW_DISTANCE_MAX		100.0		// Objects further away from the camera than this will not be displayed.

// ============================================================================
//	Global variables
// ============================================================================

// Preferences.
static int prefWindowed = 1;
static int prefWidth = 640;					// Fullscreen mode width.
static int prefHeight = 480;				// Fullscreen mode height.
static int prefDepth = 32;					// Fullscreen mode bit depth.
static int prefRefresh = 0;					// Fullscreen mode refresh rate. Set to 0 to use default rate.

// Image acquisition.
static ARUint8		*gARTImage = NULL;

// Marker detection.
static int			gARTThreshhold = 100;
static long			gCallCountMarkerDetect = 0;

// Transformation matrix retrieval.
static double		gPatt_width     = 80.0;	// Per-marker, but we are using only 1 marker.
static double		gPatt_centre[2] = {0.0, 0.0}; // Per-marker, but we are using only 1 marker.
static double		gPatt_trans[3][4];		// Per-marker, but we are using only 1 marker.
static int			gPatt_found = 0;	// Per-marker, but we are using only 1 marker.
static int			gPatt_id;				// Per-marker, but we are using only 1 marker.

// Drawing.
static ARParam		gARTCparam;
static int gDrawRotate = 0;
static float gDrawRotateAngle = 0;			// For use in drawing.

// ============================================================================
//	Functions
// ============================================================================

// Something to look at, draw a rotating colour cube.
static void DrawCube(void)
{
    // Colour cube data.
    static GLuint polyList = 0;
    float fSize = 0.5f;
    long f, i;
    const GLfloat cube_vertices [8][3] = {
        {1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {-1.0, -1.0, 1.0}, {-1.0, 1.0, 1.0},
        {1.0, 1.0, -1.0}, {1.0, -1.0, -1.0}, {-1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0} };
    const GLfloat cube_vertex_colors [8][3] = {
        {1.0, 1.0, 1.0}, {1.0, 1.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 1.0},
        {1.0, 0.0, 1.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0} };
    GLint cube_num_faces = 6;
    const short cube_faces [6][4] = {
        {3, 2, 1, 0}, {2, 3, 7, 6}, {0, 1, 5, 4}, {3, 0, 4, 7}, {1, 2, 6, 5}, {4, 5, 6, 7} };

    if (!polyList) {
        polyList = glGenLists (1);
        glNewList(polyList, GL_COMPILE);
        glBegin (GL_QUADS);
        for (f = 0; f < cube_num_faces; f++)
            for (i = 0; i < 4; i++) {
                glColor3f (cube_vertex_colors[cube_faces[f][i]][0], cube_vertex_colors[cube_faces[f][i]][1], cube_vertex_colors[cube_faces[f][i]][2]);
                glVertex3f(cube_vertices[cube_faces[f][i]][0] * fSize, cube_vertices[cube_faces[f][i]][1] * fSize, cube_vertices[cube_faces[f][i]][2] * fSize);
            }
        glEnd ();
        glColor3f (0.0, 0.0, 0.0);
        for (f = 0; f < cube_num_faces; f++) {
            glBegin (GL_LINE_LOOP);
            for (i = 0; i < 4; i++)
                glVertex3f(cube_vertices[cube_faces[f][i]][0] * fSize, cube_vertices[cube_faces[f][i]][1] * fSize, cube_vertices[cube_faces[f][i]][2] * fSize);
            glEnd ();
        }
        glEndList ();
    }

    glPushMatrix(); // Save world coordinate system.
    glTranslatef(0.0, 0.0, 0.5); // Place base of cube on marker surface.
    glRotatef(gDrawRotateAngle, 0.0, 0.0, 1.0); // Rotate about z axis.
    glDisable(GL_LIGHTING);	// Just use colours.
    glCallList(polyList);	// Draw the cube.
    glPopMatrix();	// Restore world coordinate system.

}

static void DrawCubeUpdate(float timeDelta)
{
    if (gDrawRotate) {
        gDrawRotateAngle += timeDelta * 45.0f; // Rotate cube at 45 degrees per second.
        if (gDrawRotateAngle > 360.0f) gDrawRotateAngle -= 360.0f;
    }
}

static int setupCamera(const char *cparam_name, char *vconf, ARParam *cparam)
{	
    ARParam			wparam;
    int				xsize, ysize;

    // Open the video path.
    if (arVideoOpen(vconf) < 0) {
        fprintf(stderr, "setupCamera(): Unable to open connection to camera.\n");
        return 0;
    }

    // Find the size of the window.
    if (arVideoInqSize(&xsize, &ysize) < 0) return 0;
    fprintf(stdout, "Camera image size (x,y) = (%d,%d)\n", xsize, ysize);

    // Load the camera parameters, resize for the window and init.
    if (arParamLoad(cparam_name, 1, &wparam) < 0) {
        fprintf(stderr, "setupCamera(): Error loading parameter file %s for camera.\n", cparam_name);
        return 0;
    }
    arParamChangeSize(&wparam, xsize, ysize, cparam);
    fprintf(stdout, "*** Camera Parameter ***\n");
    arParamDisp(cparam);

    arInitCparam(cparam);

    if (arVideoCapStart() != 0) {
        fprintf(stderr, "setupCamera(): Unable to begin camera data capture.\n");
        return 0;
    }

    return 1;
}

static int setupMarker(const char *patt_name, int *patt_id)
{
    // Loading only 1 pattern in this example.
    if ((*patt_id = arLoadPatt(patt_name)) < 0) {
        fprintf(stderr, "setupMarker(): pattern load error !!\n");
        return 0;
    }

    return 1;
}

//// Report state of ARToolKit global variables arFittingMode,
//// arImageProcMode, arglDrawMode, arTemplateMatchingMode, arMatchingPCAMode.
//static void debugReportMode(const ARGL_CONTEXT_SETTINGS_REF arglContextSettings)
//{
//	if (arFittingMode == AR_FITTING_TO_INPUT) {
//		fprintf(stderr, "FittingMode (Z): INPUT IMAGE\n");
//	} else {
//		fprintf(stderr, "FittingMode (Z): COMPENSATED IMAGE\n");
//	}

//	if (arImageProcMode == AR_IMAGE_PROC_IN_FULL) {
//		fprintf(stderr, "ProcMode (X)   : FULL IMAGE\n");
//	} else {
//		fprintf(stderr, "ProcMode (X)   : HALF IMAGE\n");
//	}

//	if (arglDrawModeGet(arglContextSettings) == AR_DRAW_BY_GL_DRAW_PIXELS) {
//		fprintf(stderr, "DrawMode (C)   : GL_DRAW_PIXELS\n");
//	} else if (arglTexmapModeGet(arglContextSettings) == AR_DRAW_TEXTURE_FULL_IMAGE) {
//		fprintf(stderr, "DrawMode (C)   : TEXTURE MAPPING (FULL RESOLUTION)\n");
//	} else {
//		fprintf(stderr, "DrawMode (C)   : TEXTURE MAPPING (HALF RESOLUTION)\n");
//	}

//	if (arTemplateMatchingMode == AR_TEMPLATE_MATCHING_COLOR) {
//		fprintf(stderr, "TemplateMatchingMode (M)   : Color Template\n");
//	} else {
//		fprintf(stderr, "TemplateMatchingMode (M)   : BW Template\n");
//	}

//	if (arMatchingPCAMode == AR_MATCHING_WITHOUT_PCA) {
//		fprintf(stderr, "MatchingPCAMode (P)   : Without PCA\n");
//	} else {
//		fprintf(stderr, "MatchingPCAMode (P)   : With PCA\n");
//	}
//}

static void cleanup(void)
{
    //	arglCleanup(gArglSettings);
    arVideoCapStop();
    arVideoClose();
}

static void Keyboard(unsigned char key, int x, int y)
{
    int mode, threshChange = 0;

    switch (key) {
    case 0x1B:						// Quit.
    case 'Q':
    case 'q':
        cleanup();
        exit(0);
        break;
    case ' ':
        gDrawRotate = !gDrawRotate;
        break;
        //		case 'C':
        //		case 'c':
        //			mode = arglDrawModeGet(gArglSettings);
        //			if (mode == AR_DRAW_BY_GL_DRAW_PIXELS) {
        //				arglDrawModeSet(gArglSettings, AR_DRAW_BY_TEXTURE_MAPPING);
        //				arglTexmapModeSet(gArglSettings, AR_DRAW_TEXTURE_FULL_IMAGE);
        //			} else {
        //				mode = arglTexmapModeGet(gArglSettings);
        //				if (mode == AR_DRAW_TEXTURE_FULL_IMAGE)	arglTexmapModeSet(gArglSettings, AR_DRAW_TEXTURE_HALF_IMAGE);
        //				else arglDrawModeSet(gArglSettings, AR_DRAW_BY_GL_DRAW_PIXELS);
        //			}
        //			fprintf(stderr, "*** Camera - %f (frame/sec)\n", (double)gCallCountMarkerDetect/arUtilTimer());
        //			gCallCountMarkerDetect = 0;
        //			arUtilTimerReset();
        //			debugReportMode(gArglSettings);
        //			break;
    case '-':
        threshChange = -5;
        break;
    case '+':
    case '=':
        threshChange = +5;
        break;
    case 'D':
    case 'd':
        arDebug = !arDebug;
        break;
    case '?':
    case '/':
        printf("Keys:\n");
        printf(" q or [esc]    Quit demo.\n");
        printf(" c             Change arglDrawMode and arglTexmapMode.\n");
        printf(" - and +       Adjust threshhold.\n");
        printf(" d             Activate / deactivate debug mode.\n");
        printf(" ? or /        Show this help.\n");
        printf("\nAdditionally, the ARVideo library supplied the following help text:\n");
        arVideoDispOption();
        break;
    default:
        break;
    }
    if (threshChange) {
        gARTThreshhold += threshChange;
        if (gARTThreshhold < 0) gARTThreshhold = 0;
        if (gARTThreshhold > 255) gARTThreshhold = 255;
        printf("Threshhold changed to %d.\n", gARTThreshhold);
    }
}

static void mainLoop(void)
{
    static int ms_prev;
    int ms;
    float s_elapsed;
    ARUint8 *image;

    ARMarkerInfo    *marker_info;					// Pointer to array holding the details of detected markers.
    int             marker_num;						// Count of number of markers detected.
    int             j, k;

    // Find out how long since mainLoop() last ran.
    ms = glutGet(GLUT_ELAPSED_TIME);
    s_elapsed = (float)(ms - ms_prev) * 0.001;
    if (s_elapsed < 0.01f) return; // Don't update more often than 100 Hz.
    ms_prev = ms;

    // Update drawing.
    DrawCubeUpdate(s_elapsed);

    // Grab a video frame.
    if ((image = arVideoGetImage()) != NULL) {
        gARTImage = image;	// Save the fetched image.

        gCallCountMarkerDetect++; // Increment ARToolKit FPS counter.

        // Detect the markers in the video frame.
        if (arDetectMarker(gARTImage, gARTThreshhold, &marker_info, &marker_num) < 0) {
            exit(-1);
        }

        // Check through the marker_info array for highest confidence
        // visible marker matching our preferred pattern.
        k = -1;
        for (j = 0; j < marker_num; j++) {
            if (marker_info[j].id == gPatt_id) {
                if (k == -1) k = j; // First marker detected.
                else if(marker_info[j].cf > marker_info[k].cf) k = j; // Higher confidence marker detected.
            }
        }

        if (k != -1) {
            // Get the transformation between the marker and the real camera into gPatt_trans.
            arGetTransMat(&(marker_info[k]), gPatt_centre, gPatt_width, gPatt_trans);
            gPatt_found = 1;
        } else {
            gPatt_found = 0;
        }

        // Tell GLUT the display has changed.
        glutPostRedisplay();
    }
}

//
//	This function is called on events when the visibility of the
//	GLUT window changes (including when it first becomes visible).
//
static void Visibility(int visible)
{
    if (visible == GLUT_VISIBLE) {
        glutIdleFunc(mainLoop);
    } else {
        glutIdleFunc(NULL);
    }
}

//
//	This function is called when the
//	GLUT window is resized.
//
static void Reshape(int w, int h)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Call through to anyone else who needs to know about window sizing here.
}

//
// This function is called when the window needs redrawing.
//
static void Display(void)
{
    GLdouble p[16];
    GLdouble m[16];

    // Select correct buffer for this context.
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers for new frame.

    //	arglDispImage(gARTImage, &gARTCparam, 1.0, gArglSettings);	// zoom = 1.0.
    arVideoCapNext();
    gARTImage = NULL; // Image data is no longer valid after calling arVideoCapNext().

    // Projection transformation.
    arglCameraFrustumRH(&gARTCparam, VIEW_DISTANCE_MIN, VIEW_DISTANCE_MAX, p);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(p);
    glMatrixMode(GL_MODELVIEW);

    // Viewing transformation.
    glLoadIdentity();
    // Lighting and geometry that moves with the camera should go here.
    // (I.e. must be specified before viewing transformations.)
    //none

    if (gPatt_found) {

        // Calculate the camera position relative to the marker.
        // Replace VIEW_SCALEFACTOR with 1.0 to make one drawing unit equal to 1.0 ARToolKit units (usually millimeters).
        arglCameraViewRH(gPatt_trans, m, VIEW_SCALEFACTOR);
        glLoadMatrixd(m);

        // All lighting and geometry to be drawn relative to the marker goes here.
        DrawCube();

    } // gPatt_found

    // Any 2D overlays go here.
    //none

    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    char glutGamemode[32];
    char *cparam_name = "Data/camera_para.dat";
    char *vconf = "";
    char *patt_name  = "Data/patt.hiro";

    // ----------------------------------------------------------------------------
    // Library inits.
    //

    glutInit(&argc, argv);

    // ----------------------------------------------------------------------------
    // Hardware setup.
    //

    if (!setupCamera(cparam_name, vconf, &gARTCparam)) {
        fprintf(stderr, "main(): Unable to set up AR camera.\n");
        exit(-1);
    }

    // ----------------------------------------------------------------------------
    // Library setup.
    //

    // Set up GL context(s) for OpenGL to draw into.
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    if (!prefWindowed) {
        if (prefRefresh) sprintf(glutGamemode, "%ix%i:%i@%i", prefWidth, prefHeight, prefDepth, prefRefresh);
        else sprintf(glutGamemode, "%ix%i:%i", prefWidth, prefHeight, prefDepth);
        glutGameModeString(glutGamemode);
        glutEnterGameMode();
    } else {
        glutInitWindowSize(prefWidth, prefHeight);
        glutCreateWindow(argv[0]);
    }

    //	// Setup ARgsub_lite library for current OpenGL context.
    //	if ((gArglSettings = arglSetupForCurrentContext()) == NULL) {
    //		fprintf(stderr, "main(): arglSetupForCurrentContext() returned error.\n");
    //		cleanup();
    //		exit(-1);
    //	}
    //	debugReportMode(gArglSettings);
    glEnable(GL_DEPTH_TEST);
    arUtilTimerReset();

    // Load marker(s).
    if (!setupMarker(patt_name, &gPatt_id)) {
        fprintf(stderr, "main(): Unable to set up AR marker.\n");
        cleanup();
        exit(-1);
    }

    //	// Register GLUT event-handling callbacks.
    //	// NB: mainLoop() is registered by Visibility.
    //	glutDisplayFunc(Display);
    //	glutReshapeFunc(Reshape);
    //	glutVisibilityFunc(Visibility);
    //	glutKeyboardFunc(Keyboard);

    //	glutMainLoop();

    return (0);
}
#endif
