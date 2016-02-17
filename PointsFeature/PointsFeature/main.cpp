#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#define GLEW_STATIC 1
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>

#pragma comment(lib,"glew32s.lib")

//Shader类型
typedef enum {
    EVertexShader,
    EFragmentShader,
} EShaderType;

//Shader类型
typedef enum {
    EFixFunction,
    EProgrammable,
} EPipelineMode;

//窗口句柄
static GLint window;

EPipelineMode pipeMode = EProgrammable;

GLboolean bEnableProgramPointSize = GL_FALSE;

GLboolean bEnableProgramPointSprite = GL_FALSE;

GLboolean bSpriteCoordOriginUpperLeft = GL_TRUE;

GLboolean bClipControlOriginUpperLeft = GL_FALSE;

GLboolean bUsingFBO = GL_FALSE;

GLuint prog     = 0;
GLuint tex      = 0;
GLuint fboTex   = 0;
GLuint fbo      = 0;
GLuint va[2];

GLfloat fPtSizeAPI = 65.0;
GLfloat fPtSizeUniform = 45.0;

#define POINT_POSITION_1 -70.0, -50.0
#define POINT_POSITION_2  30.0, -10.0
#define POINT_POSITION_3  50.0,  80.0

static float vertexData[] =
{
    POINT_POSITION_1, -50.0, 1.0, 0.0, 0.0,
    POINT_POSITION_2, -50.0, 1.0, 1.0, 0.0,
    POINT_POSITION_3, -50.0, 1.0, 1.0, 1.0,
};

static float modelViewMat[] =
{
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};

//void Frustum( -100.0, 100.0, -100.0, 100.0, 50.0, 150.0);
static float projectionMat[] =
{
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, -2.0, -150.0,
    0.0, 0.0, -1.0, 0.0
};

static float bltFboVertexData[] =
{
    -100.0, -100.0, -50.0, 1.0, 0.0, 0.0,
     100.0, -100.0, -50.0, 1.0, 1.0, 0.0,
    -100.0,  100.0, -50.0, 1.0, 0.0, 1.0,
     100.0,  100.0, -50.0, 1.0, 1.0, 1.0,
};

//用来检查OpenGL版本，需要GLSL 2.0支持
void getGlVersion( int *major, int *minor )
{
    const char* verstr = (const char*)glGetString( GL_VERSION );
    if( (verstr == NULL) || (sscanf( verstr, "%d.%d", major, minor ) != 2) )
    {
        *major = *minor = 0;
        fprintf( stderr, "Invalid GL_VERSION format!!!\n" );
    }
}

static void display(void)
{
    GLuint loc = -1;

    printf("**********************************************\n");
    printf("PipeMode          : %s\n", pipeMode == EProgrammable ? "Program" : "FixFuction");
    printf("PointSize         : %f\n",
        (pipeMode == EProgrammable && bEnableProgramPointSize) ? fPtSizeUniform : fPtSizeAPI);
    printf("ProgramPointSize  : %s\n", bEnableProgramPointSize ? "ON" : "OFF");
    printf("PointSprite       : %s\n", bEnableProgramPointSprite ? "ON" : "OFF");
    printf("SpriteCoordOrigin : %s\n", bSpriteCoordOriginUpperLeft ? "UpperLeft" : "LowerLeft");
    printf("ClipControlOrigin : %s\n", bClipControlOriginUpperLeft ? "UpperLeft" : "LowerLeft");
    printf("RenderTarget      : %s\n", bUsingFBO ? "FBO" : "Screen");
    printf("**********************************************\n\n");

    if (bUsingFBO)
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glClearColor(0.3,0.3,0.3,1.0);
    }
    else
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClearColor(0.2,0.2,0.2,1.0);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_POINT_SMOOTH);

    //Attrib config which affect both path
    glBindTexture(GL_TEXTURE_2D, tex);
    glPointSize(fPtSizeAPI);
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN,
        bSpriteCoordOriginUpperLeft ? GL_UPPER_LEFT : GL_LOWER_LEFT);
    glClipControl(bClipControlOriginUpperLeft ? GL_UPPER_LEFT : GL_LOWER_LEFT,
        GL_NEGATIVE_ONE_TO_ONE);

    if (bEnableProgramPointSprite)
        glEnable(GL_POINT_SPRITE);
    else
        glDisable(GL_POINT_SPRITE);

    if (bEnableProgramPointSize)
        glEnable(GL_PROGRAM_POINT_SIZE);
    else
        glDisable(GL_PROGRAM_POINT_SIZE);

    if (pipeMode == EProgrammable)
    {
        glUseProgram(prog);
        loc = glGetUniformLocation(prog, "bEnableSprite");
        if (bEnableProgramPointSprite)
        {
            glUniform1i(loc, 1);
        }
        else
        {
            glUniform1i(loc, 0);
        }

        glBindVertexArray(va[0]);
        glDrawArrays(GL_POINTS, 0, 3);

        if (bUsingFBO)
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);

            glUseProgram(prog);
            loc = glGetUniformLocation(prog, "bEnableSprite");
            glUniform1i(loc, 0);
            loc = glGetUniformLocation(prog, "bBltFBO");
            glUniform1i(loc, 1);

            glBindTexture(GL_TEXTURE_2D, fboTex);
            glBindVertexArray(va[1]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            loc = glGetUniformLocation(prog, "bBltFBO");
            glUniform1i(loc, 0);
        }
    }
    else
    {
        glUseProgram(0);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D (-100.0, 100.0, -100.0, 100.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if (bEnableProgramPointSprite)
        {
            //Only affect FixFunction
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        }
        else
        {
            glDisable(GL_TEXTURE_2D);
            glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
        }

        glBegin(GL_POINTS);
        glColor4f(1.0, 0.0, 0.0, 1.0);
        glVertex2f(POINT_POSITION_1);
        glVertex2f(POINT_POSITION_2);
        glVertex2f(POINT_POSITION_3);
        glEnd();

        if (bUsingFBO)
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);

            glDisable(GL_POINT_SPRITE);
            glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, fboTex);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f( 0.0, 0.0); glVertex2f(-100.0, -100.0);
            glTexCoord2f( 1.0, 0.0); glVertex2f( 100.0, -100.0);
            glTexCoord2f( 0.0, 1.0); glVertex2f(-100.0,  100.0);
            glTexCoord2f( 1.0, 1.0); glVertex2f( 100.0,  100.0);
            glEnd();
        }
    }

    glutSwapBuffers();
}

static void reshape(int wid, int ht)
{
    return;
}

//Shader related functions
static int shaderSize(char *fileName, EShaderType shaderType)
{
    //返回顶点着色器或者片段着色器的大小
    char name[100];
    strcpy(name, fileName);

    switch (shaderType)
    {
    case EVertexShader:
        strcat(name, ".vert");
        break;
    case EFragmentShader:
        strcat(name, ".frag");
        break;
    default:
        printf("ERROR: unknown shader file type\n");
        exit(1);
        break;
    }

    int count = -1;
    // Open the file, seek to the end to find its length
    int fd = _open(name, _O_RDONLY);
    if (fd != -1)
    {
        count = _lseek(fd, 0, SEEK_END) + 1;
        _close(fd);
    }
    return count;
}

static int readShader(char *fileName, EShaderType shaderType, char *shaderText, int size)
{
    //
    // Reads a shader from the supplied file and returns the shader in the
    // arrays passed in. Returns 1 if successful, 0 if an error occurred.
    // The parameter size is an upper limit of the amount of bytes to read.
    // It is ok for it to be too big.
    //
    FILE *fh;
    char name[100];
    int count;

    strcpy(name, fileName);

    switch (shaderType) 
    {
    case EVertexShader:
        strcat(name, ".vert");
        break;
    case EFragmentShader:
        strcat(name, ".frag");
        break;
    default:
        printf("ERROR: unknown shader file type\n");
        exit(1);
        break;
    }

    // Open the file
    fh = fopen(name, "r");
    if (!fh)
        return -1;

    // Get the shader from a file.
    fseek(fh, 0, SEEK_SET);
    count = (int) fread(shaderText, 1, size, fh);
    shaderText[count] = '\0';

    if (ferror(fh))
        count = 0;

    fclose(fh);
    return count;
}


/*public*/
int readShaderSource(char *fileName, GLchar **vertexShader, GLchar **fragmentShader)
{
    int vSize, fSize;

    //
    // Allocate memory to hold the source of our shaders.
    //
    vSize = shaderSize(fileName, EVertexShader);
    fSize = shaderSize(fileName, EFragmentShader);

    if ((vSize == -1) || (fSize == -1))
    {
        printf("Cannot determine size of the shader %s\n", fileName);
        return 0;
    }

    *vertexShader = (GLchar *) malloc(vSize);
    *fragmentShader = (GLchar *) malloc(fSize);

    //
    // Read the source code
    //
    if (!readShader(fileName, EVertexShader, *vertexShader, vSize))
    {
        printf("Cannot read the file %s.vert\n", fileName);
        return 0;
    }

    if (!readShader(fileName, EFragmentShader, *fragmentShader, fSize))
    {
        printf("Cannot read the file %s.frag\n", fileName);
        return 0;
    }

    return 1;
}

/*public*/
int installShaders(const GLchar *Vertex,
                   const GLchar *Fragment,
                   GLuint *Program)
{
    GLuint VertexShaderObject   = 0;    //顶点着色器对象
    GLuint FragmentShaderObject = 0;    //片段着色器对象
    GLint vertCompiled, fragCompiled;   //status values
    GLint linked;

    // Create a vertex shader object and a fragment shader object

    VertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    FragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    // Load source code strings into shaders

    glShaderSource(VertexShaderObject, 1, &Vertex, NULL);
    glShaderSource(FragmentShaderObject, 1, &Fragment, NULL);

    // Compile the vertex shader, and print out
    // the compiler log file.

    glCompileShader(VertexShaderObject);
    glGetShaderiv(VertexShaderObject, GL_COMPILE_STATUS, &vertCompiled);

    // Compile the fragment shader, and print out
    // the compiler log file.

    glCompileShader(FragmentShaderObject);
    glGetShaderiv(FragmentShaderObject, GL_COMPILE_STATUS, &fragCompiled);

    if (!vertCompiled || !fragCompiled)
    {
        char vertLog[1024], fragLog[1024];
        glGetShaderInfoLog(VertexShaderObject, 1024, NULL, vertLog);
        glGetShaderInfoLog(FragmentShaderObject, 1024, NULL, fragLog);

        printf("Shader compile failed!\n");
        printf("VS compile log : %s\n", vertLog);
        printf("PS compile log : %s\n", fragLog);
        return 0;
    }

    // Create a program object and attach the two compiled shaders

    *Program = glCreateProgram();
    glAttachShader(*Program, VertexShaderObject);
    glAttachShader(*Program, FragmentShaderObject);

    // Link the program object and print out the info log

    glLinkProgram(*Program);
    glGetProgramiv(*Program, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        char progLog[1024];
        glGetProgramInfoLog(*Program, 1024, NULL, progLog);

        printf("Program link failed!\n");
        printf("Program log : %s\n", progLog);
        return 0;
    }

    return 1;
}

bool init()
{
    GLchar *VertexShaderSource, *FragmentShaderSource;
    bool success = true;
    GLuint vb[2];
    GLint loc = 0;

    glGenVertexArrays(2, va);
    glGenBuffers(2, vb);
    glGenTextures(1, &tex);
    glGenTextures(1, &fboTex);
    glActiveTexture(GL_TEXTURE0);

    glBindBuffer(GL_ARRAY_BUFFER, vb[0]);
    glBufferData(GL_ARRAY_BUFFER, 72, vertexData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vb[1]);
    glBufferData(GL_ARRAY_BUFFER, 96, bltFboVertexData, GL_STATIC_DRAW);

    // Prepare texture image for sample
    int texSize = 1024;
    int blockSize = 64;
    int rowPitch = texSize * 4;
    unsigned char* texData = new unsigned char[texSize*texSize*4];
    for (int i = 0; i < texSize/blockSize; ++i)
    {
        for (int j = 0; j < texSize/blockSize; ++j)
        {
            int startX = i*blockSize;
            int startY = j*blockSize;
            bool white = (bool)(i%2 ^ j%2);

            for (int m = startX; m < startX + blockSize; ++m)
            {
                for (int n = startY; n < startY + blockSize; ++n)
                {
                    texData[n * rowPitch + m * 4 + 0] = white ? 255 : 0;
                    texData[n * rowPitch + m * 4 + 1] = white ? 255 : 0;
                    texData[n * rowPitch + m * 4 + 2] = white ? 255 : 0;
                    texData[n * rowPitch + m * 4 + 3] = 255;
                }
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, fboTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 800, 800, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    readShaderSource("point", &VertexShaderSource, &FragmentShaderSource);
    success = (bool)installShaders(VertexShaderSource, FragmentShaderSource, &prog);
    if (!success) goto Exit;

    // Install program object as part of current state
    glUseProgram(prog);

    // Set up attrib values for vertex array 0
    glBindVertexArray(va[0]);

    loc = glGetAttribLocation(prog, "model_postion");
    glBindBuffer(GL_ARRAY_BUFFER, vb[0]);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 24, (GLvoid *)0);
    glVertexAttribDivisor(loc, 0);
    glEnableVertexAttribArray(loc);

    loc = glGetAttribLocation(prog, "tex_coord_in");
    glBindBuffer(GL_ARRAY_BUFFER, vb[0]);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 24, (GLvoid *)16);
    glVertexAttribDivisor(loc, 0);
    glEnableVertexAttribArray(loc);

    // Set up attrib values for vertex array 1
    glBindVertexArray(va[1]);

    loc = glGetAttribLocation(prog, "model_postion");
    glBindBuffer(GL_ARRAY_BUFFER, vb[1]);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 24, (GLvoid *)0);
    glVertexAttribDivisor(loc, 0);
    glEnableVertexAttribArray(loc);

    loc = glGetAttribLocation(prog, "tex_coord_in");
    glBindBuffer(GL_ARRAY_BUFFER, vb[1]);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 24, (GLvoid *)16);
    glVertexAttribDivisor(loc, 0);
    glEnableVertexAttribArray(loc);

    glBindVertexArray(0);

    // Set up initial uniform values
    glUniformMatrix4fv(glGetUniformLocation(prog, "modelViewMatrix"), 1, GL_TRUE, modelViewMat);
    glUniformMatrix4fv(glGetUniformLocation(prog, "projectionMatrix"), 1, GL_TRUE, projectionMat);
    glUniform1i(glGetUniformLocation(prog, "tex"), 0);
    glUniform1f(glGetUniformLocation(prog, "pointSize"), fPtSizeUniform);
    glUniform1i(glGetUniformLocation(prog, "fboTex"), 0);
    glUniform1i(glGetUniformLocation(prog, "bEnableSprite"), 0);
    glUniform1i(glGetUniformLocation(prog, "bBltFBO"), 0);

    glUseProgram(0);

    glViewport(0,0,800,800);
    glClearColor(0.2,0.2,0.2,1.0);
    glClearDepth(0.0);
    glDisable(GL_DEPTH_TEST);

Exit:
    return success;
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
      case '1':
          pipeMode = EProgrammable;
          glutPostRedisplay();
          break;
      case '2':
          pipeMode = EFixFunction;
          glutPostRedisplay();
          break;
      case '3':
          bEnableProgramPointSize = GL_TRUE;
          glutPostRedisplay();
          break;
      case '4':
          bEnableProgramPointSize = GL_FALSE;
          glutPostRedisplay();
          break;
      case '5':
          bEnableProgramPointSprite = GL_TRUE;
          glutPostRedisplay();
          break;
      case '6':
          bEnableProgramPointSprite = GL_FALSE;
          glutPostRedisplay();
          break;
      case '7':
          bSpriteCoordOriginUpperLeft = GL_TRUE;
          glutPostRedisplay();
          break;
      case '8':
          bSpriteCoordOriginUpperLeft = GL_FALSE;
          glutPostRedisplay();
          break;
      case '9':
          bClipControlOriginUpperLeft = GL_TRUE;
          glutPostRedisplay();
          break;
      case '0':
          bClipControlOriginUpperLeft = GL_FALSE;
          glutPostRedisplay();
          break;
      case 'f':
      case 'F':
          bUsingFBO = !bUsingFBO;
          glutPostRedisplay();
          break;
      case 'q':
          exit(0);
          break;
      default:
          break;
    }
}

/******************************************************************************/
/*
/* Main
/*
/******************************************************************************/
int main( int argc, char **argv )
{
    int success = 0;
    int gl_major, gl_minor;

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(800, 800);
    window = glutCreateWindow( "Point features");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    // Initialize the "OpenGL Extension Wrangler" library
    glewInit();

    // Make sure that OpenGL 2.0 is supported by the driver
    getGlVersion(&gl_major, &gl_minor);
    printf("GL_VERSION major = %d minor = %d\n", gl_major, gl_minor);
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    printf("GL_VENDOR = %s\n", vendor);

    if (gl_major < 2)
    {
        printf("Support for OpenGL 2.0 is required for this demo...exiting\n");
        exit(1);
    }

    success = init();

    if (success)
        glutMainLoop();

    return 0;
}