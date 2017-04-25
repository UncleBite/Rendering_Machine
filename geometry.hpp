#include <QtGui>
#include <QWidget>
#include <QApplication>
#include <math.h>
#include <iostream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <string>
#include <fstream>  
#include <sstream>
#include <vector>  
#include <exception>
#include <thread>

using namespace std;

class Vector3{
public:
    double x;
    double y;
    double z;
    Vector3():x(0),y(0),z(0){
    }
    Vector3(double a,double b,double c):x(a),y(b),z(c){
    }
    double sumOfSquare(){
        return x*x+y*y+z*z;
    }
    void normalize(){
        double base=sqrt(sumOfSquare());
        x=x/base;
        y=y/base;
        z=z/base;
    }
    double dot(Vector3 a){
        return x*a.x+y*a.y+z*a.z;
    }
    Vector3 cross(Vector3 a){
        return Vector3(y*a.z-z*a.y,
            z*a.x-x*a.z,
            x*a.y-y*a.x);
    }
    Vector3 operator +(Vector3 a){
        return Vector3(x+a.x, y+a.y, z+a.z);
    }
    Vector3 operator -(Vector3 a){
        return Vector3(x-a.x, y-a.y, z-a.z);
    }
    Vector3 operator *(double a){
        return Vector3(x*a,y*a,z*a);
    }
};


class Ray{
public:
    Vector3 origin;
    Vector3 direction;

    Ray():origin(Vector3()),direction(Vector3(0,0,1)){
    }
    Ray(Vector3 o, Vector3 d):origin(o){
        d.normalize();
        direction=d;
    }
    Vector3 getPoint(double t){
        return origin+direction*t;
    }
};


class Object{
public:
    Vector3 color; // color in rgb format
    Vector3 center; // center of the object
    double lambert;
    Object():color(Vector3()),center(Vector3()),lambert(1) {
    }
    Object(Vector3 color, Vector3 center, double lambert) {
    	this->color = color;
    	this->center = center;
    	this->lambert = lambert;
    }

    virtual Vector3 getNormal(Vector3 point)=0;
    
   
     
    virtual bool isIntersected(Ray ray, double& distance)=0;
};

class Sphere : public Object{
public:
    double radius;
    Sphere():Object(){
        radius=1;
    }
    Sphere(Vector3 color, Vector3 center, double lambert, double radius) {
		this->color = color;
    	this->center = center;
    	this->lambert = lambert;
        this->radius = radius;
    }
    Vector3 getNormal(Vector3 point);
    bool isIntersected(Ray ray, double& distance);
};

Vector3 Sphere::getNormal(Vector3 point){
    return (point-center);
}

bool Sphere::isIntersected(Ray ray, double& distance){
    double threshold=0.0005;


    double a=ray.direction.sumOfSquare();
    double b=2*(ray.direction.dot(ray.origin-center));
    double c=ray.origin.sumOfSquare()+center.sumOfSquare()-2*ray.origin.dot(center)-radius*radius;
    double delta=b*b-4*a*c;
    if(delta<0) {
        return false;
    }
        double t1, t2;
        t1=(-b-sqrt(delta))/(2*a);
        t2=(-b+sqrt(delta))/(2*a);
        if(t2<=threshold&&t1<=threshold) {
            return false;
        }
        if(t2>threshold&&t1<=threshold) {
            distance=t2;
            return true;
        }
        if(t2>threshold&&t1>threshold) {
            distance=t1;
            return true;
        }
    
    return true;
}


class Plane : public Object{
public:
    Vector3 normal;
    Plane():Object(){
        normal.z = 1;
    }
    Plane(Vector3 color, Vector3 center, double lambert, Vector3 normal) {
        this->color = color;
        this->center = center;
        this->lambert = lambert;
        this->normal = normal;
    }
    Vector3 getNormal(Vector3 point);
    bool isIntersected(Ray ray, double& distance);
};


Vector3 Plane::getNormal(Vector3 point){
    return normal;
}

bool Plane::isIntersected(Ray ray, double& distance){
    double threshold=0.0005;
    distance = ((center-ray.origin).dot(normal))/(ray.direction.dot(normal));
 

    return (distance >0);
}

class PointLight {
public:
    Vector3 position;
    double intensity;
    PointLight() {
        position=Vector3();
        intensity = 1;
    }
    PointLight(Vector3 p, double i) {
        position=p;
        intensity = i;
    }
    Vector3 getL(Vector3 intersectPoint);
    Vector3 getPos();
};

 
//GetL
Vector3 PointLight::getL(Vector3 intersectPoint){
    Vector3 L = position - intersectPoint;
    L.normalize();
    return L;
}

//Getpos
Vector3 PointLight::getPos(){
    return position;
}

#define MAX_PIXELS 2560

class World{
public:
    int count = 0;
    vector<Object*> objects;
    vector<PointLight*> lights;
    int numObjects, numLights;
    //Buffer to store colors.
    Vector3 buffer[MAX_PIXELS][MAX_PIXELS];
    int length_x;
    int length_y;
    int thread_num;

    double resolution_x;
    double resolution_y;

    //Clear color.
    Vector3 background;

    Vector3 camera;
    Vector3 screen_center;

    World(int length_x, int length_y, Vector3 camera, 
        Vector3 screen_center, double resolution_x, 
        double resolution_y, int thread_num) {
    	this->length_x = length_x;
    	this->length_y = length_y;
        for(int i=0; i<length_x; i++)
        {for(int j=0; j<length_y; j++)
        {buffer[i][j]=Vector3();}
        }
//        objects = new Object*[MAX_OBJECTS];
//        lights = new PointLight*[MAX_LIGHTS];
        this->camera = camera;
        this->screen_center = screen_center;
        this->resolution_x = resolution_x;
        this->resolution_y = resolution_y;
        this->thread_num = thread_num;
        numObjects = 0;
        numObjects = 0;
    }
    ~World(){
        for (int i = 0; i < objects.size(); i++)
            delete objects[i];
        for (int i = 0; i < lights.size(); i++)
            delete lights[i];
    }
    void addLight(PointLight* l){
        lights.push_back(l);
        numLights++;
    }
    void addObject(Object* o){
        objects.push_back(o);
        numObjects++;
    }

    Object* getIntersectPoint(Ray ray, double& distance);
    Vector3 startTrace(Ray ray, int depth);
    void fillBuffer();
};

Object* World::getIntersectPoint(Ray ray, double& distance){
    double threshold = 0.00001;
    double nearestDistance=99999;
    Object* nearestObject=0;
    
    for(int i=0; i < numObjects; i++){
        double d;
        if (objects[i]->isIntersected(ray,d)){
            if(d < nearestDistance - threshold){
                nearestDistance=d;
                nearestObject=objects[i];
            }
        }
    }
    distance = nearestDistance;
    return nearestObject;
}

struct IndexWorld {
    World* world;
    int index;
};

void fillBufferThread(IndexWorld iWorld) {
    int begin_x = iWorld.index*(iWorld.world->length_x/iWorld.world->thread_num);
    int end_x = begin_x + iWorld.world->length_x/iWorld.world->thread_num;
    if (iWorld.index == iWorld.world->thread_num - 1) {
        end_x = iWorld.world->length_x;
    }
    for(int i = begin_x; i < end_x; i++){
        for(int j = 0;j < iWorld.world->length_y; j++){
            Ray ray(iWorld.world->camera, 
                Vector3((-iWorld.world->length_x/2 + i)*iWorld.world->resolution_x + 
                    iWorld.world->screen_center.x, 
                    (-iWorld.world->length_y/2 + j)*iWorld.world->resolution_y + 
                    iWorld.world->screen_center.y, iWorld.world->screen_center.z) - 
                    iWorld.world->camera);
            iWorld.world->buffer[i][j] = iWorld.world->startTrace(ray,0);
        }
    }
}

Vector3 World::startTrace(Ray ray,int depth){
    count++;
    Object* nearestObject;
    double nearestDistance;
    nearestObject = getIntersectPoint(ray, nearestDistance);
    if(nearestObject!=nullptr){
        Vector3 color;
        double max_intensity = 0;
        Vector3 intersectPoint = ray.getPoint(nearestDistance);
        for(int i=0; i < numLights; i++){
            Ray rayFromLight(lights[i]->getPos(), intersectPoint - lights[i]->getPos());
            double tmp;
            Object* tmpObj = getIntersectPoint(rayFromLight,tmp);
            double length = sqrt((intersectPoint - lights[i]->getPos()).sumOfSquare());
            if (tmpObj == nearestObject) {
                if(tmp <= length + 0.001 && tmp >= length - 0.001) {
                    Vector3 L = lights[i]->getL(intersectPoint);
                    L.normalize();
                    Vector3 V =  nearestObject->getNormal(intersectPoint);
                    V.normalize();
                    double scale = L.dot(V);
                    if (lights[i]->intensity >= max_intensity)
                        max_intensity = lights[i]->intensity;
                    if (scale > 0) {
                        color = color + nearestObject->color*(scale*nearestObject->lambert);
                    }
                }
            }
        }
        return color*max_intensity;
    }
        return background;
    
}

void World::fillBuffer() {
    time_t start,end;
    start=time(NULL);
    thread* threads = new thread[thread_num];

    IndexWorld iWorld;
    iWorld.world = this;

    for (int i = 0; i < thread_num; i++) {
        iWorld.index = i;
        threads[i] = thread(fillBufferThread, iWorld);
    }

    for (int i = 0; i < thread_num; i++) {
        threads[i].join();
    }

    end=time(NULL);
//    cout<<"total time:"<<difftime(end,start)<<" "<<endl;
}




string read(string filename) {
    std::ifstream t(filename.c_str());
    if (t.fail()) {
        cout << "Error: file not found.\n";
        exit(EXIT_FAILURE);
    }  
    std::stringstream buffer;  
    buffer << t.rdbuf();  
    std::string contents(buffer.str());
    return contents;
}

struct Camera {
    Vector3 center;
    Vector3 normal;
    double focus;
    double resolution_x;
    double resolution_y;
    double length_x;
    double length_y; 
} camera;

struct Light {
    double intensity;
    Vector3 location;
};

vector<Light> lights;

struct Object_Data {
    Vector3 center;
    Vector3 color;
    double lambert;
    double radius;
    Vector3 normal;
    string type;
};

vector<Object_Data> objects;

bool isColorValid(Vector3 color) {
    return (color.x>=0 && color.x<=255 && color.y>=0 && color.y<=255 && color.z>=0 && color.z<=255);
}

void JsonParser(string str) {
    QString json = QString::fromStdString(str);
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError) {
        if (jsonDocument.isObject()) {
            QVariantMap result = jsonDocument.toVariant().toMap();
            if (result["camera"].isNull()) {
                cout << "Error: json parameter error.\n";
                exit(EXIT_FAILURE);
            }
            QVariantMap _camera = result["camera"].toMap();
            if (_camera["center"].isNull() ||
                _camera["focus"].isNull() ||
                _camera["normal"].isNull() ||
                _camera["resolution"].isNull() ||
                _camera["size"].isNull()) {
                cout << "Error: json parameter error.\n";
                exit(EXIT_FAILURE);
            }
         if(_camera["normal"].toMap()["z"].toString()=="}")
         {
             cout << "Error: json parameter error.\n";
             exit(EXIT_FAILURE);
         }
            
            
            if (_camera["center"].toMap()["x"].isNull() ||
                _camera["center"].toMap()["y"].isNull() ||
                _camera["center"].toMap()["z"].isNull() ||
                _camera["normal"].toMap()["x"].isNull() ||
                _camera["normal"].toMap()["y"].isNull() ||
                _camera["normal"].toMap()["z"].isNull()
                ) {
                cout << "Error: json parameter error.\n";
                exit(EXIT_FAILURE);
            }
            camera.center.x = _camera["center"].toMap()["x"].toDouble();
            camera.center.y = _camera["center"].toMap()["y"].toDouble();
            camera.center.z = _camera["center"].toMap()["z"].toDouble();
            camera.focus = _camera["focus"].toDouble();
            camera.normal.x = _camera["normal"].toMap()["x"].toDouble();
            camera.normal.y = _camera["normal"].toMap()["y"].toDouble();
            camera.normal.z = _camera["normal"].toMap()["z"].toDouble();
            if (_camera["resolution"].toList().size() < 2 ||
                _camera["size"].toList().size() < 2) {
                cout << "Error: json parameter error.\n";
                exit(EXIT_FAILURE);
            }
            camera.resolution_x = _camera["resolution"].toList().at(0).toDouble();
            camera.resolution_y = _camera["resolution"].toList().at(1).toDouble();
            camera.length_x = _camera["size"].toList().at(0).toDouble();
            camera.length_y = _camera["size"].toList().at(1).toDouble();
            if (camera.length_x <= 0 || camera.length_y <= 0 ||
                camera.resolution_x <= 0 || camera.resolution_y <= 0) {
                cout << "Error: json parameter error.\n";
                exit(EXIT_FAILURE);
            }
            if (result["lights"].isNull()) {
                cout << "Error: json parameter error.\n";
                exit(EXIT_FAILURE);
            }
            foreach(QVariant tmp, result["lights"].toList())
            {    Light l;
                if (tmp.toMap()["intensity"].isNull() ||
                    tmp.toMap()["location"].isNull() ||
                    tmp.toMap()["location"].toMap()["x"].isNull() ||
                    tmp.toMap()["location"].toMap()["y"].isNull() ||
                    tmp.toMap()["location"].toMap()["z"].isNull()) {
                    cout << "Error: json parameter error.\n";
                    exit(EXIT_FAILURE);}
                l.intensity = tmp.toMap()["intensity"].toDouble();
                if (l.intensity < 0 || l.intensity > 1) {
                    cout << "Error: json parameter error.\n";
                    exit(EXIT_FAILURE);}
                l.location.x = tmp.toMap()["location"].toMap()["x"].toDouble();
                l.location.y = tmp.toMap()["location"].toMap()["y"].toDouble();
                l.location.z = tmp.toMap()["location"].toMap()["z"].toDouble();
                lights.push_back(l);
            }
            if (result["objects"].isNull()) {
                cout << "Error: json parameter error.\n";
                exit(EXIT_FAILURE);
            }
            foreach(QVariant tmp, result["objects"].toList())
            {        Object_Data o;
                if (tmp.toMap()["center"].isNull() ||
                    tmp.toMap()["center"].toMap()["x"].isNull() ||
                    tmp.toMap()["center"].toMap()["y"].isNull() ||
                    tmp.toMap()["center"].toMap()["z"].isNull() ||
                    tmp.toMap()["color"].isNull() ||
                    tmp.toMap()["color"].toMap()["r"].isNull() ||
                    tmp.toMap()["color"].toMap()["g"].isNull() ||
                    tmp.toMap()["color"].toMap()["b"].isNull() ||
                    tmp.toMap()["lambert"].isNull() ||
                    tmp.toMap()["type"].isNull()) {
                    cout << "Error: json parameter error.\n";
                    exit(EXIT_FAILURE);}
                o.center.x = tmp.toMap()["center"].toMap()["x"].toDouble();
                o.center.y = tmp.toMap()["center"].toMap()["y"].toDouble();
                o.center.z = tmp.toMap()["center"].toMap()["z"].toDouble();
                o.color.x = tmp.toMap()["color"].toMap()["r"].toDouble();
                o.color.y = tmp.toMap()["color"].toMap()["g"].toDouble();
                o.color.z = tmp.toMap()["color"].toMap()["b"].toDouble();
                o.lambert = tmp.toMap()["lambert"].toDouble();
                if (o.lambert < 0 || o.lambert > 1) {
                    cout << "Error: json parameter error.\n";
                    exit(EXIT_FAILURE);
                }
                o.type = tmp.toMap()["type"].toString().toStdString();
                if (o.type == "sphere") {
                    if (tmp.toMap()["radius"].isNull()) {
                        cout << "Error: json parameter error.\n";
                        exit(EXIT_FAILURE);
                    }
                    o.radius = tmp.toMap()["radius"].toDouble();
                    if (o.radius < 0) {
                        cout << "Error: json parameter error.\n";
                        exit(EXIT_FAILURE);
                    }
                }
                if (o.type == "plane") {
                    if (tmp.toMap()["normal"].isNull() ||
                        tmp.toMap()["normal"].toMap()["x"].isNull() ||
                        tmp.toMap()["normal"].toMap()["y"].isNull() ||
                        tmp.toMap()["normal"].toMap()["z"].isNull()) {
                        cout << "Error: json parameter error.\n";
                        exit(EXIT_FAILURE);
                    }
                    o.normal.x = tmp.toMap()["normal"].toMap()["x"].toDouble();
                    o.normal.y = tmp.toMap()["normal"].toMap()["y"].toDouble();
                    o.normal.z = tmp.toMap()["normal"].toMap()["z"].toDouble();
                }
                if (!isColorValid(o.color)) {
                    cout << "Error: json parameter error.\n";
                    exit(EXIT_FAILURE);
                }
                objects.push_back(o);

            }
        }
    } else {
cout << "Error: json parameter error.\n";
       exit(EXIT_FAILURE);
    }
}

World* init(int thread_num) {
    World* world;
    double scale = 1;
    world = new World(camera.length_x, camera.length_y,
        (camera.center - camera.normal*camera.focus)*scale,
        camera.center*scale, camera.resolution_x,
        camera.resolution_y, thread_num);
    Object* object;
    PointLight* light;
    for (int i = 0; i < lights.size(); i++) {
        light=new PointLight(lights[i].location*scale, lights[i].intensity);
        world->addLight(light);
    }
    for (int i = 0; i < objects.size(); i++) {
        if (objects[i].type == "sphere") {
            object = new Sphere(objects[i].color, objects[i].center*scale,
                objects[i].lambert, objects[i].radius*scale);
            world->addObject(object);
        }
        if (objects[i].type == "plane") {
            object = new Plane(objects[i].color, objects[i].center*scale,
                objects[i].lambert, objects[i].normal);
            world->addObject(object);
        }
    }
    return world;
}








