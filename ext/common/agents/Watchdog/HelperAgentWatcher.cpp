/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2010-2013 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  See LICENSE file for license information.
 */

class HelperAgentWatcher: public AgentWatcher {
protected:
	string helperAgentFilename;
	VariantMap params, report;
	string requestSocketFilename;
	string messageSocketFilename;
	
	virtual const char *name() const {
		return "Phusion Passenger helper agent";
	}
	
	virtual string getExeFilename() const {
		return helperAgentFilename;
	}
	
	virtual void execProgram() const {
		if (hasEnvOption("PASSENGER_RUN_HELPER_AGENT_IN_VALGRIND", false)) {
			execlp("valgrind", "valgrind", "--dsymutil=yes",
				helperAgentFilename.c_str(), (char *) 0);
		} else {
			execl(helperAgentFilename.c_str(), "PassengerHelperAgent", (char *) 0);
		}
	}
	
	virtual void sendStartupArguments(pid_t pid, FileDescriptor &fd) {
		VariantMap options = agentsOptions;
		params.addTo(options);
		options.writeToFd(fd);
	}
	
	virtual bool processStartupInfo(pid_t pid, FileDescriptor &fd, const vector<string> &args) {
		if (args[0] == "initialized") {
			requestSocketFilename = args[1];
			messageSocketFilename = args[2];
			return true;
		} else {
			return false;
		}
	}
	
public:
	HelperAgentWatcher(const ResourceLocator &resourceLocator) {
		helperAgentFilename = resourceLocator.getAgentsDir() + "/PassengerHelperAgent";

		report
			.set("request_socket_filename",
				agentsOptions.get("request_socket_filename", false,
					generation->getPath() + "/request"))
			.set("request_socket_password",
				agentsOptions.get("request_socket_password", false,
					randomGenerator->generateAsciiString(REQUEST_SOCKET_PASSWORD_SIZE)))
			.set("helper_agent_admin_socket_address",
				agentsOptions.get("helper_agent_admin_socket_address", false,
					"unix:" + generation->getPath() + "/socket"))
			.set("helper_agent_exit_password",
				agentsOptions.get("helper_agent_exit_password", false,
					randomGenerator->generateAsciiString(MESSAGE_SERVER_MAX_PASSWORD_SIZE)));

		params = report;
		params
			.set("logging_agent_address", loggingAgentAddress)
			.set("logging_agent_password", loggingAgentPassword);
	}
	
	virtual void reportAgentsInformation(VariantMap &report) {
		this->report.addTo(report);
	}
};
