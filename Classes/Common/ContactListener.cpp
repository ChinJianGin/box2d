#include "Common/ContactListener.h"

ContactListener::ContactListener()
{
	_goal = false;
	_portal = false;
	_bCreateSpark = false;
	_NumOfSparks = 10;
}

bool ContactListener::isGoal()
{
	return _goal;
}


bool ContactListener::isPortal()
{
	return _portal;
}

void ContactListener::setGoal(bool goal)
{
	_goal = goal;
}

b2Body* ContactListener::getContactObj()
{
	return _contactObj;
}

void ContactListener::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();
	Sprite* ballDataA = static_cast<Sprite*>(BodyA->GetUserData());
	Sprite* ballDataB = static_cast<Sprite*>(BodyB->GetUserData());
	b2Fixture* fixtureA = BodyA->GetFixtureList();
	b2Fixture* fixtureB = BodyB->GetFixtureList();
	b2Filter filter;
	// check 是否為落下的球經過 sensor1 ，只要經過就立刻讓他彈出去
	if (BodyA->GetFixtureList()->GetDensity() == 10000.0f) { // 代表 sensor1
		log("goal");
		if (BodyB->GetFixtureList()->GetDensity() == 5.0f)
		{
			_goal = true;
		}
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10000.0f) {// 代表 sensor1
		log("goal");
		if (BodyA->GetFixtureList()->GetDensity() == 5.0f)
		{
			_goal = true;
		}
	}
	if (BodyA->GetFixtureList()->GetDensity() == 10001.0f) { // 代表 sensor2
		log("change to red");
		if (BodyB->GetFixtureList()->GetDensity() == 5.0f)
		{
			filter.maskBits = 1 << 2 | 1 << 4 | 1;
			ballDataB->setColor(Color3B(255, 150, 0));
			fixtureB->SetFilterData(filter);
		}		
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10001.0f) {	// 代表 sensor2
		log("change to red");
		log("%f", BodyA->GetFixtureList()->GetDensity());
		if (BodyA->GetFixtureList()->GetDensity() == 5.0f)
		{
			filter.maskBits = 1 << 2 | 1 << 4 | 1;
			ballDataA->setColor(Color3B(255, 150, 0));
			fixtureA->SetFilterData(filter);
		}		
	}
	if (BodyA->GetFixtureList()->GetDensity() == 10002.0f) { // 代表 sensor2
		log("change to green");
		if (BodyB->GetFixtureList()->GetDensity() == 5.0f)
		{
			filter.maskBits = 1 << 1 | 1 << 4 | 1;
			ballDataB->setColor(Color3B(0, 215, 110));
			fixtureB->SetFilterData(filter);
		}
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10002.0f) {	// 代表 sensor2
		log("change to green");
		if (BodyA->GetFixtureList()->GetDensity() == 5.0f)
		{
			filter.maskBits = 1 << 1 | 1 << 4 | 1;
			ballDataA->setColor(Color3B(0, 215, 110));
			fixtureA->SetFilterData(filter);
		}
	}
	if (BodyA->GetFixtureList()->GetDensity() == 10003.0f) { // 代表 sensor2
		log("change to green");
		log("%f", BodyB->GetFixtureList()->GetDensity());
		if (BodyB->GetFixtureList()->GetDensity() == 5.0f)
		{
			filter.maskBits = 1 << 1 | 1 << 2 | 1;
			ballDataB->setColor(Color3B(14, 201, 220));
			fixtureB->SetFilterData(filter);
		}
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10003.0f) {	// 代表 sensor2
		log("change to green");
		log("%f", BodyA->GetFixtureList()->GetDensity());
		if (BodyA->GetFixtureList()->GetDensity() == 5.0f)
		{
			filter.maskBits = 1 << 1 | 1 << 2 | 1;
			ballDataA->setColor(Color3B(14, 201, 220));
			fixtureA->SetFilterData(filter);
		}
	}
	if (BodyA->GetFixtureList()->GetDensity() == 10010.0f)
	{
		log("Cake!");
		if (BodyB->GetFixtureList()->GetDensity() == 5.0f)
		{
			_portal = true;
			_bCreateSpark = true;
			_createLoc = BodyA->GetWorldCenter();
		}
		else if (BodyB->GetFixtureList()->GetDensity() == 18.0f)
		{
			_portal = true;
			_contactObj = BodyB;
		}
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10010.0f)
	{
		log("Cake!");
		if (BodyA->GetFixtureList()->GetDensity() == 5.0f)
		{
			_portal = true;
			_bCreateSpark = true;
			_createLoc = BodyB->GetWorldCenter();
		}
		else if (BodyA->GetFixtureList()->GetDensity() == 18.0f)
		{
			_portal = true;
			_contactObj = BodyA;
		}
	}
}

void ContactListener::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	if (BodyA->GetFixtureList()->GetDensity() == 10000.0f) { // 代表 sensor2
		log("leave goal");
		_goal = false;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10000.0f) {	// 代表 sensor2
		log("leave goal");
		_goal = false;
	}
	if (BodyA->GetFixtureList()->GetDensity() == 10010.0f) { // 代表 sensor2
		_portal = false;
		_contactObj = nullptr;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10010.0f) {	// 代表 sensor2
		log("leave goal");
		_portal = false;
		_contactObj = nullptr;
	}
}