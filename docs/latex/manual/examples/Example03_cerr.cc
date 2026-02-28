#include "DwmMclogOstreamSink.hh"
#include "DwmMclogLogger.hh"

int main(int argc, char *argv[])
{
  using Dwm::Mclog::Facility, Dwm::Mclog::Severity, Dwm::Mclog::OstreamSink;
  
  OstreamSink  cerrSink(std::cerr);
  Dwm::Mclog::logger.Open(Facility::user, {&cerrSink});
  MCLOG(Severity::info, "Hello!");
  Dwm::Mclog::logger.Close();
  
  return 0;
}
