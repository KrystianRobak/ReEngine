#pragma once

#include "RenderCommand.h"
#include <vector>
#include <mutex>

class Commander
{
private:

	std::shared_ptr<std::vector<RenderCommand>> CommandQueueWrite;
	std::shared_ptr<std::vector<RenderCommand>> CommandQueueRead;

public:

	void Init(int SizeOfCommandQueue) {
		CommandQueueWrite = std::make_shared<std::vector<RenderCommand>>();
		CommandQueueRead = std::make_shared<std::vector<RenderCommand>>();

		CommandQueueRead->reserve(SizeOfCommandQueue);
		CommandQueueWrite->reserve(SizeOfCommandQueue);
	}

	Commander operator=(const Commander& newCommander)
	{
		if (this == &newCommander)
		{
			return *this;
		}



		this->CommandQueueRead = newCommander.CommandQueueRead;
		this->CommandQueueWrite = newCommander.CommandQueueWrite;


		return *this;

	}
	void IssueCommand(RenderCommand command);

	std::vector<RenderCommand> ConsumeRenderCommands()
	{
		CommandQueueRead.swap(this->CommandQueueWrite);
		CommandQueueRead->clear();

		return *CommandQueueWrite;
	}
};

