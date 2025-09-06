#pragma once

#include "RenderCommand.h"
#include <vector>
#include <mutex>

class Commander
{
private:

	std::vector<RenderCommand> CommandQueue;
	std::mutex RenderQueueMutex;

public:


	virtual ~Commander() = default;
	virtual void IssueCommand(RenderCommand command) = 0;
	virtual std::vector<RenderCommand> ConsumeRenderCommands() = 0;
};

