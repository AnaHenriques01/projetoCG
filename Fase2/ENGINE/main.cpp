﻿#define  _USE_MATH_DEFINES
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
#include "Group.h"
#include "Trans.h"

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

//Vector that stores all the gropus
vector<Group> groups;

string pathFile;

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
Primitive readFile(string file) {

    ifstream MyReadFile(file);

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

    MyReadFile.close();

    return primitive;
}


/**
 * Function that draws all the primitives previously stored in a vector.
 */
void drawPrimitives(Group groups) {

    string scale = "scale";
    string translate = "translate";
    string rotate = "rotate";
    string color = "color";

    for (int j = 0; j < groups.getNrTrans(); j++) {
        Trans t = groups.getTrans(j);

        if (translate.compare(t.getName()) == 0) {
            glTranslatef(t.getX(), t.getY(), t.getZ());
        }
        else if (scale.compare(t.getName()) == 0) {
            glScalef(t.getX(), t.getY(), t.getZ());
        }
        else if (rotate.compare(t.getName()) == 0) {
            glRotated(t.getAngle(), t.getX(), t.getY(), t.getZ());
        }
        else if (color.compare(t.getName()) == 0)
        {
            glColor3f(t.getX() / 255.f, t.getY() / 255.f, t.getZ() / 255.f);
        }
    }

    for (int z = 0; z < groups.getNrPrimitives(); z++) {
        Primitive p = groups.getPrimitives(z);

        if (groups.getNameFile().compare("asteroids.3d") == 0) {
            
            int arrx[180];
            int arry[180];

            srand(1);

            for (int i = 0; i < 100; i++) {
                arrx[i] = rand() % 18 + (-5);
            }
            for (int i = 0; i < 100; i++) {
                arry[i] = rand() % 18 + (-5);
            }
            
            for (int i = 0; i < 180; i++) {
                 
                glPushMatrix();
                glRotated(6 * i, 0, 1, 0);
                glTranslated(80 + arrx[i], arry[i], 0);

                glBegin(GL_TRIANGLES);
                for (int c = 0; c < p.getNrVertices(); c++) {
                    Point point = p.getPoint(c);

                    glVertex3f(point.getX(), point.getY(), point.getZ());
                }
                glEnd();

                glPopMatrix();
            }
        }
        else {
            glBegin(GL_TRIANGLES);
            for (int c = 0; c < p.getNrVertices(); c++) {
                Point point = p.getPoint(c);

                glVertex3f(point.getX(), point.getY(), point.getZ());
            }
            glEnd();
        }
    }

    for (int z = 0; z < groups.getNrGroups(); z++) {
        glPushMatrix();
        drawPrimitives(groups.getGroup(z));
        glPopMatrix();
    }


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

    /*
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
    glEnd();*/


    //DRAW POINTS
    for (int i = 0; i < groups.size(); i++) {
        glPushMatrix();
        drawPrimitives(groups[i]);
        glPopMatrix();
    }

    // End of frame
    glutSwapBuffers();
}

/**
 * Function that reacts to events.
 * @param key_code
 * @param x
 * @param y
 */
void rodar(int key_code, int x, int y) {
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

void polygonMode(unsigned char key_code, int x, int y) {
    switch (key_code) {
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
    glutSpecialFunc(rodar);

    //  OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // enter GLUT's main cycle
    glutMainLoop();
    return true;
}

Group parseGroup(XMLElement* group, int father) {
    string models = "models";
    string scale = "scale";
    string translate = "translate";
    string rotate = "rotate";
    string grupo = "group";
    string color = "color";
    string transform = "transform";
    string camera = "camera";
    Group g;

    do {
        g = Group();
        XMLElement* element = group->FirstChildElement();

        while (element != nullptr) {

            if (models.compare(element->Name()) == 0) {

                XMLElement* file = element->FirstChildElement("model");

                while (file != nullptr) {

                    const char* strfile = file->Attribute("file");
                    string namefile = strfile;
                    string path = pathFile + namefile;
                    Primitive primitive = readFile(path);

                    g.addPrimitives(primitive);
                    g.setNameFile(namefile);

                    file = file->NextSiblingElement();
                }
            }
            else if (transform.compare(element->Name()) == 0){
                    
                XMLElement* file = element->FirstChildElement("transform");
                    
                while (file != nullptr) {

                    if (scale.compare(element->Name()) == 0) {
                        float x, y, z;

                        element->QueryFloatAttribute("x", &x);
                        element->QueryFloatAttribute("y", &y);
                        element->QueryFloatAttribute("z", &z);

                        Trans t = Trans("scale", x, y, z, 0);

                        g.addTrans(t);
                    }
                    else if (translate.compare(element->Name()) == 0) {
                        float x, y, z;

                        element->QueryFloatAttribute("x", &x);
                        element->QueryFloatAttribute("y", &y);
                        element->QueryFloatAttribute("z", &z);

                        Trans t = Trans("translate", x, y, z, 0);

                        g.addTrans(t);
                    }
                    else if (rotate.compare(element->Name()) == 0) {
                        float x, y, z, angle;

                        element->QueryFloatAttribute("angle", &angle);
                        element->QueryFloatAttribute("x", &x);
                        element->QueryFloatAttribute("y", &y);
                        element->QueryFloatAttribute("z", &z);

                        Trans t = Trans("rotate", x, y, z, angle);

                        g.addTrans(t);
                    }
                    else if (color.compare(element->Name()) == 0) {
                        float red, green, blue;

                        element->QueryFloatAttribute("x", &red);
                        element->QueryFloatAttribute("y", &green);
                        element->QueryFloatAttribute("z", &blue);

                        Trans t = Trans("color", red, green, blue, 0);

                        g.addTrans(t);
                    }

                    file = file->NextSiblingElement();
                    if (element == NULL && father == 1) return g;
                }
            }            
            else if (grupo.compare(element->Name()) == 0) {
                Group gr = parseGroup(element, 1);
                g.addGroups(gr);
            }

            element = element->NextSiblingElement();

            if (element == NULL && father == 1) return g;
        }

        if (father == 0) groups.push_back(g);
        group = group->NextSiblingElement();

    } while (group != nullptr);

    return g;
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

    path = path + "Models/solarSystem.xml";
    cout << path << endl;

    strcpy(tmp, path.c_str());

    XMLDocument doc;
    doc.LoadFile(tmp);

    XMLElement* world = doc.FirstChildElement();
    if (world == nullptr) {
        cout << "ERRO";
        return false; //in case of error
    }

    XMLElement* group = world->FirstChildElement("group");
    parseGroup(group, 0);

    return true;
}

/**
 * Main Function.
 * @param argc size of array.
 * @param argv array with arguments.
 * @return int
 */
int main(int argc, char** argv) {

    bool res = true;

    char tmp[256];

    getcwd(tmp, 256);

    string path(tmp);

    int found = path.find("ENGINE");

    replace(path.begin(), path.end(), '\\', '/');
    path.erase(path.begin() + found, path.end());

    path = path + "Models/";

    pathFile = path;

    if (parseDocument()) {
        if (!initGlut(argc, argv)) {
            cout << "3d File Invalid";
        }
    }
    else {
        cout << "XML File Invalid";
    }
    return 1;
}