#include <QApplication>
#include <thread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

int main(int argc,char argv[])
{
    string input_name;
    string output_name;
    string str = read(input_name);
    if (argc < 3){
        cout << "Error: at least 3 parameter required."<<endl;
        exit(EXIT_FAILURE);
    }
    if (argv[1] == "-t")
    {
       
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
}


void JsonParser(string str)
{
    
}



