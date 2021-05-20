#pragma once

#include "appfwk/DAQModule.hpp"
#include "ers/Issue.hpp"

#include <string>

#define TLVL_ENTER_EXIT_METHODS 10
#define TLVL_GENERATION 11
#define TLVL_CANDIDATE 15

namespace dunedaq {

  ERS_DECLARE_ISSUE(trigger, InvalidConfiguration, "An invalid configuration object was received", ERS_EMPTY)

  ERS_DECLARE_ISSUE_BASE(dunetrigger,
			 ProgressUpdate,
			 appfwk::GeneralDAQModuleIssue,
			 message,
			 ((std::string)name),
			 ((std::string)message))

  ERS_DECLARE_ISSUE_BASE(dunetrigger,
			 InvalidQueueFatalError,
			 appfwk::GeneralDAQModuleIssue,
			 "The " << queueType << " queue was not successfully created.",
			 ((std::string)name),
			 ((std::string)queueType))

  ERS_DECLARE_ISSUE_BASE(trigger,
			 SignalTypeError,
			 appfwk::GeneralDAQModuleIssue,
			 "Signal type " << signal_type << " invalid.",
			 ((std::string)name),
			 ((uint32_t)signal_type))

  ERS_DECLARE_ISSUE_BASE(trigger,
			 InvalidQueueFatalError,
			 appfwk::GeneralDAQModuleIssue,
			 "The " << queueType << " queue was not successfully created.",
			 ((std::string)name),
			 ((std::string)queueType))

} // namespace dunedaq   

