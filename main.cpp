#include "EVWorkClient.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  EVWorkClient client;
  //client.CreateWorkItem("/home/ji/LOCAL/CAP_Exam_Data/CAP_Exam_Data/Images/020010/1.3.6.1.4.1.5962.99.1.1543942294.27346707.1427473117334.2676.0");
  //client.CreateWorkItem("/home/ji/LOCAL/CAP_Exam_Data/CAP_Exam_Data/Images/musc003/228204961");
  client.CreateWorkItem("/home/ji/LOCAL/CAP_Exam_Data/CAP_Exam_Data/Images/Postcard_0010/1.2.124.113532.80.22163.9752.20161201.111823.360185875");
  return app.exec();
}
