#include "entry_point.h"
#include "application.h"

int main(int argc, char** argv) {
  auto app = ck::CreateApplication();
  app->Run();
  delete app;
}
