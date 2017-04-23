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



int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    string input_name;
    string output_name;
    int thread = 1;
    if (argc < 3) {
        cout << "Error: at least 3 parameter required.\n";
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "-t") == 0) {
        thread = atoi(argv[2]);
        if (thread <= 0) {
            cout << "Error: thread number must be positive number.\n";
            exit(EXIT_FAILURE);
        }
        if (argc < 5) {
            cout << "Error: at least 5 parameter required when setting thread number.\n";
            exit(EXIT_FAILURE);
        }
        input_name = argv[3];
        output_name = argv[4];
    } else {
        input_name = argv[1];
        output_name = argv[2];
    }

    string str = read(input_name);
    JsonParser(str);


    World* world = init(thread);
    world->fillBuffer();
    int x, y;
    {
        QImage image(camera.length_x, camera.length_y, QImage::Format_RGB32);
       
        for(int i=0; i<camera.length_x; i++){
            for(int j=0; j<camera.length_y; j++){
                Vector3 tmp = world->buffer[i][j];
                uint co = (((int)(tmp.x)) << 16) | (((int)(tmp.y)) << 8) | ((int)(tmp.z));
                image.setPixel(i, j, co);
            }
        }
        
        image.save(QString::fromStdString(output_name));
    }
}







