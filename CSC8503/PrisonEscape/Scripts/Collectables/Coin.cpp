#include "Coin.h"

Coin::Coin(const std::string& name) : GameObject() {
	collected = false;
	GameObject::SetName(name);

}

Coin::~Coin() {
}