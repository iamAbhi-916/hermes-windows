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
// Helper functions to convert between C handles and C++ objects
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
  OutputDebugStringA("[CDP] Creating debugger...\n");
  
  if (!result || !runtime) {
    OutputDebugStringA("[CDP] ERROR: Null pointer\n");
    return hermes_status_error;
  }

  auto* hermesRuntime = toHermesRuntime(runtime);
  auto cdpDebugAPI = facebook::hermes::cdp::CDPDebugAPI::create(*hermesRuntime);
  *result = fromCDPDebugAPI(cdpDebugAPI.release());
  
  char logBuffer[128];
  sprintf_s(logBuffer, "[CDP] Debugger created: %p\n", *result);
  OutputDebugStringA(logBuffer);
  
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_create_cdp_agent(
    hermes_cdp_debugger cdp_debugger,
    int32_t execution_context_id,
    hermes_enqueue_runtime_task_functor enqueue_runtime_task_callback,
    hermes_enqueue_frontend_message_functor enqueue_frontend_message_callback,
    hermes_cdp_state cdp_state,
    hermes_cdp_agent *result) {
  
  char logBuffer[256];
  sprintf_s(logBuffer, "[CDP] Creating agent for context %d...\n", execution_context_id);
  OutputDebugStringA(logBuffer);
  
  if (!result || !cdp_debugger) {
    OutputDebugStringA("[CDP] ERROR: Null pointer\n");
    return hermes_status_error;
  }

  auto* debugAPI = toCDPDebugAPI(cdp_debugger);
  
  // Wrap C callback as C++ lambda for enqueueing runtime tasks
  // RuntimeTask is std::function<void(HermesRuntime&)>
  // The C callback will provide the hermes_runtime when it executes
  auto enqueueRuntimeTask = [enqueue_runtime_task_callback](facebook::hermes::debugger::RuntimeTask task) {
    // Store the C++ task on heap
    auto* taskPtr = new facebook::hermes::debugger::RuntimeTask(std::move(task));
    
    // Create hermes_runtime_task_functor that will be called with hermes_runtime
    hermes_runtime_task_functor taskFunctor;
    taskFunctor.data = taskPtr;
    taskFunctor.invoke = [](void* data, hermes_runtime runtime) {
      // Get the C++ task
      auto* t = static_cast<facebook::hermes::debugger::RuntimeTask*>(data);
      // Convert hermes_runtime to HermesRuntime&
      auto* hermesRuntime = toHermesRuntime(runtime);
      // Execute the task with the runtime
      (*t)(*hermesRuntime);
    };
    taskFunctor.release = [](void* data) {
      delete static_cast<facebook::hermes::debugger::RuntimeTask*>(data);
    };
    
    // Call the enqueue callback
    enqueue_runtime_task_callback.invoke(enqueue_runtime_task_callback.data, taskFunctor);
  };
  
  // Wrap C callback as C++ lambda for sending messages to DevTools
  auto outboundMessageFunc = [enqueue_frontend_message_callback](const std::string& message) {
    enqueue_frontend_message_callback.invoke(
      enqueue_frontend_message_callback.data, 
      message.c_str(), 
      message.size()
    );
  };
  
  // Convert cdp_state if provided
  facebook::hermes::cdp::State state;
  if (cdp_state) {
    state = std::move(*toCDPState(cdp_state));
  }
  
  // Create the CDP agent
  auto agent = facebook::hermes::cdp::CDPAgent::create(
    execution_context_id,
    *debugAPI,
    std::move(enqueueRuntimeTask),
    std::move(outboundMessageFunc),
    std::move(state)
  );
  
  *result = fromCDPAgent(agent.release());
  
  sprintf_s(logBuffer, "[CDP] Agent created: %p\n", *result);
  OutputDebugStringA(logBuffer);
  
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_get_cdp_state(hermes_cdp_agent cdp_agent, hermes_cdp_state *result) {
  OutputDebugStringA("[CDP] Getting state...\n");
  
  if (!result) {
    return hermes_status_error;
  }
  
  if (!cdp_agent) {
    *result = nullptr;
    return hermes_status_ok;
  }
  
  auto* agent = toCDPAgent(cdp_agent);
  auto state = agent->getState();
  *result = fromCDPState(new facebook::hermes::cdp::State(std::move(state)));
  
  OutputDebugStringA("[CDP] State retrieved\n");
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_capture_stack_trace(hermes_runtime runtime, hermes_stack_trace *result) {
  OutputDebugStringA("[CDP] Capturing stack trace...\n");
  
  if (result) {
    *result = nullptr; // TODO: Implement when needed
  }
  
  return hermes_status_ok;
}

// Release CDP Debugger
hermes_status HERMES_CDECL impl_release_cdp_debugger(hermes_cdp_debugger cdp_debugger) {
  OutputDebugStringA("[CDP] Releasing debugger...\n");
  if (cdp_debugger) {
    delete toCDPDebugAPI(cdp_debugger);
  }
  return hermes_status_ok;
}

//Release CDP Agent
hermes_status HERMES_CDECL impl_release_cdp_agent(hermes_cdp_agent cdp_agent) {
  OutputDebugStringA("[CDP] Releasing agent...\n");
  if (cdp_agent) {
    delete toCDPAgent(cdp_agent);
  }
  return hermes_status_ok;
}

//Release CDP State
hermes_status HERMES_CDECL impl_release_cdp_state(hermes_cdp_state cdp_state) {
  OutputDebugStringA("[CDP] Releasing state...\n");
  if (cdp_state) {
    delete toCDPState(cdp_state);
  }
  return hermes_status_ok;
}

// Release Stack Trace
hermes_status HERMES_CDECL impl_release_stack_trace(hermes_stack_trace stack_trace) {
  OutputDebugStringA("[CDP] Releasing stack trace...\n");
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_cdp_agent_handle_command(hermes_cdp_agent cdp_agent, const char *json_utf8, size_t json_size) {
  char logBuffer[512];
  
  // Log command for debugging
  if (json_utf8 && json_size > 0 && json_size < 200) {
    sprintf_s(logBuffer, "[CDP] Command: %.*s\n", (int)json_size, json_utf8);
    OutputDebugStringA(logBuffer);
  } else if (json_size > 0) {
    sprintf_s(logBuffer, "[CDP] Command: %.*s... (%zu bytes)\n", 100, json_utf8, json_size);
    OutputDebugStringA(logBuffer);
  }
  
  if (!cdp_agent || !json_utf8 || json_size == 0) {
    OutputDebugStringA("[CDP] ERROR: Invalid input\n");
    return hermes_status_error;
  }
  
  auto* agent = toCDPAgent(cdp_agent);
  std::string json(json_utf8, json_size);
  agent->handleCommand(std::move(json));
  
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_cdp_agent_enable_runtime_domain(hermes_cdp_agent cdp_agent) {
  OutputDebugStringA("[CDP] Enabling Runtime domain...\n");
  
  if (!cdp_agent) {
    return hermes_status_error;
  }
  
  auto* agent = toCDPAgent(cdp_agent);
  agent->enableRuntimeDomain();
  
  OutputDebugStringA("[CDP] Runtime domain enabled\n");
  return hermes_status_ok;
}

hermes_status HERMES_CDECL impl_cdp_agent_enable_debugger_domain(hermes_cdp_agent cdp_agent) {
  OutputDebugStringA("[CDP] Enabling Debugger domain...\n");
  
  if (!cdp_agent) {
    return hermes_status_error;
  }
  
  auto* agent = toCDPAgent(cdp_agent);
  agent->enableDebuggerDomain();
  
  OutputDebugStringA("[CDP] Debugger domain enabled - scripts should now be visible!\n");
  return hermes_status_ok;
}

} // extern "C"
