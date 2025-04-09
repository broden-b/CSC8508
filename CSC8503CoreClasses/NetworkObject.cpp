#include "NetworkObject.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

NetworkObject::NetworkObject(GameObject& o, int id) : object(o)	{
	deltaErrors = 0;
	fullErrors  = 0;
	networkID   = id;
}

NetworkObject::~NetworkObject()	{
}

bool NetworkObject::ReadPacket(GamePacket& p) {
	if(p.type== Delta_State)
		return ReadDeltaPacket((DeltaPacket&)p);
	if (p.type == Full_State)
		return ReadFullPacket((FullPacket&)p);

	return false; //this isn't a packet we care about!
}

bool NetworkObject::WritePacket(GamePacket** p, bool deltaFrame, int stateID) {
	if (deltaFrame) {
		if (!WriteDeltaPacket(p, stateID)) {
			return WriteFullPacket(p);
		}
	}
	return WriteFullPacket(p);
}
//Client objects recieve these packets
bool NetworkObject::ReadDeltaPacket(DeltaPacket &p) {
	if (p.fullID != lastFullState.stateID) {
		
		return false;
	}
	UpdateStateHistory(p.fullID);
	Vector3 fullPos = lastFullState.position;
	Quaternion fullOri = lastFullState.orientation;

	fullPos.x += p.pos[0];
	fullPos.y += p.pos[1];
	fullPos.z += p.pos[2];

	fullOri.x += ((float)p.orientation[0]) / 127.0f;
	fullOri.y += ((float)p.orientation[1]) / 127.0f;
	fullOri.z += ((float)p.orientation[2]) / 127.0f;
	fullOri.w += ((float)p.orientation[3]) / 127.0f;

	object.GetTransform().SetPosition(fullPos);
	object.GetTransform().SetOrientation(fullOri);
	return true;
}

bool NetworkObject::ReadFullPacket(FullPacket &p) {
	if (p.fullState.stateID < lastFullState.stateID) {
		return false;
	}
	lastFullState = p.fullState;

	object.GetTransform().SetPosition(lastFullState.position);
	object.GetTransform().SetOrientation(lastFullState.orientation);
	
	stateHistory.emplace_back(lastFullState);
	return true;
}

bool NetworkObject::WriteDeltaPacket(GamePacket**p, int stateID) {

	DeltaPacket* deltaPacket = new DeltaPacket();
	NetworkState networkState;
	if (!GetNetworkState(stateID, networkState)) {
		return false;
	}
	deltaPacket->fullID = stateID;
	deltaPacket->objectID = networkID;	
	Vector3 currentPos = object.GetTransform().GetPosition();
	Quaternion currentOri = object.GetTransform().GetOrientation();

	currentPos -= networkState.position;
	currentOri -= networkState.orientation;

	deltaPacket->pos[0] = (char)(currentPos.x);
	deltaPacket->pos[1] = (char)(currentPos.y);
	deltaPacket->pos[2] = (char)(currentPos.z);

	deltaPacket->orientation[0] = (char)(currentOri.x * 127.0f);
	deltaPacket->orientation[1] = (char)(currentOri.y * 127.0f);
	deltaPacket->orientation[2] = (char)(currentOri.z * 127.0f);
	deltaPacket->orientation[3] = (char)(currentOri.w * 127.0f);

	*p = deltaPacket;
	return true;
}

bool NetworkObject::WriteFullPacket(GamePacket**p) {

	FullPacket* fullPacket = new FullPacket();
	fullPacket->objectID = networkID;
	fullPacket->fullState.position = object.GetTransform().GetPosition();
	fullPacket->fullState.orientation = object.GetTransform().GetOrientation();
	fullPacket->fullState.stateID = lastFullState.stateID++;
	*p = fullPacket;
	return true;
}

NetworkState& NetworkObject::GetLatestNetworkState() {
	return lastFullState;
}

bool NetworkObject::GetNetworkState(int stateID, NetworkState& state) {
	for (auto i = stateHistory.begin(); i < stateHistory.end(); ++i) {
		if ((*i).stateID == stateID) {
			state = *i;
			return true;
		}
	}
	return false;
}

void NetworkObject::UpdateStateHistory(int minID) {

	for (auto i = stateHistory.begin(); i <  stateHistory.end();) {
		if ((*i).stateID < minID) {
			i = stateHistory.erase(i);
		}
		else {
			++i;
		}
	}
}