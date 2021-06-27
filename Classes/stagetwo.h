#pragma once
//#define BOX2D_DEBUG 1

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

class StageTwo : public cocos2d::Scene
{
private:
	StaticShapeCreator* _shapeCreator;
	CButton* _gravityBtn[4];
	CButton* _resetBtn;
	b2Body* _player;
	b2Body* _Rock[5];
	Point _spawnPoint[6];
	Color3B filterColor[3] = { Color3B(255,150,0), Color3B(0,215,110), Color3B(14,201,220) };
	Node* _endNode;
	Color3B _deafaultColor;
	b2Filter filter;
public:
	~StageTwo();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	Node* _csbRoot;

	// for Box2D
	b2World* _b2World;
	cocos2d::Label* _titleLabel;
	cocos2d::Size _visibleSize;

	//Return to startscene
	CButton* _returnButton[2];
	bool _bToStartScene;

	ContactListener _contactListener;
	bool  _bSparking;  // true: 可以噴出火花，false: 不行	

	void createStaticBoundary();


#ifdef BOX2D_DEBUG
	//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void doStep(float dt);

	void setGravityButton();
	void createPlayer();
	void createFilter();
	void createRock();
	void createSensor(int type, int amount);
	void createGearJoint();
	void reset();


	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰結束事件 

	// implement the "static create()" method manually
	CREATE_FUNC(StageTwo);
};