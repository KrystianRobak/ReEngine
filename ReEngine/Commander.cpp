#include "Commander.h"

void Commander::IssueCommand(RenderCommand Command)
{
	CommandQueue->push_back(Command);
}


