#include "DwmMclogLogFile.hh"
#include "DwmMclogLogger.hh"

int main(int argc, char *argv[])
{
  using Dwm::Mclog::Facility, Dwm::Mclog::Severity, Dwm::Mclog::LogFile;
  
  LogFile  logFile("./mylog");
  logFile.Open();
  Dwm::Mclog::logger.Open(Facility::user, {&logFile});
  MCLOG(Severity::info, "Hello!");
  Dwm::Mclog::logger.Close();
  logFile.Close();
  
  return 0;
}
