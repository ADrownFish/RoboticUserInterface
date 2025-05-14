#include "SevnceTest.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    UdpSender sender;
    return a.exec();
}
