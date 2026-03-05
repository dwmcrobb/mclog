#include "DwmMclogLogger.hh"

int main(int argc, char *argv[])
{
  using Dwm::Mclog::logger;
  
  logger.Open("user");
  MCLOG(LOG_INFO, "Hello!");
  logger.Close();
  return 0;
}
