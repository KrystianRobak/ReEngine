#include "Commander.h"

void Commander::IssueCommand(RenderCommand Command)
{
	std::scoped_lock<std::mutex> lock(CommandQueueMutex);
	CommandQueueWrite->push_back(Command);
}

