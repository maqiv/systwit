#include <stdio.h>
#include <syslog.h>

void __attribute__ ((constructor)) setup(void) {
  syslog(LOG_INFO, "syslogger plugin loaded.");
}

void __attribute__ ((destructor)) dest(void) {
  syslog(LOG_INFO, "syslogger plugin unloaded.");
}

void write_content(char* content) {
  syslog(LOG_INFO, "plugin syslogger -> CPU Percentage: %s", content);
}
