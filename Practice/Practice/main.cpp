#include "Practice.hpp"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    Practice p;
    p.show();
    
    return a.exec();
}
