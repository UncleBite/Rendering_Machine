#include "catch.hpp"
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



#include "geometry.hpp"

TEST_CASE( "Test Vector3 ", "[geometry]" ) {
    
    Vector3 a(2,3,4), b, c(1, 0, 0);
    
    REQUIRE(a.x == Approx(2));
    REQUIRE(a.y == Approx(3));
    REQUIRE(a.z == Approx(4));
    
    REQUIRE(b.x == Approx(0));
    REQUIRE(b.y == Approx(0));
    REQUIRE(b.z == Approx(0));
    
    REQUIRE(c.x == Approx(1));
    REQUIRE(c.y == Approx(0));
    REQUIRE(c.z == Approx(0));
    
    REQUIRE(a.dot(c) == Approx(2));
    
    // test '+' operation
    Vector3 d = a + c;
    REQUIRE(d.x == Approx(3));
    REQUIRE(d.y == Approx(3));
    REQUIRE(d.z == Approx(4));
    
    // test '-' operation
    Vector3 e = a - c;
    REQUIRE(e.x == Approx(1));
    REQUIRE(e.y == Approx(3));
    REQUIRE(e.z == Approx(4));
    
    // test '*' operation
    Vector3 f = a*2;
    REQUIRE(f.x == Approx(4));
    REQUIRE(f.y == Approx(6));
    REQUIRE(f.z == Approx(8));
    
    // test 'normalize' operation
    a.normalize();
    REQUIRE(a.dot(a) == Approx(1));
    
}

TEST_CASE( "Test Ray:", "[geometry]" ) {
    Ray ray;
    Ray ray2(Vector3(1, 1, 1), Vector3(1, 0, 0));
    
    REQUIRE(ray.origin.x == Approx(0));
    REQUIRE(ray.origin.y == Approx(0));
    REQUIRE(ray.origin.z == Approx(0));
    
    REQUIRE(ray.direction.x == Approx(0));
    REQUIRE(ray.direction.y == Approx(0));
    REQUIRE(ray.direction.z == Approx(1));
    
    REQUIRE(ray2.origin.x == Approx(1));
    REQUIRE(ray2.origin.y == Approx(1));
    REQUIRE(ray2.origin.z == Approx(1));
    
    REQUIRE(ray2.direction.dot(ray2.direction) == Approx(1));
    Vector3 t = ray2.getPoint(2);
    REQUIRE(t.x == Approx(3));
    REQUIRE(t.y == Approx(1));
    REQUIRE(t.z == Approx(1));
}


TEST_CASE( "Test Sphere:", "[geometry]" ) {
    Sphere obj(Vector3(100, 150, 200), Vector3(0, 0, 3), 0.5, 1);
    REQUIRE(obj.color.x == Approx(100));
    REQUIRE(obj.color.y == Approx(150));
    REQUIRE(obj.color.z == Approx(200));
    
    REQUIRE(obj.center.x == Approx(0));
    REQUIRE(obj.center.y == Approx(0));
    REQUIRE(obj.center.z == Approx(3));
    
    REQUIRE(obj.lambert == Approx(0.5));
    REQUIRE(obj.radius == Approx(1));
    
    double distance;
    Ray ray(Vector3(0, 0, 0), Vector3(0, 0, 1));
    REQUIRE(obj.isIntersected(ray, distance) == Approx(true));
    REQUIRE(distance == Approx(2));
    
    Ray ray1(Vector3(0, -0.5, 1), Vector3(0, 0, 1));
    REQUIRE(obj.isIntersected(ray1, distance) == Approx(true));
    REQUIRE(distance == Approx(2 - sqrt(0.75)));
}

TEST_CASE( "Test Plane:", "[geometry]" ) {
    Plane obj(Vector3(100, 150, 200), Vector3(0, 1, 0), 0.5, Vector3(0, -1, 0));
    REQUIRE(obj.color.x == Approx(100));
    REQUIRE(obj.color.y == Approx(150));
    REQUIRE(obj.color.z == Approx(200));
    
    REQUIRE(obj.center.x == Approx(0));
    REQUIRE(obj.center.y == Approx(1));
    REQUIRE(obj.center.z == Approx(0));
    
    REQUIRE(obj.lambert == Approx(0.5));
    
    REQUIRE(obj.normal.x == Approx(0));
    REQUIRE(obj.normal.y == Approx(-1));
    REQUIRE(obj.normal.z == Approx(0));
    
    double distance;
    Ray ray(Vector3(0, 0, 0), Vector3(0, 1, 1));
    REQUIRE(obj.isIntersected(ray, distance) == Approx(true));
    REQUIRE(distance == Approx(sqrt(2)));
    
}

TEST_CASE( "Test World:", "[geometry]" ) {
    World* world_me = new World(1024, 1024, Vector3(0, 0, -10),
                                Vector3(0, 0, 0), 0.01, 0.01, 1);
    Plane *plane3 = new Plane(Vector3(255, 255, 255), Vector3(0, 5, 0), 1, Vector3(0, -1, 0));
    Sphere *sphere3 = new Sphere(Vector3(255, 0, 0), Vector3(0, 0, 5), 1, 1);
    PointLight *l3 = new PointLight(Vector3(5, -5, 0), 1);
    world_me->addObject(plane3);
    world_me->addObject(sphere3);
    world_me->addLight(l3);
    
    Ray ray(Vector3(500, -500, 0), Vector3(0, 0, 500) - Vector3(500, -500, 0));
    
    double distance;
    Plane *plane2 = new Plane(Vector3(255, 255, 255), Vector3(0, 500, 0), 1, Vector3(0, -1, 0));
    REQUIRE(plane2->isIntersected(ray, distance) == Approx(true));
    REQUIRE(distance == Approx(1000*sqrt(3)));
    
    Vector3 intersectedPoint = ray.getPoint(distance);
    REQUIRE(intersectedPoint.x == Approx(-500));
    REQUIRE(intersectedPoint.y == Approx(500));
    REQUIRE(intersectedPoint.z == Approx(1000));
    
    Ray ray1(Vector3(0, 0, -1000), Vector3(-500, 500, 1000) - Vector3(0, 0, -1000));
    Plane plane1(Vector3(255, 255, 255), Vector3(), 1, Vector3(0, 0, 1));
    REQUIRE(plane1.isIntersected(ray1, distance) == Approx(true));
    Vector3 intersectedPoint1 = ray1.getPoint(distance);
    REQUIRE(intersectedPoint1.x == Approx(-250));
    REQUIRE(intersectedPoint1.y == Approx(250));
    REQUIRE(intersectedPoint1.z == Approx(0));
    world_me->fillBuffer();
    Vector3 co = world_me->buffer[512 + (int)intersectedPoint1.x][512 + (int)intersectedPoint1.y];
    REQUIRE(co.x == Approx(0));
    REQUIRE(co.y == Approx(0));
    REQUIRE(co.z == Approx(0));
}

TEST_CASE( "Test JsonParser (good):", "[geometry]" ) {
    string str = read("/vagrant/tests/scene0.json");
    JsonParser(str);
    REQUIRE(camera.center.x == Approx(0));
    REQUIRE(camera.center.y == Approx(0));
    REQUIRE(camera.center.z == Approx(0));
    
    REQUIRE(camera.normal.x == Approx(0));
    REQUIRE(camera.normal.y == Approx(0));
    REQUIRE(camera.normal.z == Approx(1));
    
    REQUIRE(camera.focus == Approx(10));
    REQUIRE(camera.length_x == Approx(256));
    REQUIRE(camera.length_y == Approx(256));
    REQUIRE(camera.resolution_x == Approx(0.01));
    REQUIRE(camera.resolution_x == Approx(0.01));
    
    REQUIRE(lights.size() == Approx(1));
    REQUIRE(lights[0].intensity == Approx(1));
    REQUIRE(lights[0].location.x == Approx(5));
    REQUIRE(lights[0].location.y == Approx(-5));
    REQUIRE(lights[0].location.z == Approx(0));
    
    REQUIRE(objects.size() == Approx(1));
    REQUIRE(objects[0].lambert == Approx(1));
    REQUIRE(objects[0].radius == Approx(1));
    REQUIRE(objects[0].center.x == Approx(0));
    REQUIRE(objects[0].center.y == Approx(0));
    REQUIRE(objects[0].center.z == Approx(5));
    REQUIRE(objects[0].color.x == Approx(255));
    REQUIRE(objects[0].color.y == Approx(0));
    REQUIRE(objects[0].color.z == Approx(0));
    
}






