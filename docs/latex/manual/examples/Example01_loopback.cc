#include "DwmMclogLogger.hh"

int main(int argc, char *argv[])
{
  using Dwm::Mclog::Facility, Dwm::Mclog::Severity, Dwm::Mclog::logger;
  
  logger.Open(Facility::user);
  MCLOG(Severity::info, "Hello!");
  logger.Close();
  return 0;
}
