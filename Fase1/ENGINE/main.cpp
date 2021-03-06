#define  _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "tinyxml2.h"
#include "Primitive.h"
#include "Point.h"
#include <cstring>
#include <string>
#include <algorithm>

#ifdef __APPLE__
#include <unistd.h>
#elif _WIN32
#include <direct.h>
#include <io.h>
#else
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#elif _WIN32
#include <GL/glut.h>
#else
#endif

using namespace tinyxml2;
using namespace std;

//Vector that stores filenames.
vector<string> files;

//Vector that stores all Primitives.
vector<Primitive> primitives;

//Variables needed to the keyboard function.
float x = 0.1;
float y = 0.1;
float z = 0.1;
float angle = 0.0f;
float angle2 = 0.0f;

/**
 * Function that redimensionates a window.
 * @param w width of the window.
 * @param h height of the window.
 */
void changeSize(int w, int h) {

    // Prevent a divide by zero, when window is too short
    // (you cant make a window with zero width).
    if (h == 0)
        h = 1;

    // compute window's aspect ratio
    float ratio = w * 1.0 / h;

    // Set the projection matrix as current
    glMatrixMode(GL_PROJECTION);
    // Load Identity Matrix
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set perspective
    gluPerspective(45.0f, ratio, 1.0f, 1000.0f);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}


/**
 * Function that reads a file, given the filename.
 * @param file filename.
 * @return bool true if everything goes well. Otherwise, returns false.
 */
bool readFile(string file) {

    ifstream MyReadFile(file);

    if(!MyReadFile){
        return false;
    }

    string myText;

    getline(MyReadFile, myText);
    int vertices = stoi(myText);

    Primitive primitive;

    for (int i = 0; i < vertices; i++) {
        // Vector of string to save tokens
        vector<float> tokens;

        // stringstream class check1
        getline(MyReadFile, myText);
        stringstream check1(myText);

        string intermediate;

        // Tokenizing w.r.t. space ' '
        while (getline(check1, intermediate, ',')) {
            tokens.push_back(stof(intermediate));
        }

        Point point;
        point.setX(tokens[0]);
        point.setY(tokens[1]);
        point.setZ(tokens[2]);

        primitive.addPoint(point);
    }

    primitives.push_back(primitive);

    MyReadFile.close();

    return true;
}


/**
 * Function that draws all the primitives previously stored in a vector.
 */
void drawPrimitives() {

    glBegin(GL_TRIANGLES);
    //glBegin(GL_LINES);

    float corzinhas = 1.0f;
    float corzinhas2 = 0.0f;

    for (int i = 0; i < primitives.size(); i++) 
        for (int j = 0; j < primitives[i].getNrVertices(); j++) {
            corzinhas = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            corzinhas2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            glColor3f(corzinhas,corzinhas2,corzinhas);

            Point point = primitives[i].getPoint(j);

            glVertex3f(point.getX(), point.getY(), point.getZ());
        }

    glEnd();

}

/**
 * Function that creates a scene.
 */
void renderScene(void) {

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();

    gluLookAt(5.0f, 5.0f, 5.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f);

    glTranslatef(x, 0.0, z);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glRotatef(angle2, 1.0, 0.0, 0.0);

    //AXIS
    glBegin(GL_LINES);
    // X axis in red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-100.0f, 0.0f, 0.0f);
    glVertex3f(100.0f, 0.0f, 0.0f);
    // Y Axis in green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -100.0f, 0.0f);
    glVertex3f(0.0f, 100.0f, 0.0f);
    // Z Axis in blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -100.0f);
    glVertex3f(0.0f, 0.0f, 100.0f);
    glEnd();

    //DRAW POINTS
    drawPrimitives();

    // End of frame
    glutSwapBuffers();
}

/**
 * Function that reacts to events.
 * @param key_code
 * @param x
 * @param y
 */
void rotate(int key_code, int x, int y) {
    switch (key_code) {
    case GLUT_KEY_LEFT:
        angle -= 2;
        break;
    case GLUT_KEY_RIGHT:
        angle += 2;
        break;
    case GLUT_KEY_UP:
        angle2 += 2;
        break;
    case GLUT_KEY_DOWN:
        angle2 -= 2;
        break;
    }
    glutPostRedisplay();
}

void polygonMode(unsigned char key_code, int x, int y){
    switch(key_code) {
        case '1':
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case '2':
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case '3':
            glPointSize(4.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
        default:
            break;
    }
    glutPostRedisplay();
}


 /**
  * Function that inits glut.
  * @param argc size of array.
  * @param argv array with arguments.
  * @return bool true if everything goes well. Otherwise, returns false.
  */
bool initGlut(int argc, char** argv) {

     bool res;

     char tmp[256];

     getcwd(tmp, 256); //tmp which contains the directory

     string path(tmp);

     int found = path.find("ENGINE"); // finds generator's localization

     replace(path.begin(), path.end(), '\\', '/');
     path.erase(path.begin() + found, path.end());

     path = path + "Models/";


     for (int i = 0; i < files.size(); i++) {
         string file = path + files[i];
         res = readFile(file);
         if(!res) return false;
     }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("TP-CG");

    // Required callback registry
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);

    // put here the registration of the keyboard callbacks
    glutKeyboardFunc(polygonMode);
    glutSpecialFunc(rotate);  

    //  OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // enter GLUT's main cycle
    glutMainLoop();
    return true;
}

/**
 * Function that reads a XML file.
 * @return bool true if everything goes well. Otherwise, returns false.
 */
bool parseDocument() {

    char tmp[256];

    getcwd(tmp, 256); //tmp which contains the directory

    string path(tmp);

    int found = path.find("ENGINE"); // finds generator's localization

    replace(path.begin(), path.end(), '\\', '/');
    path.erase(path.begin() + found, path.end());

    path = path + "Models/model.xml";

    strcpy(tmp, path.c_str());

    XMLDocument doc;
    doc.LoadFile(tmp);

    XMLNode* scene = doc.FirstChild();
    if (scene == nullptr) {
        cout << "ERRO";
        return false; //in case of error
    }

    XMLElement* file = scene->FirstChildElement("model");

    while (file != nullptr) {
        const char* strfile;

        strfile = file->Attribute("file");
        string namefile = strfile;
        files.push_back(namefile);

        file = file->NextSiblingElement();
    }
    return true;
}

/**
 * Main Function.
 * @param argc size of array.
 * @param argv array with arguments.
 * @return int 
 */
int main(int argc, char** argv) {

    if (parseDocument()) {
        if(!initGlut(argc, argv)){
            cout << "3d File Invalid";
        }
    } else{
        cout << "XML File Invalid";
    }
    return 1;
}


