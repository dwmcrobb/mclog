#include "DwmMclogLogger.hh"

int main(int argc, char *argv[])
{
  using Dwm::Mclog::Facility, Dwm::Mclog::Severity;
  
  Dwm::Mclog::logger.Open(Facility::user);
  MCLOG(Severity::info, "Hello!");
  Dwm::Mclog::logger.Close();
  return 0;
}
