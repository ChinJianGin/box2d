#pragma once
#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#include "Common/CLight.h"
#include "startscene.h"
#include "Common/StaticShapeCreator.h"
#include "Common/ContactListener.h"

#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

class StageFour : public cocos2d::Scene
{
private:
	StaticShapeCreator* _shapeCreator;
	Point _spawnPoint[13];
	b2Body* _player;
	b2Body* _rearWheel;
	b2Body* _frontWheel;

	// for MouseJoint
	b2Body* _bottomBody; // 底部的 edgeShape
	b2MouseJoint* _MouseJoint;
	b2Body* _MouseJointBody[3];
	bool _bTouchOn;

	CButton* _resetBtn[2];

	Node* _endNode;

	b2Body* _wall[5];

	b2Body* _etcBody[2];

	float wv;
public:
	~StageFour();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	Node* _csbRoot;

	// for Box2D
	b2World* _b2World;
	cocos2d::Label* _titleLabel;
	cocos2d::Size _visibleSize;

	// For FrictionAndFilter Example
	//CButton* _rectButton;
	//int _iNumofRect;

	//Return to startscene
	CButton* _returnButton[2];
	bool _bToStartScene;

	// For Sensor And Collision Example
	//CLight* _light1;
	//bool _bReleasingBall;
	//CButton* _ballBtn;
	ContactListener _contactListener;
	//cocos2d::Sprite* _collisionSprite;
	//cocos2d::BlendFunc blendFunc;
	//float _tdelayTime; // 用於火花的產生，不要事件進入太多而導致一下產生過多的火花
	//bool  _bSparking;  // true: 可以噴出火花，false: 不行	

	// Box2D Examples
	//void setStaticWalls();
	//void setupDesnity();
	//void setupFrictionAndFilter();
	//void setupSensorAndCollision();
	void createStaticBoundary();


#ifdef BOX2D_DEBUG
	//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void doStep(float dt);
	void createSensor(int type, int amount);
	void createCar();
	void createBridgeAndRope();
	void createElevator();
	void createDynamicWalls();
	void reset();


	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰結束事件 

	// implement the "static create()" method manually
	CREATE_FUNC(StageFour);
};