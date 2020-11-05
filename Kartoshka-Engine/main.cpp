
#include "Application.h"

int main()
{
    krt::Application app;
    krt::InitializationInfo init;
    init.m_Width = 1440;
    init.m_Height = 900;
    init.m_Title = "Kartofelnoe Pyure";

    app.Run(init);
}