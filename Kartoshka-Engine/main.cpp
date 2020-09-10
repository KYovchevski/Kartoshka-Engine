
#include "Application.h"


int main()
{
    krt::Application app;
    krt::InitializationInfo init;
    init.m_Width = 800;
    init.m_Height = 800;
    init.m_Title = "Kartofelnoe Pyure";


    app.Run(init);
}