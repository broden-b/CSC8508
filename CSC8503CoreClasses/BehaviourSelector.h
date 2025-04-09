#pragma once
#include "BehaviourNodeWithChildren.h"

class BehaviourSelector : public BehaviourNodeWithChildren {
public:
	BehaviourSelector(const std::string& nodeName) : BehaviourNodeWithChildren(nodeName) {}
	~BehaviourSelector() {}
	BehaviourState Execute(float dt) override {
		//std::cout << "Executing selector " << name << "\n";
		for (auto& i : childNodes) {
			BehaviourState nodeState = i->Execute(dt);
			switch (nodeState) {
			case Failure:	continue; // Continue to next child if this one fails
			case Success:   // Fall through
			case Ongoing:   // Fall through
			{
				currentState = nodeState;
				return currentState;
			}
			}
		}
		return Failure;
	}
};