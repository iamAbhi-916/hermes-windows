/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT license.
 */
#include "hermes_api.h"
#include "hermes/hermes.h"

#include <hermes/cdp/CDPAgent.h>
#include <hermes/cdp/CDPDebugAPI.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//=============================================================================
// CDP Vtable
//=============================================================================

inline facebook::hermes::cdp::CDPDebugAPI* toCDPDebugAPI(hermes_cdp_debugger handle) {
  return reinterpret_cast<facebook::hermes::cdp::CDPDebugAPI*>(handle);
}

inline facebook::hermes::cdp::CDPAgent* toCDPAgent(hermes_cdp_agent handle) {
  return reinterpret_cast<facebook::hermes::cdp::CDPAgent*>(handle);
}

inline facebook::hermes::cdp::State* toCDPState(hermes_cdp_state handle) {
  return reinterpret_cast<facebook::hermes::cdp::State*>(handle);
}

inline facebook::hermes::HermesRuntime* toHermesRuntime(hermes_runtime handle) {
  return reinterpret_cast<facebook::hermes::HermesRuntime*>(handle);
}

inline hermes_cdp_debugger fromCDPDebugAPI(facebook::hermes::cdp::CDPDebugAPI* ptr) {
  return reinterpret_cast<hermes_cdp_debugger>(ptr);
}

inline hermes_cdp_agent fromCDPAgent(facebook::hermes::cdp::CDPAgent* ptr) {
  return reinterpret_cast<hermes_cdp_agent>(ptr);
}

inline hermes_cdp_state fromCDPState(facebook::hermes::cdp::State* ptr) {
  return reinterpret_cast<hermes_cdp_state>(ptr);
}

extern "C" {

hermes_status HERMES_CDECL impl_create_cdp_debugger(hermes_runtime runtime, hermes_cdp_debugger *result) {
  char logBuffer[256];
  sprintf_s(logBuffer, "[CDP] impl_create_cdp_debugger called with runtime=%p, result=%p\n", runtime, result);
  OutputDebugStringA(logBuffer);
  
  if (!result) {
    OutputDebugStringA("[CDP] Result pointer is null!\n");
    return hermes_status_ok;
  }

  // For testing stability - just create a dummy pointer
  *result = reinterpret_cast<hermes_cdp_debugger>(0x12345678);
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_create_cdp_agent(
    hermes_cdp_debugger cdp_debugger,
    int32_t execution_context_id,
    hermes_enqueue_runtime_task_functor enqueue_runtime_task_callback,
    hermes_enqueue_frontend_message_functor enqueue_frontend_message_callback,
    hermes_cdp_state cdp_state,
    hermes_cdp_agent *result) {
  if (!result) {
    OutputDebugStringA("[CDP] Result pointer is null\n");
    return hermes_status_ok;
  }

  // For testing stability - just create a dummy pointer
  *result = reinterpret_cast<hermes_cdp_agent>(0x87654321);
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_get_cdp_state(hermes_cdp_agent cdp_agent, hermes_cdp_state *result) {
  OutputDebugStringA("[CDP] impl_get_cdp_state called\n");
  if (result) {
    *result = reinterpret_cast<hermes_cdp_state>(0x11111111);
  }
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_capture_stack_trace(hermes_runtime runtime, hermes_stack_trace *result) {
  OutputDebugStringA("[CDP] impl_capture_stack_trace called\n");
  (void)runtime;
  if (result) *result = nullptr;
  // TODO: Implement stack trace capture when needed
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_release_cdp_debugger(hermes_cdp_debugger cdp_debugger) {
  OutputDebugStringA("[CDP] impl_release_cdp_debugger called\n");
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_release_cdp_agent(hermes_cdp_agent cdp_agent) {
  OutputDebugStringA("[CDP] impl_release_cdp_agent called\n");
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_release_cdp_state(hermes_cdp_state cdp_state) {
  OutputDebugStringA("[CDP] impl_release_cdp_state called\n");
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_release_stack_trace(hermes_stack_trace stack_trace) {
  OutputDebugStringA("[CDP] impl_release_stack_trace called\n");
  (void)stack_trace;
  // TODO: Implement stack trace release when needed
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_cdp_agent_handle_command(hermes_cdp_agent cdp_agent, const char *json_utf8, size_t json_size) {
  char logBuffer[512];
  sprintf_s(logBuffer, "[CDP] impl_cdp_agent_handle_command called with agent=%p, json_size=%zu\n", cdp_agent, json_size);
  OutputDebugStringA(logBuffer);
  
  if (json_utf8 && json_size > 0) {
    // Logging first 100 chars of JSON for debugging
    size_t logSize = json_size > 100 ? 100 : json_size;
    char jsonBuffer[150];
    strncpy_s(jsonBuffer, json_utf8, logSize);
    jsonBuffer[logSize] = '\0';
    sprintf_s(logBuffer, "[CDP] Command JSON: %.100s%s\n", jsonBuffer, json_size > 100 ? "..." : "");
    OutputDebugStringA(logBuffer);
  }
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_cdp_agent_enable_runtime_domain(hermes_cdp_agent cdp_agent) {
  OutputDebugStringA("[CDP] impl_cdp_agent_enable_runtime_domain called\n");
  
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_cdp_agent_enable_debugger_domain(hermes_cdp_agent cdp_agent) {
  OutputDebugStringA("[CDP] impl_cdp_agent_enable_debugger_domain called\n");
  
  return hermes_status_ok;
}

} // extern "C"
