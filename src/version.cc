#include "version.hh"

#define str(s) #s

std::string softeq::ble::getVersion()
{
    return str(PROJECT_VERSION_GENERATION) "." str(PROJECT_VERSION_MAJOR) "." str(PROJECT_VERSION_MINOR);
}
