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
#include <GL/glew.h>
#include <GL/glut.h>
#else
#endif


using namespace tinyxml2;
using namespace std;

//Vector that stores filenames.
vector<string> files;

//Vector that stores all the groups
vector<Group> groups;

string pathFile;

//Variables needed to the keyboard function.
float x = 0.1;
float y = 0.1;
float z = 0.1;
float angle = 0.0f;
float angle2 = 0.0f;
float zoomFactor = 1.0f;
float max_zoom = 2.5f;
float min_zoom = 0.5f;

GLdouble eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ;
GLdouble fov, near, far;

int pointcount = 1;
vector<Point> catmullPoints; 
float p[5][3] = { {-1,-1,0},{-1,1,0},{1,1,0},{0,0,0},{1,-1,0} };

float prev_y[3] = { 0, 1, 0 };
float t = 0;

vector<float> vertexB;
GLuint buffers[1];
GLuint ptr = 0;

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
    gluPerspective(fov, ratio, near, far);

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

    //nós temos 3 pontos que equivale a 9 floats
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

        //VBO
        vertexB.push_back(tokens[0]);
        vertexB.push_back(tokens[1]);
        vertexB.push_back(tokens[2]);

        primitive.addPoint(point);
    }

    MyReadFile.close();

    return primitive;
}



void buildRotMatrix(float* x, float* y, float* z, float* m) {

    m[0] = x[0]; m[1] = x[1]; m[2] = x[2]; m[3] = 0;
    m[4] = y[0]; m[5] = y[1]; m[6] = y[2]; m[7] = 0;
    m[8] = z[0]; m[9] = z[1]; m[10] = z[2]; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}


void cross(float* a, float* b, float* res) {

    res[0] = a[1] * b[2] - a[2] * b[1];
    res[1] = a[2] * b[0] - a[0] * b[2];
    res[2] = a[0] * b[1] - a[1] * b[0];
}

void normalize(float* a) {

    float l = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    a[0] = a[0] / l;
    a[1] = a[1] / l;
    a[2] = a[2] / l;
}

float length(float* v) {

    float res = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    return res;

}

void multMatrixVector(float m[4][4], float* v, float* res) {

    for (int j = 0; j < 4; ++j) {
        res[j] = 0;
        for (int k = 0; k < 4; ++k) {
            res[j] += v[k] * m[j][k];
        }
    }

}

void getCatmullRomPoint(float t, float* p0, float* p1, float* p2, float* p3, float* pos, float* deriv) {

    // catmull-rom matrix
    float m[4][4] = { {-0.5f,  1.5f, -1.5f,  0.5f},
                        { 1.0f, -2.5f,  2.0f, -0.5f},
                        {-0.5f,  0.0f,  0.5f,  0.0f},
                        { 0.0f,  1.0f,  0.0f,  0.0f} };


    for (int i = 0; i < 3; i++) {	 // i = x, y, z

        float p[4] = { p0[i], p1[i], p2[i], p3[i] };
        float a[4];

        // Compute A = M * P
        multMatrixVector(m, p, a);

        pos[i] = powf(t, 3.0) * a[0] + powf(t, 2.0) * a[1] + t * a[2] + a[3];
        // Compute pos = T * A

        // compute deriv = T' * A
        deriv[i] = 3 * powf(t, 2.0) * a[0] + 2 * t * a[1] + a[2];
    }
}


// given  global t, returns the point in the curve
void getGlobalCatmullRomPoint(float gt, float* pos, float* deriv) {

    float t = gt * pointcount; // this is the real global t
    int index = floor(t);  // which segment
    t = t - index; // where within  the segment

    // indices store the points
    int indices[4];
    indices[0] = (index + pointcount - 1) % pointcount;
    indices[1] = (indices[0] + 1) % pointcount;
    indices[2] = (indices[1] + 1) % pointcount;
    indices[3] = (indices[2] + 1) % pointcount;

    getCatmullRomPoint(t, p[indices[0]], p[indices[1]], p[indices[2]], p[indices[3]], pos, deriv);
}

void renderCatmullRomCurve() {

    float pos[3];
    float deriv[3];

    // draw curve using line segments with GL_LINE_LOOP
    glBegin(GL_LINE_LOOP);
    float i = 0;
    while (i < 100.0f) {
        getGlobalCatmullRomPoint(i / 100.0f, pos, deriv);
        glVertex3f(pos[0], pos[1], pos[2]);
        //gt += 1.0 / TESSELATION;
    }
    glEnd();

    glBegin(GL_LINES);
    for (int i = 0; i < 100; i += 1) {
        getGlobalCatmullRomPoint(i / 100.0f, pos, deriv);
        glVertex3f(pos[0], pos[1], pos[2]);
        pos[0] += deriv[0];
        pos[1] += deriv[1];
        pos[2] += deriv[2];
        glVertex3f(pos[0], pos[1], pos[2]);
    }
    glEnd();
}


void renderSceneCatmullRom(std::vector<Trans> trans, std::vector<Primitive> primitives, float time) {

    string scale = "scale";
    string translate = "translate";
    string rotate = "rotate";
    string color = "color";

    renderCatmullRomCurve();

    glPushMatrix();

    // apply transformations here

    float m[16];
    float pos[4];
    float deriv[4];
    float z[3];
    float y[3];

    t = ((float)glutGet(GLUT_ELAPSED_TIME) / 1000) / ((float)time);

    getGlobalCatmullRomPoint(t, pos, deriv);

    glTranslatef(pos[0], pos[1], pos[2]);

    float x[3] = { deriv[0], deriv[1], deriv[2] };

    normalize(x);
    cross(x, prev_y, z);
    normalize(z);
    cross(z, x, y);
    normalize(y);

    memcpy(prev_y, y, 3 * sizeof(float));

    buildRotMatrix(x, y, z, m);

    glMultMatrixf(m);

    for (int j = 0; j < trans.size(); j++) {
        Trans t = trans[j];

        if (translate.compare(t.getName()) == 0) {
            if (t.getTime() == 0) {
                glTranslatef(t.getX(), t.getY(), t.getZ());
            }
        }
        else if (scale.compare(t.getName()) == 0) {
            glScalef(t.getX(), t.getY(), t.getZ());
        }
        else if (color.compare(t.getName()) == 0) {
            glColor3f(t.getX(), t.getY(), t.getZ());
        }
        else if (rotate.compare(t.getName()) == 0) {
            if (t.getAngle() != 0 && t.getTime() == 0) {
                glRotatef(t.getAngle(), t.getX(), t.getY(), t.getZ());
            }
        }
    }
    
    for (int i = 0; i < primitives.size(); i++) {
        Primitive primitive = primitives[i];

        int nrVertices = primitive.getNrVertices();

        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        glDrawArrays(GL_TRIANGLES, ptr, nrVertices);

        ptr = ptr + nrVertices;        
    }

    glPopMatrix();
}


vector<float> calculateNewAngle(float time) {

    angle = ((float)glutGet(GLUT_ELAPSED_TIME) * 360 / 1000) / ((float)time);

    glutPostRedisplay();

    vector<float> res;

    res.push_back(angle);
    return res;
}

void applyTrans(Group g) {

    string translate = "translate";
    string rotate = "rotate";
    string scale = "scale";
    string color = "color";

    for (int j = 0; j < g.getNrTrans(); j++) {
        Trans t = g.getTrans(j);

        if (translate.compare(t.getName()) == 0) {
            if (t.getTime() == 0) {
                glTranslatef(t.getX(), t.getY(), t.getZ());
            }
        }
        else if (scale.compare(t.getName()) == 0) {
            glScalef(t.getX(), t.getY(), t.getZ());
        }
        else if (color.compare(t.getName()) == 0) {
            glColor3f(t.getX(), t.getY(), t.getZ());
        }
        else if (rotate.compare(t.getName()) == 0) {
            if (t.getAngle() != 0 && t.getTime() == 0) {
                glRotatef(t.getAngle(), t.getX(), t.getY(), t.getZ());
            }
        }
    }
    
    for (int z = 0; z < g.getNrPrimitives(); z++) {
        Primitive p = g.getPrimitives(z);

        int nrVer = p.getNrVertices();

        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        glDrawArrays(GL_TRIANGLES, ptr, nrVer);

        ptr = ptr + nrVer;

    }
}

/**
 * Function that draws all the primitives previously stored in a vector.
 */
void drawPrimitives(Group g) {

    string scale = "scale";
    string translate = "translate";
    string rotate = "rotate";
    string color = "color";
    float time = 0;

    for (int j = 0; j < g.getNrTrans(); j++) {
        Trans t = g.getTrans(j);

        if (translate.compare(t.getName()) == 0) {
            if (t.getTime() != 0) {
                time = t.getTime();

                catmullPoints = g.getPoints();

                pointcount = catmullPoints.size();

                renderSceneCatmullRom(g.getTrans(), g.getPrimitives(), time);
            }
            else if (t.getTime() == 0) {
                glTranslatef(t.getX(), t.getY(), t.getZ());
            }
        }
        else if (rotate.compare(t.getName()) == 0) {
            /*if (t.getAngle() == 0 && t.getTime() != 0) { // transformação com tempo
                time = t.getTime();

                float x = t.getX();
                float y = t.getY();
                float z = t.getZ();

                glRotatef(t.getAngle(), x, y, z);

                applyTrans(g);

                //vector<float> res = calculateNewAngle(time, x, y, z, g.getPrimitives(), g.getTrans(), g.getAngle());
                //g.setAngle(res[0]);*/
            glRotatef(t.getAngle(), x, y, z);
            /* }
            else if (t.getAngle() != 0 && t.getTime() == 0) {
                glRotatef(t.getAngle(), t.getX(), t.getY(), t.getZ());
            }*/
        }
        else if (scale.compare(t.getName()) == 0) {
            glScalef(t.getX(), t.getY(), t.getZ());
        }
        else if (color.compare(t.getName()) == 0)
        {
            glColor3f(t.getX() / 255.f, t.getY() / 255.f, t.getZ() / 255.f);
        }

    }

    for (int z = 0; z < g.getNrPrimitives(); z++) {
        Primitive p = g.getPrimitives(z);

        int nrVertices = p.getNrVertices();

        if (g.getNameFile().compare("asteroids.3d") == 0) {
            
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

                glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
                glVertexPointer(3, GL_FLOAT, 0, 0);
                glDrawArrays(GL_TRIANGLES, ptr, nrVertices);

                ptr = ptr + nrVertices;

                glPopMatrix();
            }
        }
        else {
            glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
            glVertexPointer(3, GL_FLOAT, 0, 0);
            glDrawArrays(GL_TRIANGLES, ptr, nrVertices);

            ptr = ptr + nrVertices;
        }
    }

    for (int z = 0; z < g.getNrGroups(); z++) {
        glPushMatrix();
        drawPrimitives(g.getGroup(z));
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

   gluLookAt(eyeX * zoomFactor, eyeY * zoomFactor, eyeZ * zoomFactor,
        centerX, centerY, centerZ,
        upX, upY, upZ);

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

    ptr = 0;

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
    float shift = M_PI / 2;
    switch (key_code) {
    case GLUT_KEY_LEFT:
        angle -= shift;
        break;
    case GLUT_KEY_RIGHT:
        angle += shift;
        break;
    case GLUT_KEY_UP:
        angle2 += shift;
        break;
    case GLUT_KEY_DOWN:
        angle2 -= shift;
        break;
    case GLUT_KEY_F1:
        zoomFactor = std::max(zoomFactor - 0.25f, min_zoom);
        break;
    case GLUT_KEY_F2:
        zoomFactor = std::min(zoomFactor + 0.25f, max_zoom);
        break;
    }
    glutPostRedisplay();
}

void polygonMode(unsigned char key_code, int x, int y) {
    switch (key_code){
        case '1':
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case '2':
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case '3':
            glPointSize(3.0f);
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
   // glutIdleFunc(renderScene);
    glutReshapeFunc(changeSize);

    // put here the registration of the keyboard callbacks
    glutKeyboardFunc(polygonMode);
    glutSpecialFunc(rodar);

    glewInit();  

    //  OpenGL settings
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT, GL_LINE);

    //glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_INDEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    //Create VBO
    glGenBuffers(1, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexB.size(), vertexB.data(), GL_STATIC_DRAW);

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
                
                XMLElement* transformation = element->FirstChildElement();

                while(transformation != nullptr){
                    if (scale.compare(transformation->Name()) == 0) {
                        if (transformation != nullptr) {
                            float x = atof(transformation->Attribute("x"));
                            float y = atof(transformation->Attribute("y"));
                            float z = atof(transformation->Attribute("z"));
                            Trans t = Trans("scale", x, y, z, 0, 0);
                            g.addTrans(t);
                        }
                    }
                    else if(translate.compare(transformation->Name()) == 0) {
                        if (transformation != nullptr) {

                            if (transformation->FindAttribute("time")) {
                                float time = atof(transformation->Attribute("time"));
                                string align = transformation->Attribute("align");

                                XMLElement* point = transformation->FirstChildElement("point");
                                while (point != nullptr) {
                                    float x = atof(point->Attribute("x"));
                                    float y = atof(point->Attribute("y"));
                                    float z = atof(point->Attribute("z"));

                                    Point p;
                                    p.setX(x);
                                    p.setY(y);
                                    p.setZ(z);

                                    g.addPoint(p);

                                    point = point->NextSiblingElement("point");
                                }

                                Trans t = Trans("translate", 0, 0, 0, 0, time);
                                g.addTrans(t);
                            }
                            else if (transformation->FindAttribute("x")) {
                                float x = atof(transformation->Attribute("x"));
                                float y = atof(transformation->Attribute("y"));
                                float z = atof(transformation->Attribute("z"));

                                Trans t = Trans("translate", x, y, z, 0, 0);
                                g.addTrans(t);
                            }
                        }
                    }
                    else if (rotate.compare(transformation->Name()) == 0) {
                            if (transformation != nullptr) {                              
                                float x = atof(transformation->Attribute("x"));
                                float y = atof(transformation->Attribute("y"));
                                float z = atof(transformation->Attribute("z"));

                                if (transformation->FindAttribute("time")) {
                                    float time = atof(transformation->Attribute("time"));
                                    Trans t = Trans("rotate", x, y, z, 0, time);
                                    g.addTrans(t);
                                }
                                else if (transformation->FindAttribute("angle")) {
                                    float time = atof(transformation->Attribute("angle"));
                                    Trans t = Trans("rotate", x, y, z, angle, 0);
                                    g.addTrans(t);
                                }
                            }
                    }
                    else if (color.compare(transformation->Name()) == 0) {
                            if (transformation != nullptr) {
                                float red = atof(transformation->Attribute("x"));
                                float green = atof(transformation->Attribute("y"));
                                float blue = atof(transformation->Attribute("z"));
                                Trans t = Trans("color", red, green, blue, 0, 0);
                                g.addTrans(t);
                            }
                    }

                    transformation = transformation->NextSiblingElement();
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

void parseCamera(XMLElement* camera) {
    string position = "position";
    string lookAt = "lookAt";
    string up = "up";
    string projection = "projection";

    XMLElement* element = camera->FirstChildElement();
     
    while (element != nullptr) {

        if (position.compare(element->Name()) == 0) {
            if (element != nullptr) {
                eyeX = stod(element->Attribute("x"));
                eyeY = stod(element->Attribute("y"));
                eyeZ = stod(element->Attribute("z"));
            }
        }
        else if (lookAt.compare(element->Name()) == 0) {
            if (element != nullptr) {
                centerX = stod(element->Attribute("x"));
                centerY = stod(element->Attribute("y"));
                centerZ = stod(element->Attribute("z"));
            }
        }
        else if (up.compare(element->Name()) == 0) {
            if (element != nullptr) {
                upX = stod(element->Attribute("x"));
                upY = stod(element->Attribute("y"));
                upZ = stod(element->Attribute("z"));
            }
        }
        else if (projection.compare(element->Name()) == 0) {
            if (element != nullptr) {
                fov = stod(element->Attribute("fov"));
                near = stod(element->Attribute("near"));
                far = stod(element->Attribute("far"));
            }
        }

        element = element->NextSiblingElement();
    }        
}

/**
 * Function that reads a XML file.
 * @return bool true if everything goes well. Otherwise, returns false.
 */
bool parseDocument() {

    char tmp[256];

    getcwd(tmp, 256); //tmp which contains the directory

    string path(tmp);

    int found = path.find("ENGINE"); 

    replace(path.begin(), path.end(), '\\', '/');
    path.erase(path.begin() + found, path.end());

    string fileName;

    cout << "Introduza o nome do ficheiro XML: " << endl;
    cin >> fileName;
    path = path + "Models/" + fileName + ".xml";
    //cout << path << endl;

    strcpy(tmp, path.c_str());

    XMLDocument doc;
    doc.LoadFile(tmp);

    XMLElement* world = doc.FirstChildElement();
    if (world == nullptr) {
        cout << "ERRO";
        return false; 
    }

    XMLElement* camera = world->FirstChildElement("camera");
    parseCamera(camera);

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
    vertexB.reserve(100000);

    int found = path.find("ENGINE");

    replace(path.begin(), path.end(), '\\', '/');
    path.erase(path.begin() + found, path.end());

    path = path + "Models/";

    pathFile = path;

    if (parseDocument()) {
        initGlut(argc, argv);
        /*if (!initGlut(argc, argv)) {
            cout << "3d File Invalid";
        }*/
    }
    else {
        cout << "XML File Invalid";
    }
    return 1;
}