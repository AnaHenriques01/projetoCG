#include <iterator>

#include "Group.h"

class Group::GroupBuilder {

private:
	std::vector<Primitive> primitives;
	std::vector<Group> groups;
	std::vector<Trans> trans;

public: 
		GroupBuilder() = default;

		GroupBuilder(std::vector<Primitive> primitives, std::vector<Group> groups, std::vector<Trans> trans) {
			for (size_t i = 0; i < primitives.size(); i++) {
			this->primitives.push_back(primitives.at(i));
			}
			for (size_t i = 0; i < groups.size(); i++) {
			this->groups.push_back(groups.at(1));
			}
			for (size_t i = 0; i < trans.size(); i++) {
				this->trans.push_back(trans.at(i));
			}
		}

		int getNrPrimitives() {
			return primitives.size();
		}

		Trans getTrans(int indice) {
			return trans[indice];
		}

		Primitive getPrimitives(int index) {
			return primitives[index];
		}

		std::vector<Group> getGroups() {
			return groups;
		}

		Group getGroup(int index) {
			return groups[index];
		}

		int getNrGroups() {
			return groups.size();
		}

		int getNrTrans() {
			return trans.size();
		}

		void addTrans(Trans t) {
			trans.push_back(t);
		}

		void addPrimitves(Primitive p) {
			primitives.push_back(p);
		}

		void addGroups(Group g) {
			groups.push_back(g);
		}

		void setPrimitives(std::vector<Primitive> primitives) {
			this->primitives = primitives;
		}

		~GroupBuilder() = default;
};

Group::Group() : groupBuilder{new class GroupBuilder()}{}

Group::Group(std::vector<Primitive> primitives, std::vector<Group> groups, std::vector<Trans> trans) : groupBuilder{ new GroupBuilder(primitives,groups,trans)}{}

int Group::getNrPrimitives() {
	return groupBuilder->getNrPrimitives();
}

int Group::getNrGroups() {
	return groupBuilder->getNrGroups();
}

int Group::getNrTrans() {
	return groupBuilder->getNrTrans();
}

void Group::addTrans(Trans t) {
	groupBuilder->addTrans(t);
}

void Group::addPrimitives(Primitive p) {
	groupBuilder->addPrimitves(p);
}

void Group::addGroups(Group g) {
	groupBuilder->addGroups(g);
}

void Group::setPrimitives(std::vector<Primitive> primitives) {
	groupBuilder->setPrimitives(primitives);
}

Trans Group::getTrans(int index) {
	return groupBuilder->getTrans(index);
}

Primitive Group::getPrimitives(int index) {
	return groupBuilder->getPrimitives(index);
}

std::vector<Group> Group::getGroups() {
	return groupBuilder->getGroups();
}

Group Group::getGroup(int index) {
	return groupBuilder->getGroup(index);
}

Group::~Group() {}