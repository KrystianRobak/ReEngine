#pragma once

#include "ReEngineExport.h"
#include "RenderCommand.h"
#include <vector>
#include <memory>


class ENGINE_API Commander
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

	const std::vector<RenderCommand>& ConsumeRenderCommands()
	{
		// Swap so Write becomes Read
		CommandQueueRead.swap(CommandQueueWrite);

		// Clear the new Write queue (previously Read) so it's ready for new commands
		CommandQueueWrite->clear();

		// Return a reference to the vector that lives inside this class (in ReEngine's Heap)
		return *CommandQueueRead;
	}
};

