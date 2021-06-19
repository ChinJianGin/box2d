#pragma once
#include "cocos2d.h"
#include "Box2D/Box2D.h"

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

#define EDGE 1
#define RECT 2
#define TRIANGLE 3
#define SPHERE 4

class StaticShapeCreator
{
public:
	void init(b2World& world , cocos2d::Size& visible , cocos2d::Scene& theScene , cocos2d::Node& csb);
	
	void createShape(int type, int amount);
private:
	b2World* _World;
	cocos2d::Size* _visibleSize;
	cocos2d::Scene* _Scene;
	cocos2d::Node* _csbRoot;
	cocos2d::Point _fatherPnt;

	b2BodyDef _BallBodyDef;
	b2CircleShape _BallShape;
	b2FixtureDef _BallFixtureDef;
};