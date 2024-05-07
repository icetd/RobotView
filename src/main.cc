//#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#include <iostream>
#include "Core/Application.h"
#include "Core/log.h"
#include "Ui/MainLayer.h"
#include "Ui/NodeLayer.h"
#include "Ui/SceneLayer.h"
#include "Ui/PlanManagerLayer.h"
#include "Ui/VideoLayer.h"
#include "Ui/PowerManagerLayer.h"
#include "Ui/LogLayer.h"

#define VLD 0

#if VLD
#include <vld.h>
#endif

int main(int argc, char **argv)
{
    initLogger(INFO);
    Application *app = new Application("RobotView", 1920, 1080);
    app->PushLayer<MainLayer>();
    app->PushLayer<NodeLayer>();
    app->PushLayer<VideoLayer>();
    app->PushLayer<SceneLayer>();
    app->PushLayer<PlanManagerLayer>();
    app->PushLayer<PowerManagerLayer>();
    app->PushLayer<LogLayer>();
    app->Run();

    delete app;
    return 0;
}