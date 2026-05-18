// debug_agent_log.h — NDJSON lines for Cursor debug session (copy from Serial to .cursor/debug-db9393.log)
#ifndef DEBUG_AGENT_LOG_H
#define DEBUG_AGENT_LOG_H

#include <Arduino.h>
#include <stdio.h>

static inline void agentLog(const char* hypothesisId, const char* location, const char* message,
                             float a, float b, float c, int d) {
    char buf[300];
    snprintf(buf, sizeof(buf),
             "{\"sessionId\":\"db9393\",\"hypothesisId\":\"%s\",\"location\":\"%s\",\"message\":\"%s\","
             "\"data\":{\"a\":%.5f,\"b\":%.5f,\"c\":%.5f,\"d\":%d},\"timestamp\":%lu}",
             hypothesisId, location, message, a, b, c, d, (unsigned long)millis());
    Serial.println(buf);
}

#endif
