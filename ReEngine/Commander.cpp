#include "Commander.h"

void Commander::IssueCommand(RenderCommand Command)
{
	CommandQueueWrite->push_back(Command);
}


