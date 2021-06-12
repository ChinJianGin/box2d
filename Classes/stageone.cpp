#include "stageone.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocostudio::timeline;

StageOne::~StageOne()
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

Scene* StageOne::createScene()
{
	return StageOne::create();
}

bool StageOne::init()
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

	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//���O��V
	bool AllowSleep = true;			//���\�ε�
	_b2World = new b2World(Gravity);	//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

	// Create Scene with csb file
	_csbRoot = CSLoader::createNode("stageone.csb");
#ifndef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
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
	//�]�wDebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//���ø�s���O
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//ø�s�Ϊ�
	flags += GLESDebugDraw::e_pairBit;
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//�]�wø�s����
	_DebugDraw->SetFlags(flags);
#endif

	//_b2World->SetContactListener(&_contactListener);

	auto listener = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	listener->onTouchBegan = CC_CALLBACK_2(StageOne::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	listener->onTouchMoved = CC_CALLBACK_2(StageOne::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	listener->onTouchEnded = CC_CALLBACK_2(StageOne::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(StageOne::doStep));

	return true;
}

void StageOne::doStep(float dt)
{
	if (_bToStartScene)
	{
		log("return");
		// ���N�o�� SCENE �� update�q schedule update �����X
		this->unschedule(schedule_selector(StageOne::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("gamescene.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		TransitionFade* pageTurn = TransitionFade::create(1.0F, StartScene::createScene());
		Director::getInstance()->replaceScene(pageTurn);
	}
}

bool StageOne::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	log("stage one return btn");
	_returnButton->touchesBegin(touchLoc);
	return true;
}

void StageOne::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	_returnButton->touchesMoved(touchLoc);
}

void StageOne::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	if (_returnButton->touchesEnded(touchLoc))
	{
		_bToStartScene = true;
	}
}