#pragma once

#include "ReEngineExport.h"
#include "RenderCommand.h"
#include <vector>
#include <memory>
#include <mutex>


class ENGINE_API Commander
{
private:

	std::shared_ptr<std::vector<RenderCommand>> CommandQueueWrite;
	std::shared_ptr<std::vector<RenderCommand>> CommandQueueRead;
	mutable std::mutex CommandQueueMutex;


public:
	Commander() = default;
	Commander(const Commander&) = delete;
	Commander& operator=(const Commander&) = delete;
	Commander(Commander&&) = delete;
	Commander& operator=(Commander&&) = delete;

	void Init(int SizeOfCommandQueue) {
		CommandQueueWrite = std::make_shared<std::vector<RenderCommand>>();
		CommandQueueRead = std::make_shared<std::vector<RenderCommand>>();

		CommandQueueRead->reserve(SizeOfCommandQueue);
		CommandQueueWrite->reserve(SizeOfCommandQueue);
	}

	void IssueCommand(RenderCommand command);

	const std::vector<RenderCommand>& ConsumeRenderCommands()
	{
		std::scoped_lock<std::mutex> lock(CommandQueueMutex);
		// Swap so Write becomes Read
		CommandQueueRead.swap(CommandQueueWrite);

		// Clear the new Write queue (previously Read) so it's ready for new commands
		CommandQueueWrite->clear();

		// Return a reference to the vector that lives inside this class (in ReEngine's Heap)
		return *CommandQueueRead;
	}
};
