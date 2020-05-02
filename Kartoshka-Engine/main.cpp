
#include "Application.h"


int main()
{
    krt::Application app;
    krt::InitializationInfo init;
    init.m_Width = 1280;
    init.m_Height = 720;
    init.m_Title = "Kartofelnoe Pyure";

    app.Run(init);
}