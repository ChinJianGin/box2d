#include "stagethree.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocostudio::timeline;

StageThree::~StageThree()
{
#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
	//  for releasing Plist&Texture
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("scene101bg.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();
}

Scene* StageThree::createScene()
{
	return StageThree::create();
}

bool StageThree::init()
{
	if (!Scene::init())
	{
		return false;
	}
	//  For Loading Plist+Texture
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("scene101bg.plist");

	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

	// Create Scene with csb file
	_csbRoot = CSLoader::createNode("stageone.csb");
#ifndef BOX2D_DEBUG
	// 設定顯示背景圖示
	auto bgSprite = _csbRoot->getChildByName("bg64_1");
	bgSprite->setVisible(true);
#endif
	addChild(_csbRoot, 1);

	auto btnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("returnbtn"));
	_returnButton = CButton::create();
	_returnButton->setButtonInfo("returnbtn.png", "returnbtn.png", btnSprite->getPosition());
	_returnButton->setScale(btnSprite->getScale());
	this->addChild(_returnButton, 3);
	btnSprite->setVisible(false);
	_bToStartScene = false;

#ifdef BOX2D_DEBUG
	//DebugDrawInit
	_DebugDraw = nullptr;
	_DebugDraw = new GLESDebugDraw(PTM_RATIO);
	//設定DebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//選擇繪製型別
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//繪製形狀
	flags += GLESDebugDraw::e_pairBit;
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//設定繪製類型
	_DebugDraw->SetFlags(flags);
#endif

	//_b2World->SetContactListener(&_contactListener);

	auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	listener->onTouchBegan = CC_CALLBACK_2(StageThree::onTouchBegan, this);		//加入觸碰開始事件
	listener->onTouchMoved = CC_CALLBACK_2(StageThree::onTouchMoved, this);		//加入觸碰移動事件
	listener->onTouchEnded = CC_CALLBACK_2(StageThree::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(StageThree::doStep));

	return true;
}

void StageThree::doStep(float dt)
{
	if (_bToStartScene)
	{
		log("return");
		// 先將這個 SCENE 的 update從 schedule update 中移出
		this->unschedule(schedule_selector(StageThree::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("gamescene.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		TransitionFade* pageTurn = TransitionFade::create(1.0F, StartScene::createScene());
		Director::getInstance()->replaceScene(pageTurn);
	}
}

bool StageThree::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	log("stage Three return btn");
	_returnButton->touchesBegin(touchLoc);
	return true;
}

void StageThree::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	_returnButton->touchesMoved(touchLoc);
}

void StageThree::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	if (_returnButton->touchesEnded(touchLoc))
	{
		_bToStartScene = true;
	}
}