#pragma once
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "Box2D/Box2D.h"

USING_NS_CC;

class ContactListener : public b2ContactListener
{
private:
	b2Body* _contactObj;
public:
	bool _bCreateSpark;		//產生火花
	b2Vec2 _createLoc;
	int  _NumOfSparks;
	ContactListener();
	bool _goal;
	bool _portal;
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	bool isGoal();
	bool isPortal();
	void setGoal(bool goal);

	b2Body* getContactObj();
};